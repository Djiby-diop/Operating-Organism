# ⚡ Reflex-Baremetal (Le Système Nerveux Périphérique)

## Rôle Biologique
Le `reflex-baremetal` gère les actes réflexes. Un réflexe est une action qui *contourne le Cortex* (le LLM) pour gagner un temps critique.
Si une main touche du feu (surcharge thermique) ou si une intrusion réseau massive est détectée, le LLM prendrait plusieurs millisecondes pour analyser l'information. Le `reflex-baremetal` intercepte l'information sur le bus `united-baremetal` et agit en **sub-milliseconde**.

## Architecture
- Écoute exclusive des **Interrupts (IRQs)** matériels critiques.
- Table de vérité codée en dur (InstinctLayer) : ex: `SI (Température > 95°C) -> Throttling immédiat` sans consulter le Kernel.
- Lié de très près avec le `bot-baremetal` (Système Immunitaire) pour les réflexes de sécurité (ex: bloquer un port réseau instantanément).
