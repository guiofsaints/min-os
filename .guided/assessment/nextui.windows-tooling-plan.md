# NextUI Windows Tooling Plan

This document outlines the comprehensive tooling strategy for analyzing the NextUI codebase on Windows 10/11, providing reproducible scripts for code metrics, static analysis, duplication detection, and architecture visualization.

---

## Overview

NextUI is a custom firmware for TrimUI devices built on a Docker-based cross-compilation toolchain. While the production build process requires Docker and Linux-based ARM toolchains, we can perform valuable analysis on Windows using native tools and Windows Subsystem for Linux (WSL) where necessary.

**Analysis Goals:**
1. Measure codebase size, language distribution, and complexity metrics
2. Detect code duplication and anti-patterns
3. Perform static analysis for potential bugs and security issues
4. Generate architecture documentation and dependency graphs
5. Document the build toolchain and environment
6. Create reusable, idempotent PowerShell scripts for maintainers

---

## Tool Selection and Rationale

### Mandatory Tools (Core Analysis)

#### 1. **scc** (Sloc Cloc and Code Counter)
- **Purpose**: Fast, accurate line counting with complexity metrics
- **Why scc over cloc**: 2-10x faster, better language detection, outputs JSON/CSV
- **Use Case**: Generate language statistics, LOC counts, complexity metrics
- **Install**: `winget install boyter.scc` or `choco install scc`
- **Output**: Console, JSON, CSV formats
- **NextUI Value**: Quantify codebase size, track technical debt metrics

#### 2. **ripgrep** (`rg`)
- **Purpose**: Ultra-fast text search for pattern detection
- **Use Case**: Find TODO/FIXME comments, magic numbers, unsafe patterns, dead code markers
- **Install**: `winget install BurntSushi.ripgrep.MSVC` or `choco install ripgrep`
- **Output**: Console with file paths and line numbers
- **NextUI Value**: Quick discovery of technical debt markers, search for specific patterns

#### 3. **fd**
- **Purpose**: Fast file finder (alternative to `find`)
- **Use Case**: Locate files by pattern, extension, or age
- **Install**: `winget install sharkdp.fd` or `choco install fd`
- **Output**: File paths
- **NextUI Value**: Quickly find source files, headers, configs for analysis

#### 4. **jscpd** (Copy/Paste Detector)
- **Purpose**: Detect code duplication across multiple languages
- **Use Case**: Find duplicated code blocks in C, shell scripts, makefiles
- **Install**: `npm install -g jscpd` (requires Node.js: `winget install OpenJS.NodeJS`)
- **Output**: Console report, HTML dashboard, JSON
- **NextUI Value**: Identify refactoring opportunities, measure duplication percentage

### Important Tools (Static Analysis)

#### 5. **cppcheck**
- **Purpose**: Static analysis for C/C++ code
- **Use Case**: Detect memory leaks, null pointer dereferences, buffer overflows, undefined behavior
- **Install**: `winget install Cppcheck.Cppcheck` or `choco install cppcheck`
- **Output**: Console, XML, text file
- **NextUI Value**: Catch common C errors without compiling, improve code safety

#### 6. **clang-tidy**
- **Purpose**: LLVM-based linter and static analyzer for C/C++
- **Use Case**: Enforce coding standards, detect bugs, suggest modernizations
- **Install**: Part of LLVM - `winget install LLVM.LLVM` or `choco install llvm`
- **Output**: Console with fix suggestions, YAML
- **NextUI Value**: More sophisticated analysis than cppcheck, can auto-fix issues
- **Note**: Requires compilation database (`compile_commands.json`) for best results

### Optional Tools (Documentation & Visualization)

#### 7. **doxygen**
- **Purpose**: Generate API documentation from source code comments
- **Use Case**: Create HTML docs, extract call graphs, visualize dependencies
- **Install**: `winget install DimitriVanHeesch.Doxygen` or `choco install doxygen.install`
- **Output**: HTML documentation, LaTeX, XML
- **NextUI Value**: Automatic architecture documentation, visualize module relationships

#### 8. **graphviz**
- **Purpose**: Graph visualization toolkit (required by Doxygen for diagrams)
- **Use Case**: Generate call graphs, dependency diagrams, include graphs
- **Install**: `winget install Graphviz.Graphviz` or `choco install graphviz`
- **Output**: PNG, SVG, PDF diagrams
- **NextUI Value**: Visual understanding of code structure and dependencies

### Supporting Tools

#### 9. **Git for Windows**
- **Purpose**: Version control and diff analysis
- **Use Case**: Extract commit metadata, analyze file change history
- **Install**: Usually pre-installed; `winget install Git.Git` if needed
- **NextUI Value**: Track code evolution, identify hotspots

#### 10. **Docker Desktop** (Optional)
- **Purpose**: Run the actual NextUI build toolchain
- **Use Case**: Verify builds work, extract compilation database for clang-tidy
- **Install**: `winget install Docker.DockerDesktop`
- **NextUI Value**: Test builds on Windows without WSL
- **Note**: Heavy dependency, not required for analysis

---

## Tooling Workflow

### Phase 1: Environment Setup (One-time)

```
1. Run tools/windows/install-tools.ps1
   ├─ Check for winget/choco availability
   ├─ Install mandatory tools (scc, ripgrep, fd, jscpd)
   ├─ Install optional tools (cppcheck, clang-tidy, doxygen, graphviz)
   └─ Verify installations and log versions

2. Review installation summary
   └─ Note any failed installations for manual setup
```

### Phase 2: Analysis Execution

```
1. Run tools/windows/run-analysis.ps1
   ├─ Create output directory (.guided/assessment/reports/)
   ├─ Run scc for code metrics → reports/code-metrics.json
   ├─ Run jscpd for duplication → reports/duplication.html
   ├─ Run cppcheck for static analysis → reports/cppcheck.txt
   ├─ Run clang-tidy (if compile_commands.json available) → reports/clang-tidy.txt
   ├─ Run ripgrep searches for patterns → reports/patterns.txt
   ├─ Optionally run doxygen → docs/doxygen/html/
   └─ Generate summary report → reports/analysis-summary.md

2. Review generated reports
   └─ See nextui.windows-tooling-report.md for interpretation guide
```

### Phase 3: Build Verification (Optional)

```
1. Check Docker availability
   └─ If available: run build check to extract toolchain info

2. Document build environment
   └─ Update nextui.build-toolchain-environment.md with findings
```

---

## Analysis Strategy

### Code Metrics
- **Tool**: scc
- **Target**: `workspace/all/`, `workspace/tg5040/`
- **Metrics**: Lines of code, comments, blanks, complexity, file counts
- **Output**: JSON for programmatic access, console for human review

### Duplication Detection
- **Tool**: jscpd
- **Target**: All C files (`**/*.c`, `**/*.h`)
- **Threshold**: Report blocks > 30 tokens
- **Output**: HTML report with visual duplication map

### Static Analysis
- **Tool**: cppcheck + clang-tidy
- **Target**: C source files in `workspace/all/` and `workspace/tg5040/`
- **Checks**: Memory safety, null checks, buffer overflows, undefined behavior
- **Output**: Text reports with severity levels

### Pattern Searches
- **Tool**: ripgrep
- **Patterns**:
  - `TODO|FIXME|HACK|XXX` - Technical debt markers
  - `malloc|calloc|realloc` without null checks
  - `fopen|fread|fwrite` without error checks
  - Magic numbers (e.g., `0x[0-9A-F]{4,}`)
  - Commented-out code blocks

### Architecture Visualization
- **Tool**: doxygen + graphviz
- **Target**: All source with documentation comments
- **Output**: Call graphs, include dependency graphs, module diagrams

---

## Tool Decision Matrix

| Tool | Priority | Windows Native | WSL Alternative | Auto-Installable |
|------|----------|----------------|-----------------|------------------|
| scc | Mandatory | ✅ Yes | N/A | ✅ Yes (winget/choco) |
| ripgrep | Mandatory | ✅ Yes | N/A | ✅ Yes (winget/choco) |
| fd | Mandatory | ✅ Yes | N/A | ✅ Yes (winget/choco) |
| jscpd | Mandatory | ✅ Yes (via npm) | N/A | ⚠️ Requires Node.js |
| cppcheck | Important | ✅ Yes | N/A | ✅ Yes (winget/choco) |
| clang-tidy | Important | ✅ Yes (via LLVM) | N/A | ✅ Yes (winget/choco) |
| doxygen | Optional | ✅ Yes | N/A | ✅ Yes (winget/choco) |
| graphviz | Optional | ✅ Yes | N/A | ✅ Yes (winget/choco) |
| Docker | Optional | ✅ Yes | N/A | ⚠️ Manual setup required |

---

## Integration with NextUI Assessment

### How Other Prompts Should Use These Reports

#### For `assessment.nextui.codebase-deep-dive`:
1. **Read** `.guided/assessment/reports/code-metrics.json` for quantitative data
2. **Parse** duplication report to identify refactoring targets
3. **Review** static analysis warnings to prioritize fixes
4. **Reference** pattern search results for technical debt mapping

#### For Refactor Planning:
1. **Use** duplication data to identify consolidation opportunities
2. **Use** cppcheck/clang-tidy warnings to prioritize safety fixes
3. **Use** complexity metrics to find god functions

#### For Architecture Documentation:
1. **Embed** Doxygen-generated graphs in architecture docs
2. **Reference** dependency graphs to explain module relationships

---

## Maintenance and Updates

### Keeping Tools Up-to-Date
```powershell
# Update all winget-managed tools
winget upgrade --all

# Update Node.js packages
npm update -g jscpd
```

### Re-running Analysis
The analysis script is **idempotent** - safe to run multiple times:
```powershell
.\tools\windows\run-analysis.ps1
```

Reports are timestamped and previous runs are preserved in `reports/archive/`.

---

## Constraints and Limitations

### What This Tooling Does NOT Do:
- ❌ **Compile the codebase** (requires Docker + ARM toolchain)
- ❌ **Run unit tests** (NextUI has no test suite yet)
- ❌ **Profile runtime performance** (requires target hardware)
- ❌ **Modify source files** (analysis is read-only)

### Platform-Specific Notes:
- **Windows 10/11**: All tools run natively
- **WSL**: Not required for analysis, but can be used for build verification
- **Docker**: Optional for build checks; not needed for static analysis

---

## Expected Outputs

See `nextui.windows-tooling-report.md` for detailed report interpretation.

**Quick Summary:**
- `.guided/assessment/reports/code-metrics.json` - LOC, complexity, language breakdown
- `.guided/assessment/reports/duplication.html` - Visual duplication report
- `.guided/assessment/reports/cppcheck.txt` - Static analysis warnings
- `.guided/assessment/reports/clang-tidy.txt` - Linting suggestions
- `.guided/assessment/reports/patterns.txt` - Search results for technical debt
- `.guided/assessment/reports/analysis-summary.md` - High-level findings
- `docs/doxygen/html/index.html` - Generated API documentation (if Doxygen run)

---

## Next Steps

1. **Run** `.\tools\windows\install-tools.ps1` to set up the environment
2. **Run** `.\tools\windows\run-analysis.ps1` to generate reports
3. **Review** reports using guidance in `nextui.windows-tooling-report.md`
4. **Document** build toolchain using `nextui.build-toolchain-environment.md`
5. **Feed** report data into deep-dive assessment and refactor planning

---

## Success Criteria

✅ All mandatory tools install successfully  
✅ Analysis scripts run without errors  
✅ Reports are generated in expected locations  
✅ Reports provide actionable insights  
✅ Scripts are reusable and well-documented  
✅ Other prompts can consume report data programmatically  

---

## References

- [scc Documentation](https://github.com/boyter/scc)
- [ripgrep Guide](https://github.com/BurntSushi/ripgrep)
- [jscpd Documentation](https://github.com/kucherenko/jscpd)
- [Cppcheck Manual](https://cppcheck.sourceforge.io/)
- [clang-tidy Checks](https://clang.llvm.org/extra/clang-tidy/checks/list.html)
- [Doxygen Manual](https://www.doxygen.nl/manual/)
