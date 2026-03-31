/*
  ============================================================================
  SecureWipe Frontend (Renderer Process)
  ============================================================================
  
  This file is the heart of the UI. It handles:
  - All user interactions (button clicks, form inputs)
  - Communication with the backend via IPC (api calls)
  - Progress tracking and updates
  - Theme switching (dark/light mode)
  - Workflow state management (which step user is on)
  
  The app flows through 3 steps:
  1. CONFIGURE - User picks drive and wipe method
  2. EXECUTE - Wipe runs, progress bar updates
  3. REPORTS - Results and certification
  
  ============================================================================
*/

// Backend API - All communication with main process goes through api.XXXXX()
const api = window.securewipeApi;
const EXECUTION_MODE = "real";

// ===== UI ELEMENT REFERENCES =====
// These are cached so we don't have to query DOM repeatedly
const versionEl = document.getElementById("version");
const themeToggleBtn = document.getElementById("themeToggleBtn");
const themeToggleLabelEl = document.getElementById("themeToggleLabel");
const stepBtns = Array.from(document.querySelectorAll(".step-btn"));
const stepPages = Array.from(document.querySelectorAll(".step-page"));

// Disk selection UI
const diskStatusEl = document.getElementById("diskStatus");
const diskListEl = document.getElementById("diskList");
const methodCardsEl = document.getElementById("methodCards");
const refreshDisksBtn = document.getElementById("refreshDisksBtn");
const driveSelectEl = document.getElementById("driveSelect");
const methodSelectEl = document.getElementById("methodSelect");
const methodSuggestionEl = document.getElementById("methodSuggestion");
const ackInputEl = document.getElementById("ackInput");
const termsAgreeCheckEl = document.getElementById("termsAgreeCheck");
const toExecuteBtn = document.getElementById("toExecuteBtn");

// Wipe execution UI
const startWipeBtn = document.getElementById("startWipeBtn");
const cancelWipeBtn = document.getElementById("cancelWipeBtn");
const checkStatusBtn = document.getElementById("checkStatusBtn");
const toReportsBtn = document.getElementById("toReportsBtn");
const wipeStatusEl = document.getElementById("wipeStatus");
const progressBarEl = document.getElementById("progressBar");
const progressMetaEl = document.getElementById("progressMeta");
const liveLogEl = document.getElementById("liveLog");

// Reports UI
const getCertBtn = document.getElementById("getCertBtn");
const certOutputEl = document.getElementById("certOutput");
const auditListEl = document.getElementById("auditList");

// ===== STATE VARIABLES =====
// These track the current app state
let activeOpId = "";              // UUID of current wipe operation
let pollTimer = null;             // Timer for progress polling
let maxUnlockedStep = 1;          // Which step can user access (1, 2, or 3)
let disksCache = [];              // Cached list of drives (to avoid repeated API calls)

/**
 * Format bytes into human-readable size (e.g., "1.5 GB")
 * @param {number} bytes - Number of bytes
 * @returns {string} Formatted size like "1.5 GB"
 */
function formatBytes(bytes) {
  const n = Number(bytes || 0);
  if (!Number.isFinite(n) || n <= 0) {
    return "0 B";
  }
  const units = ["B", "KB", "MB", "GB", "TB", "PB"];
  let value = n;
  let idx = 0;
  while (value >= 1024 && idx < units.length - 1) {
    value /= 1024;
    idx += 1;
  }
  return `${value.toFixed(idx === 0 ? 0 : 2)} ${units[idx]}`;
}

/**
 * Add a log line to the live log (newest on top)
 * @param {string} line - Text to log
 */
function addLog(line) {
  liveLogEl.textContent = `${new Date().toLocaleTimeString()} ${line}\n${liveLogEl.textContent}`.slice(0, 6000);
}

/**
 * Look up drive info in cache by name
 * @param {string} name - Drive letter like "D:" or "E:"
 * @returns {object|null} Drive object with properties: name, type, capacity, isEligible, blockReason
 */
function getDriveInfo(name) {
  const key = String(name || "").toUpperCase();
  return disksCache.find((d) => String(d.name || "").toUpperCase() === key) || null;
}

/**
 * Build a smart recommendation message for a given drive + method
 * Recommends the best method based on storage type (USB vs HDD vs SSD)
 * @param {object} driveInfo - Drive metadata object
 * @param {string} methodKey - Currently selected method name
 * @returns {string} Recommendation text
 */
function buildMethodSuggestionText(driveInfo, methodKey) {
  if (!driveInfo) {
    return "Select a target drive to get method guidance.";
  }

  if (!driveInfo.isEligible) {
    return `Drive ${driveInfo.name} is blocked: ${driveInfo.blockReason || "Not eligible"}.`;
  }

  const selected = String(methodKey || "");
  const removable = Number(driveInfo.type) === 2;  // Type 2 = USB/SD/Removable
  const fixed = Number(driveInfo.type) === 3;     // Type 3 = Internal HDD/SSD

  let deviceClass = "Unknown";
  let primary = "SinglePass";
  let alternative = "CryptoErase";

  if (removable) {
    deviceClass = "USB/SD/Flash";
    primary = "FlashOverwrite";       // Best for flash memory
    alternative = "SinglePass";
  } else if (fixed) {
    deviceClass = "Fixed Disk (HDD/SSD)";
    primary = "AtaSecureErase";       // Best for SSD (native command)
    alternative = "ThreePass";        // Good for HDD
  }

  // Show checkmark if user picked a recommended method
  const selectedHint = selected === primary || selected === alternative
    ? "✓ Policy Aligned"
    : `→ Recommended: ${primary}`;

  return `Storage: ${deviceClass} | Primary: ${primary} | Alt: ${alternative} | Reusable: Yes | ${selectedHint}`;
}

/**
 * Update the method suggestion text whenever user changes drive or method selection
 */
function updateMethodSuggestion() {
  if (!methodSuggestionEl) {
    return;
  }
  const drive = driveSelectEl.value;
  const driveInfo = getDriveInfo(drive);
  methodSuggestionEl.textContent = buildMethodSuggestionText(driveInfo, methodSelectEl.value);
}

/**
 * Set the app theme (dark or light)
 * @param {string} theme - "dark" or "light"
 */
function setTheme(theme) {
  document.body.setAttribute("data-theme", theme);
  localStorage.setItem("securewipe-theme", theme);
  if (themeToggleBtn) {
    const isDark = theme === "dark";
    themeToggleBtn.setAttribute("aria-pressed", isDark ? "true" : "false");
    if (themeToggleLabelEl) {
      themeToggleLabelEl.textContent = isDark ? "Dark Mode" : "Light Mode";
    }
  }
}

/**
 * Load theme from localStorage, default to dark
 */
function initTheme() {
  const saved = localStorage.getItem("securewipe-theme");
  setTheme(saved === "light" ? "light" : "dark");
}

/**
 * Enable/disable UI controls based on wipe status
 * @param {boolean} isRunning - True if wipe is in progress
 */
function setRunningUi(isRunning) {
  startWipeBtn.disabled = isRunning;
  cancelWipeBtn.disabled = !isRunning;
  driveSelectEl.disabled = isRunning;
  methodSelectEl.disabled = isRunning;
}

/**
 * Stop polling for progress updates
 */
function stopPolling() {
  if (pollTimer) {
    clearInterval(pollTimer);
    pollTimer = null;
  }
}

/**
 * Switch UI to show a specific step (Configure, Execute, or Reports)
 * @param {number} step - Step number (1, 2, or 3)
 */
function setStep(step) {
  const target = Number(step);
  for (const page of stepPages) {
    page.classList.toggle("active", page.id === `step-${target}`);
  }
  for (const btn of stepBtns) {
    const s = Number(btn.dataset.step);
    const locked = s > maxUnlockedStep;
    btn.classList.toggle("active", s === target);
    btn.classList.toggle("locked", locked);
    btn.classList.toggle("unlocked", !locked);
    btn.disabled = locked;
  }
}

function unlockUntil(step) {
  maxUnlockedStep = Math.max(maxUnlockedStep, Number(step));
}

async function loadVersion() {
  if (!versionEl) {
    return;
  }
  const result = await api.getVersion();
  if (result && result.ok) {
    versionEl.textContent = result.version;
    return;
  }
  versionEl.textContent = "Unavailable";
}

async function loadDisks() {
  diskStatusEl.textContent = "Loading disks...";
  diskListEl.innerHTML = "";
  driveSelectEl.innerHTML = "";
  const result = await api.getDisks();
  if (!result || !result.ok) {
    diskStatusEl.textContent = `Failed: ${(result && result.message) || "Unknown error"}`;
    return;
  }
  const disks = Array.isArray(result.disks) ? result.disks : [];
  disksCache = disks;
  const blocked = disks.filter((d) => !d.isEligible).length;
  const allowed = Math.max(0, disks.length - blocked);
  diskStatusEl.textContent = `${disks.length} drives discovered | ${blocked} blocked | ${allowed} eligible`;
  for (const d of disks) {
    const isBlocked = !d.isEligible;
    const option = document.createElement("option");
    option.value = d.name;
    option.textContent = `${d.name}${isBlocked ? ` (${d.blockReason || "blocked"})` : ""}`;
    option.disabled = isBlocked;
    driveSelectEl.appendChild(option);

    const row = document.createElement("div");
    row.className = "list-item";
    row.textContent = `${d.name} | ${d.driveTypeName || "Unknown"} | total ${formatBytes(d.totalBytes)} | free ${formatBytes(d.freeBytes)}${isBlocked ? ` | BLOCKED: ${d.blockReason || "Policy"}` : " | ELIGIBLE"}`;
    diskListEl.appendChild(row);
  }

  if (!Array.from(driveSelectEl.options).some((opt) => !opt.disabled)) {
    const fallback = document.createElement("option");
    fallback.value = "";
    fallback.textContent = "No eligible drives";
    fallback.disabled = true;
    fallback.selected = true;
    driveSelectEl.appendChild(fallback);
  }

  ackInputEl.value = `WIPE ${driveSelectEl.value || ""}`.trim();
  updateMethodSuggestion();
}

async function loadMethods() {
  methodCardsEl.innerHTML = "";
  methodSelectEl.innerHTML = "";
  const result = await api.getMethods();
  if (!result || !result.ok) {
    methodCardsEl.innerHTML = '<div class="list-item">Methods unavailable.</div>';
    return;
  }
  const methods = Array.isArray(result.methods) ? result.methods : [];
  for (const m of methods) {
    const option = document.createElement("option");
    option.value = m.key;
    option.textContent = m.label;
    methodSelectEl.appendChild(option);

    const card = document.createElement("div");
    card.className = "method-card";
    card.innerHTML = `<strong>${m.label}</strong>Key: ${m.key}<br/>Passes: ${m.passes}`;
    methodCardsEl.appendChild(card);
  }
  updateMethodSuggestion();
}

async function loadAuditHistory() {
  const result = await api.getAuditHistory();
  auditListEl.innerHTML = "";
  if (!result || !result.ok) {
    auditListEl.innerHTML = '<div class="list-item">Audit unavailable.</div>';
    return;
  }
  const items = (result.entries || []).slice().reverse().slice(0, 40);
  if (!items.length) {
    auditListEl.innerHTML = '<div class="list-item">No audit records yet.</div>';
    return;
  }
  for (const rec of items) {
    const row = document.createElement("div");
    row.className = "list-item";
    row.textContent = `${new Date(rec.ts).toLocaleString()} | ${rec.eventType} | actor: ${rec.actor || "unknown"}`;
    auditListEl.appendChild(row);
  }
}

async function beginExecuteStep() {
  const drive = driveSelectEl.value;
  const expected = `WIPE ${drive}`;
  if (!drive) {
    diskStatusEl.textContent = "Select a target drive first.";
    return;
  }
  if (ackInputEl.value.trim() !== expected) {
    diskStatusEl.textContent = `Type exact safety phrase: ${expected}`;
    return;
  }
  if (!termsAgreeCheckEl.checked) {
    diskStatusEl.textContent = "You must confirm authorization checkbox.";
    return;
  }
  unlockUntil(2);
  setStep(2);
}

async function startWipe() {
  const drive = driveSelectEl.value;
  const method = methodSelectEl.value;
  const ackPhrase = ackInputEl.value.trim();
  const result = await api.startWipe({
    drive,
    method,
    mode: EXECUTION_MODE,
    ackPhrase,
    termsAccepted: true,
    termsVersion: "SecureWipe-Terms-v1"
  });
  if (!result || !result.ok) {
    wipeStatusEl.textContent = `Start failed: ${(result && result.message) || "Unknown error"}`;
    return;
  }
  activeOpId = result.opId;
  progressBarEl.style.width = "0%";
  progressMetaEl.textContent = "0 B / 0 B";
  setRunningUi(true);
  addLog(`Started ${result.method} on ${result.drive}`);
  wipeStatusEl.textContent = `Operation ${result.opId} started.`;
  stopPolling();
  pollTimer = setInterval(checkStatus, 600);
}

async function checkStatus() {
  if (!activeOpId) {
    return;
  }
  const result = await api.getWipeStatus(activeOpId);
  if (!result || !result.ok) {
    wipeStatusEl.textContent = `Status failed: ${(result && result.message) || "Unknown error"}`;
    return;
  }
  progressBarEl.style.width = `${result.progress || 0}%`;
  progressMetaEl.textContent = `${formatBytes(result.processedBytes)} / ${formatBytes(result.totalBytes)}`;
  wipeStatusEl.textContent = `${result.stage || "running"} | ${result.progress || 0}%`;
  addLog(`${result.stage || "running"}: ${result.progress || 0}%`);

  if (result.done) {
    stopPolling();
    setRunningUi(false);
    unlockUntil(3);
    await loadAuditHistory();
    setStep(3);
  }
}

async function cancelWipe() {
  if (!activeOpId) {
    return;
  }
  const result = await api.cancelWipe(activeOpId);
  if (result && result.ok) {
    addLog(`Cancellation requested for ${activeOpId}`);
  }
}

async function getCertificate() {
  if (!activeOpId) {
    certOutputEl.textContent = "No operation selected.";
    return;
  }
  const result = await api.getCertificate(activeOpId);
  certOutputEl.textContent = JSON.stringify(result, null, 2);
}

if (!api) {
  throw new Error("securewipeApi bridge is undefined");
}

for (const btn of stepBtns) {
  btn.addEventListener("click", () => {
    const next = Number(btn.dataset.step);
    if (next <= maxUnlockedStep) {
      setStep(next);
    }
  });
}

themeToggleBtn.addEventListener("click", () => {
  const current = document.body.getAttribute("data-theme") === "light" ? "light" : "dark";
  setTheme(current === "light" ? "dark" : "light");
});

refreshDisksBtn.addEventListener("click", loadDisks);
driveSelectEl.addEventListener("change", () => {
  ackInputEl.value = `WIPE ${driveSelectEl.value || ""}`.trim();
  updateMethodSuggestion();
});
methodSelectEl.addEventListener("change", updateMethodSuggestion);
toExecuteBtn.addEventListener("click", beginExecuteStep);
startWipeBtn.addEventListener("click", startWipe);
cancelWipeBtn.addEventListener("click", cancelWipe);
checkStatusBtn.addEventListener("click", checkStatus);
toReportsBtn.addEventListener("click", async () => {
  unlockUntil(3);
  await loadAuditHistory();
  setStep(3);
});
getCertBtn.addEventListener("click", getCertificate);

setRunningUi(false);
initTheme();
setStep(1);

Promise.all([loadVersion(), loadDisks(), loadMethods(), loadAuditHistory()]).catch((err) => {
  wipeStatusEl.textContent = `Startup error: ${err.message}`;
});
