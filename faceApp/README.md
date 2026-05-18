# faceApp - Keurgui

Keurgui est la premiere application familiale de vie OO.

## Quickstart 60s (recommande)

Le second ecran Keurgui propose un mini assistant de premiere vie.

Touches principales dans l'ecran:

- `1` `2` `3`: langue (`fr`, `en`, `wo`)
- `P`: cycle profil onboarding (`general`, `enfant`, `senior`, `diaspora`)
- `V`: active/desactive le guide vocal
- `G`: active/desactive le mini mode guardian
- `T`: cycle preset guardian (`safe`, `balanced`, `strict`) pour le profil actif
- `I` / `K`: augmente/reduit le seuil d'inactivite guardian (+/- 5 min) pour le profil actif
- `O` / `L`: augmente/reduit la fenetre de rappel routine (+/- 5 min) pour le profil actif
- `A`: ajoute un membre rapide
- `R`: ajoute une routine rapide
- `N`: change le mode foyer (`calme`, `normal`, `urgence`)
- `Enter` ou `Space`: etape suivante Quickstart
- `ESC`: quitter

## Demarrage rapide

1. Initialiser les donnees:

```powershell
pwsh ./faceApp/scripts/seed_keurgui_defaults.ps1
```

2. Lancer le bootstrap premiere vie:

```powershell
pwsh ./faceApp/scripts/start_keurgui_first_life.ps1
```

Le bootstrap n'acheve plus la premiere vie automatiquement.
La completion se fait dans l'ecran Keurgui via le Quickstart.

3. Lancer sans ecran graphique (CI/test):

```powershell
pwsh ./faceApp/scripts/start_keurgui_first_life.ps1 -NoScreen
```

4. Lancer le gate automatique premiere vie (idempotent):

```powershell
pwsh ./faceApp/scripts/keurgui_first_life_gate.ps1
```

## Fichiers

- `documentaion.md`: specification Keurgui
- `indexdocumentation.md`: index de navigation
- `data/keurgui_state.template.json`: template du foyer
- `scripts/seed_keurgui_defaults.ps1`: seed etat
- `scripts/start_keurgui_first_life.ps1`: parcours premiere vie
- `scripts/keurgui_first_life_gate.ps1`: garde de lancement premiere vie (auto)
- `../desktop_display/keurgui_screen2.py`: second ecran Keurgui

## Voice FR/EN + Mini Guardian

- Les prompts vocaux sont actuellement localises en `fr` et `en`.
- La langue `wo` utilise provisoirement les prompts `fr`.
- La voix est contextuelle au mode foyer:
	- `calme`/`normal`: ton doux
	- `urgence`: ton ferme
- Les messages vocaux incluent une salutation naturelle selon l'heure et le profil.
- Le mini mode guardian declenche des alertes simples:
	- profil `enfant` + mode `urgence`
	- profil `senior` sans routine active
	- profil `diaspora` sans membre `admin`
	- inactivite au-dela du seuil (`guardian_idle_threshold_minutes`)
	- routine active proche de l'horaire (`guardian_reminder_window_minutes`)
- Les seuils peuvent etre ajustes par profil via `guardian_profile_thresholds` dans l'etat JSON.
- Les presets guardian par profil sont stockes dans `guardian_profile_preset_modes`.
- Les touches live `I/K/O/L` modifient directement `guardian_profile_thresholds` pour le profil courant.
- La touche `T` applique rapidement des niveaux de vigilance coherents selon le profil.
- Une banniere visuelle coloree affiche en direct le niveau de vigilance actif (`safe`, `balanced`, `strict`).
- En preset `strict`, la banniere pulse doucement lorsqu'une alerte guardian est active.
- En preset `strict` + alerte active, un tag visuel `ALERTE ACTIVE`/`ALERT ACTIVE` apparait sur la banniere.
- En preset `strict` + alerte active, le contour du panneau Guardian pulse egalement pour renforcer la priorite visuelle.

## Preview interface (capture image)

Tu peux generer une image de l'interface sans laisser la fenetre ouverte:

```powershell
$env:KEURGUI_SCREENSHOT_PATH = "desktop_display/keurgui_screen2_preview.png"
python desktop_display/keurgui_screen2.py
```

Ensuite, ouvre `desktop_display/keurgui_screen2_preview.png`.
