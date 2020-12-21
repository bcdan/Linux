

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "Sim.h"
#include <unistd.h>
#include <errno.h>



double initRandomByType(double avg, double std, double min) {
    initrand();
    double rand = pnrand(avg, std, min);
    return rand;
}


void generateCustomer(int *typeOfCustomer) {
    initrand();
    double num = 0;
    num = urand(0, 100);
    if (num <= POP_NEW) {
        *typeOfCustomer = TYPE_NEW;
    } else if (num <= POP_UPGRADE) {
        *typeOfCustomer = TYPE_UPGRADE;
    } else {
        *typeOfCustomer = TYPE_REPAIR;
    }

}

void setNew(double *avg, double *std, double *min) {
    *avg = AVRG_NEW;
    *std = SPRD_NEW;
    *min = MIN_NEW;
}

void setUpgrade(double *avg, double *std, double *min) {
    *avg = AVRG_UPGRADE;
    *std = SPRD_UPGRADE;
    *min = MIN_UPGRADE;
}

void setRepair(double *avg, double *std, double *min) {
    *avg = AVRG_REPAIR;
    *std = SPRD_REPAIR;
    *min = MIN_REPAIR;
}

void setArriveConsts(double *avg, double *std, double *min) {
    *avg = AVRG_ARRIVE;
    *std = SPRD_ARRIVE;
    *min = MIN_ARRIVE;
}

void getRandomConstsByType(int customer_t, double *avg, double *std, double *min) {
    if (customer_t == TYPE_NEW)
        setNew(avg, std, min);
    else if (customer_t == TYPE_UPGRADE)
        setUpgrade(avg, std, min);
    else if (customer_t == TYPE_REPAIR)
        setRepair(avg, std, min);
}

void gatherInfo(int type) {
    double avg;
    double std;
    double min;
    getRandomConstsByType(type, &avg, &std, &min);
    c.c_data.process_time = (int) initRandomByType(avg, std, min);
    c.c_id = numOfCustomers+10000;//todo:change later
    c.c_data.type = type;
    c.c_data.enter_time = swlap(sw);
}


void startTimer(){
    shmid=shmget(stopwatchKey,1024,0666|IPC_CREAT);
    if(shmid!=-1){
        sw = (struct stopwatch*)shmat(shmid,NULL,0);
        swstart(sw);

    }
}
void initCustomers() {
    int msgid;
    int msgquit;
    int flag = TRUE;
    int type = -1;
    double avg;
    double std;
    double min;
    double handleTimeArrive = 0;
    int addr;
    setArriveConsts(&avg, &std, &min);
    quit.msg = 2;
    key_t sorter = ftok("Sorter.c", 's');
    key_t quitKey = ftok("Sim.c", 'q');
    msgid = msgget(sorter, 0666 | IPC_CREAT);
    msgquit = msgget(quitKey, 0666 | IPC_CREAT);
    while (flag) {
        handleTimeArrive = initRandomByType(avg, std, min);
        generateCustomer(&type);
        gatherInfo(type);
        msgsnd(msgid, &c, sizeof(c), 0);
        numOfCustomers++;
        usleep(handleTimeArrive*10);
        if (msgrcv(msgquit, &quit, sizeof(quit), 1, IPC_NOWAIT) == -1) {
            if (errno == ENOMSG) {
                continue;
            } else {
                fprintf(stderr, "msgrcv failed SIM, %s\n", strerror(errno));
            }
        } else {
           // printf("\nQuitting... %ld\n", swlap(&sw[0]));
            c.c_data.type = QUIT;
            c.c_id = 1;
            msgsnd(msgid, &c, sizeof(c), 0);
            flag = FALSE;
        }


    }
    addr = shmdt(sw);
    if(addr==-1){
        printf("couldnt shmdt\n");
    }
    msgctl(msgquit, IPC_RMID, NULL);


}


void start() {
    int status;
    stopwatchKey = ftok("Sim.c",'s');
    startTimer();
    if ((pid = fork()) == 0) {
        execvp(*sorterCMD,sorterCMD);
    }else if (pid > 0){
        initCustomers();
        if ( waitpid(pid, &status, 0) != -1 ) {
            if ( WIFEXITED(status) ) {
                int returned = WEXITSTATUS(status);
                printf("Exited normally with status %d\n", returned);
            }
            else if ( WIFSIGNALED(status) ) {
                int signum = WTERMSIG(status);
                printf("Exited due to receiving signal %d\n", signum);
            }
            else if ( WIFSTOPPED(status) ) {
                int signum = WSTOPSIG(status);
                printf("Stopped due to receiving signal %d\n", signum);
            }
            else {
                printf("Something strange just happened.\n");
            }
        }








    }
    shmctl(shmid,IPC_RMID,NULL);
    printf("Ending-----\n");

}


int main(int argc, char *argv[]) {
    start();


    return 0;
}