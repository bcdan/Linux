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

void promptStart (int type){
    char *blank = NULL;
    if(type == NEW){
        blank = "new";
    }else if (type==UPGRADE){
        blank = "upgrade";
    }
    else if (type == REPAIR){
        blank = "repair";
    }
    printf("Clerk for %s is starting\n",blank);
}

void promptEnd (int type){
    char *blank = NULL;
    if(type == NEW){
        blank = "new";
    }else if (type==UPGRADE){
        blank = "upgrade";
    }
    else if (type == REPAIR){
        blank = "repair";
    }
    printf("Clerk for %s is quitting\n",blank);
}
void start(char *type) {
    stopwatch *sw;
    int running = 1;
    key_t key;
    key_t finishKey = ftok("Sim.c", 'f');
    key_t stopwatchKey = ftok("Sim.c",'s');
    int addr;
    int msgRecieveID;
    int msgForwardID;
    int customer_t = atoi(type);
    key = getKeyByType(customer_t);
    msgForwardID = msgget(finishKey, 0666 | IPC_CREAT);
    msgRecieveID = msgget(key, 0666 | IPC_CREAT);
    int shmid;
    while (running) {

        if (msgrcv(msgRecieveID, &c, sizeof(c), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed CLERK, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        shmid= shmget(stopwatchKey,1024,0666|IPC_CREAT);
        if(shmid!=-1) {
            sw = (struct stopwatch *) shmat(shmid, NULL, 0);
        }

        usleep(c.c_data.process_time);//todo:QUIT will cause trouble
        c.c_data.exit_time=swlap(sw);
        c.c_data.elapse_time=c.c_data.exit_time-c.c_data.enter_time;
        addr = shmdt(sw);
        if(addr==-1){
            printf("couldnt shdt\n");
        }

        if (c.c_data.type == QUIT) {
            printf("Customer quit\n");
            running = 0;
            c.c_id=404; // special indicator
        }
        msgsnd(msgForwardID, &c, sizeof(c), 0);
    }
    msgctl(msgRecieveID, IPC_RMID, NULL);


}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("not enough arguments\n");
        return 0;
    }
    promptStart(atoi(argv[1]));
    start(argv[1]);
    promptEnd(atoi(argv[1]));
    return 0;
}


