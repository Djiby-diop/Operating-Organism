# 🗣️ Vocal-Baremetal (Les Cordes Vocales)

## Rôle Biologique
Le `vocal-baremetal` est l'organe d'expression de l'organisme vers l'extérieur. Il permet au Cortex de communiquer ses pensées ou ses alertes de manière brute.

## Fonctions
- **Vocalisation Série** : Envoi de logs et messages via l'UART (port série).
- **Vocalisation Framebuffer** : Affichage de texte sur l'écran UEFI sans passer par des drivers complexes.
- **Alertes Sonores** : Utilisation du PC Speaker pour émettre des fréquences codées en cas d'urgence vitale.

## Architecture
- `src/uart_vocal.c` : Communication texte.
- `src/speaker_vocal.c` : Alertes sonores matérielles.
