# 🚀 Complete Deployment Guide: From Code to Production

Step-by-step instructions to build your Windows installer and deploy your website.

---

## 📋 Pre-Deployment Checklist

Before you start, verify:

- [ ] Node.js installed (`node -v`)
- [ ] Package.json has correct version
- [ ] Icon file exists: `src/electron/assets/icon.ico`
- [ ] All code is committed to Git
- [ ] README updated with latest info
- [ ] No build errors when running `npm start`

---

## PART 1: Build Windows Installer

### Step 1: Compile Native C Code

This compiles the C code that does low-level disk operations:

```bash
npm run build:addon
```

**Expected output:**

```
gyp info ok
```

**If it fails:**

- Make sure you have Visual Studio Build Tools installed
- Or: Download from https://visualstudio.microsoft.com/downloads/
- Select "Desktop development with C++"

### Step 2: Create Windows NSIS Installer

This builds the .exe installer:

```bash
npm run dist
```

**Expected output:**

```
electron-builder version: 26.0.12
  • electron-builder-notarize not found, cannot code sign macOS application
  • building: dir: build\SecureWipe 1.0.0
    ✔ done
  • building NSIS installer: SecureWipe Setup 1.0.0.exe
    ✔ done
```

**Output files:**

- `dist/SecureWipe Setup 1.0.0.exe` (~75-100 MB)
- `dist/builder-effective-config.yaml`
- `dist/latest.yml` (for auto-updates)

### Step 3: Test Installer Locally

```bash
# Navigate to dist folder and run the installer
cd dist
"SecureWipe Setup 1.0.0.exe"
```

**Test checklist:**

- [ ] Installer wizard opens
- [ ] Installation completes without errors
- [ ] App appears in Start Menu
- [ ] Desktop shortcut created
- [ ] App launches successfully
- [ ] All features work correctly

---

## PART 2: Project Structure After Build

```
DataWiping/
├── src/
│   ├── electron/
│   │   ├── main.js
│   │   ├── preload.js
│   │   └── assets/
│   │       ├── icon.ico
│   │       └── icon.png
│   └── renderer/
│       ├── index.html
│       ├── scripts/app.js
│       └── styles/main.css
├── native/
│   ├── addon.cpp
│   └── securewipe_core.c
├── website/              ← NEW: Download website
│   ├── index.html
│   ├── style.css
│   ├── script.js
│   └── README.md
├── build/
│   └── Release/
│       └── securewipe_native.node
├── dist/                 ← NEW: Installer output
│   ├── SecureWipe Setup 1.0.0.exe
│   └── latest.yml
├── package.json
├── main.c
└── binding.gyp
```

---

## PART 3: Create Download Website

### What You Have

Already created:

- ✅ `website/index.html` - Full website with all sections
- ✅ `website/style.css` - Modern dark theme design
- ✅ `website/script.js` - Download and form handling

### Update Configuration Files

**1. In `website/index.html`, replace:**

```html
<!-- Line 31: Update GitHub links -->
<a href="https://github.com/YourUsername/DataWiping/releases">View on GitHub</a>

<!-- Line 93 & other places: Replace YourUsername with your actual GitHub username -->
https://github.com/YourUsername/DataWiping
```

**2. In `website/script.js`, update:**

```javascript
// Line 13: Set your GitHub release download URL
const githubUrl = "https://github.com/YourUsername/DataWiping/releases/download/v1.0.0/SecureWipe-Setup-1.0.0.exe";

// Line 23: Update email if using contact form
link to your backend service
```

### Test Website Locally

```bash
# On Windows, use Python or any local server
python -m http.server 8000

# Or use Node.js
npx http-server website/
```

Open: `http://localhost:8000`

**Test:**

- [ ] Page loads without errors
- [ ] Responsive on mobile
- [ ] Download button works
- [ ] Links navigate correctly
- [ ] Dark theme displays properly

---

## PART 4: Deploy Website to GitHub Pages

### Option A: GitHub Pages (Recommended for beginners)

GitHub Pages hosts your website for FREE directly from your repository.

#### Step 1: Initialize Git Repository

```bash
git init
git add .
git commit -m "Initial commit: SecureWipe with website"
```

#### Step 2: Create GitHub Repository

1. Go to https://github.com/new
2. Repository name: `DataWiping` (or any name)
3. Description: "Secure storage device wiper"
4. Choose Public (for users to download)
5. Click "Create repository"
6. Copy the repository URL

#### Step 3: Push to GitHub

```bash
# Add remote repository
git remote add origin https://github.com/YourUsername/DataWiping.git

# Push code to GitHub
git branch -M main
git push -u origin main
```

#### Step 4: Enable GitHub Pages

1. Go to your repository on GitHub.com
2. Click **Settings** → **Pages**
3. Source: Select **main** branch
4. Folder: Select **/ (root)**
5. Click **Save**
6. Wait 1-2 minutes
7. Your site is live at: `https://YourUsername.github.io/DataWiping`

### Option B: Custom Domain

After enabling GitHub Pages, add your custom domain:

1. Go to **Settings** → **Pages**
2. Custom domain: `www.securewipe.com` (or your domain)
3. Configure DNS records with your domain provider

---

## PART 5: Upload Installer to GitHub Releases

### Step 1: Create Release on GitHub

1. Go to your repository
2. Click **Releases** (right side)
3. Click **Draft a new release**
4. Tag version: `v1.0.0`
5. Release title: `SecureWipe 1.0.0`
6. Description:

```markdown
## SecureWipe 1.0.0

### Features

- 9 different wipe methods
- Real-time progress tracking
- Military-grade security (DoD 5220.22-M, Gutmann, etc.)
- Audit logging for compliance
- Dark theme UI

### System Requirements

- Windows 7+ (64-bit)
- 100 MB free disk space
- Administrator rights

### Installation

1. Download SecureWipe Setup 1.0.0.exe
2. Run installer
3. Follow wizard
4. Launch from Start Menu

### Download

[SecureWipe Setup 1.0.0.exe](https://github.com/YourUsername/DataWiping/releases/download/v1.0.0/SecureWipe-Setup-1.0.0.exe)
```

### Step 2: Upload Installer

1. Scroll down to **Attach binaries**
2. Click **Attach binaries by dropping them here**
3. Select: `dist/SecureWipe Setup 1.0.0.exe`
4. Wait for upload to complete
5. Click **Publish release**

### Step 3: Verify

- Go to Releases page
- Download link should be available
- Test the download link

---

## PART 6: Auto-Updates Setup (Optional)

Allow users to get updates automatically.

### Step 1: Install electron-updater

```bash
npm install electron-updater
```

### Step 2: Add to main.js

Add this code to `src/electron/main.js`:

```javascript
const { autoUpdater } = require("electron-updater");

// Check for updates on app startup
app.on("ready", () => {
  autoUpdater.checkForUpdatesAndNotify();
});

// Handle update events
autoUpdater.on("update-available", (info) => {
  console.log("Update available:", info.version);
  // You can show a dialog here
});

autoUpdater.on("update-downloaded", (info) => {
  console.log("Update downloaded:", info.version);
  // Auto-restart when user close app
  autoUpdater.quitAndInstall();
});
```

### Step 3: Configure Auto-Update URL

In package.json, add:

```json
"publish": {
  "provider": "github",
  "owner": "YourUsername",
  "repo": "DataWiping"
}
```

### Step 4: Build and Release

When you release a new version:

```bash
# Update version in package.json (e.g., 1.0.1)
npm run build:addon
npm run dist

# Upload to GitHub Releases
# Users will be auto-notified and updated
```

---

## PART 7: Portable Version (Optional)

Create a portable .EXE that doesn't require installation.

### Step 1: Update package.json

Change this section:

```json
"win": {
  "icon": "src/electron/assets/icon.ico",
  "target": [
    "nsis",
    "portable"
  ]
}
```

### Step 2: Build

```bash
npm run dist
```

**Output:**

- `dist/SecureWipe Setup 1.0.0.exe` (installer)
- `dist/SecureWipe 1.0.0.exe` (portable)

### Step 3: Upload Both

- Upload both files to GitHub Releases
- Users can choose: Installer or Portable

---

## PART 8: Code Signing (Professional, Optional)

Remove the Windows SmartScreen warning with code signing.

### Why Code Sign?

- Removes "Unknown Publisher" warning
- Users trust your app
- Professional appearance
- Required for Windows Store

### How to Code Sign

1. Buy certificate from:
   - Sectigo (formerly Comodo)
   - DigiCert
   - GlobalSign
   - Cost: $100-300/year

2. In package.json, add:

```json
"certificateFile": "path/to/certificate.pfx",
"certificatePassword": "your-password-here",
"signingHashAlgorithms": ["sha256"]
```

3. Rebuild:

```bash
npm run dist
```

**Note:** For initial release, code signing is optional. You can add it later.

---

## PART 9: Complete Command Reference

### Build

```bash
# Compile C code
npm run build:addon

# Build Windows installer
npm run dist

# Build without creating installer (faster for testing)
npm run dist:dir

# Build both installer and portable
npm run dist:portable
```

### Development

```bash
# Start app in development mode
npm start

# Start with DevTools open
npm start -- --inspect
```

### Deployment

```bash
# Test website locally
npx http-server website/

# Deploy to GitHub
git push origin main

# Create new release (do this on GitHub.com)
# Then upload .exe file to releases
```

### Monitoring

```bash
# Check what's in the installer
unzip dist/SecureWipe\ Setup\ 1.0.0.exe

# Verify code signature (if signed)
signtool verify /pa dist/SecureWipe\ Setup\ 1.0.0.exe
```

---

## PART 10: Distribution Comparison Matrix

Choose the best method for your needs:

| Method             | Setup Time | Cost       | Users      | Best For      |
| ------------------ | ---------- | ---------- | ---------- | ------------- |
| GitHub Releases    | 15 min     | Free       | Tech-savvy | Open source   |
| GitHub Pages       | 10 min     | Free       | Everyone   | Official site |
| Personal Website   | 1-2 hours  | $10-100/mo | Everyone   | Professional  |
| Direct Cloud Share | 5 min      | Free       | Dozens     | Quick testing |
| Windows Store      | 1-2 days   | $20        | Millions   | Maximum reach |

---

## PART 11: Monitoring After Release

### What to Track

1. **Downloads**
   - Check GitHub releases page: "Download count"

2. **Active Users**
   - Add analytics to website (Google Analytics, Plausible)
   - Monitor GitHub issues for bugs

3. **User Feedback**
   - Monitor GitHub Issues
   - Save email feedback
   - Track common questions

4. **Crash Reports** (Optional)
   - Implement crash reporting (Sentry, Bugsnag)
   - Add in main.js

```javascript
const Sentry = require("@sentry/electron");
Sentry.init({ dsn: "your-sentry-dsn" });
```

### Common Monitoring Metrics

```
Total Downloads:     [Check GitHub Releases]
Active Users:        [Check website analytics]
Support Issues:      [Check GitHub Issues]
Most Downloaded:     [Installer vs Portable]
User Satisfaction:   [Issue/feature requests]
App Crashes:         [Crash reports if enabled]
```

---

## PART 12: Release Checklist

Before each release:

- [ ] Version updated in package.json
- [ ] Changelog created
- [ ] All tests pass: `npm start` (manual testing)
- [ ] No console errors
- [ ] Installer built successfully
- [ ] Installer tested locally
- [ ] Website updated with new version
- [ ] GitHub release created
- [ ] Release notes written
- [ ] .exe file uploaded to GitHub
- [ ] Website deployed
- [ ] Release announced (email, social, etc.)

---

## PART 13: Troubleshooting

### Installer Won't Build

**Error:** `electron-builder not found`

```bash
npm install
npm run dist
```

### Icon Not Showing

- Verify file exists: `src/electron/assets/icon.ico`
- Should be 256×256 pixels
- Format: ICO

### Website Not Deploying

- Verify GitHub Pages is enabled
- Check branch is set to `main`
- Clear browser cache (Ctrl+Shift+Delete)

### Users Can't Download

- Verify GitHub Releases page is public
- Check file is actually uploaded
- Test download link in new browser

### App Won't Install

- Check Windows Defender (might block unsigned app)
- Try running as Administrator
- Verify system requirements (Windows 7+, 64-bit)

### Auto-Updates Not Working

- Verify electron-updater is installed
- Check GitHub release has latest.yml file
- Verify version number is higher in package.json

---

## PART 14: Next Steps Roadmap

### Immediate (Day 1-7)

1. ✅ Build installer
2. ✅ Deploy website
3. ✅ Create GitHub releases
4. ✅ Test everything works

### Short Term (Week 1-4)

1. Announce release to users
2. Collect feedback from first users
3. Fix critical bugs → release v1.0.1
4. Monitor GitHub Issues

### Long Term (Month 1+)

1. Add more features → v1.1.0
2. Set up auto-updates
3. Add code signing
4. Consider Windows Store
5. Plan Mac/Linux versions

---

## PART 15: Version Numbering Guide

Use Semantic Versioning: `MAJOR.MINOR.PATCH`

```
1.0.0
├─ MAJOR: Big changes, breaking changes
├─ MINOR: New features, backwards compatible
└─ PATCH: Bug fixes, security updates
```

### Examples

```
1.0.0 → Initial release
1.0.1 → Bug fix
1.1.0 → New features
2.0.0 → Major overhaul
```

### Release Notes Template

```markdown
# SecureWipe 1.0.1 - Bug Fixes

## What's New

- Fixed crash when selecting certain drives
- Improved performance on large drives
- Updated documentation

## Download

[SecureWipe Setup 1.0.1.exe](link)

## Installation

Run installer, follow wizard

## Known Issues

None

## Thank You

Special thanks to all users who reported bugs!
```

---

## 🎉 You're Done!

Your app is now:

- ✅ Built as Windows installer
- ✅ Hosted on GitHub Pages
- ✅ Available for users to download
- ✅ Ready for auto-updates
- ✅ Professional presentation

**Next:** Monitor feedback, release updates, grow user base!

---

## 📚 Additional Resources

- [Electron Builder Docs](https://www.electron.build/)
- [GitHub Pages Guide](https://pages.github.com/)
- [Semantic Versioning](https://semver.org/)
- [Release Notes Best Practices](https://keepachangelog.com/)

---

**Questions?** Check `07_DEPLOYMENT_CHEATSHEET.md` for quick reference!

