# Script d'intégration : Entraînement DIOP -> Validation FFI -> Déploiement Baremetal
# Exécute ce script pour générer et déployer le cerveau (modèle) local.

$ErrorActionPreference = "Stop"
$DiopDir = "llm-baremetal\diop"
$ModelName = "diop_model.bin"

Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host "  OO-DIOP : Cycle de Vie de l'Intelligence Baremetal   " -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan

# 1. Entraînement du modèle via DIOP
Write-Host "`n[1/3] Entraînement du modèle local via DIOP..." -ForegroundColor Yellow
Push-Location $DiopDir
python -m diop train --profile micro
if ($LASTEXITCODE -ne 0) {
    Write-Host "Erreur lors de l'entraînement." -ForegroundColor Red
    Pop-Location
    exit $LASTEXITCODE
}
Pop-Location

# 2. Validation avec l'Engine C natif (FFI Bridge)
Write-Host "`n[2/3] Validation de l'inférence via le pont FFI C..." -ForegroundColor Yellow
Push-Location $DiopDir
# On simule un "worker" pour vérifier que le C natif sait exécuter le modèle
python -m diop run "Vérification système" --adapter trained
if ($LASTEXITCODE -ne 0) {
    Write-Host "Erreur lors de la validation FFI." -ForegroundColor Red
    Pop-Location
    exit $LASTEXITCODE
}
Pop-Location

# 3. Déploiement vers la clé USB UEFI (Dossier de test)
Write-Host "`n[3/3] Déploiement du modèle pour le noyau Baremetal..." -ForegroundColor Yellow
$TargetModelPath = "models\$ModelName"
if (-Not (Test-Path "models")) {
    New-Item -ItemType Directory -Path "models" | Out-Null
}

Copy-Item -Path "$DiopDir\engine\model\$ModelName" -Destination $TargetModelPath -Force
Write-Host "OK: Modèle copié vers $TargetModelPath" -ForegroundColor Green

Write-Host "`n=======================================================" -ForegroundColor Cyan
Write-Host " SUCCÈS ! Le cerveau est prêt pour le boot UEFI." -ForegroundColor Green
Write-Host " Lors du prochain démarrage, configure 'model=models/$ModelName' dans repl.cfg." -ForegroundColor Green
Write-Host "=======================================================" -ForegroundColor Cyan
