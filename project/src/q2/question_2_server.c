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
#include <string.h> // Pour memcpy
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Types de requêtes
#define TYPE_CONSUL 1 // Consultation
#define TYPE_RESERV 2 // Réservation
#define TYPE_LIST   3

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
    // Entiers pour la réservation
    int request_reservation, request_reservation_nb;
    // Booléen pour la réponse à la réservation
    bool response_reservation;

    key_t key = ftok("question_2_server", 42);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    // Supprime le sémaphore s'il existe déjà (pour éviter les erreurs)
    sem_unlink("/semaphore");
    // Ouverture du sémaphore (nommé)
    mutex = sem_open("/semaphore", O_CREAT, 0666, 1);
    
    // Création d'un segment de mémoire partagée
    shmid = shmget(SHM_KEY, sizeof(table_spectacles), IPC_CREAT | 0666);
    // Attachement à la mémoire partagée via un cast
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

    printf("[INFO] Serveur UP\n");
    
    if (fork() == 0) {

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

        Request_list request_list;
        Response_list response_list;

        while(1) {

            msgrcv(msgid, &request_list, sizeof(Request_list) - sizeof(long), TYPE_LIST, 0);

            response_list.mtype = request_list.pid;
            response_list.pid = getpid();
            
            sem_wait(mutex);
            memcpy(response_list.spectacles, shared_table, sizeof(table_spectacles));
            sem_post(mutex);

            msgsnd(msgid, &response_list, sizeof(Response_list) - sizeof(long), 0);
    }
        exit(0);
    }
    // Consultation

    if (fork() == 0) { // processus fils -> consultation

        // Définition de la structure pour les consultations
        typedef struct {
            long mtype;
            pid_t pid;
            int spectacle;
        } Request_view;

        typedef struct {
            long mtype;
            pid_t pid;
            int nb_places;

        } Response_view;
        Request_view request_view;
        Response_view response_view;

        while(1) {
        
        msgrcv(msgid, &request_view, sizeof(Request_view) - sizeof(long), TYPE_CONSUL, 0);
        
        sem_wait(mutex);

    if (request_view.spectacle >= 0 &&
        request_view.spectacle < MAX_SPECTACLE &&
        shared_table[request_view.spectacle].id != 0) {

            response_view.nb_places = shared_table[request_view.spectacle].places;
            printf("[CONSULTATION] Spectacle %d : %d places\n", request_view.spectacle, response_view.nb_places);
       } else {
            printf("[CONSULTATION] Numéro invalide : %d\n", request_view.spectacle);
        }
        
        sem_post(mutex);
        response_view.mtype = request_view.pid;
        response_view.pid = getpid();
        msgsnd(msgid,&response_view, sizeof(Response_view) - sizeof(long), 0);
        }
    exit(0); // exit du fils
    }

    // Réservation

    if (fork() == 0) { // processus fils -> réservation

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
        }  Response_reservation;

        Request_reservation request_reservation;
        Response_reservation response_reservation;

        while(1) { // Boucle infinie de traitement
        
        // attente du msg

        msgrcv(msgid, &request_reservation, sizeof(Request_reservation) - sizeof(long), TYPE_RESERV, 0);

        sem_wait(mutex); // Attente du sémaphore de type mutex

	// Vérification des entrées
        if (request_reservation.spectacle >= 0 && request_reservation.spectacle < MAX_SPECTACLE &&
                request_reservation.nb_places > 0 && shared_table[request_reservation.spectacle].places >= request_reservation.nb_places
                 && shared_table[request_reservation.spectacle].id != 0) {
            
            shared_table[request_reservation.spectacle].places -= request_reservation.nb_places;
            save_spectacles();
            response_reservation.ack = true;
            printf("[RESERVATION] %d places réservées pour spectacle %d.\nPlaces restantes : %d\n",
                   request_reservation.nb_places, request_reservation.spectacle, shared_table[request_reservation.spectacle].places);
        } else {
            response_reservation.ack = false;
            printf("[RESERVATION] Impossible pour le spectacle %d\n",request_reservation.spectacle);
        }

        sem_post(mutex);

        response_reservation.mtype = request_reservation.pid;
        response_reservation.pid = getpid();
        msgsnd(msgid,&response_reservation, sizeof(Response_reservation) - sizeof(long), 0);

        }
    
    exit(0); // exit du fils
    }

    wait(NULL);
    wait(NULL);

    msgctl(msgid, IPC_RMID, NULL);
    shmdt(shared_table);
    shmctl(shmid, IPC_RMID, NULL);
    sem_close(mutex);
    sem_unlink("/semaphore");
    return 0;
}
