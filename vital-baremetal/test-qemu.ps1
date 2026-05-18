# test-qemu.ps1
param(
    [switch]$Interactive = $false,
    [switch]$SkipBuild   = $false,
    [int]$TimeoutSec     = 60
)

$ErrorActionPreference = "Continue"

$ROOT     = Split-Path -Parent $MyInvocation.MyCommand.Path
$QEMU     = "C:\Program Files\qemu\qemu-system-x86_64.exe"
$OVMF_SRC = "C:\Program Files\qemu\share\edk2-x86_64-code.fd"
$OVMF     = "$ROOT\ovmf.fd"
if (-not (Test-Path $OVMF)) {
    Copy-Item -Force $OVMF_SRC $OVMF
}
$FAT_DIR  = "$ROOT\fat_dir"
$BOOT_DIR = "$FAT_DIR\EFI\BOOT"
$SERIAL   = "$ROOT\vital-serial.log"

function Title  { param($s) Write-Host "`n-- $s --" -ForegroundColor Cyan }
function Ok     { param($s) Write-Host "  OK $s" -ForegroundColor Green }
function Warn   { param($s) Write-Host "  WARN $s" -ForegroundColor Yellow }
function Err    { param($s) Write-Host "  ERR $s" -ForegroundColor Red }
function Info   { param($s) Write-Host "  INFO $s" -ForegroundColor Gray }

if (-not $SkipBuild) {
    Title "1/3 Build"
    $wslPath = ($ROOT -replace '\\','/') -replace 'C:','/mnt/c'
    $cmd = "cd '$wslPath' && make clean && make -j4 2>&1"
    $buildResult = wsl -e bash -c $cmd
    Write-Host $buildResult -ForegroundColor Gray
    
    if (Test-Path "$ROOT\vital.efi") {
        Ok "vital.efi build OK"
    } else {
        Err "vital.efi build failed"
        exit 1
    }
} else {
    Title "1/3 Build (skip)"
}

Title "2/3 Fat Drive"
if (-not (Test-Path $BOOT_DIR)) {
    New-Item -ItemType Directory -Force -Path $BOOT_DIR | Out-Null
}
Copy-Item -Force "$ROOT\vital.efi" "$BOOT_DIR\BOOTX64.EFI"
Ok "vital.efi -> BOOTX64.EFI"

Title "3/3 QEMU"
if (Test-Path $SERIAL) { Remove-Item $SERIAL -Force }

$qemu_args = @(
    "-machine", "q35,accel=tcg",
    "-cpu", "max",
    "-m", "512",
    "-drive", "if=pflash,format=raw,readonly=on,file=$OVMF",
    "-drive", "file=fat:rw:$FAT_DIR,format=raw,media=disk",
    "-serial", "file:$SERIAL",
    "-no-reboot"
)

if ($Interactive) {
    $qemu_args += @("-vga", "std")
} else {
    $qemu_args += @("-display", "none")
}

$proc = Start-Process -FilePath $QEMU -ArgumentList $qemu_args -PassThru -NoNewWindow
$deadline = (Get-Date).AddSeconds($TimeoutSec)
while (-not $proc.HasExited -and (Get-Date) -lt $deadline) {
    Start-Sleep -Milliseconds 500
    $lines = 0
    if (Test-Path $SERIAL) { $lines = (Get-Content $SERIAL -ea SilentlyContinue).Count }
    Write-Host -NoNewline "`r  Waiting... | UART: $lines lines  "
}
Write-Host ""
if (-not $proc.HasExited) {
    Warn "Timeout, killing QEMU"
    Stop-Process -Id $proc.Id -ea SilentlyContinue
}
$exitCode = $proc.ExitCode

Write-Host "`n-- Results --"
if (Test-Path $SERIAL) {
    $content = Get-Content $SERIAL
    Ok ("UART: " + $content.Count + " lines")
    $content | Select-Object -Last 30
    $pulseLines = ($content | Where-Object { $_ -match "Pulse:" }).Count
    if ($pulseLines -gt 0) {
        Ok "Organism pulsed $pulseLines times. IT IS ALIVE."
    }
} else {
    Warn "No serial log"
}
exit $exitCode
