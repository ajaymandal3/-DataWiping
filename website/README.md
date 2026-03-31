# 🌐 SecureWipe Website - Setup & Deployment

Complete guide for the download website.

---

## 📁 Website Files

```
website/
├── index.html          Main website
├── style.css           Responsive dark theme
├── script.js           Download & form handling
├── README.md           This file
└── downloads/          (For direct downloads - optional)
```

---

## 🚀 Quick Start

### Test Locally

```bash
# Python 3
cd website
python -m http.server 8000

# If Python 2
python -m SimpleHTTPServer 8000

# Or use Node.js
npx http-server website/
```

Open: `http://localhost:8000`

### Deploy to GitHub Pages

```bash
# Push code to GitHub
git push origin main

# Website automatically updates to:
# https://YourUsername.github.io/DataWiping
```

---

## 🔧 Customization

### Step 1: Update GitHub URLs

**In `index.html` and `script.js`:**

Find and replace:

- `YourUsername` → Your actual GitHub username
- `support@example.com` → Your email

**Locations:**

- Line 31: GitHub releases link
- Line 93: GitHub repository link
- Line 268: Contact email

---

### Step 2: Update App Info

**In `index.html`:**

```html
<!-- Update version on line 26 -->
<p class="version-info">Version 1.0.0 • Windows 7+ (64-bit)</p>

<!-- Update release date on line 351 -->
<p><strong>Release Date:</strong> April 1, 2026</p>
```

---

### Step 3: Add Download Link

**In `script.js` (line ~13):**

```javascript
// Option 1: From GitHub Releases
const githubUrl =
  "https://github.com/YourUsername/DataWiping/releases/download/v1.0.0/SecureWipe-Setup-1.0.0.exe";

// Option 2: From your server
// const downloadUrl = "/downloads/SecureWipe Setup 1.0.0.exe";
```

---

### Step 4: Customize Branding

**Change app name:**

```html
<!-- In index.html, change all "SecureWipe" to your app name -->
<!-- In style.css, update colors if desired -->
```

**Change logo:**

```html
<!-- Line 18 in index.html -->
<span class="logo-icon">🔐</span> ← Change emoji
```

**Change colors:**

```css
/* In style.css, update --primary-color and --accent-color */
:root {
    --primary-color: #0f766e;      ← Teal
    --accent-color: #06b6d4;       ← Cyan
```

---

## 📧 Setup Contact Form (Optional)

The contact form is ready but needs a backend.

### Option 1: Formspree (Free)

```bash
# 1. Go to https://formspree.io
# 2. Sign up free
# 3. Get your endpoint: https://formspree.io/f/XXXXX
# 4. Update script.js:
```

```javascript
// Replace form.action
<form action="https://formspree.io/f/YOUR_FORM_ID" method="POST">
```

### Option 2: Netlify Forms (Free)

Already set up in form - just deploy to Netlify and it works!

### Option 3: Your Own Backend

```javascript
// In script.js, update handleFormSubmit:
fetch("/api/contact", {
  method: "POST",
  headers: { "Content-Type": "application/json" },
  body: JSON.stringify({ name, email, message }),
});
```

---

## 🎨 Design Features

### Dark Theme

- Professional dark background
- Cyan/teal accents
- Easy on the eyes

### Sections Included

- Navigation bar (sticky)
- Hero section with CTA
- App description (6 cards)
- Features (8 items)
- Wipe methods comparison table
- Screenshots (4 placeholders)
- Download options (3 variants)
- System requirements
- Installation steps
- FAQ (6 items)
- Footer
- Contact form

### Responsive Features

- Mobile-friendly layout
- Adjusts for tablets/phones
- Touch-friendly buttons
- Readable on all screens

---

## 📱 Mobile Optimization

Website is fully responsive on:

- ✅ Desktop (1200px+)
- ✅ Tablets (768px-1200px)
- ✅ Mobile (< 768px)

Test on mobile:

```bash
# In DevTools: Ctrl+Shift+M (toggle device toolbar)
# Or test on actual phone browsers
```

---

## 🔍 SEO Optimization

Already configured:

- ✅ Meta descriptions
- ✅ Open graph tags
- ✅ Semantic HTML
- ✅ Mobile viewport
- ✅ Structured data (ready)

**To improve further:**

```html
<!-- Add JSON-LD structured data in <head> -->
<script type="application/ld+json">
  {
    "@context": "https://schema.org",
    "@type": "SoftwareApplication",
    "name": "SecureWipe",
    "description": "Professional data destruction",
    "operatingSystem": "Windows",
    "applicationCategory": "UtilitiesApplication",
    "url": "https://yourdomain.com",
    "downloadUrl": "https://..."
  }
</script>
```

---

## 📊 Analytics (Optional)

Add Google Analytics:

```html
<!-- Add to index.html before closing </head> -->
<script async src="https://www.googletagmanager.com/gtag/js?id=GA_ID"></script>
<script>
  window.dataLayer = window.dataLayer || [];
  function gtag() {
    dataLayer.push(arguments);
  }
  gtag("js", new Date());
  gtag("config", "GA_ID");
</script>
```

Replace `GA_ID` with your Google Analytics ID.

---

## 🔐 Security Considerations

Website is static HTML/CSS/JS - very secure!

**Best practices already included:**

- ✅ No sensitive data stored
- ✅ No external tracking
- ✅ HTTPS automatically on GitHub Pages
- ✅ No database connections
- ✅ CSP-friendly HTML

**Additional options:**

- Add HTTPS certificate (GitHub Pages does this automatically)
- Regular security audits
- Keep dependencies updated

---

## 📥 Add Direct Download Option

To host the installer on your website:

1. Create folder: `website/downloads/`
2. Copy: `dist/SecureWipe Setup 1.0.0.exe` → `website/downloads/`
3. Update `script.js`:

```javascript
// Change from GitHub to local
const downloadUrl = "downloads/SecureWipe Setup 1.0.0.exe";
```

4. Push to GitHub:

```bash
git add website/downloads/
git commit -m "Add direct download"
git push origin main
```

**Warning:** GitHub Pages has 100GB/month bandwidth. For high traffic, use CDN.

---

## 🌐 Custom Domain

To use your own domain:

### Step 1: Buy Domain

- GoDaddy, Namecheap, Google Domains, etc.

### Step 2: Configure GitHub Pages

1. GitHub.com → Settings → Pages
2. Enter custom domain
3. GitHub creates CNAME file

### Step 3: Update DNS

In your domain provider:

```
CNAME record:
Name: www
Value: YourUsername.github.io

Or:

A records (for apex domain):
185.199.108.153
185.199.109.153
185.199.110.153
185.199.111.153
```

### Step 4: Verify

Wait 10-30 minutes, then test: `https://yourdomain.com`

---

## 🚀 Deployment Platforms

### GitHub Pages (Recommended for beginners)

- ✅ Free
- ✅ Automatic updates
- ✅ HTTPS included
- ✅ No setup needed
- [How to deploy](#deploy-to-github-pages)

### Netlify (More features)

- ✅ Free tier available
- ✅ Contact form built-in
- ✅ Better analytics
- Process:
  1. Go to netlify.com
  2. Connect GitHub repo
  3. Deploy automatically

### Vercel (For Server-Side Code)

- ✅ Free tier
- ✅ API support
- ✅ Auto-scaling
- Process:
  1. Go to vercel.com
  2. Import GitHub repo
  3. Deploy

### AWS S3 + CloudFront

- ✅ Scalable
- ✅ CDN included
- Cost: ~$1/month for small sites
- More complex setup

### Traditional Hosting (GoDaddy, Bluehost, etc.)

- ✅ Full control
- Cost: $5-15/month
- Upload via FTP

---

## 📊 Performance Optimization

Website is already optimized:

- ✅ Minified CSS/JS
- ✅ Efficient images
- ✅ Lazy loading
- ✅ Mobile-first design

**Further optimizations:**

```bash
# Compress images
npx imagemin website/images/* --out-dir=website/images

# Minify HTML
npx html-minifier index.html --output index.min.html

# Create sitemap.xml
```

---

## 🧪 Testing Checklist

Before deploying:

- [ ] Links work correctly
- [ ] Download buttons function
- [ ] Contact form submits
- [ ] Mobile responsive
- [ ] All images load
- [ ] No console errors (Ctrl+Shift+J)
- [ ] Fast loading (<3 seconds)
- [ ] All features work
- [ ] CSS applied correctly
- [ ] Forms validate input

---

## 🔄 Update Website

### Automated Updates

```bash
# Edit index.html / style.css / script.js
git add website/
git commit -m "Update website: [description]"
git push origin main
# Changes appear in 1-2 minutes!
```

### Manual Rebuild (if needed)

```bash
cd website
# Test locally
python -m http.server 8000
# Then push to GitHub
```

---

## 📝 Version Management

Keep website and app in sync:

```
Version 1.0.0:
- Website shows: v1.0.0
- App shows: v1.0.0
- Release notes: v1.0.0

When updating to 1.0.1:
- Update website version
- Update app version
- Update release notes
- Push both to GitHub
```

---

## 🆘 Troubleshooting

### Website doesn't update after push

- Clear browser cache (Ctrl+Shift+Delete)
- Wait 1-2 minutes (propagation delay)
- Check GitHub Pages settings

### Links broken

- Verify GitHub username is correct
- Check relative paths don't have leading /
- Test on live site vs localhost

### Styling not applied

- Hard refresh browser (Ctrl+Shift+R)
- Check CSS file is being served (Network tab)
- Verify style.css path is correct

### Download link returns 404

- Verify file is uploaded to GitHub Releases
- Check GitHub URL is correct
- Try direct GitHub Releases link

### Contact form doesn't work

- Verify backend is configured
- Check console for errors
- Test with example data

### Mobile layout broken

- Test on actual mobile device
- Check viewport meta tag exists
- Verify media queries in CSS

---

## 📚 Resources

### Website Tools

- [Figma](https://figma.com) - Design mockups
- [ColorHunt](https://colorhunt.co) - Color schemes
- [Unsplash](https://unsplash.com) - Free images
- [Canva](https://canva.com) - Graphic design

### Validation Tools

- [W3C HTML Validator](https://validator.w3.org)
- [CSS Validator](https://jigsaw.w3.org/css-validator)
- [Mobile Test](https://search.google.com/test/mobile-friendly)

### Performance Tools

- [PageSpeed Insights](https://pagespeed.web.dev)
- [GT Metrix](https://gtmetrix.com)
- [WebPageTest](https://webpagetest.org)

---

## 📋 Checklist for Production

Before going live:

- [ ] All URLs updated to your account
- [ ] Contact form configured
- [ ] Custom domain set up (optional)
- [ ] Analytics installed (optional)
- [ ] SEO optimized
- [ ] Mobile tested
- [ ] All features working
- [ ] Performance optimized
- [ ] Security reviewed
- [ ] Backup created
- [ ] Deployment documented
- [ ] Team trained

---

## 🎉 Success!

Your professional download website is ready!

**Next:**

1. ✅ Customize with your info
2. ✅ Test locally
3. ✅ Deploy to GitHub Pages
4. ✅ Share with users
5. ✅ Monitor analytics
6. ✅ Update regularly

---

**Questions?** See main guides:

- `05_DEPLOYMENT.md` - Full deployment guide
- `10_BUILD_COMMANDS.md` - All commands
- `09_WEBSITE_DEPLOYMENT.md` - Detailed instructions

