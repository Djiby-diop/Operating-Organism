param(
    [string]$Mode = "normal"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Push-Location $PSScriptRoot

try {
    Write-Host "Building oo-sim..."
    Push-Location ".\oo-sim"
    if (Test-Path ".\oo-sim.exe") { Remove-Item ".\oo-sim.exe" -ErrorAction SilentlyContinue }
    & cc -Wall -Wextra -O2 -std=c11 -o oo-sim.exe src/oo_sim_main.c
    Pop-Location

    Write-Host "Building oo-lab..."
    Push-Location ".\oo-lab"
    if (Test-Path ".\oo-lab.exe") { Remove-Item ".\oo-lab.exe" -ErrorAction SilentlyContinue }
    & cc -Wall -Wextra -O2 -std=c11 -o oo-lab.exe src/oo_lab_main.c
    Pop-Location

    # Clear previous sim log for a clean run
    if (Test-Path ".\oo-sim\OOSIM.LOG") {
        Remove-Item ".\oo-sim\OOSIM.LOG" -ErrorAction SilentlyContinue
    }

    Write-Host "Running oo-sim in mode '$Mode'..."
    Push-Location ".\oo-sim"
    if ($Mode) {
        .\oo-sim.exe $Mode
    } else {
        .\oo-sim.exe
    }
    Pop-Location

    Write-Host ""
    Write-Host "oo-lab sim summary (from OOSIM.LOG):"
    Push-Location ".\oo-lab"
    # Run from oo-lab dir but point to the log in ../oo-sim
    if (Test-Path "..\oo-sim\OOSIM.LOG") {
        .\oo-lab.exe --sim-summary 2>$null
    } else {
        Write-Host "No OOSIM.LOG found in ../oo-sim"
    }
    Pop-Location
}
finally {
    Pop-Location
}

