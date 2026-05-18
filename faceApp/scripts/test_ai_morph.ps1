$MorphFile = "$PSScriptRoot\..\data\ai_morph.json"

$JsonData = @"
{
  "bg_color": [20, 10, 30],
  "accent_color": [180, 50, 255],
  "custom_hologram": "Analyse du profil en cours... Mode Confort applique par DIOP-CREATOR."
}
"@

Set-Content -Path $MorphFile -Value $JsonData -Encoding utf8
Write-Host "Le fichier ai_morph.json a ete injecte dans le cerveau de Keurgui !"
Write-Host "Regardez l'interface : elle devrait muter vers un theme violet neon."
