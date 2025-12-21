#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

// Nombre maximal de spectacles

#define MAX_SPECTACLE 10

// Fichier de stockage des données de spectacles

#define DATA_FILE "spectacles.csv"

// Définition de la structure pour les spectacles
typedef struct {
    int id;
    char nom[50];
    char date[11];
    int places;
} Spectacle;

// Création de la table des spectacles
Spectacle table_spectacles[MAX_SPECTACLE] = {
    {1, "Génération Céline – 4 voix pour une légende", "2027-01-15", 70},
    {2, "Claudio Capéo en concert", "2027-01-23", 80},
    {3, "The Music of Square Enix – Memories & Melodies", "2027-02-06", 90},
    {4, "Bonhomme (Jarry) – Seul en scène", "2027-02-26", 60},
    {5, "Marine (Delplace)", "2027-03-07", 55},
    {6, "Flora Fishbach en concert", "2027-03-12", 75},
    {7, "THÉA en concert", "2027-03-19", 50},
    {8, "Concert symphonique Saison Paris", "2027-04-05", 65},
    {9, "Spectacle théâtre à la Comédie Française", "2027-04-20", 40},
    {10, "Festival musique émergente", "2027-05-10", 30}
};

// fonction de chargement des spectacles
void load_spectacles() {
    int fd = open("spectacles.dat", O_RDONLY); //lecture seule

    if (fd == -1) {
        // Création du fichier et utilisation des données d'exemple si inexistant
        fd = open("spectacles.dat", O_WRONLY | O_CREAT, 0666);
        write(fd, table_spectacles, sizeof(table_spectacles));
        close(fd);
        printf("[INFO] Fichier créé et initialisé\n");
    } else {
        ssize_t n = read(fd, table_spectacles, sizeof(table_spectacles));
        close(fd);

        if (n == 0) {
            // Remplissage du fichier si vide avec des données d'exemple
            fd = open("spectacles.dat", O_WRONLY | O_TRUNC);
            write(fd, table_spectacles, sizeof(table_spectacles));
            close(fd);
            printf("[INFO] Fichier vide, données d'exemple ajoutées\n");
        } else {
            printf("[INFO] Données chargées depuis le fichier\n");
        }
    }
}

/* Sauvegarde des spectacles dans le fichier */
void save_spectacles() {
    int fd = open(DATA_FILE, O_WRONLY | O_TRUNC | O_CREAT, 0666); // O_TRUNC évite d'écrire à la suite, cela réécris le fichier ; O_CREAT créé le fichier si inexistant
    write(fd, table_spectacles, sizeof(table_spectacles));
    close(fd);
}

int main() {
    int request_view, response_view;
    int request_reservation, request_reservation_nb;
    bool response_reservation;

    // Création des tubes
    mkfifo("tub1_view", 0666);
    mkfifo("tub2_view", 0666);
    mkfifo("tub1_reservation", 0666);
    mkfifo("tub2_reservation", 0666);
    mkfifo("tub3_reservation", 0666);

    load_spectacles();

    printf("[INFO] Serveur UP\n");

    while (1) {
        // Consultation

        if (fork() == 0) { // processus fils -> consultation
            int tub1 = open("tub1_view", O_RDWR);
            int tub2 = open("tub2_view", O_RDWR);
            read(tub1, &request_view, sizeof(int));

            // Gestion du compte à partir de 0 en C
            //request_view = request_view - 1;

            if (request_view >= 0 && request_view < MAX_SPECTACLE &&
                table_spectacles[request_view].id != 0) {
                response_view = table_spectacles[request_view].places;
                printf("[CONSULTATION] Spectacle %d : %d places\n", request_view, response_view);
            } else {
                printf("[CONSULTATION] Numéro invalide : %d\n", request_view);
            }

            write(tub2, &response_view, sizeof(int));
            close(tub1); close(tub2);
            exit(0); // exit du fils
        }

        // Réservation

        if (fork() == 0) { // processus fils -> réservation
            int tub3 = open("tub1_reservation", O_RDWR);
            int tub4 = open("tub2_reservation", O_RDWR);
            int tub5 = open("tub3_reservation", O_RDWR);
            
            read(tub3, &request_reservation, sizeof(int));
            read(tub4, &request_reservation_nb, sizeof(int));

            // Gestion du compte à partir de 0 en C
            //request_reservation = request_reservation -1;

            if (request_reservation >= 0 && request_reservation < MAX_SPECTACLE &&
                table_spectacles[request_reservation].id != 0 &&
                request_reservation_nb > 0 &&
                table_spectacles[request_reservation].places >= request_reservation_nb) {

                table_spectacles[request_reservation].places -= request_reservation_nb;
                save_spectacles();
                response_reservation = true;
                printf("[RESERVATION] %d places réservées pour spectacle %d.\nPlaces restantes : %d\n",
                       request_reservation_nb, request_reservation, table_spectacles[request_reservation].places);
            } else {
                response_reservation = false;
                printf("[RESERVATION] Impossible pour le spectacle %d\n",
                       request_reservation);
            }

            write(tub5, &response_reservation, sizeof(bool));
            close(tub3); close(tub4); close(tub5);
            exit(0); // exit du fils
        }

    }
    wait(NULL);
    return 0;
}
