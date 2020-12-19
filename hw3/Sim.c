

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include "random.h"
#include "stopwatch.h"
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

void gatherInfo(int type, long time) {
    double avg;
    double std;
    double min;
    getRandomConstsByType(type, &avg, &std, &min);
    c.c_data.process_time = (int) initRandomByType(avg, std, min);
    c.c_id = numOfCustomers+10000;//todo:change later
    c.c_data.type = type;
    c.c_data.enter_time = swlap(&sw[0]);
    c.c_data.start_time = c.c_data.enter_time + (time / 1000);
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
    double handleTimeCustomer = 0;
    long temp_time;

    setArriveConsts(&avg, &std, &min);
    handleTimeArrive = initRandomByType(avg, std, min);
    quit.msg = 2;
    key_t sorter = ftok("Sorter.c", 's');
    key_t quitKey = ftok("Sim.c", 'q');
    msgid = msgget(sorter, 0666 | IPC_CREAT);
    msgquit = msgget(quitKey, 0666 | IPC_CREAT);

    while (flag) {

        generateCustomer(&type);
        gatherInfo(type, handleTimeCustomer);
        temp_time = swlap(&sw[0]);
        c.c_data.type = type;
        msgsnd(msgid, &c, sizeof(c), 0);
        numOfCustomers++;
        usleep(handleTimeArrive);

        if (msgrcv(msgquit, &quit, sizeof(quit), 1, IPC_NOWAIT) == -1) {
            if (errno == ENOMSG) {
                continue;
            } else {
                fprintf(stderr, "msgrcv failed SIM, %s\n", strerror(errno));
            }
        } else {
            printf("\nQuitting... %ld\n", swlap(&sw[0]));
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

        printf("%ld \n",customersList[i].c_id);

    }

}

void start() {
     if ((pid = fork()) == 0) {
        execvp(*sorterCMD,sorterCMD);
    }else if (pid > 0){
        swstart(&sw[0]);
        initCustomers();
        wait(&status);

    }
    checkQueues();

    free(customersList);
    printf("\n\n\n\n\n\nsize all %d\n\n\n\n\n", numOfCustomers);
    // usleep(100000);
    //finishTime = swlap(&sw[0]);
    //   printf("close the shop %ld", swlap(&sw[0]));
    // show_stats();
    // free_all_messages();
    printf("Ending-----\n");

}


int main(int argc, char *argv[]) {
    start();


    return 0;
}