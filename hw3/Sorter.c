
#include "Sorter.h"


double initRandomByType(double avg, double std, double min) {
    initrand();
    double rand = pnrand(avg, std, min);
    return rand;
}


void setConsts(double *avg, double *std, double *min) {
    *avg = AVRG_SORT;
    *std = SPRD_SORT;
    *min = MIN_SORT;
}

int getType(int customerType) {
    for (int i = 0; i < 4; i++) {
        if (customerType == i)
            return i;
    }
    return -1;
}



void startProcs(){
        for (int i = 0; i < NUMOFPROC; i++) {
        if ((pids[i] = fork()) == 0) {
            execvp(*execFiles[i], execFiles[i]);
            perror("Execvp error");
            _exit(1);
        }
        if (pids[i] < 0) {
            perror("Fork error");
        }
    }
}


void start() {
    stopwatch* sw;
    int running = 1;
    key_t customerKey;
    int msgRecieveId = 0;
    int msgForwardID = 0;
    double avg = 0;
    double std = 0;
    double min = 0;
    int addr;
    double handleTime = 0; ///////////////////
    setConsts(&avg, &std, &min);
    key_t sorter = ftok("Sorter.c", 's');

    msgRecieveId = msgget(sorter, 0666 | IPC_CREAT);
    if (msgRecieveId == -1) {
        fprintf(stderr, "msgget failed , [%d]\n", errno);
        exit(EXIT_FAILURE);
    }
    key_t upgrade = ftok("Clerk.c", 'u');
    key_t repair = ftok("Clerk.c", 'r');
    key_t new = ftok("Clerk.c", 'n');
    key_t stopwatchKey = ftok("Sim.c",'s');

    key_t keys[] = {new, upgrade, repair};
    int msgIDUpgrade= msgget(upgrade, 0666|IPC_CREAT );
    int msgIDRepair=msgget(repair, 0666|IPC_CREAT );
    int msgIDNew=msgget(new, 0666|IPC_CREAT );
    printf("\n");
    c.c_id = 1;

    while (running) {

        if (msgrcv(msgRecieveId, &c, sizeof(c), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        handleTime = initRandomByType(avg, std, min);
        usleep(handleTime); //todo: test this later
        int shmid = shmget(stopwatchKey,1024,0666|IPC_CREAT);
        if(shmid!=-1) {
            sw = (struct stopwatch *) shmat(shmid, NULL, 0);
        }
        switch (c.c_data.type) {
            case NEW:
               c.c_data.start_time=swlap(sw);
                addr = shmdt(sw);
                if(addr==-1){
                    printf("couldnt shdt\n");
                }
                msgsnd(msgIDNew, &c, sizeof(c), 0);
                break;
            case UPGRADE:
               c.c_data.start_time=swlap(sw);
                addr = shmdt(sw);
                if(addr==-1){
                    printf("couldnt shdt\n");
                }
                msgsnd(msgIDUpgrade, &c, sizeof(c), 0);
                break;
            case REPAIR:
               c.c_data.start_time=swlap(sw);
                addr = shmdt(sw);
                if(addr==-1){
                    printf("couldnt shdt\n");
                }
                msgsnd(msgIDRepair, &c, sizeof(c), 0);
                break;
            case QUIT: {
                //todo:might need to calc time
                for (int i = 0; i < 3; ++i) {
                    c.c_id = 1;
                    customerKey = keys[i];
                    msgForwardID = msgget(customerKey, 0666 );
                    msgsnd(msgForwardID, &c, sizeof(c), 0);
                }
                running = 0;
                break;
            }
        }


    }

    msgctl(msgRecieveId, IPC_RMID, NULL);

}



int main(int argc, char *argv[]) {
    int status;
    printf("Sorter running\n");
    startProcs();
    start();
        for (int i = 0; i < NUMOFPROC; i++) {
        if (pids[i] > 0) {

            waitpid(pids[i], &status, 0);
            if (status > 0) {
                // handle a process sent exit status error //todo:later
            }
        } else {
            // handle a proccess was not started
        }
    }
    printf("Sorter quitting\n");
    return 0;
}