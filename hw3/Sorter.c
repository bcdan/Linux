
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


void startProcs() {
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


void getResults() {
    key_t finishKey = ftok("Sorter.c", 'f');
    int msgRecieveID;
    msgRecieveID = msgget(finishKey, 0666 | IPC_CREAT);
    customer clerk;
    clerkList = (struct clerk *) malloc(sizeof(clerk) * NUMOFPROC);
    //todo:get number of customers

    for (int i = 0; i < NUMOFPROC; ++i) {
        if (msgrcv(msgRecieveID, &clerk, sizeof(clerk), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed CLERK, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        clerkList[i].numOfCustomers = clerk.c_data.process_time;
        clerkList[i].elapse = clerk.c_data.elapse_time;
        clerkList[i].work = clerk.c_data.exit_time;
        clerkList[i].wait = clerk.c_data.start_time;

        if (clerk.c_data.process_time != 0) {

            clerkList[i].avgWork = clerk.c_data.exit_time / clerk.c_data.process_time;
            clerkList[i].avgWait = clerk.c_data.start_time / clerk.c_data.process_time;

        } else {
            clerkList[i].avgWork = 0;
            clerkList[i].avgWait = 0;
        }
    }
    msgctl(msgRecieveID, IPC_RMID, NULL);


}


void start() {
    stopwatch *sw;
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
    key_t stopwatchKey = ftok("Sim.c", 's');

    key_t keys[] = {new, upgrade, repair};
    int msgIDUpgrade = msgget(upgrade, 0666 | IPC_CREAT);
    int msgIDRepair = msgget(repair, 0666 | IPC_CREAT);
    int msgIDNew = msgget(new, 0666 | IPC_CREAT);
    printf("\n");
    c.c_id = 1;
    int shmid = shmget(stopwatchKey, 1024, 0666 | IPC_CREAT);
    if (shmid != -1) {
        sw = (struct stopwatch *) shmat(shmid, NULL, 0);
    }
    while (running) {

        if (msgrcv(msgRecieveId, &c, sizeof(c), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        handleTime = initRandomByType(avg, std, min);
        usleep(handleTime * 10); //todo: test this later

        switch (c.c_data.type) {
            case NEW:
                c.c_data.start_time = swlap(sw);
                msgsnd(msgIDNew, &c, sizeof(c), 0);
                break;
            case UPGRADE:
                c.c_data.start_time = swlap(sw);
                msgsnd(msgIDUpgrade, &c, sizeof(c), 0);
                break;
            case REPAIR:
                c.c_data.start_time = swlap(sw);
                msgsnd(msgIDRepair, &c, sizeof(c), 0);
                break;
            case QUIT: {
                //todo:might need to calc time
                for (int i = 0; i < 3; ++i) {
                    c.c_id = 1;
                    customerKey = keys[i];
                    msgForwardID = msgget(customerKey, 0666);
                    msgsnd(msgForwardID, &c, sizeof(c), 0);
                }
                running = 0;
                break;
            }
        }


    }
    addr = shmdt(sw);
    if (addr == -1) {
        printf("couldnt shmdt\n");
    }
    msgctl(msgRecieveId, IPC_RMID, NULL);

}

void printResults(int type) {
        printf("Clerk for new customers: processed %d customers\n", clerkList[type].numOfCustomers);
        printf("elapse:%ld work:%ld wait:%ld\n", clerkList[type].elapse, clerkList[type].work, clerkList[type].wait);
        printf("per customer: work: %ld wait:%ld\n", clerkList[type].avgWork, clerkList[type].avgWait);
}

int main(int argc, char *argv[]) {

    printf("Sorter running\n");
    startProcs();
    start();
    int status;
    if (waitpid(pids[0], &status, 0) != -1) {
        if (WIFEXITED(status)) {
            int returned = WEXITSTATUS(status);
        }
//        else if ( WIFSIGNALED(status) ) {
//            int signum = WTERMSIG(status);
//            printf("Exited due to receiving signal %d\n", signum);
//        }
//        else if ( WIFSTOPPED(status) ) {
//            int signum = WSTOPSIG(status);
//            printf("Stopped due to receiving signal %d\n", signum);
//        }
//        else {
//            printf("Something strange just happened.\n");
//        }
    }
    if (waitpid(pids[1], &status, 0) != -1) {
        if (WIFEXITED(status)) {
            int returned = WEXITSTATUS(status);
        }
//        else if ( WIFSIGNALED(status) ) {
//            int signum = WTERMSIG(status);
//            printf("Exited due to receiving signal %d\n", signum);
//        }
//        else if ( WIFSTOPPED(status) ) {
//            int signum = WSTOPSIG(status);
//            printf("Stopped due to receiving signal %d\n", signum);
//        }
//        else {
//            printf("Something strange just happened.\n");
//        }
    }
    if (waitpid(pids[2], &status, 0) != -1) {
        if (WIFEXITED(status)) {

            int returned = WEXITSTATUS(status);
        }
//        else if ( WIFSIGNALED(status) ) {
//            int signum = WTERMSIG(status);
//            printf("Exited due to receiving signal %d\n", signum);
//        }
//        else if ( WIFSTOPPED(status) ) {
//            int signum = WSTOPSIG(status);
//            printf("Stopped due to receiving signal %d\n", signum);
//        }
//        else {
//            printf("Something strange just happened.\n");
//        }
    }

    printf("Sorter quitting\n");
    getResults();
    printf("Clerk for new is quitting\n");
    printResults(NEW);

    printf("Clerk for upgrade is quitting\n");
    printResults(UPGRADE);

    printf("Clerk for repair is quitting\n");
    printResults(REPAIR);

    return 0;
}