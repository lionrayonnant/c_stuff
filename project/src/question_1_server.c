// Import des dépendances
#include <stdio.h> // Pour les E/S (printf ici notamment)
#include <fcntl.h> // Pour utiliser O_RDONLY, O_WRONLY...
#include <unistd.h> // Pour effectuer des read() et write(). C'est l'accès à l'API POSIX
#include <stdlib.h> // Pour exit()
#include <sys/types.h> // Pour les types, notamment pid_t
#include <sys/stat.h> // Pour la création des tubes avec mkfifo
#include <stdbool.h> // Pour utiliser les booléens (dans la réponse au client)
#include <sys/wait.h> // Pour que le père puisse attendre ses fils
#include <sys/shm.h> // Pour la gestion de la mémoire partagée
#include <semaphore.h> // Pour utiliser les sémaphores (et non pas sem.h pour SystemV)

// Clefs statiques pour la mémoire partagée et les sémaphores
#define SHM_KEY 1234
#define SEM_KEY 5678

// Nombre maximal de spectacles
#define MAX_SPECTACLE 10

// Fichier de stockage des données de spectacles
#define DATA_FILE "spectacles.dat"

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

sem_t *mutex;

int shmid = -1;
Spectacle *shared_table = NULL;

/* Fonction de sauvegarde des spectacles dans le fichier */
void save_spectacles() {
    int fd = open(DATA_FILE, O_WRONLY | O_TRUNC | O_CREAT, 0666); // O_TRUNC évite d'écrire à la suite, cela réécris le fichier ; O_CREAT créé le fichier si inexistant
    write(fd, shared_table, sizeof(table_spectacles));
    close(fd);
}

int main() {
    int request_view, response_view;
    int request_reservation, request_reservation_nb;
    bool response_reservation;

    // Supprime le sémaphore s'il existe déjà (pour éviter les erreurs)
    sem_unlink("/semaphore");
    mutex = sem_open("/semaphore", O_CREAT, 0666, 1);
    shmid = shmget(SHM_KEY, sizeof(table_spectacles), IPC_CREAT | 0666);

    shared_table = (Spectacle *)shmat(shmid, NULL, 0);

    // Ouverture des données dans le fichier de data
    int fd = open("spectacles.dat", O_RDONLY); //lecture seule
    
    // Si le fichier n'existe pas ou est vide, on copie les données d'exemple
    if (fd == -1) {
        memcpy(shared_table, table_spectacles, sizeof(table_spectacles));
        // Puis on sauvegarde dans le fichier
        save_spectacles();
    } else {
        // Sinon, on charge depuis le fichier vers la mémoire partagée
        read(fd, shared_table, sizeof(table_spectacles));
        close(fd);
    }

    // Création des tubes
    mkfifo("tub1_view", 0666);
    mkfifo("tub2_view", 0666);
    mkfifo("tub1_reservation", 0666);
    mkfifo("tub2_reservation", 0666);
    mkfifo("tub3_reservation", 0666);

    printf("[INFO] Serveur UP\n");
        
    // Consultation

    if (fork() == 0) { // processus fils -> consultation

        int tub1 = open("tub1_view", O_RDWR);
        int tub2 = open("tub2_view", O_RDWR);

        while(1) {
        read(tub1, &request_view, sizeof(int));
        
        sem_wait(mutex);

        if (request_view >= 0 && request_view < MAX_SPECTACLE) {
            response_view = shared_table[request_view].places;
            printf("[CONSULTATION] Spectacle %d : %d places\n", request_view, response_view);
       } else {
            printf("[CONSULTATION] Numéro invalide : %d\n", request_view);
        }
        
        sem_post(mutex);
        
        write(tub2, &response_view, sizeof(int));
        }
    close(tub1); close(tub2);
    exit(0); // exit du fils
    }

    // Réservation

    if (fork() == 0) { // processus fils -> réservation

        int tub3 = open("tub1_reservation", O_RDWR); // RDWR pour ne pas avoir de problèmes sur le tube, bien que mauvaise pratique
        int tub4 = open("tub2_reservation", O_RDWR);
        int tub5 = open("tub3_reservation", O_RDWR);

        while(1) {
        read(tub3, &request_reservation, sizeof(int));
        read(tub4, &request_reservation_nb, sizeof(int));
        sem_wait(mutex);

        if (request_reservation >= 0 && request_reservation < MAX_SPECTACLE &&
                request_reservation_nb > 0 && shared_table[request_reservation].places >= request_reservation_nb) {
            
            shared_table[request_reservation].places -= request_reservation_nb;
            save_spectacles();
            response_reservation = true;
            printf("[RESERVATION] %d places réservées pour spectacle %d.\nPlaces restantes : %d\n",
                   request_reservation_nb, request_reservation, shared_table[request_reservation].places);
        } else {
            response_reservation = false;
            printf("[RESERVATION] Impossible pour le spectacle %d\n",request_reservation);
        }

        sem_post(mutex);

        write(tub5, &response_reservation, sizeof(bool));
        }
    close(tub3); close(tub4); close(tub5);
    exit(0); // exit du fils
    }

    wait(NULL);
    wait(NULL);

    shmdt(shared_table);
    shmctl(shmid, IPC_RMID, NULL);
    sem_close(mutex);
    sem_unlink("/semaphore");
    return 0;
}
