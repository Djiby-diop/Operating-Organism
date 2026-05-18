# test-qemu.ps1 — Test Bot-Baremetal (Swarm Mind) in QEMU UEFI
# Usage: .\test-qemu.ps1

$ErrorActionPreference = "Stop"

$QEMU = "C:\Program Files\qemu\qemu-system-x86_64.exe"
$OVMF = "C:\Program Files\qemu\share\edk2-x86_64-code.fd"

if (-not (Test-Path $QEMU)) {
    Write-Error "QEMU not found at $QEMU"
}
if (-not (Test-Path $OVMF)) {
    Write-Error "OVMF not found at $OVMF"
}

# Ensure the Rust EFI is built
Write-Host "==> Building Swarm Mind for UEFI..." -ForegroundColor Cyan
cd immune
cargo build --bin swarm_mind --target x86_64-unknown-uefi
cd ..

$EFI_FILE = "immune\target\x86_64-unknown-uefi\debug\swarm_mind.efi"
if (-not (Test-Path $EFI_FILE)) {
    Write-Error "Build failed or EFI file not found: $EFI_FILE"
}

# Create FAT structure
$FAT_DIR = "fat_dir"
$BOOT_DIR = "$FAT_DIR\EFI\BOOT"
if (-not (Test-Path $BOOT_DIR)) {
    New-Item -ItemType Directory -Force -Path $BOOT_DIR | Out-Null
}

Write-Host "==> Preparing UEFI Boot Drive..." -ForegroundColor Gray
Copy-Item -Force $EFI_FILE "$BOOT_DIR\BOOTX64.EFI"

# Build QEMU args
$qemu_args = @(
    "-machine", "q35,accel=tcg",
    "-m", "512",
    "-drive", "if=pflash,format=raw,readonly=on,file=$OVMF",
    "-drive", "file=fat:rw:$FAT_DIR,format=raw,media=disk",
    "-display", "none",
    "-serial", "file:bot-serial.log",
    "-no-reboot"
)

Write-Host "==> Starting QEMU (Press Ctrl+A then X to exit)..." -ForegroundColor Green
& $QEMU @qemu_args
$exit_code = $LASTEXITCODE

Write-Host ""
Write-Host "==> QEMU exited (code: $exit_code)" -ForegroundColor $(if ($exit_code -eq 0) { "Green" } else { "Yellow" })
exit $exit_code
