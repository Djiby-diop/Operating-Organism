# 🌐 Network-Baremetal (Le Système Respiratoire)

## Rôle Biologique
Le `network-baremetal` est responsable des échanges avec l'extérieur (air, gaz, environnement). C'est la pile TCP/IP écrite pour le bare-metal.

## Rôle
- **Inspiration (Rx)** : Réception des paquets depuis la carte réseau (NIC). Transforme les trames Ethernet en Globules Rouges (Données) pour le `united-baremetal`.
- **Expiration (Tx)** : Envoi des réponses de l'OO vers le monde extérieur.
- **Les Bronches** : Gestion des ports TCP/UDP. Filtrage basique de bas niveau avant transmission au Cortex.
