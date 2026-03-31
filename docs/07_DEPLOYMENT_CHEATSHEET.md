# 🎯 SecureWipe Deployment Cheat Sheet

One-page reference for releasing your Electron app.

---

## 📦 Build the Installer (5 min)

```bash
npm run build:addon        # Compile C++ code
npm run dist              # Build Windows NSIS installer
```

**Output**: `dist/SecureWipe Setup 1.0.0.exe` (~75 MB)

---

## 📤 Distribute (Choose One)

### GitHub Releases ⭐ Recommended

```bash
# Push code to GitHub
# Create Release on GitHub.com
# Upload .exe file
# Users download from:
# https://github.com/YourUser/DataWiping/releases
```

### Your Website

```bash
# Upload .exe to your server
# Create download page
# Users visit: https://yoursite.com/download
```

### Cloud Storage (Quick)

```bash
# Upload to Google Drive / OneDrive / Dropbox
# Share public link
# Users download directly
```

### USB / Local Network

```bash
# Copy .exe to USB drive
# Share via email / network
# Users run installer
```

---

## 📋 Before Release

- [ ] Version updated: `package.json` "version": "1.0.0"
- [ ] Addon built: `npm run build:addon`
- [ ] App tested: `npm start` ✅
- [ ] No errors: DevTools Console clean
- [ ] Icon looks good
- [ ] System requirements documented
- [ ] Release notes prepared

---

## 🔧 Making an Update (v1.0.1)

```bash
# 1. Update version in package.json
# 2. npm run build:addon
# 3. npm run dist
# 4. Upload to distribution channel
# 5. Announce to users
```

---

## 🎁 Optional Features

### Portable Version (No Installation)

```json
// In package.json, update "win" → "target":
"target": ["nsis", "portable"]
```

Creates both installer and portable .exe

### Auto-Updates (Advanced)

1. Install: `npm install electron-updater`
2. Add to `src/electron/main.js`:

```javascript
const { autoUpdater } = require("electron-updater");
autoUpdater.checkForUpdatesAndNotify();
```

### Code Signing (Professional)

1. Buy certificate (~$100-300/year)
2. Add to `package.json` build config
3. Rebuilds auto-sign installers

---

## 📊 Size Guide

| Item             | Size                  |
| ---------------- | --------------------- |
| Installer (.exe) | ~50-100 MB            |
| After install    | ~100-150 MB (on disk) |
| App in memory    | ~200-300 MB (running) |

---

## ✅ System Requirements

**Tell users**:

- Windows 7+ (64-bit)
- Admin rights to wipe
- 100 MB disk space
- No dependencies needed (all bundled)

---

## 🌍 Distribution Comparison

| Method  | Setup  | Users      | Cost       | Best For    |
| ------- | ------ | ---------- | ---------- | ----------- |
| GitHub  | 10 min | Tech-savvy | Free       | OSS         |
| Website | 30 min | Everyone   | $10-100/mo | Business    |
| Drive   | 5 min  | Dozens     | Free       | Quick share |
| USB     | 2 min  | Local      | Free       | Teams       |

---

## 🚀 Complete Release Timeline

```
Today:
  └─ npm run dist
  └─ Test installer
  └─ Upload to GitHub/website

Tomorrow:
  └─ Create Release Notes
  └─ Announce to users

Week 1:
  └─ Collect feedback
  └─ Fix bugs

Month 1:
  └─ Release v1.0.1 (patch)

Month 3:
  └─ Release v1.1.0 (features)
```

---

## 📝 Quick Release Notes Template

```markdown
# SecureWipe Release Notes

**Version**: 1.0.0
**Date**: April 1, 2026
**Download**: [Link](url)

## What's New

- Initial public release
- 9 wipe methods
- Real-time progress tracking

## Installation

1. Download installer
2. Run .exe
3. Follow wizard
4. Launch from Start menu

## System Requirements

- Windows 7 or later (64-bit)
- Admin rights
- 100 MB free space

## Known Issues

(None for v1.0.0)

## Feedback

Report bugs: GitHub Issues
```

---

## 🐛 Troubleshooting

| Problem               | Fix                                |
| --------------------- | ---------------------------------- |
| ".exe won't run"      | Download again, clear temp files   |
| "Admin rights needed" | App requires admin to access disks |
| "Installer corrupt"   | Redownload from source             |
| "Can't uninstall"     | Use Windows Control Panel → Apps   |
| "File too large"      | Normal for Electron app            |

---

## 📊 Monitoring After Release

Track:

- **Downloads**: How many people got it?
- **Active users**: How many use regularly?
- **Bug reports**: What breaks?
- **Feedback**: What do users want?

---

## 🔗 Important Links

- **For detailed guide**: See [05_DEPLOYMENT.md](./05_DEPLOYMENT.md)
- **Release checklist**: See [08_RELEASE_CHECKLIST.md](./08_RELEASE_CHECKLIST.md)
- **Full documentation**: See [00_DOCUMENTATION_INDEX.md](./00_DOCUMENTATION_INDEX.md)

---

## 🎯 TL;DR - Minimum Steps

```bash
npm run build:addon && npm run dist
# File: dist/SecureWipe Setup 1.0.0.exe

# Upload to GitHub / Drive / Website
# Users download and install
# 🎉 Done!
```

**That's it. You're deployed!**

---

## ✨ Next Steps After Release

1. ✅ Announce release
2. ✅ Share download link
3. ✅ Monitor feedback
4. ✅ Fix bugs → v1.0.1
5. ✅ Add features → v1.1.0
6. ✅ Set up auto-updates (optional)
7. ✅ Add code signing (optional)

---

**Questions? Check [05_DEPLOYMENT.md](./05_DEPLOYMENT.md) for full guide!** 📚

