#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main () {

pid_t ret;
ret = fork();

if(ret == 0){
    printf("Je suis le fils, mon pid est : %d\n", getpid()); 
    printf("Le pid de mon père est : %d\n", getppid());

    /*Recouvrement du code */

    execl("/bin/ps", "ps", "-a", NULL);
    }

else{
    printf("Je suis le père, mon pid est : %d\n", getpid());
    printf("Le pid de mon fils est : %d\n", ret);
    wait(NULL);
}

return 0;
}
