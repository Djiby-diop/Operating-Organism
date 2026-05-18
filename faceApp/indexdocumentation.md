# Index Documentation — faceApp / Keurgui

## Fichiers principaux

- `documentaion.md`
  - spec produit Keurgui
  - parcours premiere vie OO
  - scope V1 + idees futuristes

- `scripts/start_keurgui_first_life.ps1`
  - lance le bootstrap premiere vie
  - cree l'etat local et ouvre l'ecran Keurgui

- `scripts/seed_keurgui_defaults.ps1`
  - regenere un etat de base deterministe

- `scripts/keurgui_first_life_gate.ps1`
  - lance Keurgui seulement si premiere vie non complete
  - peut etre branche dans le flux de boot/launch OO

- `data/keurgui_state.template.json`
  - template de donnees foyer

- `desktop_display/keurgui_screen2.py`
  - second ecran visuel pour l'accueil familial
  - quickstart interactif 60s (langue, membres, routines, validation)
  - profil adaptatif onboarding (`general`, `enfant`, `senior`, `diaspora`)
  - guide vocal leger (toggle)
  - prompts vocaux localises fr/en (wo en fallback fr)
  - voix contextuelle douce/ferme selon mode foyer
  - salutations vocales naturelles (heure + profil)
  - mini mode guardian avec alertes de base + inactivite + routine proche
  - seuils guardian configurables par profil dans le JSON d'etat
  - reglage live des seuils (I/K pour idle, O/L pour reminder)
  - presets rapides safe/balanced/strict (touche T)
  - banniere visuelle coloree du preset de vigilance actif
  - pulse doux de la banniere en strict quand alerte active
  - tag visuel ALERT ACTIVE/ALERTE ACTIVE en strict + alerte
  - contour pulse du panneau Guardian en strict + alerte
  - capture preview via KEURGUI_SCREENSHOT_PATH

## Parcours recommande

1. Lancer `scripts/seed_keurgui_defaults.ps1` (optionnel)
2. Lancer `scripts/start_keurgui_first_life.ps1`
3. Verifier creation de `faceApp/data/keurgui_state.json`
4. Observer l'ecran `desktop_display/keurgui_screen2.py`
5. Completer Quickstart avec `Enter/Space` jusqu'a validation finale

Alternative auto:

1. Lancer `scripts/keurgui_first_life_gate.ps1`
2. Le gate decide de lancer ou de skip selon l'etat `first_life_completed`

## Objectif

Faire de Keurgui la premiere experience applicative de vie OO, simple, digne, et evolutive.
