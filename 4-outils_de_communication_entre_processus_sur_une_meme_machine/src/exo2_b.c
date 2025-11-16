#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

/* Boucle principale */

int main() {

/* Définition de la taille du pipe et du tableau */

int pipe1[2];
char r_msg[8];

/* Création du pipe */

pipe(pipe1);

/* Premier fork */

pid_t ret1 = fork();

if (ret1 == 0){

close(pipe1[0]);
close(pipe1[1]);
exit(0);

}

/* Second fork */

pid_t ret2 = fork();

if(ret2 == 0){

close(pipe1[1]);
/* Lecture du message */
read(pipe1[0],r_msg,7);
close(pipe1[0]);
printf("Message du père : %s\n",r_msg);
exit(0);

}

close(pipe1[0]);
/* Ecriture du message */
write(pipe1[1],"bonjour",7);
close(pipe1[1]);

waitpid(ret1,NULL,0);
waitpid(ret2,NULL,0);

exit(0);
}
