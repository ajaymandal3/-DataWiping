# 📖 SecureWipe Code Structure Guide

## For Beginners: How Everything Fits Together

This document explains the structure of every important file in SecureWipe, so you don't get lost.

---

## 🗂️ Complete Project Map

```
e:\DataWiping\
│
├─ 📄 01_QUICKSTART.md ← Start here! (5-minute intro)
│
├─ 📄 02_ARCHITECTURE.md ← Then read this (detailed architecture)
│
├─ 📄 03_CODE_STRUCTURE.md ← You are here (this file)
│
├─ src/                          (Frontend - User sees this)
│  ├─ renderer/
│  │  ├─ index.html             (HTML - UI layout)
│  │  ├─ styles/main.css        (CSS - Colors and animations)
│  │  └─ scripts/app.js         (JavaScript - UI logic & buttons)
│  │
│  └─ electron/
│     ├─ main.js               (Electron - Bridge to native)
│     ├─ preload.js            (Security - Safe IPC bridge)
│     └─ assets/icon.png       (Logo)
│
├─ native/                       (Backend - Fast disk operations)
│  ├─ addon.cpp                (C++ - Glue code for Node.js)
│  ├─ securewipe_core.c        (C - Main wipe engine)
│  │
│  └─ methods/                 (Wipe algorithm files - NEW!)
│     ├─ methods.h             (Header - Defines interface)
│     ├─ method_dispatcher.c   (Router - Picks right algorithm)
│     ├─ method_standard.c     (SinglePass, ThreePass, SevenPass)
│     ├─ method_crypto.c       (CryptoErase, EncryptionAware)
│     ├─ method_storage.c      (AtaSecureErase, FlashOverwrite, SecureFormat)
│     └─ method_raid.c         (RaidServerWipe)
│
├─ build/                        (Compiled output - AUTO-GENERATED)
│  └─ Release/securewipe_native.node
│
├─ binding.gyp                  (Build configuration)
├─ package.json                 (Project metadata)
└─ README.md                    (Overview)
```

---

## 📝 File-by-File Explanation

### 🎨 **Frontend (User Sees This)**

#### `src/renderer/index.html`

- **What**: The raw HTML markup
- **Size**: ~400 lines
- **Purpose**: Defines where all UI elements go
- **Key elements**:
  - `<div id="diskList">` - Where drive list goes
  - `<select id="driveSelect">` - Drive picker dropdown
  - `<select id="methodSelect">` - Wipe method picker
  - `<div id="progressBar">` - Progress bar
  - `<div id="liveLog">` - Live status log

**How to edit**: Add more HTML elements here if you want new UI pieces

---

#### `src/renderer/styles/main.css`

- **What**: All the styling (colors, sizes, animations)
- **Size**: ~600 lines
- **Purpose**: Make the app look nice
- **Key sections**:
  - `:root` - CSS variables (colors, sizes)
  - `.dark`, `[data-theme="dark"]` - Dark theme styles
  - `.light`, `[data-theme="light"]` - Light theme styles
  - `.theme-toggle` - The animated theme switch
  - `.progress-bar` - Progress bar animation

**Working principle**:

```css
/* CSS Variables (defined once, reused everywhere) */
:root {
  --bg: #0b1220;
  --text: #e0e6ff;
  --accent: #4f88ff;
}

/* Use variables */
body {
  background: var(--bg);
  color: var(--text);
}

/* Switch theme by changing variables */
[data-theme="light"] {
  --bg: #ffffff;
  --text: #1a1a1a;
}
```

**How to edit**:

- Change colors if you want different theme
- Adjust spacing, sizes for different look
- Add new animations for transitions

---

#### `src/renderer/scripts/app.js`

- **What**: All the UI logic (button clicks, state management)
- **Size**: ~800 lines
- **Purpose**: Make UI interactive and handle user actions
- **Key variables**:
  - `disksCache` - Cached list of drives
  - `maxUnlockedStep` - Which step can user access (1, 2, 3)
  - `activeOpId` - Current wipe operation ID
  - `pollTimer` - Timer for progress updates

**Key functions**:
| Function | Does |
|----------|------|
| `formatBytes()` | Convert bytes to "1.5 GB" |
| `getDriveInfo()` | Look up drive in cache |
| `buildMethodSuggestionText()` | Pick best wipe method for drive |
| `updateMethodSuggestion()` | Update recommendation display |
| `setTheme()` | Switch dark/light mode |
| `setStep()` | Switch UI to different workflow step |
| `refreshDiskList()` | Call backend to get drives |
| `startWipeOperation()` | Begin wipe process |
| `updateProgress()` | Poll for progress and update bar |

**How to edit**:

- Add new button handlers with `document.getElementById().addEventListener()`
- Add more functions to compute recommendations
- Call `api.XXXXX()` to talk to backend

---

### 🔗 **Electron Bridge (Talks to Native Code)**

#### `src/electron/main.js`

- **What**: The middle layer between UI and native code
- **Size**: ~500 lines
- **Purpose**: Handle IPC requests from UI, validate, call native code
- **Key sections**:

1. **Addon Loading**

   ```javascript
   function tryLoadAddon() {
     // Try to load compiled C++ binary
     // Tries multiple paths
   }
   ```

2. **Constants**

   ```javascript
   const DRIVE_TYPES = {
     REMOVABLE: 2, // USB drives
     FIXED: 3, // Internal hard drives
     REMOTE: 4, // Network drives (blocked)
     CDROM: 5, // CD/DVD (blocked)
   };

   const METHOD_SPECS = {
     ThreePass: { passes: 3, label: "Three Pass DoD" },
     // ... more methods
   };
   ```

3. **Drive Eligibility Logic**

   ```javascript
   function normalizeDisks(disks) {
     // Takes raw drive list from native code
     // Adds eligibility flags (isEligible, blockReason)
     // Blocks system drives, network drives, etc.
   }
   ```

4. **IPC Handlers** - These are what the UI calls

   ```javascript
   ipcMain.handle("getDisks", async () => {
     // Return list of drives
   });

   ipcMain.handle("startWipe", async (event, drive, method, size) => {
     // Validate and start wipe
   });

   ipcMain.handle("checkStatus", async () => {
     // Return progress
   });
   ```

**How to edit**:

- Add new constants for new features
- Modify drive eligibility logic
- Add new IPC handlers for new operations

---

#### `src/electron/preload.js`

- **What**: Secure bridge between UI and main process
- **Size**: ~50 lines
- **Purpose**: Only expose safe functions to UI (security)
- **How it works**:
  ```javascript
  // Only these functions available to UI:
  window.securewipeApi = {
    getDisks: () => ipcRenderer.invoke("getDisks"),
    startWipe: (d, m, s) => ipcRenderer.invoke("startWipe", d, m, s),
    // ... more
  };
  ```

**Key principle**: UI can ONLY call functions defined here. Limits attack surface.

---

### ⚙️ **Native Code (Fast Disk Operations)**

#### `native/addon.cpp`

- **What**: C++ glue code
- **Size**: ~50 lines
- **Purpose**: Connect JavaScript to C functions
- **How it works**:

  ```cpp
  // When JavaScript calls addon.getDisks()
  NODE_SET_METHOD(exports, "getDisks", GetDiskList);

  // That calls C function:
  void GetDiskList(const FunctionCallbackInfo<Value>& args) {
    // Call C function, return JavaScript string
  }
  ```

**Beginner note**: Don't edit this unless you're adding new C functions.

---

#### `native/securewipe_core.c`

- **What**: The actual wipe engine
- **Size**: ~600 lines
- **Purpose**: Open disks and write wipe patterns
- **How it works**:

1. **Data Structures**

   ```c
   typedef struct {
     char opId[64];          // Operation ID
     char drive[8];          // "D:"
     char method[32];        // "ThreePass"
     unsigned long long totalBytes;    // How big
     unsigned long long processedBytes; // How much done
     unsigned int currentPass;         // Which pass (1, 2, 3...)
     unsigned long long currentOffset; // Position in disk
   } WipeOp;
   ```

2. **Key Functions**
   - `open_volume_handle()` - Open disk file using Windows API
   - `perform_real_wipe_step()` - Write one chunk of wipe pattern
   - `resolve_pass_pattern()` - Get pattern byte (0x00, 0xFF, etc)
   - `get_operation_status()` - Return progress JSON
   - `start_operation()` - Initialize new wipe
   - `cancel_operation()` - Stop wipe

3. **How a Wipe Happens**
   ```
   start_operation()
      ↓
   For pass 1 to N:
      ↓
      For offset 0 to totalBytes (in 1MB chunks):
         ↓
         resolve_pass_pattern() → Get pattern (0x00, 0xFF, etc)
         ↓
         Fill 1MB buffer with pattern
         ↓
         WriteFile() → Write to disk
         ↓
         Update progress
      ↓
   Done - close volume
   ```

**Beginner note**: Most of this code can stay as-is. Just understand the flow.

---

#### `native/methods/` (NEW Modular Structure!)

These files handle **how** to wipe - the algorithms.

##### `native/methods/methods.h`

- **What**: The interface/contract file
- **Size**: ~30 lines
- **Purpose**: Define what functions exist
- **Contains**:

  ```c
  // Enum of method types
  typedef enum {
    METHOD_SINGLE_PASS,
    METHOD_THREE_PASS,
    METHOD_CRYPTO_ERASE,
    // ... 9 total
  } WipeMethodType;

  // Function declarations (implemented in method_*.c)
  unsigned char get_wipe_pattern(WipeMethodType method, unsigned int passIndex);
  WipeMethodType method_from_key(const char *key);
  unsigned int get_method_passes(WipeMethodType method);
  ```

**Purpose**: Lets other files know what functions are available.

---

##### `native/methods/method_dispatcher.c`

- **What**: The router
- **Size**: ~100 lines
- **Purpose**: Map method names to pattern functions
- **How it works**:
  ```c
  unsigned char get_wipe_pattern(WipeMethodType method, unsigned int passIndex) {
    switch(method) {
      case METHOD_SINGLE_PASS:
        return get_standard_pattern("SinglePass", passIndex);
      case METHOD_THREE_PASS:
        return get_standard_pattern("ThreePass", passIndex);
      // ... route to appropriate method_*.c file
    }
  }
  ```

**Why it exists**: Avoids having all pattern logic in one huge file.

---

##### `native/methods/method_standard.c`

- **What**: Standard overwrite patterns
- **Size**: ~30 lines
- **Contains**: SinglePass, ThreePass, SevenPass
- **How it works**:
  ```c
  unsigned char get_standard_pattern(const char *method, unsigned int passIndex) {
    if (strcmp(method, "SinglePass") == 0) {
      return 0x00;  // One pass with zeros
    }
    if (strcmp(method, "ThreePass") == 0) {
      // DoD 3-pass pattern
      unsigned char patterns[3] = {0x00, 0xFF, 0xAA};
      return patterns[passIndex % 3];
    }
    if (strcmp(method, "SevenPass") == 0) {
      // Gutmann 7-pass pattern
      unsigned char patterns[7] = {0xF6, 0x00, 0xFF, 0xAA, 0x55, 0x92, 0x49};
      return patterns[passIndex % 7];
    }
    return 0x00;
  }
  ```

**Pattern bytes explained**:

- `0x00` = all zeros (binary 00000000)
- `0xFF` = all ones (binary 11111111)
- `0xAA` = alternating (binary 10101010)
- `0x55` = alternating opposite (binary 01010101)
- `0xA5` = crypto-safe (binary 10100101)

---

##### `native/methods/method_crypto.c`

- **What**: Cryptographic patterns
- **Size**: ~15 lines
- **Contains**: CryptoErase, EncryptionAware
- **How it works**: Always returns `0xA5` (crypto-safe pattern)

---

##### `native/methods/method_storage.c`

- **What**: Device-specific patterns
- **Size**: ~30 lines
- **Contains**: AtaSecureErase (SSDs), FlashOverwrite (USB), SecureFormat
- **How it works**:
  - SSD: `0xA5` (1 pass)
  - USB/Flash: `0x00` then `0xFF` (2 passes)
  - Format: `0x00` (1 pass)

---

##### `native/methods/method_raid.c`

- **What**: Enterprise patterns
- **Size**: ~10 lines
- **Contains**: RaidServerWipe
- **How it works**: 3-pass DoD pattern like ThreePass

---

### ⚙️ **Build Configuration**

#### `binding.gyp`

- **What**: Build instructions for node-gyp
- **Size**: ~15 lines
- **Purpose**: Tell compiler which C files to compile and how
- **Structure**:
  ```json
  {
    "targets": [
      {
        "target_name": "securewipe_native",
        "sources": [
          "native/addon.cpp",
          "native/securewipe_core.c",
          "native/methods/method_*.c" // All method files!
        ]
      }
    ]
  }
  ```

**When to edit**: Add new C files here before `npm run build:addon`

---

## 🔄 Data Flow Recap

### Getting Drive List:

```
app.js: api.getDisks()
  → main.js: ipcMain.handle("getDisks")
    → addon.cpp: GetDiskList()
      → securewipe_core.c: get_disk_list() [C function]
        → Windows API: GetLogicalDrives(), GetDriveType()
      ← Returns JSON
    ← Returns drives with eligibility
  ← UI displays drive list
```

### Starting a Wipe:

```
app.js: api.startWipe(drive, method, size)
  → main.js: ipcMain.handle("startWipe")
    → Validate drive eligibility
    → addon.cpp: StartWipeOp()
      → securewipe_core.c: start_operation()
        → open_volume_handle() [opens disk]
      ← Returns operation ID
    ← Returns to UI
  ← UI starts polling
```

### Wipe Progress (Polling):

```
(Every 500ms)
app.js: api.checkStatus()
  → main.js: ipcMain.handle("checkStatus")
    → addon.cpp: GetOpStatus()
      → securewipe_core.c: get_operation_status()
        → Returns: pass, offset, percentage
      ← Returns JSON
    ← Returns status
  ← UI updates progress bar
```

### Performing Wipe Patterns:

```
addon continuously calls:
securewipe_core.c: perform_real_wipe_step()
  ↓
Get pattern: resolve_pass_pattern(pass, method)
  ↓
method_dispatcher.c: method_from_key(method)
  ↓
Get specific pattern: get_wipe_pattern(methodType, pass)
  ↓
method_standard.c / method_crypto.c / method_storage.c
  ↓
Returns pattern byte (0x00, 0xFF, 0xA5, etc)
  ↓
Fill buffer with pattern
  ↓
WriteFile() to disk
  ↓
Update progress, loop to next chunk
```

---

## 📚 Code Reading Roadmap (Beginner to Advanced)

### **Week 1: Understand the Big Picture**

1. Read `01_QUICKSTART.md` (15 min)
2. Read `02_ARCHITECTURE.md` (30 min)
3. Read this file → `03_CODE_STRUCTURE.md` (30 min)
4. Open `src/renderer/index.html` → Skim structure (~5 min)
5. Open `src/renderer/scripts/app.js` → Find one button handler (~10 min)

### **Week 2: Frontend Code**

1. Read `src/renderer/styles/main.css` → Understand CSS variables (~20 min)
2. Read `src/renderer/scripts/app.js` → Understand all functions (~45 min)
3. Modify one function (e.g., add a new log message) → Build and test (~30 min)

### **Week 3: Electron Bridge**

1. Read `src/electron/preload.js` → Understand security bridge (~10 min)
2. Read `src/electron/main.js` → Understand IPC handlers (~45 min)
3. Add a new IPC handler that echoes back data (~20 min)

### **Week 4: Native Code**

1. Read `native/securewipe_core.c` top section (~30 min)
2. Understand `WipeOp` structure (~10 min)
3. Read one method file (method_standard.c) (~15 min)
4. Understand how `get_wipe_pattern()` works (~15 min)

### **Week 5: Advanced Topics**

1. Understand full wipe loop in `securewipe_core.c` (~45 min)
2. Add new pattern to method_standard.c (~30 min)
3. Rebuild addon with `npm run build:addon` (~15 min)

---

## 🎯 Editing Checklist

### **Adding a New UI Button**

- [ ] Add HTML in `index.html`
- [ ] Add CSS in `main.css`
- [ ] Add event listener in `app.js`
- [ ] Call `api.XXXXX()` if needed
- [ ] Test by running `npm start`

### **Adding New IPC Endpoint**

- [ ] Add handler in `src/electron/main.js`
- [ ] Export function in `src/electron/preload.js`
- [ ] Call from `app.js` via `api.XXXXX()`
- [ ] Test by running `npm start`

### **Adding New Wipe Method**

- [ ] Add enum to `native/methods/methods.h`
- [ ] Implement in appropriate `method_*.c` file
- [ ] Update `method_dispatcher.c` router
- [ ] Add to `METHOD_SPECS` in `src/electron/main.js`
- [ ] Update `binding.gyp` if new file
- [ ] Run: `npm run build:addon`
- [ ] Test by running `npm start`

---

## 🐛 Common Issues & Solutions

| Issue                      | File to Check                  | Solution                               |
| -------------------------- | ------------------------------ | -------------------------------------- |
| UI button doesn't work     | `app.js`                       | Check event listener attached          |
| Button doesn't do anything | `main.js`                      | Check IPC handler exists               |
| Wipe won't start           | `main.js` → `normalizeDisks()` | Check drive eligibility logic          |
| Addon won't compile        | `binding.gyp`                  | Check all .c files listed              |
| DLL load error             | `src/electron/main.js`         | Check addon paths in `addonCandidates` |
| Progress doesn't update    | `app.js` → `updateProgress()`  | Check polling interval                 |
| Wipe pattern wrong         | `native/methods/*`             | Check pattern byte value               |

---

## 📖 File Dependencies

```
index.html
  ↓ loads
app.js
  ↓ calls
preload.js
  ↓ forwards to
main.js
  ↓ calls
addon.cpp
  ↓ calls
securewipe_core.c
  ↓ calls
method_dispatcher.c
  ↓ calls
method_*.c
```

**Rule**: Each level only talks to the level below it.

---

## ✅ You Now Know:

- [ ] Project structure and file organization
- [ ] What each major file does
- [ ] How data flows through the app
- [ ] How to find code for any feature
- [ ] Where to edit for different changes
- [ ] How wipe patterns work
- [ ] How the modular methods are organized

**Next Steps**: Open 01_QUICKSTART.md and follow the beginner path! 🚀

