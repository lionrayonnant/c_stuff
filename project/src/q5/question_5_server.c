#include <stdio.h> // Pour les E/S (printf ici notamment)
#include <fcntl.h> // Pour utiliser O_RDONLY, O_WRONLY...
#include <stdlib.h> // Pour exit()
#include <unistd.h> // Pour effectuer des read() et write(). C'est l'accès à l'API POSIX
#include <stdbool.h> // Pour utiliser les booléens (dans la réponse au client)
#include <string.h> // Pour memcpy
#include <pthread.h> // Permets d'utiliser les threads
#include <semaphore.h> // Pour utiliser les sémaphores (et non pas sem.h pour SystemV)
#include <netinet/in.h> // Pour utiliser AF_INET et AF_INET6

// Définition du port pour le socket
#define PORT 5000

// Nombre maximal de spectacles
#define MAX_SPECTACLE 10

// Définition des types de requêtes
#define TYPE_LIST   1
#define TYPE_RESERV 2

// Définition du fichier de données (persistance)
#define DATA_FILE "spectacles.dat"

// Structure des spectacles
typedef struct {
    int id;
    char nom[50];
    char date[11];
    int places;
} Spectacle;

// Structure d'une requête par un client
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

// Table des spectacles avec données par défaut
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

// Déclaration du mutex
sem_t mutex;

// Fonction de sauvegarde des spectacles sur le fichier dédié
void save_spectacles() {
    int fd = open(DATA_FILE, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    write(fd, table_spectacles, sizeof(table_spectacles));
    close(fd);
}

// Fonction de gestion des requêtes client
void* client_handler(void* arg) {
    int sock = *(int*)arg; // sock = l'argument passé à l'appel
    free(arg); // On libère l'espace utilisé par les arguments

    Request req; // Création de req, basé sur Request

    // Lecture des données tant que le client est connecté
    while (read(sock, &req, sizeof(req)) > 0) {

        if (req.type == TYPE_LIST) { // Si le type des "LIST"
            Response_list resp; // Création de resp, basé sur Response_list

            sem_wait(&mutex); // Demande du mutex
            // Copie des données en mémoire dans la réponse
            memcpy(resp.spectacles, table_spectacles, sizeof(table_spectacles));
            // Relâchement du mutex
            sem_post(&mutex);
            // Ecriture de la réponse sur le socket
            write(sock, &resp, sizeof(resp));
        }
        // Si le type est "RESERV"
        else if (req.type == TYPE_RESERV) {
            Response_reservation resp; // Création de resp, basé sur Response_reservation
            resp.ack = false; // Par défaut le ACK est à False
            // Demande du mutex
            sem_wait(&mutex); 
            // Vérification des entrées
            if (req.spectacle >= 0 &&
                req.spectacle < MAX_SPECTACLE &&
                req.nb_places > 0 &&
                table_spectacles[req.spectacle].places >= req.nb_places) {
                // Décrémentation du nombre de places
                table_spectacles[req.spectacle].places -= req.nb_places;
                // Sauvegarde des données
                save_spectacles();
                // Le ACK passe à True
                resp.ack = true;
                // Affichage d'un message de log
                printf("[RESERV] - %d places réservée pour le spectacle %d.\n"
                        "%d places restantes pour le spectacle %d\n",
                       req.nb_places, req.spectacle, table_spectacles[req.spectacle].places, req.spectacle);
            }
            // Relâchement du mutex
            sem_post(&mutex);
            // Ecriture de la réponse sur le socket
            write(sock, &resp, sizeof(resp));
        }
    }
    // Fermeture du socket
    close(sock);
    // Message de log
    printf("[INFO] - Client déconnecté\n");
    return NULL;
}
// Boucle principale
int main() {
        // Ouverture du fichier de données
    int fd = open(DATA_FILE, O_RDONLY);
    // Si il n'existe pas, création avec les données d'exemple
    if (fd == -1) {
        save_spectacles();
    } else { // Sinon on charge les données en MC
        read(fd, table_spectacles, sizeof(table_spectacles));
        close(fd);
    }
    // Création d'un socket 
    // Protocole Ipv4, mode stream (TCP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
    perror("[INFO] - Erreur dans la création du socket");
    exit(1);
    }
    // Déclaration de la structure qui contient les infos du serveur
    struct sockaddr_in addr;
    // Précise IPv4
    addr.sin_family = AF_INET;
    // Mets le port d'écoute sur 5000
    addr.sin_port = htons(PORT);
    // Le serveur écoute sur toutes les interfaces (*).
    addr.sin_addr.s_addr = INADDR_ANY;

    // Associe le socket créé à une IP et un port
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("[INFO] - Erreur dans le bind");
    close(server_fd);
    exit(1);
    }
    // Bascule le serveur en écoute
    listen(server_fd, 10);

    // Initialise le serveur
    sem_init(&mutex, 0, 1);

    // Message de log
    printf("[INFO] - Serveur TCP en écoute sur le port %d\n", PORT);

    // Boucle infinie
    while (1) {
        // Attente d'un client (mode bloquant)
        int client_fd = accept(server_fd, NULL, NULL);
        // Attribution dynamique de mémoire pour le descripteur du client
        // Comme vu en q4, on doit utiliser cela pour passer
        // les informations au thread
        int* pclient = malloc(sizeof(int));
        // On défini le descripteur créé avec ce qu'on a reçu sur le socket
        *pclient = client_fd;

        // Déclaration du thread
        pthread_t th;
        // Création du thread avec pclient en argument
        pthread_create(&th, NULL, client_handler, pclient);
        // pthread_detach pour ne pas attendre la fin
        pthread_detach(th);
    }
}
