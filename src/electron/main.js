/*
  ============================================================================
  SecureWipe Main Process (Electron Backend)
  ============================================================================
  
  This file is the bridge between UI and native code. It handles:
  - Loading the native addon (C++ binary compiled from C code)
  - Receiving IPC requests from the UI renderer process
  - Validating user input (drive eligibility, safety checks)
  - Calling native functions for low-level disk operations
  - Formatting responses back to the UI
  
  The main process runs in a separate process from the UI, providing
  security through process isolation and context separation.
  
  ============================================================================
*/

const { app, BrowserWindow, ipcMain } = require("electron");
const path = require("path");
const fs = require("fs");
const os = require("os");

// ===== NATIVE ADDON LOADING =====
// The native addon is a compiled C++ binary that does low-level disk I/O
let addon = null;
let addonLoadError = "";

// Try these paths in order to find the compiled addon
const addonCandidates = [
  path.join(__dirname, "..", "..", "build", "Release", "securewipe_native.node"),
  path.join(__dirname, "..", "..", "build", "Release", "securewipe_addon.node"),
  path.join(process.cwd(), "build", "Release", "securewipe_native.node"),
  path.join(process.cwd(), "build", "Release", "securewipe_addon.node")
];

/**
 * Try to load the native addon binary
 * The addon contains low-level C code for disk operations
 * @returns {boolean} True if addon loaded successfully
 */
function tryLoadAddon() {
  if (addon && typeof addon.getVersion === "function") {
    return true;  // Already loaded
  }

  addon = null;
  addonLoadError = "";

  for (const addonPath of addonCandidates) {
    try {
      if (!fs.existsSync(addonPath)) {
        continue;  // File doesn't exist, try next path
      }

      // Clear require cache to force reload
      delete require.cache[require.resolve(addonPath)];
      addon = require(addonPath);
      
      // Verify addon loaded correctly
      if (addon && typeof addon.getVersion === "function") {
        return true;
      }
    } catch (err) {
      addon = null;
      addonLoadError = err && err.message ? err.message : String(err);
    }
  }

  if (!addonLoadError) {
    addonLoadError = `Addon binary not found. Tried: ${addonCandidates.join(", ")}`;
  }

  return false;
}

/**
 * Create the main Electron window
 */
function createWindow() {
  const win = new BrowserWindow({
    width: 1320,
    height: 860,
    minWidth: 1180,
    minHeight: 740,
    title: "SecureWipe",
    backgroundColor: "#0b1220",
    icon: path.join(__dirname, "assets", "icon.png"),
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),  // Secure bridge to renderer
      contextIsolation: true,                        // Safety: isolate contexts
      nodeIntegration: false                         // Safety: no direct Node access
    }
  });

  win.loadFile(path.join(__dirname, "..", "renderer", "index.html"));
}

/**
 * Safely parse JSON from native addon
 * The addon returns JSON strings that we need to parse
 * @param {string} raw - JSON string from C code
 * @param {string} fallbackMessage - Message if parsing fails
 * @returns {object} Parsed object or error wrapper
 */
function parseJsonFromAddon(raw, fallbackMessage) {
  try {
    return JSON.parse(raw);
  } catch (err) {
    return {
      ok: false,
      message: fallbackMessage,
      details: err && err.message ? err.message : String(err)
    };
  }
}

/**
 * Validate drive name format (e.g., "C:" or "D:")
 * @param {string} drive - Drive letter to validate
 * @returns {boolean} True if valid Windows drive letter
 */
function validateDriveName(drive) {
  return /^[A-Z]:$/i.test(String(drive || ""));
}

// ===== CONSTANTS =====

/**
 * Windows drive type constants
 * These come from Windows API GetDriveType()
 */
const DRIVE_TYPES = {
  NO_ROOT: 1,
  REMOVABLE: 2,      // USB drives, external disks
  FIXED: 3,          // Internal hard drives
  REMOTE: 4,         // Network drives (blocked)
  CDROM: 5,          // CD/DVD drives (blocked)
  RAMDISK: 6         // RAM disks (blocked)
};

/**
 * Wipe method specifications
 * Each method has a pass count and recommended storage types
 */
const METHOD_SPECS = {
  CryptoErase: { passes: 1, forTypes: ["SSD", "HDD"], label: "Crypto Erase" },
  SinglePass: { passes: 1, forTypes: ["HDD", "SSD"], label: "Single Pass" },
  ThreePass: { passes: 3, forTypes: ["HDD"], label: "Three Pass DoD" },
  SevenPass: { passes: 7, forTypes: ["HDD"], label: "Seven Pass Gutmann" },
  SecureFormat: { passes: 1, forTypes: ["HDD", "SSD"], label: "Secure Format" },
  AtaSecureErase: { passes: 1, forTypes: ["SSD"], label: "ATA Secure Erase" },
  FlashOverwrite: { passes: 2, forTypes: ["Flash"], label: "Flash Overwrite" },
  EncryptionAware: { passes: 1, forTypes: ["SSD", "Mobile"], label: "Encryption Aware" },
  RaidServerWipe: { passes: 3, forTypes: ["RAID", "Server"], label: "RAID/Server Wipe" }
};

// ===== DRIVE ELIGIBILITY LOGIC =====

/**
 * Convert Windows drive type number to human-readable name
 * @param {number} type - Windows drive type constant
 * @returns {string} Name like "Fixed", "Removable", etc.
 */
function getDriveTypeName(type) {
  if (type === DRIVE_TYPES.REMOVABLE) {
    return "Removable";
  }
  if (type === DRIVE_TYPES.FIXED) {
    return "Fixed";
  }
  if (type === DRIVE_TYPES.REMOTE) {
    return "Network";
  }
  if (type === DRIVE_TYPES.CDROM) {
    return "CD/DVD";
  }
  if (type === DRIVE_TYPES.RAMDISK) {
    return "RAM Disk";
  }
  if (type === DRIVE_TYPES.NO_ROOT) {
    return "No Root";
  }
  return "Unknown";
}

/**
 * Determine if a drive is eligible for wiping
 * Blocks system drives, network drives, and CD/DVD drives
 * Allows removable (USB) and fixed (HDD/SSD) drives
 * @param {array} disks - List of drives from native addon
 * @returns {array} Same disks with eligibility flags added
 */
function normalizeDisks(disks) {
  const list = Array.isArray(disks) ? disks : [];
  return list.map((d) => {
    const type = Number(d.type || 0);
    const isSystem = d.isSystem === true;
    
    // Block these drive types: unknown, no root, network, CD/DVD, RAM disk
    const unsupportedType = type === 0 || 
                           type === DRIVE_TYPES.NO_ROOT || 
                           type === DRIVE_TYPES.REMOTE || 
                           type === DRIVE_TYPES.CDROM || 
                           type === DRIVE_TYPES.RAMDISK;
    
    const isEligible = !isSystem && !unsupportedType;
    let blockReason = "";

    if (isSystem) {
      blockReason = "System drive is blocked";
    } else if (unsupportedType) {
      blockReason = `${getDriveTypeName(type)} drive type is not supported`;
    }

    return {
      ...d,
      type,
      driveTypeName: getDriveTypeName(type),
      isEligible,
      blockReason
    };
  });
}

/**
 * Get list of available drives from native addon
 * Applies eligibility filtering
 * @returns {object} {ok: bool, disks: array[], message: string, details: string}
 */
function getDisksSnapshot() {
  if (!tryLoadAddon() || typeof addon.getDisks !== "function") {
    return {
      ok: false,
      message: "Native addon not loaded",
      details: addonLoadError || "Unknown load error",
      disks: []
    };
  }

  try {
    const raw = addon.getDisks();
    const parsed = parseJsonFromAddon(raw, "Failed to parse disk list from C core");
    if (!parsed.ok && parsed.disks === undefined && !Array.isArray(parsed)) {
      return {
        ok: false,
        message: parsed.message,
        details: parsed.details,
        disks: []
      };
    }

    return {
      ok: true,
      disks: normalizeDisks(Array.isArray(parsed) ? parsed : (parsed.disks || []))
    };
  } catch (err) {
    return {
      ok: false,
      message: "Failed to parse disk list from C core",
      details: err && err.message ? err.message : String(err),
      disks: []
    };
  }
}

// ===== AUDIT LOGGING =====
// Log all wipe operations for compliance

/**
 * Ensure audit log directory exists
 * @returns {string} Path to audit directory
 */
function ensureAuditDir() {
  const dir = path.join(app.getPath("userData"), "audit");
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
  }
  return dir;
}

/**
 * Append audit event to log file (newline-delimited JSON)
 * @param {string} eventType - Type of event (e.g., "WIPE_START", "WIPE_COMPLETE")
 * @param {object} payload - Event details
 */
function appendAudit(eventType, payload) {
  try {
    const dir = ensureAuditDir();
    const file = path.join(dir, "audit-log.jsonl");
    const rec = {
      ts: new Date().toISOString(),
      eventType,
      actor: "local-operator",
      payload
    };
    fs.appendFileSync(file, JSON.stringify(rec) + os.EOL, "utf8");
  } catch (_err) {
    // Keep app operational even if logging fails.
  }
}

function readRecentAudit(limit = 25) {
  try {
    const file = path.join(ensureAuditDir(), "audit-log.jsonl");
    if (!fs.existsSync(file)) {
      return [];
    }
    const lines = fs.readFileSync(file, "utf8").split(/\r?\n/).filter(Boolean);
    return lines
      .slice(Math.max(0, lines.length - limit))
      .map((line) => parseJsonFromAddon(line, "Failed to parse audit entry"))
      .filter((entry) => entry && entry.ts);
  } catch (_err) {
    return [];
  }
}

ipcMain.handle("securewipe:get-version", async () => {
  if (!tryLoadAddon()) {
    return {
      ok: false,
      message: "Native addon not loaded",
      details: addonLoadError || "Unknown load error"
    };
  }
  return {
    ok: true,
    version: addon.getVersion()
  };
});

ipcMain.handle("securewipe:get-disks", async () => {
  return getDisksSnapshot();
});

ipcMain.handle("securewipe:get-methods", async () => {
  if (!tryLoadAddon() || typeof addon.getMethods !== "function") {
    return {
      ok: false,
      message: "Native addon not loaded",
      details: addonLoadError || "Unknown load error",
      methods: []
    };
  }
  const raw = addon.getMethods();
  const parsed = parseJsonFromAddon(raw, "Failed to parse methods from C core");
  const methods = Array.isArray(parsed) ? parsed : [];
  return {
    ok: true,
    methods
  };
});

ipcMain.handle("securewipe:start-wipe", async (_event, drive) => {
  if (!tryLoadAddon() || typeof addon.startWipe !== "function") {
    return {
      ok: false,
      message: "Native addon not loaded",
      details: addonLoadError || "Unknown load error"
    };
  }

  const payload = drive && typeof drive === "object" ? drive : { drive };
  const driveName = String(payload.drive || "").toUpperCase();
  const method = String(payload.method || "SinglePass");
  const mode = String(payload.mode || "real");
  const ackPhrase = String(payload.ackPhrase || "");
  const termsAccepted = payload.termsAccepted === true;
  const termsVersion = String(payload.termsVersion || "");
  const expected = `WIPE ${driveName}`;

  if (!validateDriveName(driveName)) {
    return { ok: false, code: "INVALID_DRIVE", message: "Invalid drive format. Expected like D:" };
  }

  const disksResult = getDisksSnapshot();
  const listedDisks = disksResult && disksResult.ok ? disksResult.disks : [];
  const selected = listedDisks.find((d) => String(d.name || "").toUpperCase() === driveName);
  if (!selected) {
    return { ok: false, code: "DRIVE_NOT_FOUND", message: "Selected drive was not found." };
  }
  if (!selected.isEligible) {
    return { ok: false, code: "DRIVE_BLOCKED", message: selected.blockReason || "Selected drive is blocked by policy." };
  }

  const validMethods = [
    "CryptoErase",
    "SinglePass",
    "ThreePass",
    "SevenPass",
    "SecureFormat",
    "AtaSecureErase",
    "FlashOverwrite",
    "EncryptionAware",
    "RaidServerWipe"
  ];
  if (!validMethods.includes(method)) {
    return { ok: false, code: "INVALID_METHOD", message: "Invalid wipe method." };
  }
  if (ackPhrase !== expected) {
    return { ok: false, code: "INVALID_ACK", message: `Type exactly '${expected}' to continue.` };
  }
  if (!termsAccepted || !termsVersion) {
    return { ok: false, code: "TERMS_NOT_ACCEPTED", message: "You must accept Terms and Conditions before wiping." };
  }

  const raw = addon.startWipe(driveName, method, mode);
  const parsed = parseJsonFromAddon(raw, "Failed to start wipe");
  appendAudit("start-wipe", { requestedDrive: driveName, method, mode, termsVersion, result: parsed });
  return parsed;
});

ipcMain.handle("securewipe:get-wipe-status", async (_event, opId) => {
  if (!tryLoadAddon() || typeof addon.getWipeStatus !== "function") {
    return {
      ok: false,
      message: "Native addon not loaded",
      details: addonLoadError || "Unknown load error"
    };
  }

  try {
    const raw = addon.getWipeStatus(String(opId || ""));
    const parsed = parseJsonFromAddon(raw, "Failed to get wipe status");
    if (parsed.ok && parsed.done) {
      appendAudit("wipe-finished", parsed);
    }
    return parsed;
  } catch (err) {
    return {
      ok: false,
      message: "Failed to get wipe status",
      details: err && err.message ? err.message : String(err)
    };
  }
});

ipcMain.handle("securewipe:cancel-wipe", async (_event, opId) => {
  if (!tryLoadAddon() || typeof addon.cancelWipe !== "function") {
    return {
      ok: false,
      message: "Native addon not loaded",
      details: addonLoadError || "Unknown load error"
    };
  }
  const raw = addon.cancelWipe(String(opId || ""));
  const parsed = parseJsonFromAddon(raw, "Failed to cancel wipe");
  appendAudit("cancel-wipe", parsed);
  return parsed;
});

ipcMain.handle("securewipe:get-certificate", async (_event, opId) => {
  if (!tryLoadAddon() || typeof addon.getCertificate !== "function") {
    return {
      ok: false,
      message: "Native addon not loaded",
      details: addonLoadError || "Unknown load error"
    };
  }
  const raw = addon.getCertificate(String(opId || ""));
  const parsed = parseJsonFromAddon(raw, "Failed to get certificate");
  if (parsed.ok && parsed.certificate) {
    const dir = ensureAuditDir();
    const certFile = path.join(dir, `certificate-${parsed.certificate.opId}.json`);
    fs.writeFileSync(certFile, JSON.stringify(parsed, null, 2), "utf8");
    parsed.file = certFile;
    appendAudit("certificate-generated", { opId: parsed.certificate.opId, file: certFile });
  }
  return parsed;
});

ipcMain.handle("securewipe:get-audit-history", async () => {
  return {
    ok: true,
    entries: readRecentAudit(40)
  };
});

app.whenReady().then(() => {
  createWindow();
  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});
