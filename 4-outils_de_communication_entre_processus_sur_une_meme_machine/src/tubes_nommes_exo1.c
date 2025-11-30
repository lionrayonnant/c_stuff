#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main() { 

/* Create tubes */ 

int tub1; int tub2; int tub3; 

/* Create sync pipe */ 

int sync_pipe[2]; 

/* Define char values */ 

char nb1_c[16]; char nb2_c[16]; char sum_c[16]; 

/* Define int values */ 

int nb1_i; int nb2_i; int sum_i; 

/* Define named tubes */ 

mkfifo("tub1_nb1",S_IRUSR | S_IWUSR); 
mkfifo("tub2_nb2",S_IRUSR | S_IWUSR); 
mkfifo("tub3_sum",S_IRUSR | S_IWUSR); 

pipe(sync_pipe); 

pid_t ret1 = fork(); 

if(ret1 == 0) { 

close(sync_pipe[1]); 
char dummy; 
read(sync_pipe[0], &dummy, 1); 
close(sync_pipe[0]); 

tub1 = open("tub1_nb1",O_RDONLY);
tub2 = open("tub2_nb2",O_RDONLY);
tub3 = open("tub3_sum",O_WRONLY);

read(tub1,nb1_c,9);
read(tub2,nb2_c,9);

printf("Nb1 received : %s \n",nb1_c);
printf("Nb2 received : %s \n",nb2_c);

nb1_i = atoi(nb1_c);
nb2_i = atoi(nb2_c);

sum_i = nb1_i + nb2_i; 

sprintf(sum_c, "%d",sum_i); 

printf("Result sent in the tube : %s \n",sum_c);

write(tub3, sum_c,10);

close(tub1); close(tub2); close(tub3); 

exit(0); 

} else { 

pid_t ret2 = fork();

if(ret2 == 0) { 

close(sync_pipe[1]); 

tub1 = open("tub1_nb1",O_WRONLY);
tub2 = open("tub2_nb2",O_WRONLY);
tub3 = open("tub3_sum",O_RDONLY);

write(sync_pipe[0], "X", 1);

close(sync_pipe[0]);

printf("Welcome into the addition program based on named tubes.\n");

printf("Please insert the A value (0-999999999) : \n"); 
scanf("%d",&nb1_i); printf("You have typed : %d as A.\n",nb1_i);

printf("Please insert the B value (0-999999999) : \n");
scanf("%d",&nb2_i); printf("You have typed : %d as B.\n",nb2_i); 

sprintf(nb1_c, "%d", nb1_i); sprintf(nb2_c, "%d", nb2_i); 

write(tub1, nb1_c,9);
write(tub2, nb2_c,9);

close(tub1); close(tub2);

read(tub3,sum_c,10);

printf("Result obtained as reponse : %s \n",sum_c);

close(tub3); 

} else { 

wait(NULL);
wait(NULL);
unlink("tub1_nb1");
unlink("tub2_nb2");
unlink("tub3_sum"); } 

}

return 0;

}
