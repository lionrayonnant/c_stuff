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

/* Main loop */

int main() {

/* Create structures */

struct request req;
struct response resp;

int msqid;

/* Create MSQ */

if((msqid = msgget(cle,0750|IPC_CREAT)) == -1) {
	perror("msgget");
	exit(EXIT_FAILURE);
}

/* Display the MSQ id */

printf("MSQ id : %d\n",msqid);

/* Wait for input from client and process */

while (1) {

if (msgrcv(msqid, &req, sizeof(req) - sizeof(req.mtype), 0, 0) == -1) {
	perror("msgrcv");
        exit(EXIT_FAILURE);
   }

printf("A : %d, B : %d, PID : %d\n",req.a,req.b,req.pid_client);

resp.sum = req.a + req.b;

/* Set the correct mtype so the response is sent to the right client */

resp.mtype = req.pid_client;

if (msgsnd(msqid, &resp, sizeof(resp) - sizeof(resp.mtype), 0) == -1 ) {
	perror("msgsnd");
	exit(EXIT_FAILURE);
}

/* Show infos on response and clients */

printf("Response '%d', sent to %d\n",resp.sum, req.pid_client);

};

exit(0);
};
