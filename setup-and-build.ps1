<#
.SYNOPSIS
    Automated build script for FreshVoxel Engine
    
.DESCRIPTION
    This script automates the entire build process for FreshVoxel Engine:
    1. Checks prerequisites (Visual Studio 2022, CMake, Git)
    2. Sets up vcpkg if not present
    3. Generates Visual Studio 2022 solution
    4. Builds the project
    5. Opens Visual Studio solution for development
    
    All output is logged to logs/ directory with timestamped filenames.
    
.PARAMETER SkipBuild
    Skip the actual build step and just generate the solution
    
.PARAMETER BuildConfig
    Build configuration (Debug or Release). Default is Release.
    
.PARAMETER SkipVcpkgBootstrap
    Skip vcpkg bootstrap if it already exists
    
.PARAMETER OpenVS
    Open Visual Studio after successful build. Default is true.
    
.EXAMPLE
    .\setup-and-build.ps1
    Runs full automated setup and build process
    
.EXAMPLE
    .\setup-and-build.ps1 -SkipBuild
    Sets up and generates solution without building
    
.EXAMPLE
    .\setup-and-build.ps1 -BuildConfig Debug
    Builds Debug configuration instead of Release
#>

param(
    [switch]$SkipBuild = $false,
    [ValidateSet("Debug", "Release")]
    [string]$BuildConfig = "Release",
    [switch]$SkipVcpkgBootstrap = $false,
    [bool]$OpenVS = $true
)

# Set error action preference
$ErrorActionPreference = "Stop"

# Color output functions
function Write-Header {
    param([string]$Message)
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host " $Message" -ForegroundColor Cyan
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host ""
}

function Write-Success {
    param([string]$Message)
    Write-Host "[OK] $Message" -ForegroundColor Green
}

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Yellow
}

function Write-ErrorMsg {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host ">> $Message" -ForegroundColor Magenta
}

# Main script
try {
    # ============================================================================
    # Logging Setup — All output logged to logs/ directory
    # ============================================================================
    $LogDir = Join-Path $PSScriptRoot "logs"
    if (-not (Test-Path $LogDir)) {
        New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
    }
    $LogFile = Join-Path $LogDir "setup-and-build-$(Get-Date -Format 'yyyyMMdd-HHmmss').log"
    Start-Transcript -Path $LogFile -Append

    Write-Header "FreshVoxel Engine - Automated Build Script"
    
    # Get script directory (project root)
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    Set-Location $ScriptDir
    Write-Info "Project directory: $ScriptDir"
    
    # ============================================================================
    # Step 1: Check Prerequisites
    # ============================================================================
    Write-Header "Step 1: Checking Prerequisites"
    
    # Check Git
    Write-Step "Checking for Git..."
    try {
        $gitVersion = & git --version 2>&1
        Write-Success "Git found: $gitVersion"
    }
    catch {
        Write-ErrorMsg "Git is not installed or not in PATH"
        Write-Host "Please install Git from: https://git-scm.com/download/win"
        exit 1
    }
    
    # Check CMake
    Write-Step "Checking for CMake..."
    try {
        $cmakeVersion = & cmake --version 2>&1 | Select-Object -First 1
        Write-Success "CMake found: $cmakeVersion"
        
        # Verify CMake version is 3.20 or higher
        $versionMatch = $cmakeVersion -match "cmake version (\d+)\.(\d+)"
        if ($versionMatch) {
            $major = [int]$matches[1]
            $minor = [int]$matches[2]
            if ($major -lt 3 -or ($major -eq 3 -and $minor -lt 20)) {
                Write-ErrorMsg "CMake version 3.20 or higher is required"
                Write-Host "Current version: $major.$minor"
                Write-Host "Please update CMake from: https://cmake.org/download/"
                exit 1
            }
        }
    }
    catch {
        Write-ErrorMsg "CMake is not installed or not in PATH"
        Write-Host "Please install CMake 3.20+ from: https://cmake.org/download/"
        Write-Host "Make sure to add CMake to system PATH during installation"
        exit 1
    }
    
    # Check Visual Studio 2022
    Write-Step "Checking for Visual Studio 2022..."
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsPath = & $vsWhere -version "[17.0,18.0)" -property installationPath -latest 2>&1
        if ($vsPath -and (Test-Path $vsPath)) {
            Write-Success "Visual Studio 2022 found at: $vsPath"
            
            # Check for required workload
            $workloads = & $vsWhere -version "[17.0,18.0)" -requires Microsoft.VisualStudio.Workload.NativeDesktop -property displayName 2>&1
            if ($workloads) {
                Write-Success "Desktop development with C++ workload is installed"
            }
            else {
                Write-Info "Warning: 'Desktop development with C++' workload may not be installed"
                Write-Info "If build fails, install it via Visual Studio Installer"
            }
        }
        else {
            Write-ErrorMsg "Visual Studio 2022 not found"
            Write-Host "Please install Visual Studio 2022 from: https://visualstudio.microsoft.com/downloads/"
            Write-Host "Required workload: Desktop development with C++"
            exit 1
        }
    }
    else {
        Write-Info "Warning: Could not find vswhere.exe to verify Visual Studio installation"
        Write-Info "Continuing anyway - CMake will verify Visual Studio is available"
    }
    
    # Check .NET 9 SDK (optional)
    Write-Step "Checking for .NET 9 SDK (optional)..."
    try {
        $dotnetVersion = & dotnet --version 2>&1
        if ($dotnetVersion -match "^9\.") {
            Write-Success ".NET 9 SDK found: $dotnetVersion"
        }
        else {
            Write-Info ".NET SDK found but not version 9: $dotnetVersion"
            Write-Info ".NET 9 bindings will not be available"
        }
    }
    catch {
        Write-Info ".NET SDK not found - .NET bindings will not be available"
        Write-Info "To enable C# bindings, install .NET 9 SDK from: https://dotnet.microsoft.com/download/dotnet/9.0"
    }
    
    # ============================================================================
    # Step 2: Setup vcpkg
    # ============================================================================
    Write-Header "Step 2: Setting up vcpkg for dependency management"
    
    # Check for vcpkg in project directory first
    $vcpkgProjectPath = Join-Path $ScriptDir "vcpkg"
    $vcpkgParentPath = Join-Path (Split-Path $ScriptDir) "vcpkg"
    $vcpkgPath = $null
    $vcpkgToolchainFile = $null
    
    if (Test-Path (Join-Path $vcpkgProjectPath "vcpkg.exe")) {
        $vcpkgPath = $vcpkgProjectPath
        Write-Success "Found vcpkg in project directory: $vcpkgPath"
    }
    elseif (Test-Path (Join-Path $vcpkgParentPath "vcpkg.exe")) {
        $vcpkgPath = $vcpkgParentPath
        Write-Success "Found vcpkg in parent directory: $vcpkgPath"
    }
    else {
        Write-Info "vcpkg not found - setting up in project directory..."
        
        # Clone vcpkg
        Write-Step "Cloning vcpkg repository..."
        try {
            & git clone https://github.com/microsoft/vcpkg.git $vcpkgProjectPath
            if ($LASTEXITCODE -ne 0) {
                throw "Git clone failed"
            }
            Write-Success "vcpkg cloned successfully"
            $vcpkgPath = $vcpkgProjectPath
        }
        catch {
            Write-ErrorMsg "Failed to clone vcpkg repository"
            Write-Host "Error: $_"
            exit 1
        }
        
        # Bootstrap vcpkg
        Write-Step "Bootstrapping vcpkg..."
        try {
            Push-Location $vcpkgPath
            $bootstrapScript = Join-Path $vcpkgPath "bootstrap-vcpkg.bat"
            & cmd /c $bootstrapScript
            if ($LASTEXITCODE -ne 0) {
                throw "Bootstrap failed"
            }
            Pop-Location
            Write-Success "vcpkg bootstrapped successfully"
        }
        catch {
            Pop-Location
            Write-ErrorMsg "Failed to bootstrap vcpkg"
            Write-Host "Error: $_"
            exit 1
        }
    }
    
    # Verify vcpkg executable exists
    $vcpkgExe = Join-Path $vcpkgPath "vcpkg.exe"
    if (-not (Test-Path $vcpkgExe)) {
        if ($SkipVcpkgBootstrap) {
            Write-ErrorMsg "vcpkg.exe not found at $vcpkgExe"
            exit 1
        }
        
        # Try to bootstrap
        Write-Info "vcpkg.exe not found, attempting to bootstrap..."
        try {
            Push-Location $vcpkgPath
            $bootstrapScript = Join-Path $vcpkgPath "bootstrap-vcpkg.bat"
            & cmd /c $bootstrapScript
            if ($LASTEXITCODE -ne 0) {
                throw "Bootstrap failed"
            }
            Pop-Location
            Write-Success "vcpkg bootstrapped successfully"
        }
        catch {
            Pop-Location
            Write-ErrorMsg "Failed to bootstrap vcpkg"
            Write-Host "Error: $_"
            exit 1
        }
    }
    
    # Set vcpkg toolchain file path
    $vcpkgToolchainFile = Join-Path $vcpkgPath "scripts\buildsystems\vcpkg.cmake"
    if (-not (Test-Path $vcpkgToolchainFile)) {
        Write-ErrorMsg "vcpkg toolchain file not found at: $vcpkgToolchainFile"
        exit 1
    }
    Write-Success "Using vcpkg toolchain: $vcpkgToolchainFile"
    
    # ============================================================================
    # Step 3: Generate Visual Studio Solution
    # ============================================================================
    Write-Header "Step 3: Generating Visual Studio 2022 Solution"
    
    # Create build directory
    $buildDir = Join-Path $ScriptDir "build"
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
        Write-Info "Created build directory: $buildDir"
    }
    else {
        Write-Info "Using existing build directory: $buildDir"
    }
    
    # Generate solution with CMake
    Write-Step "Running CMake to generate Visual Studio solution..."
    Write-Info "Configuration: Windows x64 with DirectX 11/12"
    Write-Info "This may take 5-15 minutes on first run (vcpkg will download and build dependencies)"
    Write-Host ""
    
    try {
        Push-Location $buildDir
        
        $cmakeArgs = @(
            "-G", "Visual Studio 17 2022",
            "-A", "x64",
            "-DCMAKE_TOOLCHAIN_FILE=$vcpkgToolchainFile",
            ".."
        )
        
        Write-Host "Executing: cmake $($cmakeArgs -join ' ')" -ForegroundColor Gray
        Write-Host ""
        
        & cmake @cmakeArgs
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake generation failed with exit code $LASTEXITCODE"
        }
        
        Pop-Location
        Write-Success "Visual Studio solution generated successfully"
    }
    catch {
        Pop-Location
        Write-ErrorMsg "Failed to generate Visual Studio solution"
        Write-Host "Error: $_"
        Write-Host ""
        Write-Host "Troubleshooting:"
        Write-Host "  1. Verify all prerequisites are installed"
        Write-Host "  2. Check that vcpkg is properly set up"
        Write-Host "  3. See BUILD.md for detailed instructions"
        exit 1
    }
    
    # Verify solution file was created
    $solutionFile = Join-Path $buildDir "FreshVoxelEngine.sln"
    if (-not (Test-Path $solutionFile)) {
        Write-ErrorMsg "Solution file was not created: $solutionFile"
        exit 1
    }
    Write-Success "Solution file created: $solutionFile"
    
    # ============================================================================
    # Step 4: Build the Project
    # ============================================================================
    if (-not $SkipBuild) {
        Write-Header "Step 4: Building FreshVoxel Engine ($BuildConfig)"
        
        Write-Info "This may take 2-5 minutes depending on your system"
        Write-Host ""
        
        try {
            $cmakeBuildArgs = @(
                "--build", $buildDir,
                "--config", $BuildConfig,
                "--", "/m"  # Use multiple processors
            )
            
            Write-Host "Executing: cmake $($cmakeBuildArgs -join ' ')" -ForegroundColor Gray
            Write-Host ""
            
            & cmake @cmakeBuildArgs
            
            if ($LASTEXITCODE -ne 0) {
                throw "Build failed with exit code $LASTEXITCODE"
            }
            
            Write-Success "Build completed successfully!"
        }
        catch {
            Write-ErrorMsg "Build failed"
            Write-Host "Error: $_"
            Write-Host ""
            Write-Host "The Visual Studio solution has been generated."
            Write-Host "You can open it manually to investigate build errors:"
            Write-Host "  $solutionFile"
            exit 1
        }
        
        # Verify executable was created
        $exePath = Join-Path $buildDir "$BuildConfig\FreshVoxelEngine.exe"
        if (Test-Path $exePath) {
            Write-Success "Executable created: $exePath"
        }
        else {
            Write-Info "Warning: Executable not found at expected location: $exePath"
        }
    }
    else {
        Write-Header "Step 4: Build Skipped"
        Write-Info "Build step was skipped as requested"
        Write-Info "You can build from Visual Studio or run:"
        Write-Info "  cmake --build build --config $BuildConfig"
    }
    
    # ============================================================================
    # Step 5: Summary and Open Visual Studio
    # ============================================================================
    Write-Header "Setup Complete!"
    
    Write-Host ""
    Write-Success "FreshVoxel Engine setup completed successfully!"
    Write-Host ""
    
    if (-not $SkipBuild) {
        Write-Host "Next steps:" -ForegroundColor Cyan
        Write-Host "  1. The Visual Studio solution will open automatically" -ForegroundColor White
        Write-Host "  2. In Solution Explorer, right-click 'FreshVoxelEngine' project" -ForegroundColor White
        Write-Host "  3. Select 'Set as Startup Project' (project name will appear bold)" -ForegroundColor White
        Write-Host "  4. Press F5 to run the engine with debugging" -ForegroundColor White
        Write-Host "     Or press Ctrl+F5 to run without debugging" -ForegroundColor White
        Write-Host ""
        Write-Host "Executable location:" -ForegroundColor Cyan
        Write-Host "  $buildDir\$BuildConfig\FreshVoxelEngine.exe" -ForegroundColor White
    }
    else {
        Write-Host "Next steps:" -ForegroundColor Cyan
        Write-Host "  1. The Visual Studio solution will open automatically" -ForegroundColor White
        Write-Host "  2. In Solution Explorer, right-click 'FreshVoxelEngine' project" -ForegroundColor White
        Write-Host "  3. Select 'Set as Startup Project'" -ForegroundColor White
        Write-Host "  4. Press F7 to build the solution" -ForegroundColor White
        Write-Host "  5. Press F5 to run the engine" -ForegroundColor White
    }
    
    Write-Host ""
    Write-Host "Documentation:" -ForegroundColor Cyan
    Write-Host "  - Build instructions: BUILD.md" -ForegroundColor White
    Write-Host "  - Getting started: GETTING_STARTED.md" -ForegroundColor White
    Write-Host "  - Controls: CONTROLS.md" -ForegroundColor White
    Write-Host "  - Documentation index: DOCUMENTATION_INDEX.md" -ForegroundColor White
    Write-Host ""
    
    # Open Visual Studio
    if ($OpenVS) {
        Write-Step "Opening Visual Studio 2022..."
        try {
            Start-Process $solutionFile
            Write-Success "Visual Studio is opening with the solution"
            Write-Info "The build automation is now complete. You're in control!"
        }
        catch {
            Write-ErrorMsg "Failed to open Visual Studio automatically"
            Write-Host "Please open the solution manually:"
            Write-Host "  $solutionFile"
        }
    }
    else {
        Write-Info "Visual Studio not opened automatically (OpenVS = false)"
        Write-Host "To open the solution manually:"
        Write-Host "  $solutionFile"
    }
    
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host " Setup and build completed successfully!" -ForegroundColor Green
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host ""
    
    Stop-Transcript
}
catch {
    Write-Host ""
    Write-ErrorMsg "An unexpected error occurred"
    Write-Host "Error: $_"
    Write-Host "Stack Trace: $($_.ScriptStackTrace)"
    Write-Host ""
    Write-Host "For help, see BUILD.md or open an issue at:"
    Write-Host "https://github.com/shifty81/fresh/issues"
    try { Stop-Transcript } catch { }
    exit 1
}
