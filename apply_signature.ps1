param(
    [string]$Directory = "C:\Users\djibi\OneDrive\Bureau\baremetal\llm-baremetal\engine\ssm"
)

$signature = @"
/* ====================================================================
 * Operating Organism (OO) - Symbiotic Intelligence Architecture
 * Copyright (c) 2026 Djiby Diop. Tous droits réservés.
 * ==================================================================== */
"@

$files = Get-ChildItem -Path $Directory -Include *.c, *.h -Recurse

$modifiedCount = 0

foreach ($file in $files) {
    $content = Get-Content -Path $file.FullName -Raw
    
    # Check if signature already exists to avoid duplicates
    if (-not ($content -match "Copyright \(c\) 2026 Djiby Diop")) {
        $newContent = $signature + "`r`n" + $content
        Set-Content -Path $file.FullName -Value $newContent -Encoding UTF8
        Write-Host "Signed: $($file.Name)"
        $modifiedCount++
    } else {
        Write-Host "Already signed: $($file.Name)"
    }
}

Write-Host "Total files signed: $modifiedCount"
