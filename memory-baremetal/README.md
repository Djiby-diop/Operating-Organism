# 💾 Memory-Baremetal (Le Système Lymphatique / L'Hippocampe)

## Rôle Biologique
Dans l'Operating Organism, la mémoire n'est pas une simple ressource linéaire que l'on alloue (`malloc`) et que l'on libère (`free`). La mémoire est un **tissu cellulaire**.

Le `memory-baremetal` remplit deux rôles vitaux :
1. **L'Hippocampe (Cognition)** : Allocation des vastes réseaux de tenseurs (GGUF) pour le `llm-baremetal`, en utilisant de la mémoire physiquement contiguë et paginée (Huge Pages de 2MB ou 1GB) pour des performances d'inférence foudroyantes.
2. **Le Système Lymphatique (Nettoyage)** : Un ramasse-miettes (Garbage Collector) biologique en tâche de fond qui "digère" les pages mortes et purge la mémoire compromise par une infection (en lien avec `bot-baremetal`).

## Architecture
- **Paging (GDT / MMU)** : Abstraction de la mémoire physique (Page Tables, TLB).
- **Cell Pools (Allocateur par Bloc)** : Pas de `malloc` dynamique avec fragmentation. On utilise des "pools" de cellules (fixes) pour les structures système critiques (le Kernel et le Bot).
- **Neural Heaps** : Zones mémoires dédiées aux tenseurs d'inférence, avec alignement AVX/SIMD natif.

## Statut
Phase d'initialisation.
