#include "Clerk.h"

key_t getKeyByType(int type){
    switch(type){
        case NEW:
            return ftok("Clerk.c", 'n');

        case UPGRADE:
            return ftok("Clerk.c", 'u');
        case REPAIR:
            return ftok("Clerk.c", 'r');
        default:
            return -1;

    }
}
void start(char *type) {
    int running = 1;
    key_t key;
    key_t finishKey = ftok("Sim.c", 'f');
    int msgRecieveID;
    int msgForwardID;
    int customer_t = atoi(type);
    key = getKeyByType(customer_t);
    double countHandle = 0;///////////////
    msgForwardID = msgget(finishKey, 0666 | IPC_CREAT);
    msgRecieveID = msgget(key, 0666 | IPC_CREAT);
    while (running) {
        if (msgrcv(msgRecieveID, &c, sizeof(c), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed CLERK, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        usleep(c.c_data.process_time);//todo:QUIT will cause trouble
        msgsnd(msgForwardID, &c, sizeof(c), 0);
        if (c.c_data.type == QUIT) {
            printf("Customer quit\n");
            running = 0;
            msgctl(msgRecieveID, IPC_RMID, NULL);
        }
    }

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("not enough arguments\n");
        return 0;
    }
    start(argv[1]);
    return 0;
}


