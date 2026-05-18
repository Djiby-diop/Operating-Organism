# 🧬 Regen-Baremetal (Les Cellules Souches)

## Rôle Biologique
Le `regen-baremetal` est responsable de la réparation et de la mise à jour de l'organisme sans interruption de service. Il agit comme des cellules souches qui peuvent remplacer n'importe quel tissu (code) défectueux ou obsolète.

## Fonctions
- **Live Patching** : Réécriture des fonctions en mémoire en détournant les pointeurs d'appel (Hot-patching).
- **Auto-Réparation** : Si le `bot-baremetal` identifie un binaire corrompu, `regen` récupère une version saine via le `network` ou le `shadow` (backup) et répare l'organe à chaud.
- **Différenciation** : Capacité à instancier de nouveaux organes si l'évolution le demande.

## Architecture
- `src/hotpatch.c` : Logique de redirection des fonctions.
- `src/stem_cells.c` : Gestion des modèles de remplacement.
