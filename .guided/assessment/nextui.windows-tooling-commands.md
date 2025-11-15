# NextUI Windows Tooling Commands

This document provides concrete PowerShell commands for installing and running analysis tools on Windows 10/11.

---

## Prerequisites

### Package Managers

You need either **winget** (built into Windows 11, available on Windows 10) or **Chocolatey**.

**Check if winget is available:**
```powershell
winget --version
```

**Install Chocolatey (if winget unavailable):**
```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
```

---

## Installation Commands

### Using winget (Recommended)

#### Mandatory Tools

```powershell
# Code metrics tool (fast line counter)
winget install boyter.scc

# Fast text search
winget install BurntSushi.ripgrep.MSVC

# Fast file finder
winget install sharkdp.fd

# Node.js (required for jscpd)
winget install OpenJS.NodeJS

# Wait for Node.js PATH to update, then install jscpd
npm install -g jscpd
```

#### Important Tools (Static Analysis)

```powershell
# C/C++ static analyzer
winget install Cppcheck.Cppcheck

# LLVM tools including clang-tidy
winget install LLVM.LLVM
```

#### Optional Tools (Documentation)

```powershell
# API documentation generator
winget install DimitriVanHeesch.Doxygen

# Graph visualization (required by Doxygen)
winget install Graphviz.Graphviz
```

#### Build Tools (Optional)

```powershell
# Git (usually pre-installed)
winget install Git.Git

# Docker Desktop (for build verification)
winget install Docker.DockerDesktop
```

### Using Chocolatey (Alternative)

#### Mandatory Tools

```powershell
# Code metrics
choco install scc -y

# Text search
choco install ripgrep -y

# File finder
choco install fd -y

# Node.js for jscpd
choco install nodejs -y
refreshenv  # Reload PATH
npm install -g jscpd
```

#### Important Tools

```powershell
# Static analyzers
choco install cppcheck -y
choco install llvm -y
```

#### Optional Tools

```powershell
# Documentation
choco install doxygen.install -y
choco install graphviz -y
```

---

## Verification Commands

After installation, verify each tool:

```powershell
# Check mandatory tools
scc --version
rg --version
fd --version
node --version
jscpd --version

# Check static analysis tools
cppcheck --version
clang-tidy --version

# Check documentation tools
doxygen --version
dot -V  # Graphviz

# Check supporting tools
git --version
docker --version
```

---

## Analysis Commands

### Quick Start: Run Full Analysis

```powershell
# Navigate to repository root
cd C:\Workspace\NextUI

# Run complete analysis (recommended)
.\tools\windows\run-analysis.ps1
```

This script will:
1. Create output directories
2. Run all configured analysis tools
3. Generate reports in `.guided/assessment/reports/`
4. Display summary of findings

---

## Manual Analysis Commands

If you prefer to run tools individually:

### 1. Code Metrics with scc

```powershell
# Basic statistics
scc workspace/

# Detailed output with complexity metrics
scc --by-file --complexity workspace/

# JSON output for programmatic use
scc --format json --output .guided/assessment/reports/code-metrics.json workspace/

# Analyze only C source files
scc --include-ext c,h workspace/

# Compare active vs unmaintained code
scc workspace/all/ > .guided/assessment/reports/scc-active.txt
scc workspace/_unmaintained/ > .guided/assessment/reports/scc-unmaintained.txt
```

### 2. Duplication Detection with jscpd

```powershell
# Basic duplication check
jscpd workspace/

# Detailed HTML report
jscpd workspace/ `
  --min-lines 5 `
  --min-tokens 30 `
  --format "c,h,sh,mk,makefile" `
  --reporters "html,console" `
  --output .guided/assessment/reports/duplication/

# Only check C files
jscpd workspace/ --pattern "**/*.c,**/*.h" --output .guided/assessment/reports/duplication-c/
```

### 3. Static Analysis with cppcheck

```powershell
# Basic check of all C files
cppcheck workspace/all/ workspace/tg5040/ 2> .guided/assessment/reports/cppcheck.txt

# More thorough analysis (slower)
cppcheck `
  --enable=all `
  --inconclusive `
  --force `
  --suppress=missingIncludeSystem `
  --template="{file}:{line}: [{severity}] {message}" `
  workspace/all/ workspace/tg5040/ `
  2> .guided/assessment/reports/cppcheck-full.txt

# XML output for parsing
cppcheck `
  --enable=all `
  --xml `
  --xml-version=2 `
  workspace/all/ workspace/tg5040/ `
  2> .guided/assessment/reports/cppcheck.xml
```

### 4. Linting with clang-tidy

**Note**: clang-tidy works best with a compilation database. If unavailable, use a basic check:

```powershell
# Find all C files
$cFiles = fd -e c -e h . workspace/all workspace/tg5040

# Run clang-tidy on each (basic checks only)
$cFiles | ForEach-Object {
  clang-tidy $_ -- -I./workspace/all/common 2>&1
} | Out-File .guided/assessment/reports/clang-tidy.txt

# If compile_commands.json exists (from Docker build):
clang-tidy -p workspace/all/minarch workspace/all/minarch/minarch.c `
  > .guided/assessment/reports/clang-tidy-minarch.txt
```

### 5. Pattern Searches with ripgrep

```powershell
# Find TODO/FIXME comments
rg "TODO|FIXME|HACK|XXX" workspace/ `
  --type c `
  --line-number `
  > .guided/assessment/reports/patterns-todos.txt

# Find malloc without null checks (basic heuristic)
rg "malloc\(" workspace/ --type c -A 3 `
  | rg -v "if.*NULL" `
  > .guided/assessment/reports/patterns-malloc.txt

# Find magic numbers (hex constants)
rg "0x[0-9A-Fa-f]{4,}" workspace/ --type c `
  > .guided/assessment/reports/patterns-magic-numbers.txt

# Find potential buffer overflows (strcpy, sprintf)
rg "strcpy|sprintf|gets\(" workspace/ --type c `
  > .guided/assessment/reports/patterns-unsafe-funcs.txt

# Find commented-out code blocks
rg "^\s*//" workspace/ --type c --count-matches `
  > .guided/assessment/reports/patterns-commented-code.txt
```

### 6. File Discovery with fd

```powershell
# Find all C source files
fd -e c . workspace/ > .guided/assessment/reports/files-c.txt

# Find large files (potential refactoring targets)
fd -e c -e h . workspace/ -x wc -l | Sort-Object -Descending | Select-Object -First 20

# Find makefiles
fd -g "*makefile*" -g "*.mk" workspace/

# Find configuration files
fd -e cfg -e conf -e ini workspace/
```

### 7. Generate Documentation with Doxygen

```powershell
# Generate Doxygen configuration if it doesn't exist
doxygen -g Doxyfile

# Edit Doxyfile to point to workspace/, then run
doxygen Doxyfile

# Open generated documentation
Start-Process docs/doxygen/html/index.html
```

---

## Advanced Usage

### Combining Tools for Deeper Insights

```powershell
# Find largest functions (using scc complexity + ripgrep)
scc --by-file --complexity workspace/all/minarch/ | Select-String "Complexity"

# Find files with most TODOs
rg "TODO" workspace/ --type c --count | Sort-Object -Descending

# Generate complexity report per directory
Get-ChildItem workspace/all/ -Directory | ForEach-Object {
  $name = $_.Name
  scc $_.FullName --format json | ConvertFrom-Json | 
    Select-Object -ExpandProperty Languages | 
    Select-Object Name, Code, Complexity |
    Export-Csv ".guided/assessment/reports/complexity-$name.csv"
}
```

### Creating a Compilation Database for clang-tidy

If you have Docker installed and want better clang-tidy results:

```powershell
# Run Docker build with compilation database generation
docker run --rm `
  -v ${PWD}/workspace:/root/workspace `
  ghcr.io/loveretro/tg5040-toolchain:latest `
  /bin/bash -c "cd /root/workspace && make PLATFORM=tg5040 && bear -- make clean && bear -- make"

# This generates compile_commands.json which clang-tidy can use
```

**Note**: This requires Docker Desktop and the NextUI toolchain.

---

## Automated Analysis Script

The `run-analysis.ps1` script combines all these commands:

```powershell
# Full analysis with all tools
.\tools\windows\run-analysis.ps1

# Skip optional tools (faster)
.\tools\windows\run-analysis.ps1 -SkipOptional

# Only run metrics (no static analysis)
.\tools\windows\run-analysis.ps1 -MetricsOnly

# Verbose output
.\tools\windows\run-analysis.ps1 -Verbose
```

---

## Troubleshooting

### PATH Issues

If commands aren't found after installation:

```powershell
# Reload environment variables
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Or restart PowerShell
```

### Permission Errors

Run PowerShell as Administrator for installations:

```powershell
# Check execution policy
Get-ExecutionPolicy

# Allow scripts (if needed)
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Node.js PATH Not Updated

After installing Node.js:

```powershell
# Close and reopen PowerShell, or:
refreshenv  # If using Chocolatey

# Verify Node.js
node --version
npm --version

# Install jscpd
npm install -g jscpd
```

### Docker Not Starting

Docker Desktop requires:
- Windows 10/11 Pro, Enterprise, or Education
- Hyper-V enabled (or WSL 2 backend)
- Virtualization enabled in BIOS

Check Docker status:
```powershell
docker info
```

---

## Output Locations

All reports are saved to `.guided/assessment/reports/`:

```
.guided/assessment/reports/
├── code-metrics.json          # scc output
├── duplication/               # jscpd HTML report
│   └── index.html
├── cppcheck.txt               # cppcheck warnings
├── cppcheck-full.txt          # detailed cppcheck
├── clang-tidy.txt             # clang-tidy suggestions
├── patterns-todos.txt         # TODO/FIXME findings
├── patterns-malloc.txt        # malloc without null checks
├── patterns-magic-numbers.txt # hex constants
├── patterns-unsafe-funcs.txt  # strcpy, sprintf, etc.
├── files-c.txt                # list of C files
└── analysis-summary.md        # high-level summary
```

---

## Next Steps

After running analysis:

1. **Review** `.guided/assessment/reports/analysis-summary.md`
2. **Open** `duplication/index.html` in browser for visual duplication map
3. **Read** `cppcheck.txt` and `clang-tidy.txt` for actionable warnings
4. **Search** pattern files for high-priority technical debt
5. **Use** report data in assessment prompts and refactor planning

---

## Reference: One-Command Full Setup

For a completely fresh Windows machine:

```powershell
# Install winget (Windows 10 only; skip on Windows 11)
# See: https://github.com/microsoft/winget-cli

# Install all tools
winget install boyter.scc BurntSushi.ripgrep.MSVC sharkdp.fd OpenJS.NodeJS Cppcheck.Cppcheck LLVM.LLVM DimitriVanHeesch.Doxygen Graphviz.Graphviz

# Reload PATH
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Install jscpd
npm install -g jscpd

# Clone NextUI (if not already)
git clone https://github.com/LoveRetro/NextUI.git C:\Workspace\NextUI
cd C:\Workspace\NextUI

# Run analysis
.\tools\windows\run-analysis.ps1
```

---

## Maintenance Commands

```powershell
# Update all tools
winget upgrade --all

# Update jscpd
npm update -g jscpd

# Clean old reports (archive them first)
Move-Item .guided/assessment/reports reports-backup-$(Get-Date -Format 'yyyyMMdd')
```
