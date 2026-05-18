# 🦠 Bot-Baremetal — Catalogue des Agents

## Philosophie

Chaque agent est une cellule spécialisée de l'organisme immunitaire.
Un agent ne fait qu'**une seule chose**, mais il la fait parfaitement.

> "La spécialisation est la force de l'essaim.
>  Aucun agent ne peut tout faire.
>  Ensemble, ils peuvent tout protéger."

---

## Agents de Surveillance (Sensor Net)

### 🔴 MemWatch Agent
**Fichier** : `immune/src/mem_watch.rs`
**Rôle** : Gardien de la RAM

| Propriété | Valeur |
|-----------|--------|
| Domaines | Antivirus, Anti-Rootkit |
| Réaction type | Quarantine, Capture mémoire |
| Seuil d'alerte | Confidence ≥ 65% |
| Latence cible | < 500µs par scan |

Détecte :
- NOP sleds (shellcode préparation)
- MZ headers en zones non-PE (injection DLL)
- Heap spray patterns
- ROP chains
- Modifications de zones exécutables

---

### 🟡 NetWatch Agent
**Fichier** : `immune/src/net_watch.rs`
**Rôle** : Gardien du réseau

| Propriété | Valeur |
|-----------|--------|
| Domaines | Anti-Vol, Anti-Bot |
| Réaction type | HoneyTrap, Coupure réseau |
| Seuil d'alerte | 10 connexions répétées ou > 50 MB sortant |
| Fenêtre d'observation | Configurable (défaut: glissante) |

Détecte :
- C2 beacons (connexions répétitives)
- Exfiltration de volume (> seuil bytes sortants)
- DNS tunneling
- Ports inhabituels
- Communication vers IPs jamais vues

---

### 🟠 FsWatch Agent
**Fichier** : `immune/src/fs_watch.rs`
**Rôle** : Gardien du système de fichiers

| Propriété | Valeur |
|-----------|--------|
| Domaines | Anti-Ransomware, Anti-Vol |
| Réaction type | Quarantine processus, Snapshot |
| Seuil ransomware | 50 modifications rapides |
| Extensions surveillées | .locked, .encrypted, .crypt, .enc, .zepto |

Détecte :
- Chiffrement de masse (ransomware heuristique)
- Extensions malveillantes connues
- Suppressions massives rapides
- Modifications de fichiers critiques OO
- Modifications du secteur de boot

---

### 🔵 ProcWatch Agent
**Fichier** : `immune/src/proc_watch.rs` *(à créer)*
**Rôle** : Gardien des processus

Détecte :
- Process hollowing (parent suspect → child inattendu)
- Injection de code (CreateRemoteThread, WriteProcessMemory)
- Escalade de privilèges
- Débogueurs non autorisés (protection du Bot)
- Processus se faisant passer pour des services système

---

### ⚫ KernelWatch Agent
**Fichier** : `immune/src/kernel_watch.rs` *(à créer)*
**Rôle** : Gardien du kernel (priorité maximale)

Détecte :
- Hooks de syscalls (SSDT, inline hooks)
- DKOM (Direct Kernel Object Manipulation)
- Signatures rootkit
- Tentatives de lecture/écriture de structures kernel

> **Note** : KernelWatch opère au niveau le plus bas.
> Si KernelWatch est compromis → SURVIVAL automatique.

---

### ⚪ BootWatch Agent
**Fichier** : `immune/src/boot_watch.rs` *(à créer)*
**Rôle** : Gardien du boot UEFI

Détecte :
- Modifications du VBR/MBR
- Bootkits UEFI
- Altérations des variables UEFI
- Modifications des binaires de boot
- Intégrité des fichiers OO au démarrage

---

## Agents de Réponse (Immune Response)

### 🔒 Quarantine Agent
**Fichier** : `immune/src/quarantine.rs`
**Rôle** : Isolateur instantané

Capacités :
- Suspension de processus (< 5ms)
- Isolation fichier (rename + chmod strict)
- Coupure réseau ciblée par PID
- Coupure réseau totale (urgence)
- Log de tout ce qu'il isole

Règle : **isoler d'abord, analyser ensuite**.
Jamais l'inverse.

---

### 🍯 HoneyTrap Agent
**Fichier** : `agents/honey_trap/` *(à créer)*
**Rôle** : Maître du leurre

L'attaquant pénètre dans un mirage.

Crée :
- Faux fichiers "précieux" (credentials, configs, secrets)
- Faux services réseau (faux RDP, faux SSH)
- Faux kernel maps (pour tromper les scanners mémoire)
- Faux répertoires de données sensibles

Pendant que l'attaquant explore le leurre :
- MimicryAgent capture ses techniques en temps réel
- Chaque mouvement est journalisé avec précision
- Le vrai système est hors de portée

---

### 💀 Neutralizer Agent
**Fichier** : `immune/src/neutralizer.rs` *(à créer)*
**Rôle** : Neutralisation confirmée

N'agit que lorsque la menace est **prouvée** (confidence ≥ 90%).

Actions :
- Terminaison définitive de processus malveillants
- Suppression de fichiers malveillants isolés
- Nettoyage d'entrées de registre suspectes
- Rapport complet de toutes ses actions (auditabilité)

> **Règle** : Le Neutralizer ne frappe **jamais** en premier.
> Quarantine → analyse → confirmation → neutralisation.

---

### 🦎 Chameleon Agent
**Fichier** : `agents/chameleon/` *(à créer)*
**Rôle** : Invisibilité du Bot

Protège le Bot lui-même.

Techniques :
- Masquage des processus du Bot dans la liste PID
- Modification de la carte mémoire visible par scanners externes
- Fausse signature de fichiers Bot
- Migration de ressources critiques en zones inattendues
- Changement de nom de fichiers sensibles en patterns aléatoires

---

## Agents d'Infrastructure

### 🔄 RegenAgent
**Fichier** : `agents/regen/` *(à créer)*
**Rôle** : Auto-reconstruction de la flotte

Le Bot ne peut pas mourir durablement.

Stratégie :
1. Détection d'agent mort (health = 0 ou processus absent)
2. Localisation de la dernière copie saine de l'ADN
3. Recréation de l'agent depuis l'ADN sain
4. Migration de l'état (si disponible)
5. Notification au SwarmMind
6. Mise à jour du compteur de génération

---

### 🔗 OOBridgeAgent
**Fichier** : `oo_bridge/` *(à créer)*
**Rôle** : Interface avec le LLM-Baremetal

Communication bidirectionnelle via le journal OO.

Bot → LLM :
- Événements de sécurité critiques
- Demandes d'analyse de patterns complexes
- Alertes d'état de survie

LLM → Bot :
- Directives stratégiques optionnelles
- Résultats d'analyses comportementales profondes
- Confirmations/invalidations de patterns

> **Règle absolue** : Le Bot n'a **jamais besoin** du LLM
> pour une décision de survie ou de défense immédiate.
> Le LLM est un conseiller, pas un commandant.

---

### 👑 SwarmMind
**Fichier** : `immune/src/swarm_mind.rs`
**Rôle** : Chef d'État — coordinateur suprême

Il ne combat pas. Il commande.

Responsabilités :
- Maintenir le niveau de menace global (atomique, lock-free)
- Recevoir les événements de tous les agents
- Décider les transitions de niveau selon la matrice
- Ordonnancer le déploiement/retrait des agents
- Superviser la santé de la flotte
- Déclencher les régénérations nécessaires

---

## Matrice de Déploiement par Niveau de Menace

| Agent | DORMANT | VIGILANCE | ALERT | COMBAT | SURVIVAL | CONFINEMENT |
|-------|---------|-----------|-------|--------|----------|-------------|
| MemWatch | 🟡 éco | ✅ actif | ✅ actif | ✅ actif | ⏸ | ⏸ |
| NetWatch | 🟡 éco | ✅ actif | ✅ actif | ✅ actif | ⏸ | ⏸ |
| FsWatch | 🟡 éco | ✅ actif | ✅ actif | ✅ actif | ⏸ | ⏸ |
| ProcWatch | 🟡 éco | ✅ actif | ✅ actif | ✅ actif | ⏸ | ⏸ |
| KernWatch | ✅ actif | ✅ actif | ✅ actif | ✅ actif | ✅ actif | ✅ actif |
| BootWatch | ✅ actif | ✅ actif | ✅ actif | ✅ actif | ✅ actif | ✅ actif |
| Quarantine | ⏸ | ⏸ stand-by | 🚀 déployé | ✅ actif | ✅ actif | ⏸ |
| HoneyTrap | ⏸ | 🔄 prêt | 🚀 déployé | ✅ actif | ⏸ | ⏸ |
| Neutralizer | ⏸ | ⏸ | ⏸ | ✅ autorisé | ⏸ | ⏸ |
| Chameleon | ⏸ | ⏸ | ⏸ | ⏸ | ✅ actif | ✅ actif |
| MimicryAgent | 🔄 arrière | 🔄 arrière | ✅ actif | ✅ actif | ⏸ | ⏸ |
| RegenAgent | 🔄 veille | 🔄 veille | 🔄 veille | 🔄 veille | ✅ priorité | ✅ priorité |
| OOBridge | 🔄 veille | 🔄 veille | 🔄 veille | ✅ notifie | ✅ alerte | ✅ alerte |

---

*Bot-Baremetal Agent Catalog — Part of the Operating Organism*
