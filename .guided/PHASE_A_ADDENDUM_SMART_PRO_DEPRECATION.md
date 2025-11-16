# Phase A Addendum: TrimUI Smart Pro Deprecation

**Date**: November 15, 2025  
**Type**: Strategic Decision  
**Impact**: Breaking Change for Smart Pro Users  
**Status**: Implemented

---

## Decision Summary

After completing Phase A platform simplification, the project has made an additional strategic decision to **deprecate TrimUI Smart Pro support**, making min-os/NextUI a **TrimUI Brick exclusive** custom firmware.

---

## Rationale

### Technical Considerations

1. **Hardware Similarity**
   - Smart Pro and Brick share the same tg5040 platform base
   - Both use ARM Cortex-A53 processors
   - Similar hardware capabilities and limitations

2. **Development Focus**
   - Supporting both devices splits testing effort
   - Edge case differences require conditional code
   - Single device allows optimal tuning for specific hardware

3. **Resource Allocation**
   - Limited development resources
   - Better to excel at one device than be mediocre at two
   - Allows deeper optimization for Brick-specific features

### Strategic Alignment

1. **Market Position**
   - TrimUI Brick is the more popular device
   - Larger user base provides better feedback
   - Community momentum is stronger for Brick

2. **Optimization Opportunities**
   - Phase C performance work can be Brick-specific
   - No need to maintain compatibility across devices
   - Can leverage Brick's exact specifications

3. **Simplification Philosophy**
   - Phase A established single-platform focus
   - This completes the vision: one device, one purpose
   - Reduces "which device should this optimize for?" questions

---

## Impact Analysis

### Before This Change

**Supported Devices**: 2
- TrimUI Brick (tg5040)
- TrimUI Smart Pro (tg5040)

**Platform Count**: 2 (but shared platform ID caused confusion)

**Testing Burden**: 2 devices to verify

### After This Change

**Supported Device**: 1
- TrimUI Brick (tg5040) **ONLY**

**Platform Count**: 1 (clear, no ambiguity)

**Testing Burden**: 1 device (50% reduction)

---

## Affected Platforms Count Update

### Original Phase A
- **Archived**: 11 platforms (miyoomini, rg35xx, rgb30, etc.)
- **Active**: 2 devices (Brick + Smart Pro)

### Updated Phase A (with this addendum)
- **Archived**: 12 platforms (including Smart Pro)
- **Active**: 1 device (Brick only)

---

## Migration Path for Smart Pro Users

TrimUI Smart Pro users have **three options**:

### Option 1: Switch to MinUI (Recommended)
- **MinUI actively supports Smart Pro**
- Similar interface and feature set
- Receives regular updates
- Repository: https://github.com/shauninman/MinUI

### Option 2: Use Last Compatible Release
- Download last NextUI release supporting Smart Pro
- Stable but no future updates
- Check releases before November 15, 2025

### Option 3: Fork from Archive Branch
- Archive branch preserves Smart Pro code
- Community can maintain independently
- Requires development expertise

---

## Code Changes

### Files Updated

1. **Documentation**
   - `README.md` - Removed Smart Pro from supported devices
   - `MIGRATION.md` - Added Smart Pro to deprecated list
   - `PAKS.md` - Updated platform description
   - `Doxyfile` - Changed project brief

2. **Build System**
   - `makefile` - Updated comment to clarify Brick-only

3. **Planning Documents**
   - `PHASE_A_COMPLETION_SUMMARY.md` - Updated metrics
   - `PLATFORM_SIMPLIFICATION_SUMMARY.md` - Clarified scope
   - All `.guided/assessment/*.md` files updated
   - All `.guided/architecture/*.md` files updated

### Code Preservation

- No tg5040 platform code deleted (shared with Brick)
- Smart Pro likely still works with builds (untested)
- Archive branch documents Smart Pro as deprecated

---

## FAQ

### Q: Will Smart Pro builds still work?
**A**: Possibly, since Smart Pro uses the same tg5040 platform. However:
- No testing on Smart Pro hardware
- No guarantees of compatibility
- Optimizations may favor Brick-specific characteristics
- **Use at your own risk**

### Q: Why not keep both if they share a platform?
**A**: 
- Hardware differences exist (screen resolution, controls)
- Testing burden doubles
- Edge cases require conditional code
- Optimization becomes "which device do we favor?"
- Better to excel at one device

### Q: Can I request Smart Pro be re-supported?
**A**: No. This is a permanent strategic decision aligning with Phase A goals.

### Q: What about future TrimUI devices?
**A**: Future decisions will be made on a case-by-case basis. For now, min-os is Brick-exclusive.

---

## Documentation Updates

All references to "Brick/Smart Pro", "Brick and Smart Pro", or "Brick & Smart Pro" have been updated to:
- "TrimUI Brick" or "Brick" (singular)
- Added Smart Pro to deprecated devices list
- Updated platform count from 2 to 1 active device

### Updated Documents (16 files)

**Core Documentation**:
1. `README.md`
2. `MIGRATION.md`
3. `PAKS.md`
4. `Doxyfile`
5. `makefile`

**Planning Documents**:
6. `.guided/PHASE_A_COMPLETION_SUMMARY.md`
7. `.guided/PLATFORM_SIMPLIFICATION_SUMMARY.md`

**Assessment Documents**:
8. `.guided/assessment/nextui.overview.md`
9. `.guided/assessment/nextui.optimisation-execution-plan.md`
10. `.guided/assessment/nextui.build-simplification-checklist.md`
11. `.guided/assessment/nextui.optimisation-risks-and-impacts.md`
12. `.guided/assessment/nextui.build-toolchain-environment.md`
13. `.guided/assessment/nextui.platform-simplification-plan.md`

**Architecture Documents**:
14. `.guided/architecture/nextui.platform-architecture-after-simplification.md`
15. `.guided/architecture/nextui.structure-architecture.md`

**This Addendum**:
16. `.guided/PHASE_A_ADDENDUM_SMART_PRO_DEPRECATION.md`

---

## Commit Information

**Commit Message**:
```
refactor!: Deprecate TrimUI Smart Pro support (Phase A addendum)

BREAKING CHANGE: TrimUI Smart Pro is no longer supported

min-os/NextUI is now TrimUI Brick exclusive.

Rationale:
- Focus development resources on single device
- Enable Brick-specific optimizations in Phase C
- Reduce testing burden (2 devices → 1)
- Complete Phase A simplification vision
- Smart Pro and Brick share platform but have edge case differences

Migration for Smart Pro users:
- Option 1: Switch to MinUI (recommended, actively supported)
- Option 2: Use last compatible NextUI release
- Option 3: Fork from archive branch

Updated Documentation:
- 16 files updated with Brick-exclusive messaging
- Added Smart Pro to deprecated devices (total 12 archived)
- Updated MIGRATION.md with Smart Pro guidance
- Created Phase A addendum documenting decision

Platform Count:
- Before: 2 devices (Brick + Smart Pro)
- After: 1 device (Brick only)
- Archived: 12 platforms total

See: .guided/PHASE_A_ADDENDUM_SMART_PRO_DEPRECATION.md
```

---

## Success Criteria

### Must-Have ✅
- ✅ All documentation updated to reflect Brick-only
- ✅ Smart Pro added to deprecated devices list
- ✅ Migration path documented for Smart Pro users
- ✅ Rationale clearly explained
- ✅ FAQ addresses common questions

### Should-Have ✅
- ✅ Addendum document created
- ✅ All 16+ files updated consistently
- ✅ No ambiguous "Brick/Smart Pro" references remain
- ✅ Archive count updated (11 → 12 platforms)

### Nice-to-Have
- ⏳ Community announcement (Discord, GitHub discussions)
- ⏳ Tag release with breaking change
- ⏳ Update website documentation

---

## Phase A Metrics Update

### With Smart Pro Deprecation

| Metric | Before Phase A | After Phase A | After Addendum | Total Change |
|--------|---------------|---------------|----------------|--------------|
| **Platforms** | 13 | 2 | 1 | ↓ 92% |
| **Active Devices** | 13 | 2 | 1 | ↓ 92% |
| **Archived Platforms** | 0 | 11 | 12 | +12 |
| **Testing Burden** | 13 devices | 2 devices | 1 device | ↓ 92% |

**Enhanced Simplification**: 92% reduction vs. original 85%

---

## Alignment with Project Vision

### min-os Vision
> Streamlined, single-platform custom firmware optimized for excellence

### How This Decision Supports Vision
1. ✅ **Streamlined**: Brick-only is simpler than Brick+SmartPro
2. ✅ **Single-platform**: Literally one device now
3. ✅ **Optimized**: Can tune for exact Brick specifications
4. ✅ **Excellence**: Focus resources on one device's perfection

---

## Risks and Mitigation

### Risk: Smart Pro User Backlash
**Likelihood**: Medium  
**Impact**: Low (small user base)  
**Mitigation**: 
- Clear migration guide
- MinUI alternative available
- Archive branch for forks

### Risk: Code Still Has Smart Pro Conditionals
**Likelihood**: Low (shared platform)  
**Impact**: Low (cleanup opportunity)  
**Mitigation**:
- Future: search for Smart Pro-specific code
- Remove if found during Phase C work

### Risk: Lost Testing Coverage
**Likelihood**: N/A (intentional)  
**Impact**: Positive (less to test)  
**Mitigation**: N/A (this is the goal)

---

## Next Steps

### Immediate (This Commit)
- ✅ Update all 16 documentation files
- ✅ Create this addendum
- ✅ Commit with breaking change annotation

### Short-term (This Week)
- ⏳ Announce to community (Discord)
- ⏳ Update GitHub description
- ⏳ Tag release if appropriate

### Long-term (Phase C)
- Search for any Smart Pro-specific code
- Remove if found (further simplification)
- Optimize for Brick-specific characteristics

---

## Conclusion

This addendum completes Phase A's vision of radical simplification. min-os/NextUI is now:

- ✅ **Single device** (TrimUI Brick)
- ✅ **Single platform** (tg5040)
- ✅ **Single focus** (Brick optimization)
- ✅ **12 archived platforms** (excellent preservation)
- ✅ **92% reduction** in platform complexity

The project is now positioned to deliver the **best possible TrimUI Brick experience** in Phase C optimization work.

---

**Document Status**: ✅ Complete  
**Phase**: Phase A - Addendum  
**Impact**: Breaking Change  
**Author**: GitHub Copilot  
**Date**: November 15, 2025
