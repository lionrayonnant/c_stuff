#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Port utilisé pour le socket
#define PORT 5000

// Nombre maximal de spectacles
#define MAX_SPECTACLE 10

// Types de requêtes
#define TYPE_LIST   1
#define TYPE_RESERV 2

// Structure des spectacles

typedef struct {
    int id;
    char nom[50];
    char date[11];
    int places;
} Spectacle;

// Structure d'une requête au serveur
typedef struct {
    int type;
    int spectacle;
    int nb_places;
} Request;

// Structure d'une réponse à une demande de consultation
typedef struct {
    Spectacle spectacles[MAX_SPECTACLE];
} Response_list;

// Structure d'une réponse à une demande de réservation
typedef struct {
    bool ack;
} Response_reservation;

Request request_list;
Request request_reservation;

Response_list response_list;
Response_reservation response_reservation;

// Boucle principale
int main() {

    int choice;

    // Définition du socket
    // Ipv4, mode stream (TCP)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[INFO] - Erreur dans la création du socket");
        exit(1);
    }

    // Déclaration de la structure qui contient les infos du serveur
    struct sockaddr_in serv;
    // Précise IPv4
    serv.sin_family = AF_INET;
    // Mets le port d'écoute sur 5000
    serv.sin_port = htons(PORT);

    // Conversion de l'IP fournie en texte
    // On écrit cette IP dans la structure qui contient les infos du serveur
    inet_pton(AF_INET, "IP_DU_SERVEUR", &serv.sin_addr);

    // Connexion au serveur
    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("[INFO] - Erreur dans le connection au serveur");
        exit(1);
    }

    printf("[INFO] Connecté au serveur\n");

    // Boucle principale
    while (1) {
        // Menu principal
        printf("\n=== MENU ===\n");
        printf("1. Consultation\n");
        printf("2. Réservation\n");
        printf("0. Quitter\n");
        printf("> ");
        scanf("%d", &choice);

        // Quitte le client
        if (choice == 0) break;

        // Consultation
        if (choice == 1) {
            // Remplissage de la requête de consultation
            request_list.type = TYPE_LIST;
            request_list.spectacle = 0;
            request_list.nb_places = 0;

            // Ecriture sur le socket
            write(sock, &request_list, sizeof(Request));

            // Lecture de la réponse sur le socket
            read(sock, &response_list, sizeof(Response_list));

            // Affichage de la réponse du serveur
            for (int i = 0; i < MAX_SPECTACLE; i++) {
                printf("[%d] %s | %s | %d places\n",
                       i,
                       response_list.spectacles[i].nom,
                       response_list.spectacles[i].date,
                       response_list.spectacles[i].places);
            }
        }
        // Réservation
        else if (choice == 2) {
            int index, nb;
            printf("Numéro du spectacle : ");
            scanf("%d", &index);
            printf("Nombre de places : ");
            scanf("%d", &nb);

            // Remplissage de la requête utilisateur avec les entrées
            request_reservation.type = TYPE_RESERV;
            request_reservation.spectacle = index;
            request_reservation.nb_places = nb;

            // Ecriture sur le socket
            write(sock, &request_reservation, sizeof(Request));

            // Lecture de la réponse du serveur
            read(sock, &response_reservation, sizeof(Response_reservation));

            // Vérification du ACK
            if (response_reservation.ack)
                printf("Réservation OK. À bientôt dans nos salles !\n");
            else
                printf("Réservation NOK. Réessayez.\n");
        }
    }
    // Fermeture du socket
    close(sock);
    // Message de log
    printf("[INFO] -  Déconnexion du client\n");
    return 0;
}
