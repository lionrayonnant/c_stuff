#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Types de requêtes
#define TYPE_CONSUL 1
#define TYPE_RESERV 2
#define TYPE_LIST   3

#define MAX_SPECTACLE 10

#define DATA_FILE "spectacles.dat"

typedef struct {
    int id;
    char nom[50];
    char date[11];
    int places;
} Spectacle;

// Demande de la liste
typedef struct {
    long mtype;
    pid_t pid;
} Request_list;

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

typedef struct {
    long mtype;
    pid_t pid;
    bool ack;
} Response_reservation;

Request_reservation request_reservation;
Response_reservation response_reservation;

Request_list request_list;
Response_list response_list;

/* Table des spectacles pour consultation */
Spectacle table_spectacles[MAX_SPECTACLE] = {};

int main() {
    int choice;
    int index, nb_places;
    bool success;

    key_t key = ftok("question_2_server", 42);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Consultation\n");
        printf("2. Réservation\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choice);

        if (choice == 0) break;

        switch (choice) {
            case 1: /* CONSULTATION */

                request_list.mtype = TYPE_LIST;
                request_list.pid = getpid();
                msgsnd(msgid,&request_list, sizeof(Request_list) - sizeof(long), 0);

                msgrcv(msgid, &response_list, sizeof(Response_list) - sizeof(long), request_list.pid, 0);

                printf("\n=== SPECTACLES DISPONIBLES ===\n");

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

            case 2: /* RESERVATION */
                printf("\n=== RÉSERVATION ===\n");
                printf("Numéro du spectacle à réserver : ");
                scanf("%d", &index);
                printf("Nombre de places à réserver : ");
                scanf("%d", &nb_places);

                request_reservation.mtype = TYPE_RESERV;
                request_reservation.pid = getpid();
                request_reservation.spectacle = index;
                request_reservation.nb_places = nb_places;

                msgsnd(msgid, &request_reservation,
                    sizeof(Request_reservation) - sizeof(long), 0);

                msgrcv(msgid, &response_reservation,
                    sizeof(Response_reservation) - sizeof(long),
                    request_reservation.pid, 0);

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
