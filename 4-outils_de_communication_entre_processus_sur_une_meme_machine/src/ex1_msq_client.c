#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/wait.h>

key_t cle = 217;

/* Define structures */

struct request {
	long mtype;
	long a;
	long b;
	pid_t pid_client;
};

struct response {
    long mtype;
    long sum;
};

/* Main lop */

int main() {

/* Create structures */

struct request req;
struct response resp;

/* Join the MSQ */

int msqid = msgget(cle,0);

/* Display the MSQ id */

printf("MSQ id : %d\n",msqid);

/* Ask for user input */

printf("Please give A :\n");
scanf("%d", &req.a);
printf("Please give B :\n");
scanf("%d", &req.b);

printf("You gave %d and %d\n",req.a, req.b);

/* Get PID */

req.pid_client = getpid();

/* Set initial mtype */

req.mtype = 1;

printf("My PID : %d\n",req.pid_client);

/* Send request to the server */

if (msgsnd(msqid, &req, sizeof(req) - sizeof(req.mtype), 0) == -1 ) {
	perror("msgsnd");
	exit(EXIT_FAILURE);
}

/* Wait for response from the server */

if (msgrcv(msqid, &resp, sizeof(resp) - sizeof(resp.mtype), req.pid_client, 0) == -1 ) {
	perror("msgrcv");
	exit(EXIT_FAILURE);
}

/* Print response */

printf("Result : %d\n",resp.sum);

exit(0);

};
