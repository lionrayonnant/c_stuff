#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <time.h>

key_t cle = 100;

/* Define structures */

struct add {
        long mtype;
        long a;
        long b;
        pid_t pid_client;
};

struct response_add {
    long mtype;
    long result;
};

/* Main loop */

int main() {

/* Create structures */

struct add add1;
struct response_add add1_resp;

int msqid;

time_t currentTime;
time(&currentTime);

/* Create MSQ */

if((msqid = msgget(cle,0750|IPC_CREAT)) == -1) {
	perror("msgget");
	exit(EXIT_FAILURE);
}

/* Display the MSQ id */

printf("MSQ id : %d\n",msqid);

/* Wait for input from client and process */

while (1) {

if (msgrcv(msqid, &add1, sizeof(add1) - sizeof(add1.mtype), 0, 0) == -1) {
	perror("msgrcv");
        exit(EXIT_FAILURE);
   }

printf("[%s] : A = %ld, B = %ld, PID = %d\n",ctime(&currentTime),add1.a,add1.b,add1.pid_client);

add1_resp.result = add1.a + add1.b;

/* Set the correct mtype so the response is sent to the right client */

add1_resp.mtype = add1.pid_client;

if (msgsnd(msqid, &add1_resp, sizeof(add1_resp) - sizeof(add1_resp.mtype), 0) == -1 ) {
	perror("msgsnd");
	exit(EXIT_FAILURE);
}

/* Show infos on response and clients */

printf("Response '%d', sent to %d\n",add1_resp.result, add1.pid_client);

};

exit(0);
};
