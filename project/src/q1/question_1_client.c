#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_SPECTACLE 10

#define DATA_FILE "spectacles.dat"

typedef struct {
    int id;
    char nom[50];
    char date[11];
    int places;
} Spectacle;

/* Table des spectacles pour consultation */
Spectacle table_spectacles[MAX_SPECTACLE] = {};

void load_spectacles() {

    int fd = open(DATA_FILE, O_RDONLY); //lecture seule

    if (fd == -1) {
        printf("Aucun spectacle n'est disponible pour le moment.\n");
   
    } else {
        read(fd, table_spectacles, sizeof(table_spectacles)); //écriture dans la table des spectacles
        close(fd);
    }

}

int main() {
    int tub1, tub2, tub3, tub4, tub5;
    int choice;
    int index, nb_places;
    bool success;

    load_spectacles();

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

                printf("\n=== SPECTACLES DISPONIBLES ===\n");
                for (int i = 0; i < MAX_SPECTACLE; i++) {
                    if (table_spectacles[i].id != 0) {
                        printf("[%d] %s | Date : %s\n", i, table_spectacles[i].nom, table_spectacles[i].date);
                    }
                }

                printf("\nEntrez le numéro du spectacle à consulter : ");
                scanf("%d", &index);

                /* Envoi au serveur */
                tub1 = open("tub1_view", O_WRONLY);
                tub2 = open("tub2_view", O_RDONLY);

                write(tub1, &index, sizeof(int));
                read(tub2, &nb_places, sizeof(int));

                if (nb_places >= 0) {
                    printf("Selon le serveur : %d places sont disponibles\n", nb_places);
                } else {
                    printf("Erreur : spectacle inexistant\n");
                }

                close(tub1);
                close(tub2);
                break;

            case 2: /* RESERVATION */
                printf("\n=== RÉSERVATION ===\n");
                printf("Numéro du spectacle à réserver : ");
                scanf("%d", &index);
                printf("Nombre de places à réserver : ");
                scanf("%d", &nb_places);

                tub3 = open("tub1_reservation", O_WRONLY);
                tub4 = open("tub2_reservation", O_WRONLY);
                tub5 = open("tub3_reservation", O_RDONLY);

                write(tub3, &index, sizeof(int));
                write(tub4, &nb_places, sizeof(int));
                read(tub5, &success, sizeof(bool));

                if (success) {
                    printf("Réservation OK. A bientôt dans nos salle !\n");
                } else {
                    printf("Réservation NOK. Reessayez avec un nombre de places plus petit ou un autre spectacle.\n");
                }

                close(tub3);
                close(tub4);
                close(tub5);
                break;

            default:
                printf("Choix invalide\n");
        }
    }

    return 0;
}
