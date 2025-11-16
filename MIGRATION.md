# Migration Guide for Deprecated Device Users

**Effective Date**: November 15, 2025  
**Affected Platforms**: 11 devices (see below)  
**Reason**: Platform consolidation to focus on TrimUI Brick/Smart Pro

---

## ⚠️ Important Notice

As of **November 2025**, min-os/NextUI has transitioned to a **single-platform focus** supporting only:
- ✅ **TrimUI Brick**
- ✅ **TrimUI Smart Pro**

Support for the following devices has been **discontinued**:

### Discontinued Devices
- Miyoo Mini, Miyoo 282, Miyoo 355
- Anbernic RG35XX, RG35XX Plus
- Powkiddy RGB30, Zero 2.8
- TrimUI Smart (original)
- GKD Pixel
- Magic M17, Magic Mini

---

## Why This Change?

### Development Focus
- **50% codebase reduction** (160K → 80K LOC) allows faster development
- **Eliminates 33.71% code duplication** across platform-specific copies
- **Higher quality** for supported devices vs. spreading thin across 13+ platforms
- **Better testing** with focused hardware verification

### Strategic Alignment
- TrimUI Brick/Smart Pro represent the **future** of affordable handheld emulation
- Active hardware availability and manufacturer support
- Strong community adoption and ecosystem

---

## Migration Options

You have **three paths forward** depending on your needs:

---

### 🔵 Option 1: Stay on Last Compatible Release (Recommended for Most Users)

**Best for**: Users who want stability without configuration changes

#### Steps:
1. **Identify your last working version**
   - Visit: https://github.com/guiofsaints/min-os/releases
   - Look for releases **before November 15, 2025**
   - Check release notes for your device support

2. **Download and install**
   - Download the appropriate `.zip` for your device
   - Follow original installation instructions
   - **Do not update** to newer releases

3. **What you get**:
   - ✅ Stable, tested firmware
   - ✅ All features as of last release
   - ❌ No new features or bug fixes
   - ❌ No security updates

#### Example Devices and Last Versions:
| Device | Last Supported Release | Notes |
|--------|----------------------|-------|
| Miyoo Mini | TBD (check releases) | Look for "miyoomini" in release |
| RG35XX | TBD (check releases) | Look for "rg35xx" in release |
| RGB30 | TBD (check releases) | Look for "rgb30" in release |
| All others | TBD (check releases) | Check individual release notes |

---

### 🟢 Option 2: Switch to Upstream MinUI (Recommended for Active Support)

**Best for**: Users who want continued updates and official support

#### Why MinUI?
- ✅ **Active maintenance** for most archived platforms
- ✅ **Official support** and documentation
- ✅ **Bug fixes and updates** regularly released
- ✅ **Proven stability** across many devices
- ✅ **Similar interface** and philosophy to NextUI/min-os

#### Steps:
1. **Visit MinUI repository**
   - GitHub: https://github.com/shauninman/MinUI
   - Check device compatibility list

2. **Download MinUI for your device**
   - Follow installation guide: https://github.com/shauninman/MinUI#installation
   - Backup your current SD card before switching

3. **Migrate your content**
   - Copy ROM files to MinUI's expected directory structure
   - Copy BIOS files to appropriate locations
   - Saves may need conversion (check MinUI docs)

4. **Learn the differences**
   - MinUI has similar but not identical features
   - Check MinUI documentation for feature comparison

#### Supported Devices in MinUI:
- ✅ Miyoo Mini (and Mini Plus)
- ✅ Anbernic RG35XX (original and Plus)
- ✅ Powkiddy RGB30
- ✅ TrimUI Smart
- ✅ And many more

**MinUI Resources**:
- Documentation: https://github.com/shauninman/MinUI/wiki
- Discord: Check MinUI GitHub for community links
- Support: GitHub Issues on MinUI repository

---

### 🟡 Option 3: Fork and Self-Maintain (Advanced Users Only)

**Best for**: Developers or communities willing to maintain their own fork

#### What this involves:
- ✅ Full control over features and updates
- ✅ Can merge upstream improvements selectively
- ❌ Requires C programming knowledge
- ❌ Requires build toolchain setup
- ❌ Ongoing maintenance burden
- ❌ Testing responsibility falls on you

#### Steps:

##### 1. Fork the Archive Branch
```bash
# Clone the archive branch
git clone -b archive/unmaintained-platforms https://github.com/guiofsaints/min-os.git min-os-fork
cd min-os-fork

# Create your own fork on GitHub
# Then update remote:
git remote set-url origin https://github.com/YOUR_USERNAME/min-os-fork.git
git push -u origin archive/unmaintained-platforms
```

##### 2. Set Up Build Environment
```bash
# Install Docker (required for cross-compilation)
# macOS:
brew install docker

# Linux:
sudo apt install docker.io

# Pull platform toolchain (example for Miyoo Mini)
docker pull ghcr.io/loveretro/miyoomini-toolchain:latest
```

##### 3. Build for Your Platform
```bash
# Example: Build for Miyoo Mini
make PLATFORM=miyoomini

# This will create build artifacts in skeleton/
```

##### 4. Test Thoroughly
- Test on real hardware (emulators won't catch all issues)
- Verify all emulators work correctly
- Check save states, battery life, audio/video sync
- Document any issues or limitations

##### 5. Maintain Your Fork
- Fix bugs as they arise
- Consider merging upstream improvements from main branch
- Release updates for your community
- Provide user support

#### Resources for Fork Maintainers:
- **Archive Branch**: https://github.com/guiofsaints/min-os/tree/archive/unmaintained-platforms
- **Platform Code**: `workspace/_unmaintained/[your-platform]/`
- **Toolchains**: Check `.github/workflows/` for Docker image references
- **Build Docs**: `docs/BUILD.md` (may need updating for your platform)

---

## Frequently Asked Questions

### Q: Will my device still work with old NextUI releases?
**A**: Yes! Old releases remain available and functional. Your device won't stop working.

### Q: Can I request my device be re-supported?
**A**: Unfortunately, no. The decision to focus on TrimUI Brick/Smart Pro is strategic and permanent for this project.

### Q: What if I find a bug in my old version?
**A**: You have two options:
1. Switch to MinUI (which receives active updates)
2. Fork the code and fix it yourself

### Q: Will the archive branch be deleted?
**A**: No. The `archive/unmaintained-platforms` branch will remain accessible indefinitely in Git history.

### Q: Can I donate to get my device re-supported?
**A**: No. This is a resource allocation decision, not a funding issue. The core team is focusing on quality over quantity.

### Q: Is there a community fork for my device?
**A**: Check GitHub for community forks. Search for "min-os [your device]" or "NextUI [your device]" forks.

### Q: Will TrimUI Smart (original) ever be re-supported?
**A**: No. TrimUI Smart was superseded by Smart Pro. Consider upgrading hardware or using MinUI.

### Q: How do I export my saves before switching?
**A**: Saves are typically in `/Saves/[CONSOLE]/` on your SD card. Copy these files before formatting. MinUI uses similar paths, so they may work directly.

### Q: What about my game collections and artwork?
**A**: ROMs and artwork are platform-agnostic. Copy them to your new firmware's expected directories.

### Q: Can I mix and match (use NextUI on some devices, MinUI on others)?
**A**: Absolutely! Each device is independent. Use NextUI on TrimUI Brick and MinUI on other devices.

---

## Getting Help

### For NextUI on TrimUI Brick/Smart Pro:
- 📖 Documentation: https://nextui.loveretro.games/docs/
- 💬 Discord: https://discord.gg/HKd7wqZk3h
- 🐛 GitHub Issues: https://github.com/guiofsaints/min-os/issues

### For MinUI Migration:
- 📖 MinUI Docs: https://github.com/shauninman/MinUI/wiki
- 🐛 MinUI Issues: https://github.com/shauninman/MinUI/issues

### For Forking Support:
- 📖 Archive Branch: https://github.com/guiofsaints/min-os/tree/archive/unmaintained-platforms
- 💻 Platform Code: `workspace/_unmaintained/[platform]/`

---

## Timeline

| Date | Event |
|------|-------|
| **Nov 15, 2025** | Platform simplification completed |
| **Nov 15, 2025** | Archive branch created with historical code |
| **Nov 15, 2025** | This migration guide published |
| **Ongoing** | Main branch focuses on TrimUI Brick/Smart Pro only |

---

## Thank You

We appreciate the community support across all devices over the years. This consolidation allows us to deliver higher quality experiences for supported platforms rather than spread thin across many devices.

For those moving to MinUI, you're in good hands—it's excellent software with great support.

For those forking, good luck! The code is yours to carry forward.

For those staying on old releases, thank you for your understanding.

**The NextUI/min-os Team**  
November 15, 2025

---

**Links**:
- Main Repository: https://github.com/guiofsaints/min-os
- Archive Branch: https://github.com/guiofsaints/min-os/tree/archive/unmaintained-platforms
- Upstream MinUI: https://github.com/shauninman/MinUI
- Releases: https://github.com/guiofsaints/min-os/releases
