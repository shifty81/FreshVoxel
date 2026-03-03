# CI/CD Automated Build System — Reusable Setup Guide

A portable, step-by-step guide for adding GitHub Actions CI/CD automated builds to **any** repository. Based on the patterns used in the FreshVoxel Engine.

> **Audience:** Developers who want to add automated building, testing, code quality, and release workflows to their own repositories.

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Quick Start](#quick-start)
4. [Core Concepts](#core-concepts)
5. [Workflow Templates](#workflow-templates)
   - [C++ / CMake Projects](#c--cmake-projects)
   - [.NET / C# Projects](#net--c-projects)
   - [Node.js / TypeScript Projects](#nodejs--typescript-projects)
   - [Python Projects](#python-projects)
   - [Rust Projects](#rust-projects)
   - [Go Projects](#go-projects)
6. [Adding Code Quality Checks](#adding-code-quality-checks)
7. [Adding Code Coverage](#adding-code-coverage)
8. [Automated Release Workflow](#automated-release-workflow)
9. [Best Practices](#best-practices)
10. [Troubleshooting](#troubleshooting)

---

## Overview

An automated build system uses **GitHub Actions** to automatically build, test, and validate your code every time you push a commit or open a pull request. This ensures:

- ✅ **Every commit builds successfully** — catch compile/build errors instantly
- ✅ **Tests run automatically** — regressions are caught before merging
- ✅ **Code quality is enforced** — linters and static analysis run on every change
- ✅ **Releases are automated** — tag a version and artifacts are built and published

### How It Works

```
Developer pushes code
        │
        ▼
GitHub detects the push event
        │
        ▼
GitHub Actions reads .github/workflows/*.yml
        │
        ▼
Jobs run on cloud VMs (Linux, Windows, macOS)
        │
        ├── Build the project
        ├── Run tests
        ├── Run linters / static analysis
        └── Upload artifacts / create releases
        │
        ▼
Results appear as ✅ or ❌ on the commit / PR
```

### What You Need

1. A GitHub repository
2. One or more `.yml` workflow files in `.github/workflows/`
3. That's it — GitHub Actions is free for public repositories

---

## Prerequisites

- A GitHub account and repository
- Your project has a build command (e.g., `cmake --build`, `dotnet build`, `npm run build`, `cargo build`)
- Your project has a test command (e.g., `ctest`, `dotnet test`, `npm test`, `cargo test`)

No additional setup, servers, or accounts are required. GitHub Actions runners come pre-installed with most common tools.

---

## Quick Start

To add automated builds to your repo, create a workflow file at `.github/workflows/ci.yml`:

### Step 1: Create the Workflow Directory

```bash
mkdir -p .github/workflows
```

### Step 2: Create a Workflow File

Create `.github/workflows/ci.yml` with the template for your language (see [Workflow Templates](#workflow-templates) below).

### Step 3: Commit and Push

```bash
git add .github/workflows/ci.yml
git commit -m "Add CI workflow"
git push
```

### Step 4: Verify

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. You should see your workflow running

That's it. Every future push and pull request will trigger the workflow automatically.

---

## Core Concepts

### Workflow File Structure

Every GitHub Actions workflow file has this structure:

```yaml
name: CI                          # Display name in the Actions tab

on:                               # When to run
  push:
    branches: [ main, develop ]   # Run on pushes to these branches
  pull_request:
    branches: [ main, develop ]   # Run on PRs targeting these branches

jobs:                             # What to run
  build:                          # Job name (you choose this)
    runs-on: ubuntu-latest        # VM type (ubuntu-latest, windows-latest, macos-latest)

    steps:                        # Sequential steps within the job
    - uses: actions/checkout@v4   # Step 1: Check out your code

    - name: Build                 # Step 2: Build (name is for display)
      run: echo "build command here"

    - name: Test                  # Step 3: Test
      run: echo "test command here"
```

### Key Concepts

| Concept | Description |
|---------|-------------|
| **Workflow** | A `.yml` file in `.github/workflows/` that defines automation |
| **Trigger (`on`)** | Events that start the workflow (push, pull_request, tag, schedule) |
| **Job** | A set of steps that run on the same VM. Jobs run in parallel by default |
| **Step** | A single command or action within a job |
| **Action** | A reusable step published on GitHub Marketplace (e.g., `actions/checkout@v4`) |
| **Runner** | The VM that executes the job (`ubuntu-latest`, `windows-latest`, `macos-latest`) |
| **Matrix** | Run the same job with different configurations (e.g., Debug + Release) |
| **Artifact** | A file produced by a build that you can download later |
| **Secret** | An encrypted variable for tokens/keys (Settings → Secrets → Actions) |

### Build Matrix

A build matrix lets you test multiple configurations in parallel:

```yaml
strategy:
  matrix:
    os: [ubuntu-latest, windows-latest]
    build_type: [Debug, Release]
    # This creates 4 jobs: ubuntu-Debug, ubuntu-Release, windows-Debug, windows-Release
```

### Caching Dependencies

Speed up builds by caching downloaded dependencies:

```yaml
- uses: actions/cache@v4
  with:
    path: ~/.cache       # Path to cache
    key: ${{ runner.os }}-deps-${{ hashFiles('**/lockfile') }}
    restore-keys: |
      ${{ runner.os }}-deps-
```

---

## Workflow Templates

Copy the template that matches your project's language and build system.

### C++ / CMake Projects

This is the pattern used by FreshVoxel Engine.

```yaml
name: CI - Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: windows-latest       # Change to ubuntu-latest for Linux
    
    strategy:
      matrix:
        build_type: [Debug, Release]
    
    steps:
    - uses: actions/checkout@v4
    
    # Option A: Use vcpkg for dependencies (recommended for C++)
    - name: Setup vcpkg
      uses: lukka/run-vcpkg@v11
      with:
        vcpkgJsonGlob: '**/vcpkg.json'
        runVcpkgInstall: true
    
    # Option B: Use apt for dependencies (Linux only)
    # - name: Install Dependencies
    #   run: sudo apt-get update && sudo apt-get install -y libglfw3-dev libglm-dev

    - name: Configure CMake
      run: |
        cmake -B build \
          -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} \
          -DBUILD_TESTS=ON
    
    - name: Build
      run: cmake --build build --config ${{ matrix.build_type }}
    
    - name: Test
      working-directory: build
      run: ctest -C ${{ matrix.build_type }} --output-on-failure
    
    - name: Upload Artifacts
      if: matrix.build_type == 'Release'
      uses: actions/upload-artifact@v4
      with:
        name: build-${{ runner.os }}-x64
        path: build/Release/
        retention-days: 7
```

**vcpkg.json** (dependency manifest for C++ projects):

```json
{
  "name": "my-project",
  "version-string": "1.0.0",
  "dependencies": [
    "gtest"
  ]
}
```

### .NET / C# Projects

```yaml
name: CI - .NET Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Setup .NET
      uses: actions/setup-dotnet@v4
      with:
        dotnet-version: '9.0.x'
    
    - name: Restore Dependencies
      run: dotnet restore
    
    - name: Build
      run: dotnet build --no-restore --configuration Release
    
    - name: Test
      run: dotnet test --no-build --configuration Release --verbosity normal
    
    - name: Publish
      run: dotnet publish --no-build --configuration Release --output ./publish
    
    - name: Upload Artifacts
      uses: actions/upload-artifact@v4
      with:
        name: dotnet-app
        path: ./publish
        retention-days: 7
```

### Node.js / TypeScript Projects

```yaml
name: CI - Node.js Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    strategy:
      matrix:
        node-version: [18, 20, 22]
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Setup Node.js ${{ matrix.node-version }}
      uses: actions/setup-node@v4
      with:
        node-version: ${{ matrix.node-version }}
        cache: 'npm'
    
    - name: Install Dependencies
      run: npm ci
    
    - name: Lint
      run: npm run lint
    
    - name: Build
      run: npm run build
    
    - name: Test
      run: npm test
```

### Python Projects

```yaml
name: CI - Python Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    strategy:
      matrix:
        python-version: ['3.10', '3.11', '3.12']
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Setup Python ${{ matrix.python-version }}
      uses: actions/setup-python@v5
      with:
        python-version: ${{ matrix.python-version }}
        cache: 'pip'
    
    - name: Install Dependencies
      run: |
        python -m pip install --upgrade pip
        pip install -r requirements.txt
        pip install pytest flake8
    
    - name: Lint
      run: flake8 . --count --select=E9,F63,F7,F82 --show-source --statistics
    
    - name: Test
      run: pytest --verbose
```

### Rust Projects

```yaml
name: CI - Rust Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Setup Rust
      uses: dtolnay/rust-toolchain@stable
      with:
        components: clippy, rustfmt
    
    - name: Cache Cargo
      uses: actions/cache@v4
      with:
        path: |
          ~/.cargo/registry
          ~/.cargo/git
          target
        key: ${{ runner.os }}-cargo-${{ hashFiles('**/Cargo.lock') }}
    
    - name: Check Formatting
      run: cargo fmt --all -- --check
    
    - name: Lint (Clippy)
      run: cargo clippy --all-targets -- -D warnings
    
    - name: Build
      run: cargo build --release
    
    - name: Test
      run: cargo test --verbose
```

### Go Projects

```yaml
name: CI - Go Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Setup Go
      uses: actions/setup-go@v5
      with:
        go-version: '1.22'
    
    - name: Build
      run: go build -v ./...
    
    - name: Test
      run: go test -v -race -coverprofile=coverage.out ./...
    
    - name: Vet
      run: go vet ./...
```

---

## Adding Code Quality Checks

Add static analysis as a separate workflow or job. This catches bugs and style issues beyond what the compiler checks.

### C++ Static Analysis (cppcheck + clang-tidy)

```yaml
name: Code Quality

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  static-analysis:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Install Tools
      run: sudo apt-get update && sudo apt-get install -y cppcheck clang-tidy
    
    - name: Run cppcheck
      run: |
        cppcheck --enable=all --std=c++20 --error-exitcode=1 \
          --suppress=missingIncludeSystem \
          --inline-suppr \
          -I include/ \
          src/ 2>&1 | tee cppcheck-report.txt
    
    - name: Upload Report
      if: always()
      uses: actions/upload-artifact@v4
      with:
        name: cppcheck-report
        path: cppcheck-report.txt
        retention-days: 7
```

### Generic Linter (Super-Linter — works with any language)

```yaml
  lint:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v4
      with:
        fetch-depth: 0
    
    - name: Super-Linter
      uses: super-linter/super-linter@v6
      env:
        DEFAULT_BRANCH: main
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

---

## Adding Code Coverage

Track how much of your code is tested.

### C++ Coverage (gcovr)

```yaml
name: Code Coverage

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  coverage:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Install Dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake build-essential gcovr lcov
    
    - name: Configure with Coverage Flags
      run: |
        cmake -B build \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
          -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
          -DBUILD_TESTS=ON
    
    - name: Build and Test
      run: |
        cmake --build build -j$(nproc)
        cd build && ctest --output-on-failure
    
    - name: Generate Coverage Report
      run: |
        gcovr -r . --xml --xml-pretty -o coverage.xml \
          --exclude='.*tests/.*' \
          --exclude='.*build/.*'
    
    - name: Upload to Codecov
      uses: codecov/codecov-action@v4
      with:
        files: ./coverage.xml
        fail_ci_if_error: false
```

### .NET Coverage

```yaml
    - name: Test with Coverage
      run: dotnet test --collect:"XPlat Code Coverage"
    
    - name: Upload to Codecov
      uses: codecov/codecov-action@v4
      with:
        directory: ./**/TestResults/
```

### Node.js Coverage

```yaml
    - name: Test with Coverage
      run: npx jest --coverage
    
    - name: Upload to Codecov
      uses: codecov/codecov-action@v4
      with:
        directory: ./coverage/
```

---

## Automated Release Workflow

Automatically create GitHub Releases with build artifacts when you push a version tag.

### How to Use

1. Add the workflow below to `.github/workflows/release.yml`
2. When ready to release, tag the commit:

```bash
git tag v1.0.0
git push origin v1.0.0
```

3. The workflow triggers, builds the project, and creates a GitHub Release with downloadable artifacts.

### Release Workflow Template

```yaml
name: Release

on:
  push:
    tags:
      - 'v*.*.*'       # Triggers on version tags: v1.0.0, v2.1.3, etc.

permissions:
  contents: write       # Required to create releases

jobs:
  create-release:
    runs-on: ubuntu-latest
    outputs:
      upload_url: ${{ steps.create_release.outputs.upload_url }}
      version: ${{ steps.get_version.outputs.version }}
    
    steps:
    - uses: actions/checkout@v4
      with:
        fetch-depth: 0
    
    - name: Get Version from Tag
      id: get_version
      run: echo "version=${GITHUB_REF#refs/tags/v}" >> $GITHUB_OUTPUT
    
    - name: Generate Release Notes
      id: changelog
      run: |
        # Extract notes from CHANGELOG.md if it exists
        VERSION="${{ steps.get_version.outputs.version }}"
        if [ -f CHANGELOG.md ] && grep -q "## \[$VERSION\]" CHANGELOG.md; then
          sed -n "/## \[$VERSION\]/,/## \[/p" CHANGELOG.md | sed '$d' > RELEASE_NOTES.md
        else
          echo "Release $VERSION" > RELEASE_NOTES.md
        fi
    
    - name: Create GitHub Release
      id: create_release
      uses: actions/create-release@v1
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
      with:
        tag_name: ${{ github.ref }}
        release_name: v${{ steps.get_version.outputs.version }}
        body_path: RELEASE_NOTES.md
        draft: false
        prerelease: ${{ contains(github.ref, 'alpha') || contains(github.ref, 'beta') || contains(github.ref, 'rc') }}

  build-and-upload:
    needs: create-release
    runs-on: ubuntu-latest    # Change per your platform needs
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Build Release
      run: |
        # Replace with your build commands
        mkdir -p dist
        echo "Build your project here"
    
    - name: Package
      run: |
        tar czf my-project-v${{ needs.create-release.outputs.version }}.tar.gz -C dist .
    
    - name: Upload Release Asset
      uses: actions/upload-release-asset@v1
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
      with:
        upload_url: ${{ needs.create-release.outputs.upload_url }}
        asset_path: my-project-v${{ needs.create-release.outputs.version }}.tar.gz
        asset_name: my-project-v${{ needs.create-release.outputs.version }}.tar.gz
        asset_content_type: application/gzip
```

---

## Best Practices

### 1. Start Simple, Add Incrementally

Begin with a single `ci.yml` that builds and tests. Add code quality, coverage, and releases later.

### 2. Use a Build Matrix for Multiple Configurations

```yaml
strategy:
  matrix:
    build_type: [Debug, Release]
    os: [ubuntu-latest, windows-latest]
```

### 3. Cache Dependencies

Caching dramatically speeds up builds:

```yaml
- uses: actions/cache@v4
  with:
    path: |
      ~/.cache/pip
      ~/.npm
      ~/.cargo/registry
    key: ${{ runner.os }}-${{ hashFiles('**/lockfile') }}
```

### 4. Fail Fast

Stop other matrix jobs when one fails (saves runner minutes):

```yaml
strategy:
  fail-fast: true
  matrix:
    build_type: [Debug, Release]
```

### 5. Keep Artifacts with Reasonable Retention

```yaml
- uses: actions/upload-artifact@v4
  with:
    name: my-artifact
    path: build/output/
    retention-days: 7     # Don't keep forever — saves storage
```

### 6. Protect Main Branches

In your repo settings (Settings → Branches → Branch protection rules):
- ✅ Require status checks to pass before merging
- ✅ Require branches to be up to date before merging
- Select your CI workflow as a required check

### 7. Use Secrets for Sensitive Data

Never hardcode tokens or keys. Use GitHub Secrets:

1. Go to Settings → Secrets and variables → Actions
2. Click **New repository secret**
3. Reference in workflows: `${{ secrets.MY_SECRET }}`

### 8. Pin Action Versions

Use specific versions instead of `@main` to avoid breaking changes:

```yaml
# Good — pinned to v4
- uses: actions/checkout@v4

# Bad — may break without warning
- uses: actions/checkout@main
```

### 9. Separate Concerns into Multiple Workflows

| Workflow | Purpose | Trigger |
|----------|---------|---------|
| `ci.yml` | Build and test | Push, PR |
| `quality.yml` | Linting, static analysis | Push, PR |
| `coverage.yml` | Code coverage reporting | Push, PR |
| `release.yml` | Build artifacts, create releases | Version tags |

### 10. Document Your Workflows

Add a `README.md` in `.github/workflows/` explaining each workflow.

---

## Troubleshooting

### Workflow Doesn't Trigger

- **Check the branch filter:** The `on.push.branches` and `on.pull_request.branches` must match your branch names exactly
- **Check the file location:** Workflows must be in `.github/workflows/` (not `.github/workflow/`)
- **Check YAML syntax:** Use a YAML linter or `python3 -c "import yaml; yaml.safe_load(open('ci.yml'))"`

### Build Fails in CI but Works Locally

- **Missing dependencies:** CI runners start fresh. Ensure all dependencies are installed in the workflow
- **Path differences:** CI runs on Linux (usually). Check for case-sensitive paths and correct path separators
- **Environment variables:** Any env vars you set locally need to be set in the workflow via `env:`

### Tests Fail in CI

- **Headless environment:** CI runners have no display. GUI tests need a virtual framebuffer (`xvfb-run`)
- **Timing issues:** CI runners may be slower. Increase timeouts if tests are timing out
- **Missing test data:** Ensure test fixtures are committed to the repo or downloaded in the workflow

### Viewing Workflow Logs

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Click on the failed run
4. Click on the failed job
5. Expand the failed step to see full output

### Common Action Versions

| Action | Purpose | Version |
|--------|---------|---------|
| `actions/checkout` | Check out code | `@v4` |
| `actions/upload-artifact` | Upload build artifacts | `@v4` |
| `actions/cache` | Cache dependencies | `@v4` |
| `actions/setup-node` | Install Node.js | `@v4` |
| `actions/setup-python` | Install Python | `@v5` |
| `actions/setup-dotnet` | Install .NET | `@v4` |
| `actions/setup-go` | Install Go | `@v5` |
| `dtolnay/rust-toolchain` | Install Rust | `@stable` |
| `lukka/run-vcpkg` | Install C++ deps via vcpkg | `@v11` |
| `codecov/codecov-action` | Upload code coverage | `@v4` |

---

## Checklist: Adding CI/CD to a New Repo

Use this checklist when setting up automated builds in a new repository:

- [ ] Create `.github/workflows/` directory
- [ ] Add `ci.yml` with build and test steps for your language
- [ ] Push and verify the workflow runs in the Actions tab
- [ ] (Optional) Add `quality.yml` for linting / static analysis
- [ ] (Optional) Add `coverage.yml` for code coverage reporting
- [ ] (Optional) Add `release.yml` for automated releases on version tags
- [ ] (Optional) Add branch protection rules requiring CI to pass
- [ ] (Optional) Add a `.github/workflows/README.md` documenting your workflows
- [ ] (Optional) Add a status badge to your `README.md`:

```markdown
![CI](https://github.com/YOUR_USER/YOUR_REPO/actions/workflows/ci.yml/badge.svg)
```

---

## Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [GitHub Actions Marketplace](https://github.com/marketplace?type=actions)
- [Workflow Syntax Reference](https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions)
- [GitHub Actions Billing](https://docs.github.com/en/billing/managing-billing-for-github-actions/about-billing-for-github-actions) (free for public repos)
- [FreshVoxel CI Workflows](../../.github/workflows/) — real-world examples from this project
- [FreshVoxel Workflow Docs](../WORKFLOWS.md) — detailed workflow documentation

---

**Last Updated**: 2026-03-03
**Guide Version**: 1.0
