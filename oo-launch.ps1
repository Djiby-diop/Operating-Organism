# oo-launch.ps1 — Script d'automatisation master OO
# =====================================================================
# Lie TOUS les composants du projet :
#   - Build EFI (optionnel, via WSL make)
#   - Reconstruction image boot GPT (optionnel)
#   - Sélection modèle (stories15M / cortex_oo / diop_model / diop_architect)
#   - Lancement QEMU (headless ou GTK interactif)
#   - Lancement SOMA Desktop HUD (preview_hud.py)
#
# Usage:
#   .\oo-launch.ps1                           # QEMU headless + HUD
#   .\oo-launch.ps1 -Interactive              # QEMU GTK + HUD
#   .\oo-launch.ps1 -NoKeurgui                # ne lance pas le gate premiere vie Keurgui
#   .\oo-launch.ps1 -Model cortex_oo          # utiliser cortex_oo.bin
#   .\oo-launch.ps1 -Model diop_model         # utiliser diop_model.bin (79MB)
#   .\oo-launch.ps1 -Rebuild                  # recompile EFI + reconstruit image
#   .\oo-launch.ps1 -RebuildImage             # reconstruit image seulement
#   .\oo-launch.ps1 -NoHUD                    # QEMU seulement, sans HUD pygame
# =====================================================================

param(
    [switch]$Interactive    = $false,
    [switch]$Rebuild        = $false,
    [switch]$RebuildImage   = $false,
    [switch]$NoHUD          = $false,
    [switch]$NoKeurgui      = $false,
    [ValidateSet("stories15M","cortex_oo","diop_model","diop_architect","stories110M")]
    [string]$Model          = "stories15M",
    [int]$Nodes             = 1,         # Nombre de nœuds dans l'essaim
    [int]$TimeoutSec        = 120
)

Set-StrictMode -Off
$ErrorActionPreference = "Continue"

$ROOT    = "C:\Users\djibi\OneDrive\Bureau\baremetal"
$LLMROOT = "$ROOT\llm-baremetal"
$DESKTOP = "$ROOT\desktop_display"
$IMGPATH = "$ROOT\llm-baremetal-boot.img"
$QEMU    = "C:\Program Files\qemu\qemu-system-x86_64.exe"
$OVMF    = "C:\Program Files\qemu\share\edk2-x86_64-code.fd"
$VARS    = "$env:TEMP\edk2-x86_64-oo-vars.fd"
$UART    = "$ROOT\OO_UART.log"
$VOICE_JSON = "$ROOT\desktop_display\oo_voice_state.json"
$KEURGUI_GATE = "$ROOT\faceApp\scripts\keurgui_first_life_gate.ps1"

# ── Couleurs terminal ──────────────────────────────────────────────────────────
function Title  { param($s) Write-Host "`n== $s ==" -ForegroundColor Cyan }
function Ok     { param($s) Write-Host "  [OK] $s" -ForegroundColor Green }
function Warn   { param($s) Write-Host "  [!]  $s" -ForegroundColor Yellow }
function Err    { param($s) Write-Host "  [X]  $s" -ForegroundColor Red }
function Info   { param($s) Write-Host "  . $s" -ForegroundColor Gray }

# ── Répertoire de travail ──────────────────────────────────────────────────────
Set-Location $ROOT
Title "OO Launch — Operating Organism"
Info "Root    : $ROOT"
Info "Model   : $Model"
Info "Mode    : $(if ($Interactive) {'Interactive (GTK)'} else {'Headless'})"

# ── Gate premiere vie Keurgui ─────────────────────────────────────────────────
if (-not $NoKeurgui -and (Test-Path $KEURGUI_GATE)) {
    Title "0/5  Keurgui First-Life Gate"
    if ($NoHUD) {
        & $KEURGUI_GATE -NoScreen
    } else {
        & $KEURGUI_GATE
    }
} elseif (-not $NoKeurgui) {
    Warn "Gate Keurgui introuvable: $KEURGUI_GATE"
}

# ── 1. Sélection modèle ────────────────────────────────────────────────────────
Title "1/5  Sélection du modèle"
$MODEL_FILES = @{
    "stories15M"    = "$LLMROOT\bootable_img_temp\stories15M.bin"
    "stories110M"   = "$LLMROOT\stories110M.bin"
    "cortex_oo"     = "$LLMROOT\oo-model-repo\models\cortex_oo.bin"
    "diop_model"    = "$LLMROOT\diop\engine\model\diop_model.bin"
    "diop_architect"= "$LLMROOT\diop\engine\model\diop_architect.bin"
}
$MODEL_PATH = $MODEL_FILES[$Model]

# Fallback si fichier absent
if (-not (Test-Path $MODEL_PATH)) {
    Warn "Modèle '$Model' introuvable : $MODEL_PATH"
    # fallback sur le premier stories15M trouvé
    $fallback = Get-ChildItem -Path $ROOT -Recurse -Filter "stories15M.bin" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($fallback) {
        $MODEL_PATH = $fallback.FullName
        Warn "Fallback → $MODEL_PATH"
    } else {
        Err "Aucun modèle trouvable. Arrêt."
        exit 1
    }
}
$modelSizeMB = [math]::Round((Get-Item $MODEL_PATH).Length / 1MB, 1)
Ok "Modèle sélectionné : $(Split-Path $MODEL_PATH -Leaf)  ($modelSizeMB MB)"

# ── 2. Build EFI (optionnel) ───────────────────────────────────────────────────
if ($Rebuild -or $RebuildImage) {
    Title "2/5  Build EFI"
    if ($Rebuild) {
        Info "Compilation via WSL make..."
        $wsl = wsl -e bash -c "cd /mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal && make -j4 2>&1 | tail -20"
        Write-Host $wsl
        if ($LASTEXITCODE -eq 0) { Ok "Build réussi" }
        else { Warn "Build terminé avec avertissements (code $LASTEXITCODE)" }
    } else {
        Info "(skip build — utilisez -Rebuild pour recompiler)"
    }

    Title "3/5  Reconstruction image boot"
    Info "make-boot-img.sh via WSL..."
    $wsl2 = wsl -e bash -c "cd /mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal && bash tools/scripts/make-boot-img.sh 2>&1 | tail -20"
    Write-Host $wsl2
    if ($LASTEXITCODE -eq 0) { Ok "Image reconstruite : $IMGPATH" }
    else { Warn "Reconstruction avec avertissements" }
} else {
    Title "2-3/5  Build (skip)"
    Info "Utilisez -Rebuild ou -RebuildImage pour reconstruire"
    $item = Get-Item $IMGPATH -ea SilentlyContinue
    if ($item) {
        $sz = [math]::Round($item.Length / 1MB, 0)
        Ok "Image existante : $sz MB"
    }
}

# ── 4. Patch image avec le modèle sélectionné (si pas stories15M par défaut) ──
Title "4/5  Préparation image"
if ($Model -ne "stories15M" -and (Test-Path $IMGPATH)) {
    Info "Patch image : injection du modèle $Model..."
    # Monte l'image FAT32 via WSL et remplace le fichier modèle
    $wslImgPath = $IMGPATH -replace '\\','/' -replace 'C:','/mnt/c'
    $wslModelPath = $MODEL_PATH -replace '\\','/' -replace 'C:','/mnt/c'
    $modelBasename = [System.IO.Path]::GetFileName($MODEL_PATH)
    $wslPatch = @"
set -e
TMPDIR=/tmp/oo_img_mnt
sudo mkdir -p \$TMPDIR
# Detect partition offset (FAT32 starts at 1MB = 2048 sectors * 512)
sudo mount -o loop,offset=1048576 '$wslImgPath' \$TMPDIR 2>/dev/null || \
  sudo mount -o loop '$wslImgPath' \$TMPDIR
sudo cp '$wslModelPath' \$TMPDIR/model.bin
echo "model.bin => \$(ls -lh \$TMPDIR/model.bin | awk '{print \$5}')"
sudo umount \$TMPDIR
echo "PATCH OK"
"@
    $patchResult = wsl -e bash -c $wslPatch 2>&1
    Write-Host $patchResult -ForegroundColor Gray
    if ($patchResult -match "PATCH OK") {
        Ok "Modèle injecté dans l'image"
    } else {
        Warn "Patch partiel — l'image utilisera le modèle déjà présent"
    }
} else {
    Info "Utilisation du modèle déjà dans l'image boot"
}

# ── 5a. Lancement SOMA HUD ─────────────────────────────────────────────────────
$hudProcess = $null
if (-not $NoHUD) {
    Title "5/5  SOMA Desktop HUD"
    $python = Get-Command python -ea SilentlyContinue
    if (-not $python) { $python = Get-Command python3 -ea SilentlyContinue }

    if ($python -and (Test-Path "$DESKTOP\preview_hud.py")) {
        Info "Lancement preview_hud.py..."
        # Crée le json de bridge vide si absent
        if (-not (Test-Path $VOICE_JSON)) {
            '{"emotion":"NEUTRAL","inference":false,"voice_active":false,"voice_wave":[],"warden_pressure":0,"tokens_per_sec":0,"input_buf":"","response_lines":[]}' | 
                Set-Content $VOICE_JSON -Encoding UTF8
        }
        $pinfo = New-Object System.Diagnostics.ProcessStartInfo
        $pinfo.FileName = $python.Source
        $pinfo.Arguments = "`"$DESKTOP\preview_hud.py`""
        $pinfo.WorkingDirectory = $DESKTOP
        $pinfo.UseShellExecute = $false
        $hudProcess = [System.Diagnostics.Process]::Start($pinfo)
        Ok "SOMA HUD lancé (PID $($hudProcess.Id))"
    } else {
        Warn "python ou preview_hud.py introuvable — HUD désactivé"
    }
}

# ── 5b. Lancement de l'Essaim (Swarm Mode) ────────────────────────────────────
Title "5/5  Essaim OO ($Nodes nœuds)"

$qemuProcesses = @()
$UART_LOGS = @()

# Dossier temporaire pour l'essaim
$SWARM_DIR = "$ROOT\swarm_temp"
if (-not (Test-Path $SWARM_DIR)) { New-Item -ItemType Directory -Path $SWARM_DIR -Force | Out-Null }

for ($i = 1; $i -le $Nodes; $i++) {
    $NODE_ID = "NODE_$i"
    $NODE_WORK_DIR = "$SWARM_DIR\$NODE_ID"
    $NODE_UART = "$ROOT\OO_UART_$NODE_ID.log"
    $NODE_VARS = "$env:TEMP\edk2-x86_64-oo-vars-$NODE_ID.fd"
    $UART_LOGS += $NODE_UART
    
    Info "Initialisation $NODE_ID..."
    if (-not (Test-Path $NODE_WORK_DIR)) { New-Item -ItemType Directory -Path $NODE_WORK_DIR -Force | Out-Null }
    if (Test-Path $NODE_UART) { Remove-Item $NODE_UART -Force }
    
    # Isolation de l'image de boot pour chaque nœud
    $NODE_IMG = "$NODE_WORK_DIR\boot.img"
    Copy-Item $IMGPATH $NODE_IMG -Force
    
    # OVMF vars par nœud
    if (-not (Test-Path $NODE_VARS)) {
        Copy-Item $OVMF $NODE_VARS -Force
    }

    $args_node = @(
        "-machine", "q35,accel=tcg",
        "-cpu",     "max",
        "-m",       "2048",
        "-drive",   "if=pflash,format=raw,readonly=on,file=$OVMF",
        "-drive",   "if=pflash,format=raw,file=$NODE_VARS",
        "-drive",   "format=raw,file=$NODE_IMG,if=ide",
        "-serial",  "file:$NODE_UART",
        "-name",    "OO_$NODE_ID",
        "-no-reboot"
    )

    if ($Interactive) {
        $args_node += @("-vga", "std")
    } else {
        $args_node += @("-display", "none")
    }

    $p = Start-Process -FilePath $QEMU -ArgumentList $args_node -PassThru -NoNewWindow
    $qemuProcesses += $p
    Ok "$NODE_ID lancé (PID $($p.Id))"
    Start-Sleep -Seconds 3 # Décalage pour éviter la saturation CPU au boot
}

Write-Host "`n  Essaim actif. Appuyez sur Ctrl+C pour dissoudre l'essaim." -ForegroundColor Yellow

# Surveillance de l'essaim
try {
    while ($true) {
        $running = $qemuProcesses | Where-Object { -not $_.HasExited }
        if ($running.Count -eq 0) { break }
        
        # Petit feedback sur les logs
        $totalLines = 0
        foreach($log in $UART_LOGS) {
            if (Test-Path $log) { $totalLines += (Get-Content $log).Count }
        }
        Write-Host -NoNewline "`r  Essaim en ligne ($($running.Count)/$Nodes) | Total Logs: $totalLines lignes   "
        
        Start-Sleep -Seconds 1
    }
} catch {
    Info "Interruption utilisateur reçue."
} finally {
    Title "Dissolution de l'Essaim"
    foreach ($p in $qemuProcesses) {
        if (-not $p.HasExited) {
            Info "Arrêt de $($p.ProcessName) (PID $($p.Id))..."
            Stop-Process -Id $p.Id -Force -ea SilentlyContinue
        }
    }
    
    if ($hudProcess -and -not $hudProcess.HasExited) {
        Info "Arrêt SOMA HUD..."
        Stop-Process -Id $hudProcess.Id -ea SilentlyContinue
    }
}

Write-Host ""
Write-Host "══════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  OO Swarm Session terminée" -ForegroundColor Cyan
Write-Host "  Nœuds lancés   : $Nodes"
Write-Host "  Modèle utilisé : $Model"
Write-Host "══════════════════════════════════════════════════" -ForegroundColor Cyan
