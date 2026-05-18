# 🩸 United-Baremetal (Le Système Circulatoire / Le Cœur)

## Rôle Biologique
Le `united-baremetal` est le "Rond-point" absolu de l'Operating Organism. C'est le **centre de contrôle** qui unifie tous les organes (LLM, Bot, Sense, Kernel).
Il agit comme le flux sanguin (Bloodstream) permettant aux différentes cellules de transiter d'un organe à l'autre en temps réel.

## Les Trois Types de Globules (Paquets de Données)
Dans ce système, l'IPC (Inter-Process Communication) n'existe pas sous forme de "sockets" ou de "pipes". Les données voyagent sous forme de **Globules** :

1. 🛡️ **Globules Blancs (White Cells)** :
   - Émis par le `bot-baremetal` (Système Immunitaire) et les capteurs de sécurité.
   - Contiennent les alertes (Threat Level), les signatures virales, et les anticorps (Patterns).
   - *Priorité absolue* sur le réseau sanguin en cas d'hémorragie ou d'infection.

2. 🔴 **Globules Rouges (Red Cells)** :
   - Émis par le `memory-baremetal` et le `sense-baremetal`.
   - Transportent l'Oxygène (les Données brutes : Tenseurs LLM, I/O réseau, frappes clavier).
   - Alimentent le Cortex (`llm-baremetal`) en informations pour qu'il puisse générer ses réponses.

3. ⚡ **Globules Jaunes (Yellow Cells / Plasma)** :
   - Émis par le `kernel-baremetal` (Tronc Cérébral).
   - Transportent l'Énergie (Ordres de scheduling, signaux de mise en veille, allocations de ressources).
   - Assurent la coagulation (fermeture d'un composant défaillant) et la régénération.

## Architecture
- **Le Myocarde (The Hub)** : Un bus annulaire (Ring Buffer) ultra-rapide et lock-free où tous les organes déposent et lisent leurs globules.
- **Les Capillaires** : Des files d'attente (Queues) spécifiques à chaque organe pour filtrer uniquement les globules qui les intéressent.
