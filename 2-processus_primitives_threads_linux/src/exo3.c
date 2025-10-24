#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main () {
int i = 2;

pid_t ret = fork();

if(ret == 0) {
    printf("Je suis le fils, mon PID est : %d\n", getpid());
    while(i <= 35) {
	    printf("La valeur de i est :  %d\n", i);
	    i = i + 3;

    }   
}
else {
    printf("Je suis le père, mon PID est : %d\n", getpid());
    while(i <=24) {
        printf("La valeur de i est : %d\n", i);
        i = i+2;
}
wait(NULL);
}
return 0;
}
