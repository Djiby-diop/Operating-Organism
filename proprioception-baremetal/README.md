# 🧘 Proprioception-Baremetal (La Conscience du Corps)

## Rôle Biologique
Le `proprioception-baremetal` permet à l'Operating Organism de connaître sa position et son état de santé interne. Il surveille l'intégrité structurelle de l'OS.

## Fonctions
- **Stack Monitoring** : Vérifie qu'aucun organe ne fait déborder sa pile (Stack Overflow).
- **Heap Integrity** : Surveille la fragmentation et la validité des pointeurs de l'Hippocampe (`memory-baremetal`).
- **Posturologie** : S'assure que les Page Tables et la GDT sont toujours dans un état cohérent.

## Architecture
- `src/posture.c` : Analyse de la structure mémoire.
- `src/vestibular.c` : Équilibre et détection des anomalies de flux.
