#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/wait.h>

key_t cle;

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
        scanf("%ld", &add1.a);
        printf("Please give B :\n");
        scanf("%ld", &add1.b);
        printf("You gave %ld and %ld\n",add1.a, add1.b);
	add1.pid_client = my_pid;
        /* Send request to the server */

        if (msgsnd(msqid, &add1, sizeof(add1) - sizeof(add1.mtype), 0) == -1 ) {
                perror("msgsnd");
                exit(EXIT_FAILURE);

	}

        /* Wait for response from the server */

        if (msgrcv(msqid, &add1_resp, sizeof(add1_resp) - sizeof(add1_resp.mtype), my_pid, 0) == -1 ) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
        }

        /* Return response */

	return add1_resp.result;

        };

int mul() {

	struct mul mul1;
        struct response_mul mul1_resp;
        mul1.mtype = 1;
        printf("Please give A :\n");
        scanf("%f", &mul1.a);
        printf("Please give B :\n");
        scanf("%f", &mul1.b);
	printf("Please give C :\n");
	scanf("%f", &mul1.c);
        printf("You gave %f, %f and %f\n",mul1.a, mul1.b, mul1.c);
        mul1.pid_client = my_pid;
        /* Send request to the server */

        if (msgsnd(msqid, &mul1, sizeof(mul1) - sizeof(mul1.mtype), 0 ) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);

        }

	/* Wait for response from the server */

        if (msgrcv(msqid, &mul1_resp, sizeof(mul1_resp) - sizeof(mul1_resp.mtype), my_pid, 0 ) == -1 ) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
        }

        /* Print response */

        return mul1_resp.result;

	};

/* Main loop */

int main() {

int choice;
long add_result;
float mul_result;

my_pid = getpid();
printf("My PID : %d\n",my_pid);

printf("MSQ id : %d\n",msqid);

printf("Welcome. Please select 'add' or 'mul' :\n [0] : add\n [1] : mul\n");
scanf("%d",&choice);

switch(choice) {
	case 0:
	cle = 100;
	msqid = msgget(cle,0);
	add_result = add();
	printf("Result : %ld\n",add_result);
	break;

	case 1:
	cle = 200;
	msqid = msgget(cle,0);
	mul_result = mul();
	printf("Result : %f\n",mul_result);
	break;
}

exit(0);

};
