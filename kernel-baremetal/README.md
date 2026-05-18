# 🧠 Kernel-Baremetal (Le Tronc Cérébral)

## Rôle Biologique
Le `kernel-baremetal` est le **Tronc Cérébral** de l'Operating Organism. 
Là où `llm-baremetal` est le Cortex (qui "pense") et `bot-baremetal` est le Système Immunitaire (qui "protège"), le Kernel est responsable de l'**homéostasie**.

Il gère les fonctions vitales inconscientes :
- **Rythme cardiaque** : Le Timer système et l'ordonnancement (Scheduler).
- **Respiration** : Les entrées/sorties de base (I/O, Interruptions).
- **Circulation sanguine** : Le routage de la mémoire et la communication entre les organes (IPC).

## Architecture Prévue
1. **Biological Scheduler (`core/scheduler`)** : Contrairement à un OS classique (Linux/Windows) qui utilise des priorités rigides (Round-Robin, FIFO), notre scheduler est basé sur l'état de l'organisme (Dormant, Combat, Survie). Si le Bot détecte une menace, le scheduler réduit instantanément le temps CPU du LLM (Cortex) pour allouer l'énergie aux réflexes et à l'immunité.
2. **HAL (Hardware Abstraction Layer)** : Gère les interruptions (IDT), la pagination (GDT/Paging), et le contexte CPU.
3. **Organism IPC** : Le bus de communication ultra-rapide permettant au Cortex (`llm`) et au Système Immunitaire (`bot`) de dialoguer sans passer par des fichiers sur disque.

## Statut
Phase d'initialisation.
