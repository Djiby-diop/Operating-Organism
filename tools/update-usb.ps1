#Requires -RunAsAdministrator
<#
.SYNOPSIS
    OO USB Quick Update — copies new llama2.efi to an existing OO USB stick.
    No repartitioning. Much faster than make-usb-installer.ps1.

.PARAMETER Drive
    Drive letter of the OO USB (e.g. "E"). Auto-detected if omitted.

.EXAMPLE
    .\update-usb.ps1          # auto-detect OO-BOOT volume
    .\update-usb.ps1 -Drive E  # explicit drive letter
#>
param([string]$Drive = "")

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$REPO_ROOT  = Split-Path $PSScriptRoot -Parent
$EFI_SOURCE = Join-Path $REPO_ROOT "llm-baremetal\llama2.efi"

if (-not (Test-Path $EFI_SOURCE)) {
    Write-Host "[ERROR] llama2.efi not found. Run 'make' first." -ForegroundColor Red; exit 1
}

# Auto-detect drive with label OO-BOOT
if ($Drive -eq "") {
    $vol = Get-Volume | Where-Object { $_.FileSystemLabel -eq "OO-BOOT" } | Select-Object -First 1
    if (-not $vol) {
        Write-Host "[ERROR] No volume with label 'OO-BOOT' found. Specify -Drive manually." -ForegroundColor Red; exit 1
    }
    $Drive = $vol.DriveLetter
    Write-Host "[Auto] Found OO-BOOT on drive $Drive:" -ForegroundColor Cyan
}

$dest = "${Drive}:\EFI\BOOT\BOOTX64.EFI"
if (-not (Test-Path "${Drive}:\EFI\BOOT")) {
    Write-Host "[ERROR] ${Drive}: doesn't look like an OO USB (missing EFI\BOOT)." -ForegroundColor Red; exit 1
}

$size = [math]::Round((Get-Item $EFI_SOURCE).Length / 1MB, 1)
Write-Host "Updating OO USB on ${Drive}: ..." -ForegroundColor Yellow
Write-Host "  Source : $EFI_SOURCE ($size MB)" -ForegroundColor DarkGray

Copy-Item $EFI_SOURCE $dest -Force
Write-Host "  Dest   : $dest" -ForegroundColor DarkGray

# Also update oo.cfg if changed
$cfg_src = Join-Path $REPO_ROOT "USB-BOOT-FILES\oo.cfg"
if (Test-Path $cfg_src) {
    Copy-Item $cfg_src "${Drive}:\OO\config\oo.cfg" -Force
    Write-Host "  Config : oo.cfg updated" -ForegroundColor DarkGray
}

Write-Host ""
Write-Host "✅ OO USB updated — $size MB EFI on ${Drive}:" -ForegroundColor Green
Write-Host "   Safely eject and boot on target PC." -ForegroundColor White
