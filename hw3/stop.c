#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include "random.h"
#include "stopwatch.h"
#include <unistd.h>
#include <errno.h>


struct qMessage{
    long msgType;
    int msg;
}quit;


int main() {
    printf("Quit.c started!\n");
    key_t key = ftok("Sim.c", 'q');
    int msgid;
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    quit.msgType=1;
    quit.msg=1;
    if (msgsnd(msgid, &quit, sizeof(quit), 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
    sleep(10);

    return 0;

}