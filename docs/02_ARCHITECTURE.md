# SecureWipe Architecture & Codebase Guide

This document explains how SecureWipe is organized for beginners.

## 📁 Project Structure

```
SecureWipe/
├── src/                          # Frontend (Electron renderer + main process)
│   ├── electron/
│   │   ├── main.js              # Main Electron process (IPC handlers, drive logic)
│   │   ├── preload.js           # Secure bridge between renderer & main
│   │   └── assets/              # App icons and static images
│   │
│   └── renderer/
│       ├── index.html           # UI layout (HTML)
│       ├── styles/main.css      # Styling (dark/light theme)
│       └── scripts/app.js       # UI logic (buttons, forms, state management)
│
├── native/                       # Backend (C/C++ low-level disk operations)
│   ├── addon.cpp                # Node.js native binding entry point
│   ├── securewipe_core.c        # Main wipe engine (disk I/O, progress tracking)
│   └── methods/                 # Modular wipe algorithm implementations
│       ├── methods.h            # Shared interface (enums, function declarations)
│       ├── method_dispatcher.c  # Router (maps method names to implementations)
│       ├── method_standard.c    # Standard patterns (SinglePass, ThreePass, SevenPass)
│       ├── method_crypto.c      # Crypto patterns (CryptoErase, EncryptionAware)
│       ├── method_storage.c     # Storage patterns (SSD, Flash, Format)
│       └── method_raid.c        # Enterprise patterns (RAID server wipe)
│
├── build/                        # Compiled output (auto-generated)
│   └── Release/securewipe_native.node
│
├── binding.gyp                   # Build configuration (node-gyp)
├── package.json                  # Project metadata & dependencies
└── README.md                      # Project overview
```

## 🔄 Data Flow (How It Works)

### 1️⃣ **User clicks "Get Disks" button**

```
UI (app.js)
  └─> IPC call api.getDisks()
      └─> electron/main.js (getDrives handler)
          └─> Native addon (getDisks function)
              ├─> Query Windows for disk list
              ├─> Classify by type (HDD/SSD/USB/etc)
              ├─> Check eligibility (block System drives)
              └─> Return JSON with drive metadata
          └─> main.js filters/formats response
              └─> Return to renderer
  └─> UI displays drive list with eligibility status
```

### 2️⃣ **User selects drive & method**

```
UI (app.js)
  └─> User picks drive from dropdown
      └─> app.js calls buildMethodSuggestionText()
          ├─> Reads drive type (HDD/SSD/USB)
          ├─> Recommends best method for that type
          └─> Displays recommendation with policy info
  └─> User picks wipe method
      └─> UI updates recommendation again
```

### 3️⃣ **User clicks "Start Wipe"**

```
UI (app.js)
  └─> Validate: drive eligible? method selected? confirmation checked?
      └─> IPC call api.startWipe(driveName, methodName, capacity)
          └─> electron/main.js (validateAndStartWipe handler)
              ├─> Verify drive is still safe to wipe
              ├─> Load native addon
              └─> Call addon.startWipeOperation(drive, method, size)
                  └─> native/securewipe_core.c (perform_real_wipe_step)
                      ├─> Open volume handle to disk
                      ├─> Calculate pattern for current pass
                      │   └─> method_dispatcher.c (get_wipe_pattern)
                      │       └─> Routes to appropriate method_*.c
                      ├─> Write pattern to disk in chunks
                      ├─> Track progress
                      └─> Repeat for all passes
          └─> Return operation ID
      └─> UI starts polling for progress
          └─> Progress bar animates
```

## 🧩 Key Components Explained

### **Frontend (React-like approach)**

**app.js** - Main UI coordinator:

- Manages UI state (which step user is on, selected drive, etc)
- Listens to button clicks and form inputs
- Makes IPC calls to electron/main.js
- Polls for wipe progress
- Updates progress bar and status

**index.html** - UI Structure:

- 3 steps: Configure → Execute → Reports
- Drive selector dropdown
- Method selector dropdown
- Progress bar
- Live log display

**main.css** - Styling:

- CSS variables for dark/light theme
- Smooth theme toggle switch animation
- Responsive layout

### **Electron Bridge (main.js)**

**Responsibilities**:

1. Load native addon (C++ binary)
2. Handle all IPC requests from renderer
3. Apply business logic (drive eligibility checking)
4. Pass safe requests to native addon
5. Format responses for frontend

**Key Functions**:

- `getDisks()` - Get list of available drives
- `getMethods()` - Get list of wipe methods
- `startWipe()` - Start a wipe operation
- `checkStatus()` - Get progress of current wipe
- `getDriveInfo()` - Get details about a specific drive

### **Native Backend (securewipe_core.c)**

**Responsibilities**:

- Perform actual disk I/O operations
- Handle Windows API calls (CreateFileA, WriteFile, etc)
- Calculate wipe patterns for each pass
- Track progress
- Report completion/errors

**Key Variables**:

- `g_op.volumeHandle` - Open disk handle
- `g_op.currentPass` - Current pass number (1, 2, 3...)
- `g_op.currentOffset` - Current position on disk
- `g_op.totalBytes` - Total bytes to wipe

**How a Wipe Works**:

1. Open disk volume using Windows API
2. For each pass (1-7 depending on method):
   - For each chunk (1MB at a time):
     - Get pattern byte (0x00, 0xFF, 0xA5, etc)
     - Fill buffer with that pattern
     - Write to disk
     - Track progress
3. Close volume handle

### **Modular Wipe Methods (native/methods/)**

**Why Modular?**

- Each method is in its own file
- Easy to add new methods without touching core code
- Different storage types need different patterns

**Method Categories**:

| File                    | Purpose                     | Methods                                      |
| ----------------------- | --------------------------- | -------------------------------------------- |
| **method_standard.c**   | Standard overwrite patterns | SinglePass, ThreePass, SevenPass             |
| **method_crypto.c**     | Cryptographic patterns      | CryptoErase, EncryptionAware                 |
| **method_storage.c**    | Storage-specific patterns   | AtaSecureErase, FlashOverwrite, SecureFormat |
| **method_raid.c**       | Enterprise patterns         | RaidServerWipe                               |
| **method_dispatcher.c** | Router                      | Maps method names to implementations         |

**Pattern Bytes**:

- `0x00` - All zeros (one pass)
- `0xFF` - All ones (one pass)
- `0xAA` - Alternating pattern (one pass)
- `0xA5` - Crypto-safe pattern (one pass)
- Combinations repeated for multi-pass

## 🔐 Security Features

1. **Drive Eligibility Check**
   - ✅ Removable drives (USB, external)
   - ✅ Fixed drives (internal non-system)
   - ❌ System drive (Windows OS)
   - ❌ Network drives
   - ❌ CD/DVD drives

2. **Multi-Pass Overwriting**
   - Each pass writes a different pattern
   - Makes data recovery harder
   - Industry-standard patterns (DoD, Gutmann, etc)

3. **Progress Tracking**
   - Real-time progress bar
   - ETA calculation
   - Pause capability

## 📊 Wipe Methods At A Glance

| Method          | Passes | Best For          | Time      | Security   |
| --------------- | ------ | ----------------- | --------- | ---------- |
| CryptoErase     | 1      | Any modern system | Fast      | ⭐⭐⭐⭐⭐ |
| SinglePass      | 1      | Bulk erasure      | Very Fast | ⭐⭐⭐     |
| ThreePass       | 3      | General HDD       | Medium    | ⭐⭐⭐⭐   |
| SevenPass       | 7      | Sensitive HDD     | Slow      | ⭐⭐⭐⭐⭐ |
| AtaSecureErase  | 1      | SSD (native)      | Very Fast | ⭐⭐⭐⭐⭐ |
| FlashOverwrite  | 2      | USB/SD cards      | Medium    | ⭐⭐⭐⭐   |
| SecureFormat    | 1      | Quick format      | Very Fast | ⭐⭐       |
| EncryptionAware | 1      | Mobile/Encrypted  | Fast      | ⭐⭐⭐⭐⭐ |
| RaidServerWipe  | 3      | Enterprise RAID   | Medium    | ⭐⭐⭐⭐⭐ |

## 🛠️ Build Process

```
npm run build:addon
    ↓
node-gyp reads binding.gyp
    ↓
Compiles C/C++ files:
    - addon.cpp (entry point)
    - securewipe_core.c (main logic)
    - native/methods/*.c (wipe algorithms)
    ↓
Links into securewipe_native.node (binary addon)
    ↓
Placed in build/Release/
    ↓
main.js loads and uses it at runtime
```

## 📝 File Purpose Quick Reference

| File                  | Purpose                      | Languages  |
| --------------------- | ---------------------------- | ---------- |
| **main.js**           | IPC handlers, business logic | JavaScript |
| **app.js**            | UI state, button handlers    | JavaScript |
| **index.html**        | Page structure               | HTML       |
| **main.css**          | Styling, theme               | CSS        |
| **securewipe_core.c** | Disk I/O, progress           | C          |
| **addon.cpp**         | Node.js binding              | C++        |
| **methods.h**         | Method interface             | C header   |
| **method\_\*.c**      | Pattern algorithms           | C          |

## 🔗 How Files Connect

```
User clicks button (index.html)
    ↓
app.js handles click
    ↓
calls api.XXXXXX() [IPC]
    ↓
main.js receives IPC request
    ↓
calls addon.XXXXXX() [native function]
    ↓
addon.cpp routes to securewipe_core.c
    ↓
securewipe_core.c calls method_dispatcher.c
    ↓
method_dispatcher.c calls method_*.c
    ↓
Returns result back through chain
    ↓
UI updates with result
```

## 🎯 To Understand A Feature, Follow These Steps

1. **Find the UI button** → Look in `index.html`
2. **Find the button handler** → Search in `app.js` for the click event
3. **Find the IPC call** → See what `api.XXXXX()` is called
4. **Find the main process handler** → Look in `main.js` for `ipcMain.handle()`
5. **Find the native call** → See what `addon.XXXXX()` is called
6. **Find the C implementation** → Look in `native/securewipe_core.c` or `native/methods/`

## 💡 Common Questions

**Q: How do I add a new wipe method?**
A:

1. Create `native/methods/method_mymethod.c` with pattern function
2. Add enum to `methods.h`
3. Update `method_dispatcher.c` to route it
4. Update `binding.gyp` to compile new file
5. Run `npm run build:addon`

**Q: Where does my drive get wiped?**
A: `native/securewipe_core.c` → `perform_real_wipe_step()` function

**Q: How is the drive list generated?**
A: Native addon calls Windows API to enumerate disks, main.js filters and formats

**Q: What controls the progress bar?**
A: `app.js` polls `api.checkStatus()` which calls `get_operation_status()` in main.js

**Q: How is dark mode implemented?**
A: CSS variables in `main.css` switched by theme toggle button in `app.js`

---

**Remember**: Start with the user action → work backwards through the code chain!

