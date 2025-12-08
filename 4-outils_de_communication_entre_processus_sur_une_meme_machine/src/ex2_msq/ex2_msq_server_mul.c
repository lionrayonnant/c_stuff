#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <time.h>

key_t cle = 200;

/* Define structures */

struct mul {
        long mtype;
        float a;
        float b;
	float c;
        pid_t pid_client;
};

struct response_mul {
    long mtype;
    float result;
};

/* Main loop */

int main() {

/* Create structures */

struct mul mul1;
struct response_mul mul1_resp;

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

if (msgrcv(msqid, &mul1, sizeof(mul1) - sizeof(mul1.mtype), 0, 0) == -1) {
	perror("msgrcv");
        exit(EXIT_FAILURE);
   }

printf("[%s] : A = %f, B = %f, C = %f, PID = %d\n",ctime(&currentTime),mul1.a,mul1.b,mul1.pid_client);

mul1_resp.result = mul1.a * mul1.b * mul1.c;

/* Set the correct mtype so the response is sent to the right client */

mul1_resp.mtype = mul1.pid_client;

if (msgsnd(msqid, &mul1_resp, sizeof(mul1_resp) - sizeof(mul1_resp.mtype), 0) == -1 ) {
	perror("msgsnd");
	exit(EXIT_FAILURE);
}

/* Show infos on response and clients */

printf("Response '%f', sent to %d\n",mul1_resp.result, mul1.pid_client);

};

exit(0);
};
