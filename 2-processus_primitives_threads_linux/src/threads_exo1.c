#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/* Déclaration des fonctions */

void *addition(void* args){
   
    int* arguments = (int*)args;

    int a = arguments[0];
    int b = arguments[1];
    int x;
	
    void *ret1;

    x=a+b;

    printf("Addition de %d et de %d : %d\n", a, b, x);

	/* Fermeture propre et retour du code d'erreur*/

    pthread_exit(ret1);
}

void *multiplication(void* args) {
   
    int* arguments = (int*)args;

    int a = arguments[0];
    int b = arguments[1];
    int x;

    void *ret1;

    x=a*b;

    printf("Multiplication de %d et de %d : %d\n", a, b, x);

	/* Fermeture propre et retour du code d'erreur*/
	
    pthread_exit(ret1);
}

/* Fonction principale */

int main (){

int args[2] = {6,7};
void *ret2;
void *ret1;

/* Création des threads */

pthread_t t_addition;
pthread_create(&t_addition, NULL, addition, args);

pthread_t t_multiplication;
pthread_create(&t_multiplication, NULL, multiplication, args);

/* Rejoindre les threads */

pthread_join(t_addition, &ret1);
pthread_join(t_multiplication, &ret2);

/* Affichage du code de retour */

printf("Le thread t_addition a retourné %d\n", ret1);
printf("Le thread t_multiplication a retourné %d\n", ret2);
return 0;
}
