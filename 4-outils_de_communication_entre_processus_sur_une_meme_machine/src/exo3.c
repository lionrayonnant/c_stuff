#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {

/* Define pipe and data size */

int pipe1[2];

int f_nb1;
int f_nb2;
char f_nb1c[10];
char f_nb2c[10];
char f_sumc[11];

int c_nb1;
int c_nb2;
int c_sum;
char c_nb1c[10];
char c_nb2c[10];
char c_sumc[11];

/* Initiate pipe */

pipe(pipe1);

/* Initiate fork */

pid_t ret1 = fork();

if(ret1==0){

read(pipe1[0],c_nb1c,9);
read(pipe1[0],c_nb2c,9);
close(pipe1[0]);

c_nb1=atoi(c_nb1c);
c_nb2=atoi(c_nb2c);

c_sum = c_nb1 + c_nb2;

sprintf(c_sumc, "%d", c_sum);

write(pipe1[1],c_sumc,10);
close(pipe1[1]);

exit(0);
}
/* Ask for user input */

printf("Bienvenue dans le programme d'addition en C.\n");
printf("Entrez la valeur A (0-9999999999) : \n");
scanf("%d",&f_nb1);
printf("Vous avez donné la valeur A suivante : %d\n",f_nb1);
printf("Entrez la valeur B (0-9999999999) : \n");
scanf("%d",&f_nb2);
printf("Vous avez donné la valeur B suivante : %d\n",f_nb2);

/*Send data over pipe */

sprintf(f_nb1c, "%d", f_nb1);
sprintf(f_nb2c, "%d", f_nb2);

write(pipe1[1],f_nb1c,9);
write(pipe1[1],f_nb2c,9);

close(pipe1[1]);

waitpid(ret1,NULL,0);

read(pipe1[0],f_sumc,10);

close(pipe1[0]);

printf("Somme de vos deux valeurs : %s\n", f_sumc);

}
