# 🛠️ SecureWipe Development Guide

A hands-on guide to working with the SecureWipe codebase. Includes step-by-step examples.

---

## 📌 Before You Start

1. **Read these in order**:
   - `01_QUICKSTART.md` (5 min overview)
   - `02_ARCHITECTURE.md` (30 min architecture)
   - `03_CODE_STRUCTURE.md` (30 min file breakdown)
   - This file → `04_DEVELOPMENT.md` (you are here)

2. **Have open during coding**:
   - One terminal for commands
   - VS Code for editing
   - `02_ARCHITECTURE.md` as reference

3. **Copy code carefully** - Pay attention to:
   - Indentation (2 or 4 spaces)
   - Semicolons in JavaScript
   - Curly braces `{}` placement
   - Quotes: `"double"` vs `'single'`

---

## 🎮 Getting Started

### 1️⃣ Install & Run

```bash
# From e:\DataWiping directory
npm install                  # Install dependencies
npm run build:addon         # Compile C code
npm start                   # Launch Electron app
```

**If addon won't compile**:
1. Open `binding.gyp`
2. Check all `.c` files are listed in "sources"
3. Run again: `npm run build:addon`

---

### 2️⃣ Make Your First Change (Easy)

Let's change the wipe button color to green!

**Step 1**: Open `src/renderer/styles/main.css`

**Step 2**: Find `.` button color definition. Search for `button` (Ctrl+F):

```css
button {
  background: var(--accent);  /* Currently light blue */
  color: var(--text);
  border: none;
  padding: 8px 16px;
  border-radius: 4px;
  cursor: pointer;
}
```

**Step 3**: Change to green specifically for Start button:

```css
/* Add this new rule anywhere in the file */
#startWipeBtn {
  background: #00aa00;  /* Green! */
}
```

**Step 4**: Save file (Ctrl+S)

**Step 5**: If app is still running, it auto-reloads. If not:
```bash
npm start
```

**Step 6**: Click the wipe button - should be **green**! 🟢

---

## 🎯 Common Tasks & How To Do Them

### Task 1: Add a Status Message

**Goal**: Add "Ready to wipe!" message before user starts

**Step 1**: Open `src/renderer/index.html`

**Step 2**: Find where you want to put it. Search for `<div id="wipeStatus">`:

```html
<div id="wipeStatus" class="status-text muted">
  Ready to start
</div>
```

**Step 3**: Change the text:

```html
<div id="wipeStatus" class="status-text muted">
  Ready to wipe - select a drive and method!
</div>
```

**Step 4**: Save and reload (`npm start`)

✅ Done! Message appears in the UI.

---

### Task 2: Add a Console Log (for Debugging)

**Goal**: See when user clicks "Start Wipe" button in the console

**Step 1**: Open `src/renderer/scripts/app.js`

**Step 2**: Search for `startWipeOperation` function:

```javascript
function startWipeOperation() {
  // Existing code...
}
```

**Step 3**: Add console.log at the top:

```javascript
function startWipeOperation() {
  console.log("START WIPE CLICKED!", {
    drive: driveSelectEl.value,
    method: methodSelectEl.value
  });
  
  // Existing code continues...
}
```

**Step 4**: Open DevTools: Press `Ctrl+Shift+I` in the running app

**Step 5**: Go to "Console" tab

**Step 6**: Click "Start Wipe" button

**Expected output**:
```
START WIPE CLICKED! { drive: "D:", method: "ThreePass" }
```

✅ Done! You can see what the user selected.

---

### Task 3: Modify a Recommendation

**Goal**: Change drive recommendation from "ThreePass" to "SinglePass" for HDDs

**Step 1**: Open `src/renderer/scripts/app.js`

**Step 2**: Find `buildMethodSuggestionText` function:

```javascript
function buildMethodSuggestionText(driveInfo, methodKey) {
  // ... existing code ...
  
  if (removable) {
    deviceClass = "USB/SD/Flash";
    primary = "FlashOverwrite";
    alternative = "SinglePass";
  } else if (fixed) {
    deviceClass = "Fixed Disk (HDD/SSD)";
    primary = "AtaSecureErase";    // ← Change this
    alternative = "ThreePass";     // ← or this
  }
  
  // ... rest of code ...
}
```

**Step 3**: Change "AtaSecureErase" to "SinglePass":

```javascript
} else if (fixed) {
  deviceClass = "Fixed Disk (HDD/SSD)";
  primary = "SinglePass";          // Changed!
  alternative = "ThreePass";
}
```

**Step 4**: Save and reload

**Step 5**: Test by:
1. Click "Get Disks"
2. Select a fixed drive (like C: or D:)
3. See the recommendation now says "Primary: SinglePass"

✅ Done! Recommendation updated.

---

### Task 4: Add a New UI Element (Advanced)

**Goal**: Add a checkbox "Advanced Options" that'll control a dropdown

**Step 1**: Open `src/renderer/index.html`

**Step 2**: Find the section with checkboxes. Search for `termsAgreeCheck`:

```html
<label>
  <input type="checkbox" id="termsAgreeCheck" />
  I acknowledge data will be permanently erased
</label>
```

**Step 3**: Add your new checkbox right after:

```html
<label>
  <input type="checkbox" id="advancedOptionsCheck" />
  Show Advanced Options
</label>
```

**Step 4**: Now add the dropdown that appears when checked. Add right after:

```html
<div id="advancedOptionsPanel" style="display: none; margin-top: 10px; padding: 10px; border: 1px solid var(--accent); border-radius: 4px;">
  <label>
    Buffer Size:
    <select id="bufferSizeSelect">
      <option value="1mb">1 MB</option>
      <option value="2mb">2 MB</option>
      <option value="4mb">4 MB</option>
    </select>
  </label>
</div>
```

**Step 5**: Open `src/renderer/scripts/app.js`

**Step 6**: Add element references at the top (where other `getElementById` calls are):

```javascript
const advancedOptionsCheck = document.getElementById("advancedOptionsCheck");
const advancedOptionsPanel = document.getElementById("advancedOptionsPanel");
const bufferSizeSelect = document.getElementById("bufferSizeSelect");
```

**Step 7**: Add event listener to make checkbox toggle the panel:

```javascript
// Add this somewhere in the initialization section
advancedOptionsCheck.addEventListener("change", function() {
  advancedOptionsPanel.style.display = this.checked ? "block" : "none";
  console.log("Advanced options toggled:", this.checked);
});
```

**Step 8**: Save and reload

**Step 9**: Test by clicking the "Show Advanced Options" checkbox - panel appears/disappears!

✅ Done! You added an interactive UI element!

---

### Task 5: Add a New Backend Function (IPC Handler)

**Goal**: Add a handler to echo text back from backend (for learning)

**Step 1**: Open `src/electron/preload.js`

**Step 2**: Find the `window.securewipeApi` object:

```javascript
window.securewipeApi = {
  getDisks: () => ipcRenderer.invoke("getDisks"),
  getMethods: () => ipcRenderer.invoke("getMethods"),
  // ... more functions ...
};
```

**Step 3**: Add your new function:

```javascript
window.securewipeApi = {
  getDisks: () => ipcRenderer.invoke("getDisks"),
  getMethods: () => ipcRenderer.invoke("getMethods"),
  echo: (text) => ipcRenderer.invoke("echo", text),  // ← New!
  // ... more functions ...
};
```

**Step 4**: Open `src/electron/main.js`

**Step 5**: Find the `// ===== IPC HANDLERS =====` section

**Step 6**: Add your handler:

```javascript
// ===== IPC HANDLERS =====

// Existing handlers...

// New handler for echo (testing/learning)
ipcMain.handle("echo", async (event, text) => {
  console.log("Backend received:", text);
  return {
    ok: true,
    message: `You said: ${text}`,
    timestamp: new Date().toISOString()
  };
});
```

**Step 7**: Open `src/renderer/scripts/app.js`

**Step 8**: Add a test button that calls it. Find the init/setup section and add:

```javascript
// Add this in your initialization
document.getElementById("someButton").addEventListener("click", async () => {
  const result = await api.echo("Hello, backend!");
  console.log("Backend response:", result);
  alert(result.message);
});
```

Or create a simple button inline in `index.html`:

```html
<button onclick="testEcho()">Test Echo</button>
```

Then in `app.js`:

```javascript
async function testEcho() {
  const result = await api.echo("Hello, backend!");
  alert(result.message);
}
```

**Step 9**: Test by clicking the button!

✅ Done! You added a backend function!

---

## 🧪 Testing Your Changes

### Test Checklist Before Submitting

- [ ] Code has no syntax errors
- [ ] App launches: `npm start` doesn't crash
- [ ] Your change works as expected
- [ ] No console errors (DevTools → Console)
- [ ] Existing features still work (don't break things!)
- [ ] You can repeat the test successfully

### How to Test

```bash
# Terminal 1: Launch app
npm start

# Terminal 2: Make your change to code

# You can:
# 1. Save file → app auto-reloads (JavaScript)
# 2. For C changes, run: npm run build:addon, then restart app
# 3. Test the feature
# 4. Check DevTools (Ctrl+Shift+I) for errors
```

---

## 🐛 Debugging Techniques

### Technique 1: Console Logging

Add logs to see what's happening:

```javascript
console.log("Variable value:", myVar);
console.log("At point X in function");
console.error("Something went wrong:", errorObj);
```

View in DevTools: Press `Ctrl+Shift+I` → Console tab

---

### Technique 2: Breakpoints

Set breakpoints to pause execution:

1. Open DevTools: `Ctrl+Shift+I`
2. Go to "Sources" tab
3. Find your `.js` file
4. Click line number to set breakpoint
5. Execute code that hits that line
6. Execution pauses, inspect variables!

---

### Technique 3: Network Tab

See what IPC calls happen:

1. In `main.js`, add logging:
   ```javascript
   ipcMain.handle("getDisks", async () => {
     console.log("IPC: getDisks called");
     // ... rest
   });
   ```

2. Open DevTools Console
3. Trigger action in UI
4. See log messages!

---

### Technique 4: UI Inspection (Elements Tab)

See what HTML is generated:

1. DevTools → Elements tab
2. Right-click element in page
3. Select "Inspect"
4. See its HTML structure
5. Can temporarily modify styles to test

---

## 📝 Code Style Guidelines

### JavaScript
```javascript
// Good
function doSomething(param1, param2) {  // CamelCase for functions
  const myVariable = 5;                 // CamelCase for variables
  let result = myVariable * 2;
  return result;
}

// Bad
function do_something(param1, param2) { // snake_case❌
  var myVariable = 5;                   // var❌ (use const/let)
  var result = myVariable * 2;
  return result;
}
```

### CSS
```css
/* Good */
.my-element {
  background-color: #ffffff;
  padding: 10px;
  margin-top: 5px;
}

/* Bad */
.myelement {
  background-color: white;  /* Use hex */
  padding-left: 10px;
  paddingRight: 10px;       /* Redundant */
  margin-top: 5px;
}
```

### C
```c
// Good
typedef struct {
  int value;
  char name[64];
} MyStruct;

static int count_items(MyStruct *items, int count) {
  int total = 0;
  for (int i = 0; i < count; i++) {
    total += items[i].value;
  }
  return total;
}

// Bad
typedef struct {
  int v;           // Too short
  char n[64];
} MyStruct;

int CountItems(MyStruct *items, int count) {  // PascalCase ❌
  // No type prefixes
  int total = 0;
  for (int i = 0; i < count; i++) {
    total += items[i].v;
  }
  return total;
}
```

---

## 🚀 Advanced: Adding a New Wipe Method

### Step-by-Step Example: "ZeroPass" Method

**Goal**: Add a wipe method called "ZeroPass" that does a single pass with all zeros.

**Step 1**: Open `native/methods/methods.h`

**Step 2**: Add enum:

```c
typedef enum
{
    METHOD_SINGLE_PASS,
    METHOD_THREE_PASS,
    METHOD_SEVEN_PASS,
    METHOD_CRYPTO_ERASE,
    METHOD_ATA_SECURE_ERASE,
    METHOD_FLASH_OVERWRITE,
    METHOD_ENCRYPTION_AWARE,
    METHOD_RAID_SERVER_WIPE,
    METHOD_SECURE_FORMAT,
    METHOD_ZERO_PASS,        // ← Add this!
} WipeMethodType;
```

**Step 3**: Open `native/methods/method_dispatcher.c`

**Step 4**: Add case in `method_from_key()`:

```c
if (strcmp(key, "ZeroPass") == 0 || strcmp(key, "zeropass") == 0)
{
    return METHOD_ZERO_PASS;  // ← Add this!
}
```

**Step 5**: Add case in `get_wipe_pattern()`:

```c
case METHOD_ZERO_PASS:
    return get_standard_pattern("ZeroPass", passIndex);
```

**Step 6**: Add case in `get_method_passes()`:

```c
case METHOD_ZERO_PASS:
    return 1;  // Single pass
```

**Step 7**: Open `native/methods/method_standard.c`

**Step 8**: Add logic:

```c
if (strcmp(method, "ZeroPass") == 0) {
    return 0x00;  // Always zeros
}
```

**Step 9**: Open `src/electron/main.js`

**Step 10**: Add to `METHOD_SPECS`:

```javascript
const METHOD_SPECS = {
  // ... existing methods ...
  ZeroPass: { passes: 1, forTypes: ["HDD", "SSD"], label: "Zero Pass" },
};
```

**Step 11**: Open `binding.gyp`

**Step 12**: Verify all method files are listed (should already be):

```json
"sources": [
  "native/addon.cpp",
  "native/securewipe_core.c",
  "native/methods/method_standard.c",
  "native/methods/method_crypto.c",
  "native/methods/method_storage.c",
  "native/methods/method_raid.c",
  "native/methods/method_dispatcher.c"
]
```

**Step 13**: Rebuild and test:

```bash
npm run build:addon    # Recompile C code
npm start              # Launch app
```

**Step 14**: Test:
1. Get disks
2. Select a drive
3. In method dropdown, "Zero Pass" should appear
4. Select it
5. Try wipe (on test drive!)

✅ Done! New method added!

---

## 📊 Project Stats

```
Total Lines of Code:  ~3000
├─ JavaScript:  ~1200 (frontend + Electron)
├─ C:           ~1200 (native core)
├─ CSS:         ~400
├─ HTML:        ~150
└─ JSON:        ~50

File Count:     ~30 total
├─ Source:      ~15
├─ Build:       ~10 (auto-generated)
└─ Config:      ~5

Compilation Time:  ~5-10 seconds
App Startup Time:  ~2-3 seconds
```

---

## 🎓 Learning Resources

### JavaScript
- [MDN JavaScript Guide](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide)
- [Async/Await](https://developer.mozilla.org/en-US/docs/Learn/JavaScript/Asynchronous/Async_await)

### Electron  
- [Electron Docs](https://www.electronjs.org/docs)
- [IPC (Inter-Process Communication)](https://www.electronjs.org/docs/latest/api/ipc-main)

### C Programming
- [C Docs](https://en.cppreference.com/w/c)
- [Windows API](https://docs.microsoft.com/en-us/windows/win32/api/)

### Web Development
- [CSS Variables](https://developer.mozilla.org/en-US/docs/Web/CSS/--*)
- [DevTools Guide](https://developer.chrome.com/docs/devtools/)

---

## ✅ Completion Checklist

After reading this guide, you should be able to:

- [ ] Launch the app: `npm start`
- [ ] Edit an HTML file and see changes
- [ ] Edit a CSS file and see style changes  
- [ ] Edit JavaScript and see behavior changes
- [ ] Use DevTools to debug
- [ ] Add a new button
- [ ] Add event listeners
- [ ] Call backend functions via IPC
- [ ] Add a new IPC handler
- [ ] Add console logs for debugging
- [ ] Understand the data flow
- [ ] Add a new wipe method
- [ ] Rebuild addon: `npm run build:addon`

---

## 🆘 Stuck? Here's What to Do

1. **Error in console?** → Copy exact error, search GitHub/Stack Overflow
2. **Doesn't compile?** → Check `binding.gyp` and file paths
3. **Feature missing?** → Read 03_CODE_STRUCTURE.md to find the file
4. **Don't understand code?** → Add `console.log()` to see what's happening
5. **Want to learn more?** → Read the linked resources above

---

**Remember**: Every expert was once a beginner. Take it slow, test often, and don't be afraid to experiment! 🚀

