#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

/* Boucle principale */

int main() {

/* Définition du pipe et du tableau pour stocker le message */
int pipe1[2];
char r_msg[8];

/* Création du pipe */

pipe(pipe1);

/* Premier fork (écrivain) */

pid_t ret1 = fork();

if (ret1 == 0){

close(pipe1[0]);
write(pipe1[1],"Bonjour",7);
close(pipe1[1]);
exit(0);

}

/* Second fork (lecteur) */

pid_t ret2 = fork();

if (ret2 == 0) {

close(pipe1[1]);
read(pipe1[0],r_msg,7);
close(pipe1[0]);
printf("Message du processus 1 : %s\n",r_msg);
exit(0);

}

close(pipe1[0]);
close(pipe1[1]);

waitpid(ret1,NULL,0);
waitpid(ret2,NULL,0);

}
