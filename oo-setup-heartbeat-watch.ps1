# oo-setup-heartbeat-watch.ps1
# Suite 2 : auto-démarrage heartbeat watch sur PC principal
# Lance oo-host heartbeat watch au démarrage de Windows
# Usage: .\oo-setup-heartbeat-watch.ps1 [-ColonyUrl http://192.168.1.8:8080] [-IntervalS 15]

param(
    [string]$ColonyUrl   = "http://192.168.1.8:8080",
    [int]   $IntervalS   = 15,
    [string]$OoHostExe   = "$env:USERPROFILE\OneDrive\Bureau\baremetal\oo-host\target\debug\oo-host.exe",
    [string]$WorkDir     = "$env:USERPROFILE\OneDrive\Bureau\baremetal\oo-host",
    [switch]$Uninstall
)

$TaskName = "OO-HeartbeatWatch"

if ($Uninstall) {
    Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false -ErrorAction SilentlyContinue
    Write-Host "[oo-heartbeat] Task '$TaskName' removed." -ForegroundColor Yellow
    exit 0
}

# Vérifier que l'exe existe
if (-not (Test-Path $OoHostExe)) {
    Write-Error "[oo-heartbeat] oo-host.exe introuvable: $OoHostExe"
    Write-Host "[oo-heartbeat] Build d'abord: cd $WorkDir && cargo build --bin oo-host"
    exit 1
}

# Supprimer tâche existante si elle existe
Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false -ErrorAction SilentlyContinue

# Créer la tâche planifiée
$arg = "heartbeat watch --colony $ColonyUrl --interval-s $IntervalS --cycles 0 --continue-on-error"
$action    = New-ScheduledTaskAction -Execute $OoHostExe -Argument $arg -WorkingDirectory $WorkDir
$trigger   = New-ScheduledTaskTrigger -AtLogOn
$settings  = New-ScheduledTaskSettingsSet -ExecutionTimeLimit 0 -RestartCount 3 -RestartInterval (New-TimeSpan -Minutes 1)
$principal = New-ScheduledTaskPrincipal -UserId $env:USERNAME -RunLevel Highest -LogonType Interactive

Register-ScheduledTask -TaskName $TaskName -Action $action -Trigger $trigger -Settings $settings -Principal $principal

Write-Host ""
Write-Host "[oo-heartbeat] ✅ Tâche '$TaskName' créée." -ForegroundColor Green
Write-Host "[oo-heartbeat]    Déclencheur : au login de l'utilisateur"
Write-Host "[oo-heartbeat]    Colony URL  : $ColonyUrl"
Write-Host "[oo-heartbeat]    Intervalle  : ${IntervalS}s"
Write-Host ""
Write-Host "[oo-heartbeat]    Lancer maintenant (sans redémarrer) :"
Write-Host "    Start-ScheduledTask -TaskName '$TaskName'"
