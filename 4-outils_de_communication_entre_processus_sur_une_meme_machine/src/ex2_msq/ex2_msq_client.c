#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/wait.h>

key_t cle = 100;

/* Globals */

pid_t my_pid;
int msqid;

/* Define structures */

struct add {
	long mtype;
	long a;
	long b;
	pid_t pid_client;
};

struct mul {
	long mtype;
	float a;
	float b;
	float c;
	pid_t pid_client;
};

struct response_add {
    long mtype;
    long result;
};

struct response_mul {
	long mtype;
	float result;
};

int add(){

        struct add add1;
	struct response_add add1_resp;
        add1.mtype = 1;
        printf("Please give A :\n");
        scanf("%d", &add1.a);
        printf("Please give B :\n");
        scanf("%d", &add1.b);
        printf("You gave %d and %d\n",add1.a, add1.b);
	add1.pid_client = my_pid;
        /* Send request to the server */

        if (msgsnd(msqid, &add1, sizeof(add1) - sizeof(add1.mtype), 0) == -1 ) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
        }

        /* Wait for response from the server */

        if (msgrcv(msqid, &add1_resp, sizeof(add1_resp) - sizeof(add1_resp), my_pid, 0) == -1 ) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
        }

        /* Print response */

        printf("Result : %d\n",add1_resp.result);

        };

int mul() {

};

/* Main loop */

int main() {

my_pid = getpid();
printf("My PID : %d",my_pid);

int msqid = msgget(cle,0);

int choice;

printf("Welcome. Please select 'add' or 'mul' :\n [0] : add\n [1] : mul");
scanf("%d",choice);

switch(choice) {
	case 0:
	add();
	break;

	case 1:
	break;
}

exit(0);

};
