# 🚀 SecureWipe Deployment Guide

How to build, package, and distribute your Electron app for public use.

---

## 📦 Deployment Overview

Your app can be deployed through several methods:

| Method               | Download Size | Installation       | Distribution    | Best For          |
| -------------------- | ------------- | ------------------ | --------------- | ----------------- |
| **NSIS Installer**   | ~50-100 MB    | Standard installer | File hosting    | General users ✅  |
| **Portable EXE**     | ~50-100 MB    | No installation    | File hosting    | Windows only      |
| **ZIP Archive**      | ~50-100 MB    | Extract & run      | Manual sharing  | Developers        |
| **GitHub Releases**  | ~50-100 MB    | Auto-download      | GitHub          | Technical users   |
| **Website**          | ~50-100 MB    | Download page      | Your domain     | Professional      |
| **Electron Updater** | Dynamic       | Auto-updates       | Cloud storage   | Regular updates   |
| **Windows Store**    | Depends       | Native install     | Microsoft Store | Wide distribution |

---

## ⚡ Quick Start: Build Your First Installer (5 minutes)

### Step 1: Ensure Addon is Built

```bash
cd e:\DataWiping
npm run build:addon        # Compile C++ native addon
```

**Should see**: `gyp info ok` at the end

### Step 2: Build the App

```bash
npm run dist               # Create Windows installer
```

**What happens**:

- Compiles Electron app
- Bundles all files
- Creates `dist/SecureWipe Setup 1.0.0.exe` installer

**File size**: ~50-100 MB (depends on content)

### Step 3: Test the Installer

```bash
# Navigate to dist folder
cd dist

# Run the installer
"SecureWipe Setup 1.0.0.exe"

# Follow installation wizard
# App installs to AppData\Local\Programs\SecureWipe
# Creates Start menu shortcuts
```

### Step 4: Uninstall to Test Again

In Windows Settings → Apps → SecureWipe → Uninstall

✅ **Done!** Now you have a packaged app anyone can install.

---

## 📊 Distribution Methods

### Method 1: GitHub Releases (Recommended for Open Source)

**Advantages**: Free, automatic, GitHub integration, version history
**Users**: Technical people, developers

#### Steps:

1. **Create GitHub Repository**

   ```bash
   git init
   git add .
   git commit -m "Initial commit"
   git branch -M main
   git remote add origin https://github.com/YourUsername/DataWiping.git
   git push -u origin main
   ```

2. **Create Release in GitHub**
   - Go to GitHub repo
   - Click "Releases" → "Create a new release"
   - Tag: `v1.0.0`
   - Title: `SecureWipe 1.0.0`
   - Description: What's new?
   - Upload: `dist/SecureWipe Setup 1.0.0.exe`

3. **Users Download From**
   ```
   https://github.com/YourUsername/DataWiping/releases
   ```

---

### Method 2: Personal Website (Most Professional)

**Advantages**: Professional image, custom branding, full control
**Users**: Anyone visiting your website

#### Steps:

1. **Build the installer** (see Quick Start above)

2. **Host file on web server**:
   - Upload `dist/SecureWipe Setup 1.0.0.exe` to your server
   - Note the URL: `https://yoursite.com/downloads/SecureWipe.exe`

3. **Create download page** (HTML)

   ```html
   <h1>SecureWipe Desktop</h1>
   <p>Securely wipe storage devices</p>
   <a href="/downloads/SecureWipe.exe" class="btn">
     Download for Windows (v1.0.0)
   </a>
   <p>File size: 75 MB</p>
   <p>Windows 7 or later required</p>
   ```

4. **Add version info** to track updates
   ```json
   {
     "version": "1.0.0",
     "url": "https://yoursite.com/downloads/SecureWipe.exe",
     "releaseNotes": "Initial release"
   }
   ```

---

### Method 3: Direct File Sharing (Quickest)

**Advantages**: Simple, immediate access
**Users**: Friends, colleagues, internal use

#### Methods:

**Option A: Google Drive**

1. Upload `dist/SecureWipe Setup 1.0.0.exe`
2. Share link: `https://drive.google.com/file/d/.../view`
3. Send to people

**Option B: OneDrive**

1. Upload to OneDrive
2. Share link and set to public
3. Send link to people

**Option C: Dropbox**

1. Upload to Dropbox
2. Share public link
3. Dropbox auto-creates download page

---

### Method 4: Portable Version (No Installation)

Users can run directly without installing.

#### Steps:

1. **Add to package.json build config**:

   ```json
   "win": {
     "icon": "src/electron/assets/icon.ico",
     "target": [
       "nsis",
       "portable"
     ]
   }
   ```

2. **Rebuild**:

   ```bash
   npm run dist
   ```

3. **Output**:
   - `dist/SecureWipe 1.0.0.exe` - Portable version
   - `dist/SecureWipe Setup 1.0.0.exe` - Installer version

**Portable advantages**:

- Run from USB drive
- No installation needed
- Run from any folder
- File size: Same as installer (~75 MB)

---

## 🔄 Advanced: Auto-Updates (For Regular Updates)

Allow users to update automatically without re-downloading.

### Step 1: Add electron-updater

```bash
npm install electron-updater
```

### Step 2: Update main.js

Add this at the top of `src/electron/main.js`:

```javascript
const { autoUpdater } = require("electron-updater");

// Check for updates on startup
autoUpdater.checkForUpdatesAndNotify();
```

### Step 3: Host update metadata

Create file on your server: `https://yoursite.com/updates/latest.json`

```json
{
  "version": "1.1.0",
  "files": [
    {
      "url": "https://yoursite.com/updates/SecureWipe-1.1.0.exe",
      "sha512": "x+LjCvSxANqZFQqq...",
      "size": 75000000
    }
  ],
  "path": "https://yoursite.com/updates/SecureWipe-1.1.0.exe",
  "sha512": "x+LjCvSxANqZFQqq...",
  "releaseDate": "2026-04-01"
}
```

### Step 4: Users get auto-update prompt

When new version available → "Update available" dialog → "Restart & Update"

---

## 🔒 Code Signing (Optional but Recommended)

Prevents Windows SmartScreen warning when downloading.

### What is Code Signing?

Creates a certificate that proves "this app is legitimate".

### How to Add:

1. **Get Certificate** (~$100-300/year or free)
   - Cheap: Sectigo, Comodo
   - Free: Self-signed (not trusted by Windows)
   - Best: DigiCert ($300/year)

2. **Add to package.json build config**:

   ```json
   "win": {
     "icon": "src/electron/assets/icon.ico",
     "certificateFile": "path/to/certificate.pfx",
     "certificatePassword": "your-password",
     "target": ["nsis"]
   }
   ```

3. **Rebuild**:
   ```bash
   npm run dist
   ```

**When NOT needed**: For internal use or initial release. Add later if desired.

---

## 📋 Pre-Deployment Checklist

Before releasing, verify:

- [ ] App version updated in `package.json` ("version": "1.0.0")
- [ ] Addon built: `npm run build:addon`
- [ ] App tested: `npm start` ✅
- [ ] No console errors (DevTools → Console)
- [ ] All features working correctly
- [ ] Download link/URL decided
- [ ] System requirements documented
- [ ] Icon looks good in installer
- [ ] First run works smoothly

---

## 🎯 Step-by-Step: From Development to Deployed

### Scenario: Release v1.0.0

**Day 1: Preparation**

```bash
# Ensure everything works
npm run build:addon
npm start
# [Test all features manually]

# Update version
# Edit package.json:
# "version": "1.0.0"

# Build installer
npm run dist

# Find in: e:\DataWiping\dist\SecureWipe Setup 1.0.0.exe
```

**Day 2: Distribution**

```bash
# Option A: Upload to GitHub Releases
# - Create release on GitHub
# - Upload dist files
# - Share link

# Option B: Upload to your website
# - Upload dist/*.exe to server
# - Create download page

# Option C: Send directly
# - Email dist/*.exe to people
# - Or share via Google Drive
```

**Day 3+: Support**

- Track bug reports
- Collect feedback
- Start working on v1.0.1

---

## 📊 Distribution Comparison

| Aspect           | GitHub    | Website    | Direct Share | Portable     |
| ---------------- | --------- | ---------- | ------------ | ------------ |
| **Setup Time**   | 10 min    | 30 min     | 5 min        | 5 min        |
| **Professional** | ⭐⭐⭐    | ⭐⭐⭐⭐⭐ | ⭐⭐         | ⭐⭐⭐       |
| **Cost**         | Free      | $10-100/mo | Free         | Free         |
| **Auto-Updates** | Manual    | Possible   | Manual       | Manual       |
| **User Count**   | Thousands | Depends    | Dozens       | Any          |
| **Best For**     | OSS       | Business   | Teams        | USB/Portable |

---

## 🖥️ System Requirements to Document

Tell users what they need:

```
SecureWipe Requirements:

• Windows 7 or later (Windows 10+ recommended)
• 64-bit only
• 100 MB disk space for installation
• Admin rights to wipe drives
• .NET Framework 4.5+ (comes with Windows 7+)
```

---

## 🔗 Distribution Links Template

Share on your website/GitHub:

```markdown
# SecureWipe Desktop v1.0.0

Securely wipe storage devices with pattern-based overwriting.

## Download

[📥 Download for Windows (75 MB)](link-to-installer)

[📦 Portable Version (75 MB)](link-to-portable)

## System Requirements

- Windows 7 or later (64-bit)
- Admin rights
- 100 MB free disk space

## Installation

1. Download the installer
2. Run SecureWipe Setup 1.0.0.exe
3. Follow the installation wizard
4. Launch from Start menu

## Features

- Multi-pass wipe algorithms
- Support for HDD, SSD, USB drives
- Real-time progress tracking
- Audit logging

## Updates

Latest version: v1.0.0 (April 1, 2026)
```

---

## 🐛 Common Deployment Issues

| Issue                             | Solution                                     |
| --------------------------------- | -------------------------------------------- |
| **Installer too large**           | Remove unnecessary files from `build` config |
| **App won't start after install** | Check file paths use relative paths          |
| **Windows SmartScreen warning**   | Add code signing certificate                 |
| **Dependency missing**            | Ensure all files in `build.files` exist      |
| **Users can't uninstall**         | NSIS handles automatically                   |
| **Shortcuts don't appear**        | electron-builder creates them automatically  |
| **Icon wrong in installer**       | Verify icon path in package.json correct     |

---

## 📈 Monitoring After Release

Once deployed, track:

- **Download count**: How many people downloaded?
- **Crash reports**: Any errors from users?
- **Feature requests**: What do users want?
- **Bug reports**: Any issues found?
- **Usage**: How many active users?

---

## 🚀 Next Steps

### Immediate (This Week):

1. Test installer locally: `npm run dist`
2. Choose distribution method
3. Build and release v1.0.0

### Short Term (This Month):

1. Collect user feedback
2. Bug fixes → v1.0.1
3. Add requested features

### Long Term (Ongoing):

1. Set up auto-updates
2. Add code signing
3. Multi-platform support (macOS, Linux)

---

## 📚 Useful Resources

**electron-builder docs**: https://www.electron.build/
**GitHub Releases**: https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases
**Code Signing**: https://www.electron.build/code-signing
**Electron Security**: https://www.electronjs.org/docs/tutorial/security

---

## ✅ You Can Now:

- [ ] Build Windows installer for SecureWipe
- [ ] Distribute via GitHub, website, or direct share
- [ ] Create portable version
- [ ] Add auto-updates (advanced)
- [ ] Add code signing (advanced)
- [ ] Document system requirements
- [ ] Track usage and feedback

**Ready to deploy? Start with: `npm run dist`** 🎉

