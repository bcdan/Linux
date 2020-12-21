#include "Clerk.h"

key_t getKeyByType(int type) {
    switch (type) {
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

char *toStringByType(int type) {
    switch (type) {
        case NEW:
            return "new";
        case UPGRADE:
            return "upgrade";
        case REPAIR:
            return "repair";
        default:
            return "null";
    }
}

void promptStart(int type) {
    char *blank = NULL;
    if (type == NEW) {
        blank = "new";
    } else if (type == UPGRADE) {
        blank = "upgrade";
    } else if (type == REPAIR) {
        blank = "repair";
    }
    printf("Clerk for %s is starting\n", blank);
}

void promptEnd(int type) {
    char *blank = NULL;
    if (type == NEW) {
        blank = "new";
    } else if (type == UPGRADE) {
        blank = "upgrade";
    } else if (type == REPAIR) {
        blank = "repair";
    }
    printf("Clerk for %s is quitting\n", blank);
}

void start(char *type) {
    stopwatch *sw;
    int running = 1;
    key_t key;
    key_t finishKey = ftok("Sorter.c", 'f');
    key_t stopwatchKey = ftok("Sim.c", 's');
    int addr;
    int msgRecieveID;
    int msgForwardID;
    int customer_t = atoi(type);
    key = getKeyByType(customer_t);
    msgForwardID = msgget(finishKey, 0666 | IPC_CREAT);
    msgRecieveID = msgget(key, 0666 | IPC_CREAT);
    int shmid;
    shmid = shmget(stopwatchKey, 1024, 0666 | IPC_CREAT);
    if (shmid != -1) {
        sw = (struct stopwatch *) shmat(shmid, NULL, 0);
    }
    long totalWork = 0;
    long totalWait = 0;
    long clerkStartTime = swlap(sw);
    long clerkEndTime = 0;
    int numOfCustomers = 0;
    while (running) {

        if (msgrcv(msgRecieveID, &c, sizeof(c), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed CLERK, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        numOfCustomers++;
        c.c_data.start_time = swlap(sw);
        usleep(c.c_data.process_time*10);//todo:QUIT will cause trouble
        c.c_data.exit_time = swlap(sw);
        c.c_data.elapse_time = c.c_data.exit_time - c.c_data.start_time;

        if (c.c_data.type == QUIT) {
            numOfCustomers--;
            clerkEndTime = swlap(sw);
            running = 0;
        } else {
            printf("%ld:  %s arrived: %ld started: %ld processed : %ld exited: %ld elapse: %ld\n",
                   c.c_id,
                   toStringByType(c.c_data.type),
                   c.c_data.enter_time,
                   c.c_data.start_time,
                   c.c_data.exit_time - c.c_data.start_time,
                   c.c_data.exit_time,
                   c.c_data.elapse_time);
            totalWait += c.c_data.start_time - c.c_data.enter_time;
            totalWork += c.c_data.exit_time - c.c_data.start_time;
        }

    }
    customer Clerk;
    Clerk.c_id=1;
    Clerk.c_data.type = customer_t;
    Clerk.c_data.elapse_time = clerkEndTime - clerkStartTime;
    Clerk.c_data.exit_time=totalWork;//total work time for clerk will be sent through exit time
    Clerk.c_data.start_time=totalWait; //total wait time for clerk will be sent through starttime
    Clerk.c_data.process_time=numOfCustomers;
    if(msgsnd(msgForwardID, &Clerk, sizeof(Clerk), 0)==-1){
        fprintf(stderr,"Couldn't send msg\n");
    }

    //todo:divide everything by 1000
    addr = shmdt(sw);
    if (addr == -1) {
        printf("couldnt shdt\n");
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
    //promptEnd(atoi(argv[1]));
    return 0;
}


