# 🚀 SecureWipe Release Checklist

Quick reference for releasing your app.

---

## ✅ Pre-Release Checklist

- [ ] App tested locally: `npm start` ✅
- [ ] No console errors (DevTools: Ctrl+Shift+I)
- [ ] Addon compiled: `npm run build:addon` ✅
- [ ] Version updated in `package.json`
- [ ] Release notes prepared
- [ ] Download location ready (GitHub/website/drive)
- [ ] System requirements documented
- [ ] Icon looks good

---

## 🔨 Building the Installer (5 Minutes)

```bash
# 1. Navigate to project
cd e:\DataWiping

# 2. Build native addon (if you modified C code)
npm run build:addon

# 3. Create Windows installer
npm run dist

# ✅ Output: dist/SecureWipe Setup 1.0.0.exe (~75 MB)
```

**That's it!** You have a ready-to-distribute installer.

---

## 📤 Distribution Options

### Option 1: GitHub Releases (Easiest for Open Source)

```bash
# 1. Create GitHub repo if not already done
git init
git add .
git commit -m "Release v1.0.0"
git branch -M main
git remote add origin https://github.com/YourUsername/DataWiping
git push -u origin main

# 2. On GitHub.com:
# - Click "Releases"
# - Click "Create a new release"
# - Tag: v1.0.0
# - Title: SecureWipe v1.0.0
# - Upload: dist/SecureWipe Setup 1.0.0.exe
# - Click "Publish"

# 3. Share: https://github.com/YourUsername/DataWiping/releases
```

**Users see**: Clean release page with download button

---

### Option 2: Your Website

```bash
# 1. Upload to your web server:
dist/SecureWipe Setup 1.0.0.exe
  → https://yoursite.com/downloads/SecureWipe.exe

# 2. Create download page with HTML:
<a href="/downloads/SecureWipe.exe">
  Download SecureWipe (v1.0.0, 75 MB)
</a>

# 3. Share website link
```

**Users see**: Professional download page

---

### Option 3: Direct File Share

```bash
# 1. Upload to cloud storage:
#    - Google Drive
#    - OneDrive
#    - Dropbox

# 2. Get public share link

# 3. Send to users
```

**Users see**: File in cloud, can download directly

---

## 📋 Creating Release Notes

Create file: `RELEASE_NOTES.md`

```markdown
# SecureWipe v1.0.0

**Release Date**: April 1, 2026

## What's New

- Initial release
- 9 wipe methods supported
- Multi-pass overwriting
- Real-time progress tracking

## Features

- Smart drive detection
- Audit logging
- Dark/light theme
- Professional installer

## System Requirements

- Windows 7 or later (64-bit)
- Admin rights to wipe drives
- 100 MB free disk space

## Download

[SecureWipe Setup 1.0.0.exe](link)

## Bug Reports

Please report issues on GitHub Issues
```

---

## 🔄 Update Process (For Future Versions)

When you release v1.0.1:

```bash
# 1. Update version in package.json
#    "version": "1.0.1"

# 2. Build addon if needed
npm run build:addon

# 3. Create installer
npm run dist
# Output: dist/SecureWipe Setup 1.0.1.exe

# 4. Upload to distribution channel
#    (GitHub, website, etc.)

# 5. Announce update to users
```

---

## ✨ Portable Version (Optional)

Create a version users don't need to install:

1. Update `package.json` build config:

```json
"win": {
  "target": [
    "nsis",
    "portable"
  ]
}
```

2. Build:

```bash
npm run dist
```

3. Output has both:
   - `SecureWipe Setup 1.0.0.exe` (installer)
   - `SecureWipe 1.0.0.exe` (portable)

Both are same size (~75 MB).

---

## 🔒 Code Signing (Optional)

Prevents Windows "Unknown Publisher" warning.

**Not required for**: Internal use, initial release
**Recommended for**: Professional distribution

**To add later**:

1. Buy certificate (~$100-300/year)
2. Add to `package.json`:

```json
"win": {
  "certificateFile": "path/to/cert.pfx",
  "certificatePassword": "your-password"
}
```

3. Rebuild: `npm run dist`

---

## 📊 Version Numbering

Format: `MAJOR.MINOR.PATCH`

| Version | When          | Example                |
| ------- | ------------- | ---------------------- |
| 1.0.0   | First release | Initial launch         |
| 1.0.1   | Bug fix       | Fixed crash on startup |
| 1.1.0   | New feature   | Added Gutmann method   |
| 2.0.0   | Major change  | Complete redesign      |

---

## 🖇️ System Requirements to Share

```
SecureWipe Requirements:

✓ Windows 7 or later
✓ 64-bit only
✓ Admin rights (to access drives)
✓ 100 MB free disk space
✓ .NET Framework 4.5+ (included with Windows)

Not required:
✗ Node.js (bundled in app)
✗ Python (bundled in app)
✗ Visual Studio (not needed for users)
```

---

## 📢 Distribution Template

Copy this to your website/GitHub:

```markdown
# 📥 Download SecureWipe

**Securely wipe storage devices with pattern-based overwriting.**

## Download

[Download SecureWipe v1.0.0 for Windows](link-to-exe)
**Size**: 75 MB | **Windows 7+** | **64-bit**

## Install

1. Download the installer
2. Run `SecureWipe Setup 1.0.0.exe`
3. Follow the installation wizard
4. Launch from Start menu

## Features

✅ 9 multi-pass wipe methods
✅ HDD, SSD, USB support
✅ Real-time progress tracking
✅ Audit logging for compliance
✅ Dark/light theme

## Supported Wipe Methods

- CryptoErase (1-pass)
- SinglePass (1-pass)
- ThreePass (DoD 3-pass)
- SevenPass (Gutmann 7-pass)
- AtaSecureErase (SSD)
- FlashOverwrite (USB/SD)
- EncryptionAware (mobile)
- RaidServerWipe (enterprise)
- SecureFormat (quick)

## Blocked Drives (for safety)

❌ System drive (C: with Windows)
❌ Network drives
❌ CD/DVD drives

## Questions?

Report issues: [GitHub Issues](link)
```

---

## 🎯 After Release

1. **Collect Feedback**
   - Bugs? → Fix in v1.0.1
   - Features? → Plan for v1.1.0
   - UI issues? → Priority fixes

2. **Monitor Usage**
   - How many downloads?
   - Any crash reports?
   - Users happy?

3. **Plan Next Version**
   - New features?
   - Performance improvements?
   - Platform support?

---

## 🚀 From Code to Release

```
Development ✅
    ↓
npm run build:addon ✅
    ↓
npm run dist ✅
    ↓
Test installer ✅
    ↓
Upload to GitHub/Website ✅
    ↓
Create Release Notes ✅
    ↓
📢 Announce to Users ✅
    ↓
🎉 Released!
```

---

## 📞 Need Help?

| Question                          | Answer                                                        |
| --------------------------------- | ------------------------------------------------------------- |
| My installer is too big           | Check `build.files` in package.json, remove unnecessary files |
| Windows warns "Unknown Publisher" | Add code signing (optional for now)                           |
| Users can't uninstall             | NSIS handles automatically via Control Panel                  |
| App won't launch after install    | Check file paths - use relative paths, not absolute           |
| How often should I release?       | Every 1-2 months with bug fixes and features                  |

---

**Ready? Start with: `npm run dist`** 🚀

_For detailed info, see [05_DEPLOYMENT.md](./05_DEPLOYMENT.md)_

