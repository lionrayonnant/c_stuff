#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {

int pipe1[2];
char r_msg[8];

pipe(pipe1);

pid_t ret1 = fork();

if (ret1 == 0){

close(pipe1[0]);
close(pipe1[1]);
exit(0);

}

pid_t ret2 = fork();

if(ret2 == 0){

close(pipe1[1]);
read(pipe1[0],r_msg,7);
close(pipe1[0]);
printf("Message du père : %s\n",r_msg);
exit(0);

}

close(pipe1[0]);
write(pipe1[1],"bonjour",7);
close(pipe1[1]);

waitpid(ret1,NULL,0);
waitpid(ret2,NULL,0);

exit(0);
}
