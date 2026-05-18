# Keurgui — App Familiale de Premiere Vie OO

## 1. Vision

- Nom produit: Keurgui (wolof: maison, foyer)
- Role dans OO: premiere application relationnelle au debut de vie utilisateur
- Mission: etablir un foyer numerique simple, humain, et gouverne
- Principe: privacy-first, famille-first, zero friction au premier demarrage

Keurgui doit apparaitre des la premiere phase d'onboarding utilisateur OO.

## 2. Parcours Premiere Vie

1. OO detecte premiere vie utilisateur (pas de profil Keurgui).
2. OO lance le bootstrap Keurgui.
3. Keurgui cree un profil foyer minimal.
4. Keurgui ouvre un ecran de bienvenue (second ecran desktop).
6. L'utilisateur valide son foyer (nom, langue, profil, membres initiaux).
6. OO enregistre l'etat et passe en mode vie quotidienne.

## 3. Donnees minimales

Etat minimal attendu:

- id_foyer
- nom_foyer
- langue
- onboarding_profile (general, enfant, senior, diaspora)
- voice_guide_enabled
- guardian_mode_enabled
- guardian_idle_threshold_minutes
- guardian_reminder_window_minutes
- guardian_last_interaction_at
- guardian_last_alert
- guardian_profile_thresholds
- guardian_profile_preset_modes
- fuseau
- membres
- routines
- mode_foyer (calme, normal, urgence)
- created_at
- first_life_completed

Stockage local recommande:

- `faceApp/data/keurgui_state.json`

## 4. Scripts clefs

- `faceApp/scripts/start_keurgui_first_life.ps1`
  - initialise le profil
  - marque la premiere vie
  - lance le second ecran Keurgui

- `faceApp/scripts/seed_keurgui_defaults.ps1`
  - regenere un etat de base stable

## 5. Ecran Keurgui (Second ecran)

- Fichier: `desktop_display/keurgui_screen2.py`
- Objectif: ecran foyer immersif, lisible, chaleureux
- Contenu:
  - statut du foyer
  - profil adaptatif onboarding
  - guide vocal localise fr/en (wo fallback fr temporaire)
  - voix contextuelle douce/ferme selon mode foyer
  - salutation vocale naturelle selon heure + profil
  - mini mode guardian (alertes enfant/senior/diaspora + inactivite + routine proche)
  - reglage live des seuils guardian par profil (idle/reminder)
  - presets guardian rapides par profil (safe/balanced/strict)
  - banniere visuelle coloree du niveau de vigilance guardian
  - membres
  - routines du jour
  - actions rapides
  - idees futures OO

## 6. Idees futuristes

- Keurgui Mesh: federation de foyers de diaspora (partage controle)
- Keurgui Guardian: mode protecteur enfants/seniors avec alertes contextuelles
- Keurgui Memory Atlas: graphe de souvenirs famille multimodal (texte/audio/photo)
- Keurgui Ritual Engine: routines familiales adaptatives selon energie/temps
- Keurgui Twin Room: salon virtuel de reunion famille distribuee
- Keurgui Civic Bridge: interface services de quartier et solidarite locale

## 7. Regles de design

- Interface sobre, forte, non anxiogene
- Contraste eleve et lisibilite immediate
- Touche futuriste sans bruit visuel
- Priorite a la confiance, la clarte et l'inclusion multilingue

## 8. Definition of Done (V1)

- bootstrap premiere vie execute sans erreur
- `keurgui_state.json` cree
- second ecran Keurgui affichable localement
- parcours foyer basique testable en moins de 2 minutes
