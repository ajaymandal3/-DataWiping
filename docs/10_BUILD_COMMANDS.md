# 📦 Complete Build & Deploy Commands Reference

All commands needed to build your Windows installer and deploy your website.

---

## 🎯 Quick Start (3-Step Version)

```bash
# Step 1: Compile native C code
npm run build:addon

# Step 2: Create Windows installer
npm run dist

# Step 3: Deploy to GitHub
git push origin main
```

**Result:**

- Installer: `dist/SecureWipe Setup 1.0.0.exe`
- Website: `https://YourUsername.github.io/DataWiping`

---

## 📋 Complete Command List

### 1️⃣ Development Commands

#### Start App in Dev Mode

```bash
npm start
```

- Launches app with Electron
- Opens DevTools automatically
- Hot reload on code changes

#### Start with Debugging

```bash
npm start -- --inspect
```

- Enable Node.js debugger
- Useful for debugging native code

---

### 2️⃣ Build Commands

#### Compile Native C Code

```bash
npm run build:addon
```

**What it does:**

- Runs node-gyp
- Compiles C code → native binary
- Output: `build/Release/securewipe_native.node`

**When to use:**

- When you modify C code
- First time building
- After pulling new code

**Expected output:**

```
gyp info ok
```

**If it fails:**

```bash
# Try force rebuild
npm run build:addon -- --force

# Or manual tool installation
npm install node-gyp --save-dev
npx node-gyp rebuild
```

---

#### Create Windows Installer (NSIS)

```bash
npm run dist
```

**What it does:**

- Bundles electron app
- Runs electron-builder
- Creates NSIS installer
- Outputs: `dist/SecureWipe Setup 1.0.0.exe` (~75 MB)

**When to use:**

- Before releasing to users
- Testing installer workflow

**Expected output:**

```
✔ building NSIS installer
✔ SecureWipe Setup 1.0.0.exe
```

---

#### Create Portable Version (No Install)

```bash
npm run dist:portable
```

**What it does:**

- Creates both installer AND portable .exe
- Users can run portable without installing

**Outputs:**

- `dist/SecureWipe Setup 1.0.0.exe` (Installer)
- `dist/SecureWipe 1.0.0.exe` (Portable)

**When to use:**

- When you want both options

---

#### Build Without Installer (Testing)

```bash
npm run dist:dir
```

**What it does:**

- Bundles app without creating installer
- Faster for testing packaging

**When to use:**

- Testing app before building full installer
- Debugging build issues

---

### 3️⃣ Git & GitHub Commands

#### Initialize Git (First Time Only)

```bash
git init
git config user.name "Your Name"
git config user.email "you@example.com"
```

#### Stage All Changes

```bash
git add .
```

#### Commit Changes

```bash
git commit -m "Release v1.0.0: Initial public release"
```

#### Add GitHub Repository

```bash
git remote add origin https://github.com/YourUsername/DataWiping.git
```

#### Push Code to GitHub

```bash
git branch -M main
git push -u origin main
```

**For future pushes:**

```bash
git push origin main
```

#### View Git Status

```bash
git status
```

#### View Commit History

```bash
git log --oneline
```

---

### 4️⃣ GitHub Releases Commands

#### Create Git Tag (For Release)

```bash
# Create local tag
git tag v1.0.0
git push origin v1.0.0
```

**Then on GitHub UI:**

1. Go to Releases
2. Click "Draft new release"
3. Select tag: `v1.0.0`
4. Upload `dist/SecureWipe Setup 1.0.0.exe`
5. Publish

---

### 5️⃣ Website Commands

#### Test Website Locally (Python)

```bash
cd website
python -m http.server 8000
```

Open: `http://localhost:8000`

#### Test Website Locally (Node.js)

```bash
cd website
npx http-server
```

Open: `http://localhost:8000`

#### Test Website Locally (Python 2)

```bash
cd website
python -m SimpleHTTPServer 8000
```

#### Deploy Website to GitHub Pages

**Automatic (via git):**

```bash
git push origin main
# Website updates automatically to:
# https://YourUsername.github.io/DataWiping
```

**Manual (if not using GitHub Pages):**

- Upload `website/` to your server via FTP/SSH

---

### 6️⃣ Dependency Commands

#### Install Dependencies

```bash
npm install
```

**What it does:**

- Installs all packages from package.json
- Creates node_modules folder
- Do this first after cloning

#### Check Installed Packages

```bash
npm list
```

#### Update Dependencies

```bash
npm update
```

#### Install Specific Package

```bash
npm install electron-updater
```

---

### 7️⃣ Cleanup Commands

#### Remove Build Artifacts

```bash
# Remove build folder
rmdir /s /q build

# Remove dist folder
rmdir /s /q dist

# Remove node_modules
rmdir /s /q node_modules

# Remove package-lock.json
del package-lock.json
```

#### Fresh Install (After Cleanup)

```bash
npm install
npm run build:addon
npm run dist
```

---

## 📊 Complete Release Workflow

```bash
# 1. Make your changes
# ... edit code ...

# 2. Build addon if you modified C code
npm run build:addon

# 3. Test locally
npm start
# ... verify it works ...

# 4. Update version in package.json
# Change: "version": "1.0.0" → "version": "1.0.1"

# 5. Commit your changes
git add .
git commit -m "Release v1.0.1: Bug fixes and improvements"

# 6. Create git tag
git tag v1.0.1

# 7. Push to GitHub
git push origin main
git push origin v1.0.1

# 8. Build installer
npm run dist

# 9. Go to GitHub.com → Releases
# 10. Create new release from tag v1.0.1
# 11. Upload dist/SecureWipe\ Setup\ 1.0.1.exe
# 12. Publish release

# 13. Website updates automatically!
```

---

## 🔍 Verification Commands

#### Check Node Version

```bash
node -v
```

(Should be v14 or higher)

#### Check npm Version

```bash
npm -v
```

(Should be v6 or higher)

#### List All npm Scripts

```bash
npm run
```

#### Verify App Starts

```bash
npm start
```

#### Check Network/Download

```bash
# Test website download speed
curl -o /dev/null -s -w "%{time_total}\n" https://YourUsername.github.io/DataWiping

# Check file size
dir dist\*.exe
```

---

## ⚙️ Advanced Commands

#### Build with Specific Architecture

```bash
# 64-bit only (default)
npm run dist -- --x64

# Both 32-bit and 64-bit
npm run dist -- --ia32 --x64
```

#### Build with Code Signing

```bash
# After adding certificate info to package.json
npm run dist -- --cscLink="path/to/cert.pfx" --cscKeyPassword="password"
```

#### Build Portable Only

```bash
electron-builder --win portable
```

#### Debug Build Process

```bash
npm run dist -- --publish never --debug
```

---

## 📝 Batch Build Script

Create `build.bat` (Windows):

```batch
@echo off
echo === SecureWipe Build Script ===
echo.
echo [1/4] Building addon...
call npm run build:addon
if errorlevel 1 goto error

echo.
echo [2/4] Testing app...
timeout /t 5 /nobreak
taskkill /im electron.exe /f 2>nul
goto release_build

:release_build
echo.
echo [3/4] Creating installer...
call npm run dist
if errorlevel 1 goto error

echo.
echo [4/4] Build complete!
echo.
echo Installer location: dist\SecureWipe Setup 1.0.0.exe
echo Size:
dir dist\*.exe
echo.
echo Next steps:
echo - Test installer locally
echo - Upload to GitHub Releases
echo - git push origin main
echo.
pause
exit /b 0

:error
echo.
echo ERROR: Build failed!
pause
exit /b 1
```

Run it:

```bash
.\build.bat
```

---

## 🐛 Troubleshooting Commands

#### Check What Files Would Be Packaged

```bash
npm run dist:dir
dir build\SecureWipe-*
```

#### Verify Icon Exists

```bash
dir src\electron\assets\icon.ico
```

#### Check Node-gyp Version

```bash
npx node-gyp -v
```

#### Rebuild Node Modules

```bash
npm ci --prefer-offline --no-audit
```

#### Clear npm Cache

```bash
npm cache clean --force
```

#### Check Git Configuration

```bash
git config --list
```

---

## 📊 Command Frequency Chart

| Command               | Frequency   | When               |
| --------------------- | ----------- | ------------------ |
| `npm start`           | Daily       | Testing changes    |
| `npm run build:addon` | Weekly      | Modify C code      |
| `npm run dist`        | Per Release | Build installer    |
| `git add .`           | Daily       | Stage changes      |
| `git commit -m`       | Daily       | Commit changes     |
| `git push`            | Daily       | Upload to GitHub   |
| `git tag`             | Per Release | Create release tag |

---

## 🎯 Common Command Combinations

### Daily Development

```bash
npm start
# [Test/Edit code]
# [Save files]
git add .
git commit -m "Updates: [description]"
git push origin main
```

### Prepare for Release

```bash
# Update version in package.json
npm run build:addon
npm start
# [Test thoroughly]
git add .
git commit -m "Release v1.0.1"
git tag v1.0.1
git push origin main --follow-tags
npm run dist
```

### Deploy Website

```bash
# Edit website/ files
cd website
npx http-server
# [Test locally]
cd ..
git add website/
git commit -m "Update website"
git push origin main
# Website updates automatically!
```

### Clean and Start Fresh

```bash
rmdir /s /q build node_modules dist
del package-lock.json
npm install
npm run build:addon
npm start
```

---

## 📚 Reference

**Full documentation:**

- `05_DEPLOYMENT.md` - Complete deployment guide
- `09_WEBSITE_DEPLOYMENT.md` - Website-specific instructions
- `07_DEPLOYMENT_CHEATSHEET.md` - Quick reference
- `00_DOCUMENTATION_INDEX.md` - Navigation hub

**Official docs:**

- [Electron Builder](https://www.electron.build)
- [Git Commands](https://git-scm.com/docs)
- [npm Scripts](https://docs.npmjs.com/cli/run-script)

---

## ✅ Success Checklist

By the end of following all these commands, you should have:

- [ ] Local development working (`npm start`)
- [ ] Native code compiling (`npm run build:addon`)
- [ ] Windows installer building (`npm run dist`)
- [ ] Git repository initialized (`git init`)
- [ ] Code pushed to GitHub (`git push`)
- [ ] Releases published (`git tag`)
- [ ] Website deployed (`github.io`)
- [ ] Download link working
- [ ] Auto-updates ready (optional)

🎉 **You're ready to distribute your app!**

---

**Need help?** Check the specific guide:

- Building: `05_DEPLOYMENT.md`
- Website: `09_WEBSITE_DEPLOYMENT.md`
- Quick ref: `07_DEPLOYMENT_CHEATSHEET.md`

