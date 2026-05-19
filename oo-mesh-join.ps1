# oo-mesh-join.ps1
# Suite 3 : enregistrer ce PC comme peer dans le colony-server
# et vérifier le statut du mesh gossip
# Usage: .\oo-mesh-join.ps1 [-ColonyUrl http://192.168.1.8:8080] [-MyUrl http://192.168.1.X:8080]

param(
    [string]$ColonyUrl = "http://192.168.1.8:8080",
    [string]$MyUrl     = "",   # laisser vide pour auto-détection
    [string]$Role      = "organism_host"
)

# Auto-détecter l'IP LAN si $MyUrl vide (préfère 192.168.x.x ou 10.x.x.x, ignore WSL 172.x)
if ([string]::IsNullOrEmpty($MyUrl)) {
    $ip = (Get-NetIPAddress -AddressFamily IPv4 | Where-Object {
        ($_.IPAddress -match '^192\.168\.' -or $_.IPAddress -match '^10\.') -and
        $_.PrefixOrigin -ne 'WellKnown'
    } | Select-Object -First 1).IPAddress

    if ($ip) {
        $MyUrl = "http://${ip}:8080"
        Write-Host "[oo-mesh] Auto-détecté MyUrl = $MyUrl"
    } else {
        Write-Error "[oo-mesh] Impossible de détecter l'IP LAN locale. Passe -MyUrl manuellement (ex: -MyUrl http://192.168.1.5:8080)."
        exit 1
    }
}

Write-Host ""
Write-Host "[oo-mesh] Enregistrement comme peer dans la colonie..." -ForegroundColor Cyan
Write-Host "[oo-mesh]   Colony  : $ColonyUrl"
Write-Host "[oo-mesh]   Ce node : $MyUrl (role=$Role)"
Write-Host ""

# 1. Enregistrer ce PC comme peer
# PeerRegistration { peer_id, address, role }
$PeerId = [System.Guid]::NewGuid().ToString()
$body = @{
    peer_id = $PeerId
    address = $MyUrl
    role    = $Role
} | ConvertTo-Json

try {
    $resp = Invoke-WebRequest -Uri "$ColonyUrl/mesh/peer" -Method POST `
        -ContentType "application/json" -Body $body -UseBasicParsing
    Write-Host "[oo-mesh] ✅ Peer enregistré:" -ForegroundColor Green
    $resp.Content | ConvertFrom-Json | ConvertTo-Json -Depth 4
} catch {
    Write-Warning "[oo-mesh] /mesh/peer erreur: $_"
}

Write-Host ""

# 2. Voir le statut du mesh
Write-Host "[oo-mesh] Statut du mesh:" -ForegroundColor Cyan
try {
    $mesh = Invoke-WebRequest -Uri "$ColonyUrl/mesh/status" -UseBasicParsing
    $mesh.Content | ConvertFrom-Json | ConvertTo-Json -Depth 6
} catch {
    Write-Warning "[oo-mesh] /mesh/status erreur: $_"
}

Write-Host ""

# 3. Voir les métriques gossip
Write-Host "[oo-mesh] Métriques gossip:" -ForegroundColor Cyan
try {
    $metrics = Invoke-WebRequest -Uri "$ColonyUrl/mesh/metrics" -UseBasicParsing
    $metrics.Content | ConvertFrom-Json | ConvertTo-Json -Depth 4
} catch {
    Write-Warning "[oo-mesh] /mesh/metrics erreur: $_"
}

Write-Host ""
Write-Host "[oo-mesh] Pour configurer le gossip bidirectionnel, définis COLONY_BOOTSTRAP_PEERS sur le serveur:"
Write-Host "   (sur 192.168.1.8) : `$env:COLONY_BOOTSTRAP_PEERS = '$MyUrl'"
Write-Host "   puis redémarre colony-server ou relance la tâche OO-ColonyServer"
