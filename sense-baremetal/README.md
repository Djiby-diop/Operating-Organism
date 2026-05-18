# 👁️ Sense-Baremetal (Le Système Sensoriel)

## Rôle Biologique
Le `sense-baremetal` est l'ensemble des organes sensoriels périphériques. Il capte les signaux bruts du monde extérieur (le hardware) et les traduit en influx nerveux (Globules Rouges) pour le bus central (`united-baremetal`).

## Composants
- **Rétine (Vision)** : Parseurs d'écrans, caméras ou flux vidéo.
- **Tympan (Ouïe)** : Entrées audio / micros.
- **Toucher (Périphériques)** : Claviers, Souris, I2C, SPI.
- **Thermoception (Douleur/Chaleur)** : Lecture des sondes de température CPU, tensions, et alertes matérielles critiques (NMI).

Il ne prend aucune décision. Il se contente de générer des stimuli de manière asynchrone (via Interruptions).
