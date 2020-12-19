

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>

#include "Sim.h"
#include <unistd.h>
#include <errno.h>


char* toStringByType(int type){
    switch(type){
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


void startTimer(key_t stopwatchKey){
    shmid=shmget(stopwatchKey,1024,0666|IPC_CREAT);
    if(shmid!=-1){
        sw = (struct stopwatch*)shmat(shmid,NULL,0);
        swstart(sw);

    }
    int addr = shmdt(sw);
    if(addr==-1){
        printf("couldnt shdt\n");
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
    handleTimeArrive = initRandomByType(avg, std, min);
    quit.msg = 2;
    key_t sorter = ftok("Sorter.c", 's');
    key_t quitKey = ftok("Sim.c", 'q');
    key_t stopwatchKey = ftok("Sim.c",'s');
    msgid = msgget(sorter, 0666 | IPC_CREAT);
    msgquit = msgget(quitKey, 0666 | IPC_CREAT);
    startTimer(stopwatchKey);
    while (flag) {

        generateCustomer(&type);
        usleep(handleTimeArrive);
        shmid=shmget(stopwatchKey,1024,0666);
        if(shmid!=-1){
            sw = (struct stopwatch*)shmat(shmid,NULL,0);
        }
        gatherInfo(type);
        addr = shmdt(sw);
        if(addr==-1){
            printf("couldnt shdt\n");
        }
        msgsnd(msgid, &c, sizeof(c), 0);
        numOfCustomers++;

        if (msgrcv(msgquit, &quit, sizeof(quit), 1, IPC_NOWAIT) == -1) {
            if (errno == ENOMSG) {
                continue;
            } else {
                fprintf(stderr, "msgrcv failed SIM, %s\n", strerror(errno));
            }
        } else {
            shmid = shmget(stopwatchKey,1024,0666|IPC_CREAT);
            if(shmid!=-1){
                sw = (struct stopwatch*)shmat(shmid,NULL,0);
                printf("\nQuitting... %ld\n", swlap(&sw[0]));

            }
            c.c_data.type = QUIT;
            c.c_id = 1;
            msgsnd(msgid, &c, sizeof(c), 0);
            flag = FALSE;
        }

    }
    msgctl(msgquit, IPC_RMID, NULL);


}



void initCustomersList() {
    customersList = (customer *) malloc(sizeof(customer) * numOfCustomers);
    for (int i = 0; i < numOfCustomers; i++) {
        customersList[i].c_id = -1;
        customersList[i].c_data.type = -1;
        customersList[i].c_data.process_time = -1;
        customersList[i].c_data.enter_time = -1;
        customersList[i].c_data.start_time = -1;
        customersList[i].c_data.exit_time = -1;
        customersList[i].c_data.elapse_time = -1;

    }
}


void checkQueues() {
    key_t key;
    key= ftok("Sim.c", 'f');
    int msgid;
    int tempCount = 0;
    msgid = msgget(key, 0666 | IPC_CREAT);
    initCustomersList();
    customer temp;
    for (int i = 0; i < numOfCustomers; i++) {
        msgrcv(msgid, &temp, sizeof(temp), 0, 0);
        if(temp.c_id!= 404 && temp.c_id!=-1){
            customersList[tempCount]=temp;
            tempCount++;
        }
        temp.c_id=-1;

    }
    numOfCustomers=tempCount;
    msgctl(msgid, IPC_RMID, NULL);
    for (int i = 0; i <numOfCustomers ; ++i) {

        printf("%ld: %s arrived: %ld started: %ld processed: %ld exited: %ld elapse %ld\n",
               customersList[i].c_id,
               toStringByType(customersList[i].c_data.type),
               customersList[i].c_data.enter_time,
               customersList[i].c_data.start_time,
               customersList[i].c_data.exit_time - customersList[i].c_data.start_time,
               customersList[i].c_data.exit_time,
               customersList[i].c_data.elapse_time
        );

    }
    shmctl(shmid,IPC_RMID,NULL);


}

void start() {
     if ((pid = fork()) == 0) {
        execvp(*sorterCMD,sorterCMD);
    }else if (pid > 0){
        initCustomers();
        wait(&status);

    }
    checkQueues();

    free(customersList);
    printf("\n\n\n\n\n\nsize all %d\n\n\n\n\n", numOfCustomers);
    printf("Ending-----\n");

}


int main(int argc, char *argv[]) {
    start();


    return 0;
}