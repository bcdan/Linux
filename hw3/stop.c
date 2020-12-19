#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include "random.h"
#include "stopwatch.h"
#include <unistd.h>
#include <errno.h>

#define TYPE_NEW      0		// new customer
#define TYPE_UPGRADE  1		// customer wishes to upgrade hardware or program
#define TYPE_REPAIR   2		// customer wisher to repair hardware
#define TYPE_QUIT     3		// special customer type, used to terminate the simulation

struct qMessage{
    long msgType;
    int msg;
}quit;

enum queueKeys {
    NEW = TYPE_NEW, UPGRADE = TYPE_UPGRADE, REPAIR = TYPE_REPAIR, QUIT = TYPE_QUIT, SORTER
};

int main() {
    printf("Quit.c started!\n");
    key_t key = ftok("Sim.c", 'q');
    int msgid;
    //msgid = msgget(key, 0666 | IPC_CREAT);
    msgid = msgget(key, 0666 );

    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    quit.msgType=1;
    quit.msg=1;
   // sprintf(message.msgText, "%s", "QUIT\n");
    if (msgsnd(msgid, &quit, sizeof(quit), 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
    sleep(5);
    msgctl(msgid, IPC_RMID, NULL);

    return 0;

}