#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Types de requêtes
#define TYPE_LIST 1 // Consultation
#define TYPE_RESERV 2 // Réservation

// Nombre maximal de spectacles
#define MAX_SPECTACLE 10

typedef struct {
    int id;
    char nom[50];
    char date[11];
    int places;
} Spectacle;

// Demande de consultation
typedef struct {
    long mtype;
    pid_t pid;
} Request_list;

// Réponse de consultation
typedef struct {
    long mtype;
    pid_t pid;
    Spectacle spectacles[MAX_SPECTACLE];
} Response_list;

// Demande de réservation
typedef struct {
    long mtype;
    pid_t pid;
    int spectacle;
    int nb_places;
} Request_reservation;

// Réponse de réservation
typedef struct {
    long mtype;
    pid_t pid;
    bool ack;
} Response_reservation;

Request_reservation request_reservation;
Response_reservation response_reservation;

Request_list request_list;
Response_list response_list;

int main() {
    int choice;
    int index, nb_places;

    // ftok pour générer la clef
    key_t key = ftok("question_2_server", 42);

    // On rejoins la MSQ
    int msgid = msgget(key, 0666 | IPC_CREAT);

    // Boucle principale
    while (1) {
        // Menu principal
        printf("\n=== MENU ===\n");
        printf("1. Consultation\n");
        printf("2. Réservation\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choice);

        // Quitte le client
        if (choice == 0) break;

        switch (choice) {
            case 1: // CONSULTATION

                // Type de la requête : LIST (1)
                request_list.mtype = TYPE_LIST;
                // On précise notre PID
                request_list.pid = getpid();
                // Envoi du message sur la file
                msgsnd(msgid,&request_list, sizeof(Request_list) - sizeof(long), 0);
                // Attente d'une réponse avec notre PID dans le mtype
                msgrcv(msgid, &response_list, sizeof(Response_list) - sizeof(long), request_list.pid, 0);

                // Affichage de la réponse
                printf("\nRéponse du serveur :\n");

                for (int i = 0; i < MAX_SPECTACLE; i++) {
                    if (response_list.spectacles[i].id != 0) {
                        printf("[%d] %s | %s | %d places\n",
                            i,
                            response_list.spectacles[i].nom,
                            response_list.spectacles[i].date,
                            response_list.spectacles[i].places);
                    }
                }
                break;

            case 2: // RESERVATION
                // Demande à l'utilisateur le n° du spectacle
                printf("Numéro du spectacle à réserver : ");
                scanf("%d", &index);
                // Demande à l'utilisateur le nombre de places
                printf("Nombre de places à réserver : ");
                scanf("%d", &nb_places);

                // Type de la requête : RESERV(2)
                request_reservation.mtype = TYPE_RESERV;
                // On précise notre PID
                request_reservation.pid = getpid();
                request_reservation.spectacle = index;
                request_reservation.nb_places = nb_places;
                
                // Envoi du message sur la file
                msgsnd(msgid, &request_reservation,
                    sizeof(Request_reservation) - sizeof(long), 0);
                    
                // Attente d'une réponse avec notre PID dans le mtype
                msgrcv(msgid, &response_reservation,
                    sizeof(Response_reservation) - sizeof(long),
                    request_reservation.pid, 0);

                // Vérification de la confirmation du serveur
                if (response_reservation.ack) {
                    printf("Réservation OK. À bientôt dans nos salles !\n");
                } else {
                    printf("Réservation NOK. Réessayez.\n");
                }
                break;

            default:
                printf("Choix invalide\n");
        }
    }

    return 0;
}
