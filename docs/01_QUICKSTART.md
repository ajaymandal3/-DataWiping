# 🚀 QUICKSTART Guide for SecureWipe Developers

This guide helps you get started working with SecureWipe code in 5 minutes.

## 📦 What is SecureWipe?

A desktop app that safely wipes storage devices (USB drives, external disks, HDDs, SSDs) by overwriting data with patterns. **Think of it like secure "delete" for physical drives.**

## 🎯 Before You Edit Code

1. Read `02_ARCHITECTURE.md` first (explains project structure)
2. Understand the data flow diagram
3. Identify which file does what

## 👀 Finding Your Way Around

### "I want to add a new UI button"

1. Open `src/renderer/index.html` → Add button HTML
2. Open `src/renderer/scripts/app.js` → Add button click handler
3. In handler, call `api.doSomething()` (IPC call)

### "I want to handle a new button click in the backend"

1. Open `src/electron/main.js`
2. Find section `// ===== IPC HANDLERS =====`
3. Add new handler: `ipcMain.handle("doSomething", async (event, args) => {...})`

### "I want to add a new wipe method"

1. Create new file: `native/methods/method_mynewmethod.c`
2. Write a function that returns pattern byte
3. Add enum to `native/methods/methods.h`
4. Update `native/methods/method_dispatcher.c` to route it
5. Update `binding.gyp` to compile it
6. Run: `npm run build:addon`

### "I need to understand how a wipe actually happens"

1. Start in `src/renderer/scripts/app.js` → find `startWipeOperation()` function
2. It calls `api.startWipe(drive, method, size)`
3. Go to `src/electron/main.js` → find `ipcMain.handle("startWipe"...)`
4. It calls `addon.startWipeOperation()`
5. The actual wipe happens in `native/securewipe_core.c` → `perform_real_wipe_step()`

## 🔍 Code Reading Map

**Beginner Path** (Read in this order):

```
1. 02_ARCHITECTURE.md (this folder)
   ↓
2. src/renderer/index.html (see the UI structure)
   ↓
3. src/renderer/styles/main.css (see the styling)
   ↓
4. src/renderer/scripts/app.js (see the UI logic - START HERE!)
   ↓
5. src/electron/main.js (see the middle layer)
   ↓
6. native/methods/methods.h (see the interface)
   ↓
7. native/securewipe_core.c (see the real work)
```

**Advanced Path** (After basics):

```
1. native/methods/method_dispatcher.c (routing)
   ↓
2. native/methods/method_standard.c (patterns)
   ↓
3. native/addon.cpp (C++ binding)
```

## 🎬 Key Workflows To Understand

### Workflow 1: "Get available drives"

```javascript
// app.js
function refreshDiskList() {
  api.getDisks().then((disks) => {
    // Display drives to user
  });
}

// → calls main.js handler
ipcMain.handle("getDisks", async () => {
  const drives = getDisksSnapshot(); // Get list with eligibility check
  return drives;
});

// → which calls native addon
const addon = require("./build/Release/securewipe_native.node");
addon.getDisks(); // Returns JSON
```

### Workflow 2: "Start a wipe"

```javascript
// app.js
function startWipeOperation() {
  api.startWipe(drive, method, sizeBytes);
  // Then poll for progress every 500ms
}

// → calls main.js handler
ipcMain.handle("startWipe", async (event, drive, method, size) => {
  // Validate drive is safe to wipe
  // Call native addon
  addon.startWipeOperation(drive, method, size);
});

// → calls native C code
// securewipe_core.c → perform_real_wipe_step()
// Loops through wipe patterns
// method_dispatcher routes to specific pattern function
```

## 📁 File-by-File Overview

### `src/renderer/index.html`

**What**: The UI layout
**Look for**: `<div id="..."` to find UI sections
**Example**: `<select id="driveSelect">` is the drive selector dropdown

### `src/renderer/scripts/app.js`

**What**: UI event handlers and state management
**Functions to know**:

- `refreshDiskList()` - Get drive list from backend
- `startWipeOperation()` - Begin wipe process
- `updateProgress()` - Poll progress and update bar
- `buildMethodSuggestionText()` - Pick best method for drive type
  **Look for**: "TODO:" comments to find areas needing work

### `src/electron/main.js`

**What**: The bridge between UI and native code
**Key sections**:

- `// ===== ADDON LOADING =====` - How to load C++ binary
- `// ===== IPC HANDLERS =====` - All the "API" functions
- `// ===== DRIVE LOGIC =====` - Drive eligibility checking
- `// ===== CONSTANTS =====` - Drive types, method specs
  **Important variables**:
- `METHOD_SPECS` - Metadata for each wipe method
- `DRIVE_TYPES` - Windows drive type constants
- `g_op` - Current wipe operation state

### `native/securewipe_core.c`

**What**: The actual disk wiping code
**Key functions**:

- `open_volume_handle()` - Open a disk for writing
- `perform_real_wipe_step()` - Perform one I/O operation
- `get_operation_status()` - Return progress info
  **Variables to know**:
- `resolve_pass_pattern()` - Gets pattern byte (0x00, 0xFF, etc)
- `g_op.currentPass` - Which pass are we on? (1, 2, 3...)
- `g_op.processedBytes` - How many bytes wiped so far?

### `native/methods/`

**What**: Wipe algorithm implementations
**Files**:

- `methods.h` - Defines enums and function signatures
- `method_dispatcher.c` - Routes from method name to implementation
- `method_standard.c` - Standard patterns (0x00, 0xFF, 0xAA)
- `method_crypto.c` - Cryptographic pattern (0xA5)
- `method_storage.c` - Device-specific patterns (SSD, Flash)
- `method_raid.c` - Enterprise patterns
  **Pattern bytes**:
- `0x00` = all zeros (one pass of wipe)
- `0xFF` = all ones (128 bits that are all 1)
- `0xAA` = alternating 1010... pattern
- `0xA5` = crypto-safe alternating pattern

## 🔧 Common Changes & Where to Make Them

| Change              | File                        | Section                                                  |
| ------------------- | --------------------------- | -------------------------------------------------------- |
| Add UI button       | `index.html`                | Find `<div id="controls">`                               |
| Handle button click | `app.js`                    | Find `document.getElementById()` + `.addEventListener()` |
| Add IPC handler     | `main.js`                   | Find `// ===== IPC HANDLERS =====`                       |
| Add validation      | `main.js`                   | Find `normalizeDisks()` function                         |
| Change wipe pattern | `native/methods/method_*.c` | Find `get_*_pattern()` function                          |
| Add new method      | `native/methods/methods.h`  | Add enum, then implement in dispatcher                   |
| Change UI colors    | `main.css`                  | Find `:root { --` CSS variables                          |
| Change theme logic  | `app.js`                    | Find `toggleTheme()` function                            |

## 💻 Useful Commands

```bash
# Install dependencies
npm install

# Start the app (Electron)
npm start

# Rebuild native addon after C code changes
npm run build:addon

# Rebuild everything
npm run build

# Check for errors
npm run lint
```

## 🐛 Debugging Tips

### "The app won't start"

1. Check: `npm install` completed
2. Check: `npm run build:addon` succeeded
3. Check: Console for error messages

### "Wipe button doesn't work"

1. Open DevTools: `Ctrl+Shift+I` in app
2. Check Console tab for errors
3. Check if drive is "eligible" (green checkmark)
4. Check if method is selected

### "Native addon won't compile"

1. Run: `npm run build:addon`
2. Check error message (usually path or syntax issue)
3. Verify `binding.gyp` lists all `.c` files
4. Check that all `.c` files exist

### "I don't understand the callback chain"

1. Add `console.log()` statements at each step
2. Follow the flow: UI → main.js → addon → C code
3. Watch the console to see what gets printed

## 📚 Important Concepts

### IPC (Inter-Process Communication)

- UI runs in one process ("renderer")
- Backend runs in another ("main")
- They talk via IPC calls (like RPC/HTTP)
- All calls in `main.js` with `ipcMain.handle()`

### Native Addon

- C/C++ code compiled to `.node` binary
- Very fast for disk I/O
- Loaded by Node.js at runtime
- All disk operations happen here

### Pattern-Based Wiping

- Instead of securely deleting files, overwrite entire disk
- Each pass writes a pattern byte (0x00, 0xFF, etc)
- Multi-pass = harder to recover
- Different methods = different pattern sequences

### Drive Types

- **Removable** (USB) ✅ Can wipe
- **Fixed** (internal HDD/SSD) ✅ Can wipe (unless system)
- **System** (C: drive with Windows) ❌ Blocked
- **Network** (mapped drive) ❌ Blocked
- **CDROM** ❌ Blocked
- **RAMDISK** (memory) ❌ Blocked

## ✅ Checklist: Before Opening an Issue

- [ ] I read 02_ARCHITECTURE.md
- [ ] I ran `npm install`
- [ ] I ran `npm run build:addon`
- [ ] I ran `npm start` successfully
- [ ] I checked the DevTools console for errors
- [ ] I tried selecting a different drive/method
- [ ] I can reproduce the issue every time

## 🆘 Getting Help

1. **Feature isn't working?** → Check 02_ARCHITECTURE.md data flow
2. **Don't understand code?** → Read the comments, then trace through with `console.log()`
3. **Compilation error?** → Check `binding.gyp` and file paths
4. **Unsure where to edit?** → Use "Finding Your Way Around" section above
5. **Want to add feature?** → Copy similar existing code and modify

---

**Next Step**: Open 02_ARCHITECTURE.md and draw the data flow on paper while reading it! 📝

