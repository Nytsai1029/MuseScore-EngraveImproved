# Runs the freshly built app under a debugger and checks that it survives startup.
# If it crashes, the call stack (with symbols from the build tree) is printed to the job log
# and saved together with the app's own logs to the output directory.
#
# Exit code: 0 - the app was still running after the timeout
#            1 - the app crashed or exited during startup
#            2 - the check itself could not be run

param(
    [string]$InstallDir = "build.install",
    [string]$SymbolsDir = "build.release",
    [string]$OutDir = "build.startup_check",
    [int]$TimeoutSec = 90
)

$exe = Join-Path $InstallDir "bin\MuseScore4.exe"
if (-not (Test-Path $exe)) {
    Write-Host "::error::Startup check: not found $exe"
    exit 2
}
$exe = (Resolve-Path $exe).Path

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$OutDir = (Resolve-Path $OutDir).Path

$debuggerOutput = Join-Path $OutDir "debugger_output.txt"
$debuggerErrors = Join-Path $OutDir "debugger_errors.txt"
$summary = Join-Path $OutDir "summary.txt"

# ============================
# Find the debugger
# ============================
$cdbCandidates = @(
    "${env:ProgramFiles(x86)}\Windows Kits\10\Debuggers\x64\cdb.exe",
    "$env:ProgramFiles\Windows Kits\10\Debuggers\x64\cdb.exe"
)

$cdb = $cdbCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $cdb) {
    Write-Host "cdb.exe not found, installing Debugging Tools for Windows..."
    choco install -y --no-progress windows-sdk-10-version-2004-windbg | Out-Host
    $cdb = $cdbCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

# ============================
# Run
# ============================
$timedOut = $false

if ($cdb) {
    Write-Host "Debugger: $cdb"

    # Stop at the first access violation, print the stacks and quit.
    # The lines after "g" are executed if the process stops for any other reason (e.g. it exits).
    $cdbScript = Join-Path $OutDir "startup_check.cdb"
    @'
.lines -e
sxe -c ".echo ===== ACCESS VIOLATION =====; r; .echo ===== STACK OF THE FAULTING THREAD =====; k 80; .echo ===== ALL THREADS =====; ~*k 25; .echo ===== END =====; q" av
g
.echo ===== STOPPED WITHOUT ACCESS VIOLATION =====
k 40
q
'@ | Set-Content -Path $cdbScript -Encoding ascii

    $symbolsPath = (Resolve-Path $SymbolsDir).Path
    $arguments = "-cf `"$cdbScript`" -lines -y `"$symbolsPath`" `"$exe`""

    $process = Start-Process -FilePath $cdb -ArgumentList $arguments -NoNewWindow -PassThru `
        -RedirectStandardOutput $debuggerOutput -RedirectStandardError $debuggerErrors
} else {
    Write-Host "::warning::Startup check: no debugger available, running the app without it (no call stack on a crash)"
    "(no debugger)" | Set-Content -Path $debuggerOutput

    $process = Start-Process -FilePath $exe -PassThru
}

if (-not $process.WaitForExit($TimeoutSec * 1000)) {
    $timedOut = $true

    Get-Process -Name "MuseScore4" -ErrorAction SilentlyContinue | Stop-Process -Force
    if (-not $process.WaitForExit(20000)) {
        $process | Stop-Process -Force -ErrorAction SilentlyContinue
    }
}

# ============================
# Collect the app's own logs
# ============================
$appLogsDir = Join-Path $OutDir "app_logs"
New-Item -ItemType Directory -Force -Path $appLogsDir | Out-Null

$userDataRoot = Join-Path $env:LOCALAPPDATA "MuseScore"
if (Test-Path $userDataRoot) {
    Get-ChildItem -Path $userDataRoot -Recurse -Filter "*.log" -ErrorAction SilentlyContinue | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination (Join-Path $appLogsDir ($_.Directory.Parent.Name + "_" + $_.Name)) -Force
    }
}

# ============================
# Report
# ============================
$output = ""
if (Test-Path $debuggerOutput) {
    $output = Get-Content -Path $debuggerOutput -Raw
}

$appLogCrashed = $false
Get-ChildItem -Path $appLogsDir -Filter "*.log" -ErrorAction SilentlyContinue | ForEach-Object {
    if (Select-String -Path $_.FullName -Pattern "Application crashed with signal" -Quiet) {
        $appLogCrashed = $true
    }
}

$crashed = ($output -match "===== ACCESS VIOLATION =====") -or $appLogCrashed

if ($crashed) {
    $result = "CRASHED during startup"
    $exitCode = 1
} elseif ($timedOut) {
    $result = "OK: still running after $TimeoutSec seconds"
    $exitCode = 0
} else {
    $result = "EXITED during startup (no access violation)"
    $exitCode = 1
}

Write-Host "================ debugger output ================"
Write-Host $output
Write-Host "================ last app log ================"
$lastAppLog = Get-ChildItem -Path $appLogsDir -Filter "*.log" -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -notmatch "audiopluginregistration" } |
    Sort-Object LastWriteTime | Select-Object -Last 1
if ($lastAppLog) {
    Get-Content -Path $lastAppLog.FullName -Tail 40 | Write-Host
}
Write-Host "================ result ================"
Write-Host "Startup check: $result"

"Startup check: $result" | Set-Content -Path $summary

if ($env:GITHUB_STEP_SUMMARY) {
    "### Startup check: $result" | Add-Content -Path $env:GITHUB_STEP_SUMMARY
}

if ($exitCode -ne 0) {
    Write-Host "::error::Startup check: $result (see the debugger output in this step and the startup check artifact)"
}

exit $exitCode
