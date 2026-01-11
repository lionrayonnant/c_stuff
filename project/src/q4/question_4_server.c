#include <stdio.h> // Pour les E/S (printf ici notamment)
#include <fcntl.h> // Pour utiliser O_RDONLY, O_WRONLY...
#include <unistd.h> // Pour effectuer des read() et write(). C'est l'accès à l'API POSIX
#include <stdbool.h> // Pour utiliser les booléens (dans la réponse au client)
#include <sys/types.h> // Pour les types, notamment pid_t
#include <sys/ipc.h> // Permets la gestion des IPCs (ftok...)
#include <sys/msg.h> // Permets d'utiliser les MSQ
#include <stdlib.h> // Pour exit()
#include <string.h> // Pour memcpy
#include <semaphore.h> // Pour utiliser les sémaphores (et non pas sem.h pour SystemV)
#include <pthread.h> // Permets d'utiliser les threads

// Types de requêtes
#define TYPE_LIST 1
#define TYPE_RESERV 2

// Nombre maximal de spectacles
#define MAX_SPECTACLE 10

// Fichier de données
#define DATA_FILE "spectacles.dat"

// Définition de la structure des spectacles
typedef struct {
    int id;
    char nom[50];
    char date[11];
    int places;
} Spectacle;

// Définition des requêtes (générique)
typedef struct {
    long mtype;
    pid_t pid;
    int spectacle;
    int nb_places;
} Request;

// Définition des réponses de type LIST
typedef struct {
    long mtype;
    pid_t pid;
    Spectacle spectacles[MAX_SPECTACLE];
} Response_list;

// Définition des réponses de type RESERV
typedef struct {
    long mtype;
    pid_t pid;
    bool ack;
} Response_reservation;

// Données d'exemple
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

// Déclaration du sémaphore (mutex)
sem_t mutex;

// Sauvegarde dans fichier
void save_spectacles() {
    int fd = open(DATA_FILE, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    write(fd, table_spectacles, sizeof(table_spectacles));
    close(fd);
}

// Thread qui traite une seule requête
void* thread_handler(void* arg) {
    // Strucutre pour les données reçues en argument
    struct {
        Request* req; // Déclaration du pointeur req de type Request
        int msgid; // Déclaration de msgid
    } *data = arg; // Déclaration du pointeur data avec le contenu de arg

    Request* req = data->req; // Le pointeur req contient data->req (la requête)
    int msgid = data->msgid; // msgid contient data->msgid (le msgid à utiliser)

    // Si le mtype est de type LIST
    if (req->mtype == TYPE_LIST) {
        // Log sur la console
        printf("[CONSULATION] Client %i a effectué une demande de consultation.\n",req->pid);
        Response_list resp; // Déclaration de la réponse de type list
        resp.mtype = req->pid; // resp.mtype est égal au pid contenu dans la requête
        resp.pid = getpid(); // resp.pid = pid du serveur

        sem_wait(&mutex); // Demande du mutex
        // Copie des données en mémoire dans la réponse
        memcpy(resp.spectacles, table_spectacles, sizeof(table_spectacles));
        sem_post(&mutex); // Relâchement du mutex
        // Envoi de la réponse sur la MSQ
        msgsnd(msgid, &resp, sizeof(Response_list) - sizeof(long), 0);
    } else if (req->mtype == TYPE_RESERV) { // Si la requête est de type RESERV
        Response_reservation resp; // Déclaration de la réponse de type reserv
        resp.mtype = req->pid; // resp.mtype est égal au pid fourni dans la requête
        resp.pid = getpid(); // resp.pid = pid du serveur
        // Relâchemen duu sémaphore
        sem_wait(&mutex);
        // Vérification des entrées :
        // Si le spectacle demandé existe, que lenb de places n'est pas égal à 0
        // et que il reste suffisament de places
        if (req->spectacle >= 0 && req->spectacle < MAX_SPECTACLE &&
            req->nb_places > 0 &&
            table_spectacles[req->spectacle].places >= req->nb_places) {
            // Décrémentation du nombre de places demandées
            table_spectacles[req->spectacle].places -= req->nb_places;
            // Sauvegarde dans le fichier DAT
            save_spectacles();
            // On définis le ACK à True
            resp.ack = true;
            // Log sur la console
            printf("[RESERVATION] Client %i réservé %d place(s) pour le spectacle %d. Places restantes : %d\n",
                   req->pid, req->nb_places, req->spectacle,
                   table_spectacles[req->spectacle].places);
        } else {
            // Si entrées non valides, ACK à False
            resp.ack = false;
            // Log de l'erreur dans la console
            printf("[RESERVATION] Impossible pour le spectacle %d et le client %i\n",
                   req->spectacle, req->pid);
        }
        // Relâchement du mutex
        sem_post(&mutex);
        // Envoi de la réponse sur la MSQ
        msgsnd(msgid, &resp, sizeof(Response_reservation) - sizeof(long), 0);
    }
    // Libération de l'espace mémoire utilisé
    free(req);
    free(data);
    // Fin du thread
    pthread_exit(NULL);
}

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
    // Création de clef de la MSQ avec ftok()
    key_t key = ftok("question_4_server", 42);
    // Rejoindre ou créer la MSQ
    int msgid = msgget(key, 0666 | IPC_CREAT);
    // Initialisation du mutex
    sem_init(&mutex, 0, 1);
    // Log dans la console
    printf("[INFO] Serveur UP\n");

    // Boucle principale
    while (1) {
        // Le pointeur req de type Request obtient dynamiquement un espace en mémoire
        Request* req = malloc(sizeof(Request));
        // Réception de n'importe quel message sur la MSQ
        msgrcv(msgid, req, sizeof(Request) - sizeof(long), 0, 0);

        // On crée une structure pour passer le msgid et la requête au thread
        // On ne peu passer qu'un seul argument au thread, on est donc obligés de faire ça
        struct {
            Request* req; // Pointeur req de type Request
            int msgid; // Déclarationd e msgid
        } *data = malloc(sizeof(*data)); // Le pointeur data obtient dynamiquement
                                         // un espace en MC de la taille de *data

        data->req = req; // On rempli le contenu de data->req avec la requête reçue
        data->msgid = msgid; // On rempli le contenu de msgid-> msgid avec le msgid de la MSQ
        // Déclaration du thread
        pthread_t th;
        // Création du thread avec la fonction associée, la requête et le msgid en argument
        pthread_create(&th, NULL, thread_handler, data);
        // On utilise detach au lieu de join pour pouvoir lancer plusieurs threads en parrallèle
        // sans avoir besoin d'attendre la fin du thread précédent. On gagne en performances
        // en ayant plusieurs threads en parralèle.
        pthread_detach(th);
    }
    // Cette partie n'est jamais atteinte car pas de gestion des signaux et boucle infinie
    // Destruction du sémpahore (mutex)
    sem_destroy(&mutex);
    // Destruction de la MSQ
    msgctl(msgid, IPC_RMID, NULL);
    // Retourne 0 pour indiquer qu'il n'y a pas d'erreurs
    return 0;
}
