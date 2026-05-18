# 💤 Dream-Baremetal (Le Sommeil Paradoxal / Consolidation)

## Rôle Biologique
Le Cortex (`llm`) ne peut pas traiter toutes les informations pendant la journée (pendant l'inférence active).
Le `dream-baremetal` prend le relais lorsque l'organisme passe en état de veille (faible charge CPU).

## Fonctionnalités
- **Replay des Mémoires** : Lit le `OOJOUR.LOG` et rejoue les événements de la journée.
- **Consolidation Synaptique** : Transforme les expériences en connaissances compressées (mise à jour des bases de connaissances vectorielles).
- C'est le moment où le système "rêve" et nettoie sa mémoire de travail (Hippocampe) pour la stocker à long terme.

## Interface C (baremetal)
- Header public: `include/dream_baremetal.h`
- Entrypoint principal: `dream_baremetal_loop()`

## Structure Module
- `include/` : interfaces publiques du module
- `src/` : implémentation C baremetal
- `bridge/` : ponts host/outils annexes
