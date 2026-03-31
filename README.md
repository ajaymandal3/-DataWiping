# 🔐 SecureWipe - Desktop Secure Wipe Application

A professional Windows desktop app for securely wiping storage devices using pattern-based overwriting.

## 📋 Requirements

Built with:

- **Frontend**: Electron 37.10.3 (Chromium + Node.js)
- **UI**: HTML/CSS/JavaScript
- **Backend**: C code (low-level disk I/O)
- **Build**: Node-gyp (C/C++ compilation)

System Requirements:

- Windows 7 or later (64-bit)
- Node.js 18+ (for development only)
- Python 3 (for node-gyp)
- Visual Studio C++ Build Tools (for compilation)

## 🚀 Quick Start

### For Development:

```bash
# 1. Install dependencies
npm install

# 2. Build native addon (C code)
npm run build:addon

# 3. Run the app
npm start
```

### For Users (Distribution):

See **[05_DEPLOYMENT.md](./docs/05_DEPLOYMENT.md)** for how to:

- Create installer for others to download
- Package as portable app
- Auto-update system
- Code signing

**Quick build command**:

```bash
npm run dist        # Creates Windows installer in /dist/
```

## 📁 Project Structure

```
SecureWipe/
├── src/
│   ├── electron/           # Main process (IPC, backend logic)
│   │   ├── main.js         # Entry point, IPC handlers
│   │   ├── preload.js      # Security bridge
│   │   └── assets/         # Icons
│   │
│   └── renderer/           # Renderer process (UI)
│       ├── index.html      # Page structure
│       ├── styles/main.css # Styling & theme
│       └── scripts/app.js  # UI logic
│
├── native/                 # Low-level C code
│   ├── addon.cpp           # Node.js binding
│   ├── securewipe_core.c   # Main wipe engine
│   └── methods/            # Wipe algorithms
│       ├── methods.h
│       ├── method_*.c      # Pattern implementations
│       └── method_dispatcher.c
│
├── build/                  # Compiled output (auto-generated)
└── dist/                   # Installers (auto-generated)

```

## 📚 Documentation

Start here for different purposes:

| Need                    | Read                                                    |
| ----------------------- | ------------------------------------------------------- |
| 5-min overview          | [01_QUICKSTART.md](./docs/01_QUICKSTART.md)                   |
| Understand architecture | [02_ARCHITECTURE.md](./docs/02_ARCHITECTURE.md)               |
| Deep dive into code     | [03_CODE_STRUCTURE.md](./docs/03_CODE_STRUCTURE.md)           |
| Ready to code?          | [04_DEVELOPMENT.md](./docs/04_DEVELOPMENT.md)                 |
| Deploy to users?        | [05_DEPLOYMENT.md](./docs/05_DEPLOYMENT.md)                   |
| Navigation help         | [00_DOCUMENTATION_INDEX.md](./docs/00_DOCUMENTATION_INDEX.md) |

## 🎯 Features

✅ **9 Wipe Methods**:

- CryptoErase, SinglePass, ThreePass, SevenPass
- AtaSecureErase (SSD optimized)
- FlashOverwrite (USB/SD cards)
- SecureFormat, EncryptionAware, RaidServerWipe

✅ **Smart Drive Detection**:

- Automatically detect storage devices
- Block system/network drives for safety
- Show recommendations per device type

✅ **Professional Interface**:

- Dark/light theme toggle
- Step-by-step workflow
- Real-time progress tracking
- Audit logging & certificates

✅ **Security**:

- Multi-pass pattern overwriting
- Admin rights enforcement
- Audit trail for compliance
- No data recovery possible

## 🔧 Build Commands

```bash
npm run build:addon        # Compile C/C++ native code
npm run dist              # Create Windows NSIS installer
npm run dist:dir          # Create unpacked app files
npm start                 # Run for development
```

## 📦 Wipe Methods Available

| Method          | Passes | Best For         | Speed      |
| --------------- | ------ | ---------------- | ---------- |
| CryptoErase     | 1      | Any system       | ⚡⚡⚡⚡⚡ |
| SinglePass      | 1      | Bulk erasure     | ⚡⚡⚡⚡⚡ |
| ThreePass       | 3      | General HDD      | ⚡⚡⚡     |
| SevenPass       | 7      | Sensitive HDD    | ⚡⚡       |
| AtaSecureErase  | 1      | SSD native       | ⚡⚡⚡⚡⚡ |
| FlashOverwrite  | 2      | USB/SD           | ⚡⚡⚡⚡   |
| EncryptionAware | 1      | Mobile/Encrypted | ⚡⚡⚡⚡⚡ |
| RaidServerWipe  | 3      | Enterprise       | ⚡⚡⚡     |
| SecureFormat    | 1      | Quick format     | ⚡⚡⚡⚡⚡ |

## 🔐 Drive Eligibility

**Can wipe** ✅:

- Removable drives (USB, external disks)
- Fixed internal drives

**Cannot wipe** ❌:

- System drive (C: with Windows)
- Network drives
- CD/DVD drives

## 📊 Audit & Compliance

- Every wipe operation logged
- Timestamp + drive info + method recorded
- Certificate generated post-wipe
- No recovery possible after wipe
- Audit logs in: `%APPDATA%/SecureWipe/audit/`

## 🚀 Deployment

To distribute your app to others:

```bash
# 1. Build addon
npm run build:addon

# 2. Create installer
npm run dist

# 3. Upload dist/SecureWipe Setup 1.0.0.exe
#    to GitHub, website, or file hosting

# 4. Share download link with users
```

See [05_DEPLOYMENT.md](./docs/05_DEPLOYMENT.md) for detailed deployment guide including:

- GitHub Releases distribution
- Website hosting
- Auto-updates setup
- Code signing
- Version management

## 🐛 Development Notes

- Native addon exports: `getVersion`, `getDisks`, `startWipe`, `getWipeStatus`, `cancelWipe`, `getCertificate`
- All communication via IPC (Inter-Process Communication)
- UI is 3-step workflow: Configure → Execute → Reports
- Theme toggle in header (persisted to localStorage)
- Wipe operations are non-blocking with progress polling
- Requires admin rights to open disk volumes

## 🔗 External Dependencies

| Package          | Purpose            | Version |
| ---------------- | ------------------ | ------- |
| electron         | Desktop framework  | 37.10.3 |
| electron-builder | Installer creation | 26.0.12 |
| node-gyp         | C/C++ compiler     | 11.4.2  |

## 📝 License

MIT

## 🤝 Contributing

1. Read [04_DEVELOPMENT.md](./docs/04_DEVELOPMENT.md) for code setup
2. Make changes
3. Test: `npm start`
4. Build: `npm run dist`
5. Submit PR

## ❓ Common Questions

**Q: Can I wipe my C: drive?**
A: No, system drive is protected for safety.

**Q: How long does a wipe take?**
A: Depends on drive size and method. Multi-pass slower than single-pass.

**Q: Is data recoverable after wipe?**
A: No, pattern overwriting makes recovery impossible.

**Q: Can I use on Linux/Mac?**
A: Currently Windows-only (requires cross-compilation for other OS).

**Q: How do I update to new version?**
A: Manual download for now. Auto-updates can be configured (see 05_DEPLOYMENT.md).

---

**For more details, docs, and deployment info → See [00_DOCUMENTATION_INDEX.md](./docs/00_DOCUMENTATION_INDEX.md)** 📚

