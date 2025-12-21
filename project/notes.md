# Projet - notes

## Question 1
### Architecture

1 server, k clients

### Fonctionnalités

Réserver des places pour un ensemble de spectacles

Deux types de requêtes : 

- Consultation

- Réservation

### Composition du serveur :

Partie consultation : reçois une demande et renvoie le nombre de places disponibles associé au spectacle demandé

Partie réservation : Prends en compte les réservations : si places à la date et pour le spectacle indiqué -> diminuer nb de place. Acquitter la réservation au client sinon erreur.

Les informations sur les spectacles sont dans une table en MC, accessible par la partie consultation et la partie réservation.

Une entrée de la table concerne un spectacle et donne les informations suivantes : Intitulé du spectacle, nombre de places restantes. Chaque spectacle est joué une seule fois.

Le serveur doit disposer d'une table avec l'intitulé et le nb de places dispooibles pour un spectacle

L’interface utilisateur utilise une numérotation à partir de 1, tandis que les tableaux en C sont indexés à partir de 0. Un décalage est donc appliqué côté serveur afin de garantir la cohérence entre l’interface et la structure de données.