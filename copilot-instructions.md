# Instructions Copilot (à lire en premier)

## Checklist (nouveau PC, 2 minutes)

1) Installer/activer **WSL2** (Ubuntu recommandé) et vérifier `wsl -l -v`.
2) Installer dans WSL : `build-essential gnu-efi mtools parted dosfstools grub-pc-bin`.
3) Installer QEMU sur Windows si tu veux exécuter les tests QEMU.
4) Ouvrir VS Code sur ce repo, puis valider rapidement :

```powershell
cd llm-baremetal
./bench-matrix.ps1 -Repeat 1
```

Si ça échoue : lancer `./test-qemu-autorun.ps1 -Mode smoke` et regarder les logs.

### Sanity check (commande unique)

Première commande à tenter (valide build + run QEMU autorun au plus simple) :

```powershell
cd llm-baremetal
./test-qemu-autorun.ps1 -Mode smoke
```

### Debug express (si ça ne marche pas)

1) WSL OK : `wsl -l -v` (WSL2, distro running)
2) Packages WSL OK : `mtools`, `parted`, `dosfstools` installés
3) QEMU OK : `qemu-system-x86_64` accessible dans le PATH Windows
4) Relancer en matrice : `./bench-matrix.ps1 -Repeat 1`

Ce dépôt contient un projet **bare-metal x86_64 UEFI** avec un **REPL LLM** (dossier `llm-baremetal/`) + des outils annexes (ex: `llama2.c/`).

Objectif de ce fichier : quand j’ouvre ce repo dans VS Code sur un autre PC, je veux que l’assistant comprenne immédiatement :
- ce qui est important dans le projet,
- comment le builder / harness QEMU fonctionne,
- quelles conventions respecter (repo minimal, pas de renommage “Karpathy”, etc.),
- comment reproduire les commandes de build / test.

## Ambition du projet (vision)

Ambition : faire tourner un **LLM en mode chat** directement sur du matériel x86_64 UEFI, sans OS, avec une UX REPL utilisable et des performances acceptables.

Ce repo n’est pas un “demo hello world” : c’est une tentative de pousser une stack LLM **jusqu’au bare metal**, avec des contraintes réelles (RAM limitée, I/O rudimentaire, pas de runtime OS).

Objectifs long terme (direction, pas forcément déjà fini partout) :
- **Boot from USB** fiable sur des machines réelles (pas seulement QEMU).
- **REPL chat** stable (chargement modèle, config, commandes, logs, erreurs propres).
- **Support GGUF** pragmatique pour inspection et inférence (quantizations compatibles selon l’état du code).
- **Performance** : matmul optimisés, AVX2 quand dispo, minimiser copies/malloc, instrumentation (tok/s, temps, mémoire).
- **Robustesse** : harness QEMU reproductible, timeouts, assertions utiles mais pas fragiles.
- **Repo propre** : pas de modèles/toolchains lourdes dans git ; scripts de bootstrap + checksums.

Non-objectifs (à éviter si ce n’est pas demandé) :
- Ajouter plein de fonctionnalités UI/menus “nice-to-have” au détriment de la stabilité.
- Refactor massif/renommage large (surtout côté Karpathy / `llama2.c`).
- Vendoring de gros binaires (toolchains, images, modèles) dans le dépôt.

## Principes de décision (pour l’IA)

Quand il y a ambiguïté, appliquer ces règles, dans cet ordre :

1) **Stabilité d’abord** : un harness fiable + boot stable > nouvelles features.
2) **Surgical changes** : petites modifications, faciles à reviewer/revert.
3) **Reproductible** : préférer scripts deterministes, checksums, timeouts.
4) **Pas de lourds artefacts dans git** : modèles/images/toolchains restent hors repo (bootstrap + `.gitignore`).
5) **QEMU ≠ vrai hardware** : valider QEMU, mais penser “USB boot machine réelle” (UEFI, timings, périphériques).
6) **Mesurer avant d’optimiser** : garder `[stats] tok_s`, temps, mémoire ; éviter l’optimisation speculative.
7) **Respecter les noms Karpathy** : ne pas renommer/reshaper `llama2.c` et fichiers associés.
8) **Si ça casse, réduire le scope** : revenir à `smoke` / `bench-matrix` et isoler la régression.

## Règles de repo (priorités)

### Style & communication (à appliquer partout)

- Éviter les emojis dans le code, les scripts, et la doc (looks “LLM-ish” + compat).
- Favoriser l’ASCII pur dans les fichiers texte “core” (README, scripts, logs) pour compat maximale.
- README: rester court, focalisé, et professionnel (aller droit au but).
- Modèles/weights: ne pas les committer; préférer un lien vers un repo HuggingFace (ou équivalent) + script de download (curl/wget) avec checksum.
- Penser multi-OS: quand on ajoute/modifie un script, viser Windows + WSL, et idéalement une voie Linux/macOS (ou documenter clairement la limitation).

1) **Uniquement l’essentiel dans git**
- Ne pas ajouter de gros binaires (images `.img`, modèles `.gguf`, toolchains complètes, etc.) dans l’historique.
- Préférer des **scripts de download + vérification SHA256** et des dossiers ignorés via `.gitignore`.

2) **Renommage : attention**
- Ne pas renommer les fichiers “Karpathy” (sources / fichiers historiques liés à `llama2.c` / Karpathy).
- Si un fichier porte un nom “Justine / cosmopolitan” (ex: téléchargé depuis justine.lol), on peut le renommer vers un nom neutre **uniquement si nécessaire**, en gardant la traçabilité (README, script de download, checksum).

3) **Suppression de dossiers externes**
- Si un dossier du type `extern complement` existe (ou ré-apparaît après un copier/coller), il doit être supprimé avant d’intégrer quoi que ce soit.
- Dans cet espace de travail actuel, il n’a pas été trouvé lors des scans, donc il est probablement déjà absent.

## Carte rapide du repo

- `llm-baremetal/` : le cœur (UEFI REPL, scripts de build, création d’image bootable, tests QEMU).
- `llama2.c/` : repo/tooling style Karpathy (entraînement / export / scripts Python). Ne pas renommer.
- `USB-BOOT-FILES/` : fichiers liés au boot USB.
- Racine : artefacts (ex: `KERNEL.EFI`, logs QEMU) selon les runs.

## Prérequis (nouveau PC)

### Windows
- Windows 10/11 x64
- PowerShell 7 recommandé (mais Windows PowerShell peut fonctionner)
- **WSL2** + une distro Linux (Ubuntu recommandé)
- QEMU (si vous utilisez les scripts `run.ps1` / tests QEMU)

### WSL / Linux packages (Ubuntu/Debian)
Dans WSL, installer les packages nécessaires (si vous build/test côté Linux) :

```bash
sudo apt-get update
sudo apt-get install -y build-essential gnu-efi mtools parted dosfstools grub-pc-bin
```

Note : le harness Windows pilote souvent WSL pour certaines opérations (ex: `mtools` via WSL). Garder WSL fonctionnel est important.

## Build & run (workflow standard)

### Build (Windows + WSL)
Le point d’entrée est :
- `llm-baremetal/build.ps1`

Depuis la racine du repo :

```powershell
cd llm-baremetal
./build.ps1
```

Modèle (optionnel) :
- Les modèles sont **volontairement ignorés par git**.
- `tokenizer.bin` est requis pour certaines images.

### Run (QEMU)
Point d’entrée :
- `llm-baremetal/run.ps1`

Exemple :

```powershell
cd llm-baremetal
./run.ps1 -Gui
```

## Tests / harness QEMU (important)

Les scripts de test les plus importants :
- `llm-baremetal/test-qemu-autorun.ps1` : harness principal.
- `llm-baremetal/bench-matrix.ps1` : exécute une matrice de modes (smoke/q8bench/ram/gen/gguf_smoke) en boucle et parse les stats.

### Bench matrix (recommandé pour valider rapidement)

```powershell
cd llm-baremetal
./bench-matrix.ps1 -Repeat 2
```

Le script parse des lignes du type :
- `[stats] ... tok_s=...`

### Détails importants (fiabilité)

- **mtools** peut se bloquer si on écrit dans une image située sur un chemin Windows monté dans WSL (ex: `/mnt/c/...`).
- La solution utilisée par le harness : **stager l’image dans `/tmp` (WSL local)**, faire les écritures FAT là-bas, puis recopier le résultat.
- Ne pas “simplifier” ce mécanisme : c’est un fix clé pour la stabilité.

### Modes & expectations
- `gguf_smoke` : test plus strict (injection/validation) parce qu’il doit être fiable.
- `ram` : valide surtout le **format** des lignes RAM (WEIGHTS/KV/SCRATCH/ACTS/ZONEC) au lieu d’imposer un modèle spécifique par défaut.
- `q8bench` : doit forcer `gguf_q8_blob=1` et utilise par défaut un `.gguf` Q8_0 (le harness rend ce mode auto-suffisant si `-BootModel` n’est pas fourni).

## Cosmopolitan / cosmocc (outil externe, optionnel)

Le repo évite de committer la toolchain.
Dans `llm-baremetal/tools/` :
- `get-cosmocc.ps1` : télécharge `cosmocc` (par défaut v4.0.2), vérifie SHA256, extrait dans `llm-baremetal/tools/_toolchains/cosmocc`.
- `cosmocc.ps1` : wrapper pratique qui exécute `bin/cosmocc`.

Exemple :

```powershell
cd llm-baremetal
./tools/get-cosmocc.ps1
./tools/cosmocc.ps1 --help
```

Le dossier `llm-baremetal/tools/_toolchains/` est ignoré par git.

## Conventions pour les contributions (ce que l’IA doit faire)

- Faire des changements **surgicaux**, pas de refactor massif.
- Éviter d’ajouter des features “nice-to-have” si ce n’est pas demandé.
- Garder les scripts PowerShell robustes : timeouts, capture de sortie, messages d’erreur clairs.
- Ne pas ajouter de dépendances lourdes sans raison.

## Debug rapide (si ça casse sur un nouveau PC)

1) Vérifier WSL :
- `wsl -l -v`

2) Vérifier les packages Linux si un script appelle `mtools`/`parted`/etc.

3) Vérifier QEMU dans le PATH (si `run.ps1` / tests QEMU).

4) Lancer un test minimal :

```powershell
cd llm-baremetal
./test-qemu-autorun.ps1 -Mode smoke
```

## À lire si besoin

- `llm-baremetal/README.md` : usage, contraintes modèles, workflow.
- `llm-baremetal/OO_SPEC.md` : spécifications/architecture OO du projet.
