

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include "random.h"
#include "stopwatch.h"
#include "Sim.h"
#include <unistd.h>
#include <sys/wait.h>
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
    c.c_id = 1;//todo:change later
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

    struct qMessage {
        long msgType;
        int msg;
    } quit;
    setArriveConsts(&avg, &std, &min);
    handleTimeArrive = initRandomByType(avg, std, min);
    quit.msg = 2;
    key_t sorter = ftok("Sorter.c", 's');
    key_t quitKey = ftok("Sim.c", 'q');
    sleep(1);
    msgid = msgget(sorter, 0666 | IPC_CREAT);

    while (flag) {

        generateCustomer(&type);
        gatherInfo(type, handleTimeCustomer);
        temp_time = swlap(&sw[0]);
        c.c_data.type = type;
        msgsnd(msgid, &c, sizeof(c), 0);
        numOfCustomers++;
        usleep(handleTimeArrive);
        msgquit = msgget(quitKey, 0666 | IPC_CREAT);

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


}


void initProcs() {
    for (int i = 0; i < NUMOFPROC; i++) {
        if ((pid[i] = fork()) == 0) {
            execvp(*execFiles[i], execFiles[i]);
            perror("Execvp error");
            _exit(1);
        }
        if (pid[i] < 0) {
            perror("Fork error");
        }
    }

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
    msgid = msgget(key, 0666 | IPC_CREAT);
    initCustomersList();
    for (int i = 0; i < numOfCustomers; i++) {
        msgrcv(msgid, customersList + i, sizeof(customer), 0, 0);
    }
    msgctl(msgid, IPC_RMID, NULL);

    for (int i = 0; i <numOfCustomers ; ++i) {
        printf("Type : %d \n",customersList[i].c_data.type);

    }

//    for (int i = 0; i < numOfCustomers; i++) {
//        printf("id [%ld]: %s   type [%d] proc: [%d] enter:%ld  start:%ld  exit : %ld  total :%ld\n",
//               customersList[i].c.c_id,
//               customersList[i].msgText,
//               customersList[i].c.c_data.type,
//               customersList[i].c.c_data.process_time,
//               customersList[i].c.c_data.enter_time,
//               customersList[i].c.c_data.start_time,
//               customersList[i].c.c_data.exit_time,
//               customersList[i].c.c_data.elapse_time
//
//        );
//
//    }
}

void start() {
    int msgid = 0;
    int msgquit = 0;
    initProcs();
    swstart(&sw[0]);
    initCustomers(&msgid, &msgquit);
    checkQueues();
    sleep(5);
//    for (int i = 0; i < NUMOFPROC; i++) {
//        if (pid[i] > 0) {
//            int statusd;
//
//            waitpid(pid[i], &statusd, 0);
//            if (statusd > 0) {
//                // handle a process sent exit status error
//            }
//        } else {
//            // handle a proccess was not started
//        }
//    }
    //sleep(1);
    printf("here\n");
    free(customersList);
    printf("\n\n\n\n\n\nsize all %d\n\n\n\n\n", numOfCustomers);
    // wait(&status);
    // usleep(100000);
    // get_all_client();
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