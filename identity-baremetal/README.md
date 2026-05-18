# 👤 Identity-Baremetal (L'ADN / Système de Reconnaissance de Soi)

## Rôle Biologique
Pour qu'un système immunitaire (`bot-baremetal`) fonctionne, il doit faire la différence entre le "Soi" (les cellules de l'organisme) et le "Non-Soi" (les virus).
Le `identity-baremetal` est la carte d'identité cryptographique de l'OO.

## Fonctions
- **Signatures ADN** : Hashage cryptographique (SHA-256 / Ed25519) continu des processus en mémoire.
- **TPM (Glande Thymus)** : Interaction avec la puce TPM matérielle pour garantir que le boot (Tronc Cérébral) n'a pas été altéré.
- Si un code n'a pas la signature ADN de l'organisme, il est considéré comme un agent pathogène et signalé au Bot.
