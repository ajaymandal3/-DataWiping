/**
 * SecureWipe Download Website - Interactive JavaScript
 * Handles download functionality, form submission, and UI interactions
 */

// ============================================================================
// Download Functionality
// ============================================================================

/**
 * Triggers the app download
 * In production, this would download from a CDN or your server
 */
function downloadApp() {
    // Option 1: Direct download from GitHub Releases
    const githubUrl = "https://github.com/YourUsername/DataWiping/releases/download/v1.0.0/SecureWipe-Setup-1.0.0.exe";
    
    // Option 2: Download from your server
    // const downloadUrl = "/downloads/SecureWipe Setup 1.0.0.exe";
    
    // For demo, show instructions
    showDownloadInstructions(githubUrl);
}

/**
 * Show download instructions or initiate download
 */
function showDownloadInstructions(url) {
    const userAgent = navigator.userAgent.toLowerCase();
    
    // If it's a mobile device, show instructions instead
    if (/mobile|tablet|android|iphone|ipad/i.test(userAgent)) {
        alert("Download SecureWipe on a Windows computer from this page.");
        return;
    }
    
    // Attempt browser download
    const link = document.createElement('a');
    link.href = url;
    link.target = '_blank';
    link.click();
    
    // Show notification
    showNotification('Download started! If it didn\'t start, check your browser downloads folder.');
}

// ============================================================================
// Form Handling
// ============================================================================

/**
 * Handle contact form submission
 */
function handleFormSubmit(event) {
    event.preventDefault();
    
    const form = event.target;
    const name = form.elements[0].value;
    const email = form.elements[1].value;
    const message = form.elements[2].value;
    
    // Validate inputs
    if (!name.trim() || !email.trim() || !message.trim()) {
        showNotification('Please fill in all fields', 'error');
        return;
    }
    
    if (!isValidEmail(email)) {
        showNotification('Please enter a valid email address', 'error');
        return;
    }
    
    // In production, you would send this to a server
    // For now, show success message
    console.log('Form submission:', { name, email, message });
    
    // Show success notification
    showNotification(`Thanks for your message, ${name}! We'll get back to you soon.`, 'success');
    
    // Reset form
    form.reset();
}

/**
 * Validate email address format
 */
function isValidEmail(email) {
    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    return emailRegex.test(email);
}

// ============================================================================
// Notifications
// ============================================================================

/**
 * Show notification toast
 */
function showNotification(message, type = 'info') {
    // Create notification element
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 1rem 1.5rem;
        background-color: ${type === 'success' ? '#10b981' : type === 'error' ? '#ef4444' : '#06b6d4'};
        color: white;
        border-radius: 0.5rem;
        box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
        z-index: 10000;
        font-weight: 500;
        animation: slideIn 0.3s ease-out;
        max-width: 400px;
    `;
    
    // Add to page
    document.body.appendChild(notification);
    
    // Remove after 4 seconds
    setTimeout(() => {
        notification.style.animation = 'slideOut 0.3s ease-out';
        setTimeout(() => notification.remove(), 300);
    }, 4000);
}

// ============================================================================
// Scroll to Top Button
// ============================================================================

const scrollTopBtn = document.getElementById('scrollTopBtn');

/**
 * Show/hide scroll-to-top button based on scroll position
 */
function toggleScrollTopButton() {
    if (window.pageYOffset > 300) {
        scrollTopBtn.classList.add('show');
    } else {
        scrollTopBtn.classList.remove('show');
    }
}

/**
 * Scroll smoothly to top
 */
function scrollToTop() {
    window.scrollTo({
        top: 0,
        behavior: 'smooth'
    });
}

// Listen for scroll events
window.addEventListener('scroll', toggleScrollTopButton);

// ============================================================================
// Smooth Scroll for Navigation Links
// ============================================================================

document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function (e) {
        const href = this.getAttribute('href');
        
        // Skip if it's just '#'
        if (href === '#') return;
        
        e.preventDefault();
        
        const element = document.querySelector(href);
        if (element) {
            element.scrollIntoView({
                behavior: 'smooth',
                block: 'start'
            });
            
            // Close mobile menu if open
            const mobileMenu = document.querySelector('.mobile-menu');
            if (mobileMenu) {
                mobileMenu.classList.remove('open');
            }
        }
    });
});

// ============================================================================
// Analytics & Tracking
// ============================================================================

/**
 * Track download attempts (optional)
 */
function trackDownload() {
    if (typeof window.gtag !== 'undefined') {
        window.gtag('event', 'download', {
            'file_name': 'SecureWipe Setup 1.0.0.exe',
            'file_size': '75-100 MB'
        });
    }
}

/**
 * Track external link clicks
 */
document.querySelectorAll('a[target="_blank"]').forEach(link => {
    link.addEventListener('click', function() {
        if (typeof window.gtag !== 'undefined') {
            window.gtag('event', 'external_link', {
                'destination': this.href
            });
        }
    });
});

// ============================================================================
// Device Detection
// ============================================================================

/**
 * Detect if user is on Windows
 */
function isWindowsUser() {
    return /windows|win32/i.test(navigator.userAgent);
}

/**
 * Show platform-specific messages
 */
function initializePlatformDetection() {
    if (!isWindowsUser()) {
        const notification = document.createElement('div');
        notification.style.cssText = `
            background-color: #fbbf24;
            color: #000;
            padding: 1rem;
            text-align: center;
            border-bottom: 1px solid #f59e0b;
        `;
        notification.innerHTML = `
            <strong>Note:</strong> SecureWipe is currently available for Windows only. 
            Mac and Linux versions coming soon. <a href="https://github.com/YourUsername/DataWiping" target="_blank" style="color: #1f2937; text-decoration: underline;">View on GitHub</a>
        `;
        document.body.insertBefore(notification, document.body.firstChild);
    }
}

// ============================================================================
// Intersection Observer for Fade-in Animations
// ============================================================================

/**
 * Animate elements as they come into view
 */
function initializeIntersectionObserver() {
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('fade-in');
                observer.unobserve(entry.target);
            }
        });
    }, {
        threshold: 0.1
    });
    
    // Observe all cards and sections
    document.querySelectorAll('.card, .feature, .download-card, .screenshot, .faq-item').forEach(el => {
        observer.observe(el);
    });
}

// ============================================================================
// Keyboard Shortcuts
// ============================================================================

/**
 * Handle keyboard shortcuts
 */
document.addEventListener('keydown', (e) => {
    // ESC to scroll to top
    if (e.key === 'Escape') {
        scrollToTop();
    }
    
    // Alt+D to focus download section
    if (e.altKey && e.key === 'd') {
        document.getElementById('download').scrollIntoView({ behavior: 'smooth' });
    }
});

// ============================================================================
// Initialize on Page Load
// ============================================================================

document.addEventListener('DOMContentLoaded', () => {
    // Initialize all features
    initializePlatformDetection();
    initializeIntersectionObserver();
    
    // Add keyboard shortcuts hint
    console.log('%c🔐 SecureWipe', 'color: #06b6d4; font-size: 1.5em; font-weight: bold;');
    console.log('%cKeyboard Shortcuts:', 'color: #10b981; font-weight: bold;');
    console.log('%cESC = Scroll to top', 'color: #cbd5e1;');
    console.log('%cAlt+D = Go to downloads', 'color: #cbd5e1;');
});

// ============================================================================
// CSS Animations (injected into page)
// ============================================================================

// Create and inject animation keyframes
const style = document.createElement('style');
style.textContent = `
    @keyframes slideIn {
        from {
            transform: translateX(400px);
            opacity: 0;
        }
        to {
            transform: translateX(0);
            opacity: 1;
        }
    }
    
    @keyframes slideOut {
        from {
            transform: translateX(0);
            opacity: 1;
        }
        to {
            transform: translateX(400px);
            opacity: 0;
        }
    }
    
    @keyframes fadeIn {
        from {
            opacity: 0;
            transform: translateY(20px);
        }
        to {
            opacity: 1;
            transform: translateY(0);
        }
    }
`;
document.head.appendChild(style);

// ============================================================================
// Version Check (Optional - for checking for updates)
// ============================================================================

const APP_VERSION = '1.0.0';
const VERSION_CHECK_URL = 'https://api.github.com/repos/YourUsername/DataWiping/releases/latest';

/**
 * Check for new version (optional - can be called periodically)
 */
async function checkForUpdates() {
    try {
        const response = await fetch(VERSION_CHECK_URL);
        const data = await response.json();
        
        if (data.tag_name && data.tag_name.replace('v', '') !== APP_VERSION) {
            console.log(`New version available: ${data.tag_name}`);
            showUpdateNotification(data.tag_name);
        }
    } catch (error) {
        console.log('Could not check for updates:', error);
    }
}

/**
 * Show update notification
 */
function showUpdateNotification(newVersion) {
    const notification = document.createElement('div');
    notification.style.cssText = `
        position: fixed;
        bottom: 30px;
        left: 30px;
        background-color: #10b981;
        color: white;
        padding: 1rem 1.5rem;
        border-radius: 0.5rem;
        z-index: 10000;
        box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
    `;
    notification.innerHTML = `
        <strong>New Version Available!</strong><br>
        SecureWipe ${newVersion} is now available.
        <a href="https://github.com/YourUsername/DataWiping/releases" target="_blank" style="color: white; text-decoration: underline; margin-left: 0.5rem;">Download Now</a>
    `;
    document.body.appendChild(notification);
}

// Check for updates on page load (commented out to avoid API calls if offline)
// document.addEventListener('DOMContentLoaded', checkForUpdates);

// ============================================================================
// Mobile Menu Toggle (for future mobile support)
// ============================================================================

/**
 * Toggle mobile menu (placeholder for future enhancement)
 */
function toggleMobileMenu() {
    const menu = document.querySelector('.nav-links');
    if (menu) {
        menu.classList.toggle('open');
    }
}

// ============================================================================
// Export functions for global use
// ============================================================================

window.app = {
    downloadApp,
    handleFormSubmit,
    scrollToTop,
    checkForUpdates,
    toggleMobileMenu
};
