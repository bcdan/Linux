#include "simulation.h"

struct stopwatch *sw;

double initRandomByType(double avg, double std, double min) {
    initrand();
    double rand = pnrand(avg, std, min);
    return rand;
}


void generateCustomer(int *typeOfCustomer) {
    initrand();
    double num;
    num = urand(0, 100);
    if (num <= POP_NEW) {
        *typeOfCustomer = TYPE_NEW;
    } else if (num <= POP_UPGRADE) {
        *typeOfCustomer = TYPE_UPGRADE;
    } else {
        *typeOfCustomer = TYPE_REPAIR;
    }

}

/* set values by given type */

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

void gatherInfo(customer *c, int type, int num) {
    double avg;
    double std;
    double min;
    getRandomConstsByType(type, &avg, &std, &min);
    c->c_data.process_time = (int) initRandomByType(avg, std, min);
    c->c_id = num + 10000;
    c->c_data.type = type;
    c->c_data.enter_time = swlap(sw);
}


key_t getKeyByType(int type) {
    switch (type) {
        case NEW:
            return new;
        case UPGRADE:
            return upgrade;
        case REPAIR:
            return repair;
        default:
            return -1;//shouldn't happen

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
            return "null";//shouldn't happen
    }
}

void setConsts(double *avg, double *std, double *min) {
    *avg = AVRG_SORT;
    *std = SPRD_SORT;
    *min = MIN_SORT;
}


void startSorter() { //sorter process

    int running = 1;
    key_t customerKey;
    int msgRecieveId;
    int msgForwardID;

    double avg = 0;
    double std = 0;
    double min = 0;
    double handleTime;

    setConsts(&avg, &std, &min);
    customer sorterTemp;
    msgRecieveId = msgget(sorter, 0666 | IPC_CREAT);
    if (msgRecieveId == -1) {
        fprintf(stderr, "msgget failed , [%d]\n", errno);
        exit(EXIT_FAILURE);
    }

    key_t keys[] = {new, upgrade, repair};
    int msgIDUpgrade = msgget(upgrade, 0666 | IPC_CREAT);
    int msgIDRepair = msgget(repair, 0666 | IPC_CREAT);
    int msgIDNew = msgget(new, 0666 | IPC_CREAT);
    int finishID = msgget(finishKey, 0666 | IPC_CREAT);
    sorterTemp.c_id = 1;
    while (running) {

        if (msgrcv(msgRecieveId, &sorterTemp, sizeof(sorterTemp), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        handleTime = initRandomByType(avg, std, min);
        usleep(handleTime * 10); //todo: test this later

        switch (sorterTemp.c_data.type) {
            case NEW:
                numOfCustomers++;
                sorterTemp.c_data.start_time = swlap(sw);
                msgsnd(msgIDNew, &sorterTemp, sizeof(sorterTemp), 0);
                break;
            case UPGRADE:
                numOfCustomers++;
                sorterTemp.c_data.start_time = swlap(sw);
                msgsnd(msgIDUpgrade, &sorterTemp, sizeof(sorterTemp), 0);
                break;
            case REPAIR:
                numOfCustomers++;
                sorterTemp.c_data.start_time = swlap(sw);
                msgsnd(msgIDRepair, &sorterTemp, sizeof(sorterTemp), 0);
                break;
            case QUIT: {
                for (int i = 0; i < 3; ++i) {
                    sorterTemp.c_id = 1;
                    customerKey = keys[i];
                    msgForwardID = msgget(customerKey, 0666);
                    msgsnd(msgForwardID, &sorterTemp, sizeof(sorterTemp), 0);
                }
                running = 0;
                break;
            }
        }

    }
    customer cTemp;
    for (int i = 0; i < numOfCustomers; i++) { //gets messages from clerks
        if (msgrcv(finishID, &cTemp, sizeof(cTemp), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("%ld:  %s arrived: %ld started: %ld processed : %ld exited: %ld elapse: %ld\n",
               cTemp.c_id,
               toStringByType(cTemp.c_data.type),
               cTemp.c_data.enter_time,
               cTemp.c_data.start_time,
               cTemp.c_data.exit_time - cTemp.c_data.start_time,
               cTemp.c_data.exit_time,
               cTemp.c_data.elapse_time);
    }
    msgctl(msgRecieveId, IPC_RMID, NULL);
    msgctl(finishID, IPC_RMID, NULL);


}


void startClerk(char *type) { //clerk process
    customer clerkTemp;
    key_t key;
    int running = 1;
    int msgRecieveID;
    int msgForwardID;
    int arrivalID;
    int customer_t = atoi(type);
    int numOfCustomersClerk = 0;

    key = getKeyByType(customer_t);
    msgForwardID = msgget(finishKey, 0666 | IPC_CREAT);
    msgRecieveID = msgget(key, 0666 | IPC_CREAT);
    arrivalID = msgget(arrival, 0666 | IPC_CREAT);


    long totalWork = 0;
    long totalWait = 0;
    long clerkStartTime = swlap(sw);
    long clerkEndTime = 0;
    long tempStart;
    long tempExit;
    while (running) {

        if (msgrcv(msgRecieveID, &clerkTemp, sizeof(clerkTemp), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed CLERK, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        tempStart = swlap(sw);
        usleep(clerkTemp.c_data.process_time * 10);
        tempExit = swlap(sw);

        if (clerkTemp.c_data.type == QUIT) {
            clerkEndTime = swlap(sw);
            running = 0;
        } else {
            numOfCustomersClerk++;
            clerkTemp.c_data.start_time = tempStart;
            clerkTemp.c_data.exit_time = tempExit;
            clerkTemp.c_data.elapse_time = clerkTemp.c_data.exit_time - clerkTemp.c_data.start_time;

            if (msgsnd(msgForwardID, &clerkTemp, sizeof(clerkTemp), 0) == -1) {
                fprintf(stderr, "Couldn't send msg\n");
            }
            totalWait += clerkTemp.c_data.start_time - clerkTemp.c_data.enter_time;
            totalWork += clerkTemp.c_data.exit_time - clerkTemp.c_data.start_time;
        }

    }


    struct clerk Clerk;
    Clerk.msgType = 1;
    Clerk.id = customer_t;
    Clerk.elapse = clerkEndTime - clerkStartTime;
    Clerk.work = totalWork;
    Clerk.wait = totalWait;
    Clerk.numOfCustomers = numOfCustomersClerk;

    if (Clerk.numOfCustomers != 0) {

        Clerk.avgWork = Clerk.work / Clerk.numOfCustomers;
        Clerk.avgWait = Clerk.wait / Clerk.numOfCustomers;

    } else {
        Clerk.avgWork = 0;
        Clerk.avgWait = 0;
    }
    if (msgsnd(arrivalID, &Clerk, sizeof(Clerk), 0) == -1) { // send clerk struct to main
        fprintf(stderr, "Couldn't send msg\n");
    }


    msgctl(msgRecieveID, IPC_RMID, NULL);


}


void initCustomers() {
    int msgid;
    int msgquit;
    int flag = TRUE;
    int type = -1;
    int handleTimeArrive;
    int num = 0; //local num of customers
    double avg;
    double std;
    double min;
    setArriveConsts(&avg, &std, &min);
    quit.msg = 2; //special indicator -> msg type for quit message
    msgid = msgget(sorter, 0666 | IPC_CREAT);
    msgquit = msgget(quitKey, 0666 | IPC_CREAT);
    customer arrivalTemp;
    while (flag) {
        usleep(10);
        handleTimeArrive = (int) initRandomByType(avg, std, min);
        generateCustomer(&type);
        num++;
        gatherInfo(&arrivalTemp, type, num);
        msgsnd(msgid, &arrivalTemp, sizeof(arrivalTemp), 0);
        usleep(handleTimeArrive * 10);//todo:change later
        if (msgrcv(msgquit, &quit, sizeof(quit), 1, IPC_NOWAIT) == -1) {
            if (errno == ENOMSG) {
                continue;
            } else {
                fprintf(stderr, "msgrcv failed SIM, %s\n", strerror(errno));
            }
        } else {
            arrivalTemp.c_data.type = QUIT;
            arrivalTemp.c_id = 1;
            msgsnd(msgid, &arrivalTemp, sizeof(arrivalTemp), 0);
            flag = FALSE;
        }


    }

    msgctl(msgquit, IPC_RMID, NULL);


}

void printClerk(int type) {
    printf("Clerk for %s customers: processed %d customers\n", toStringByType(type), clerkList[type].numOfCustomers);
    printf("elapse:%ld work:%ld wait:%ld\n", clerkList[type].elapse, clerkList[type].work, clerkList[type].wait);
    printf("per customer: work: %ld wait:%ld\n", clerkList[type].avgWork, clerkList[type].avgWait);
    printf("\n");

}

void getMsgFromClerk() { // get struct clerk sent by a single clerk and add to list
    int arrivalID = msgget(arrival, 0666);
    struct clerk temp;
    if (msgrcv(arrivalID, &temp, sizeof(temp), 0, 0) == -1) {
        fprintf(stderr, "msgrcv failed CLERK, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    clerkList[temp.id] = temp;
}

void init() { // init keys stopwatch and allocate memory for the clerk list
    sw = (struct stopwatch *) malloc(sizeof(struct stopwatch));
    sorter = ftok("simulation.c", 's');
    upgrade = ftok("simulation.c", 'u');
    repair = ftok("simulation.c", 'r');
    new = ftok("simulation.c", 'n');
    quitKey = ftok("simulation.c", 'q');
    finishKey = ftok("simulation.c", 'f');
    arrival = ftok("simulation.c", 'a');
    clerkList = (struct clerk *) malloc(sizeof(struct clerk) * 3);
}

void startSim() { // fork children and start by type
    init();
    int arrivalID = msgget(arrival, 0666 | IPC_CREAT);
    int status;
    swstart(sw);
    if ((pids[0] = fork()) == 0) {
        printf("sorter started\n");
        startSorter();
        return;
    }
    if (pids[0] < 0) {
        perror("sorter Fork error");
    }
    if ((pids[1] = fork()) == 0) {
        printf("clerk started\n");
        startClerk("0");
        return;
    }
    if (pids[1] < 0) {
        perror("Fork clerk 1 error");
    }
    if ((pids[2] = fork()) == 0) {
        printf("clerk started\n");
        startClerk("1");
        return;

    }
    if (pids[2] < 0) {
        perror("Fork clerk 2 error");
    }
    if ((pids[3] = fork()) == 0) {
        printf("clerk started\n");
        startClerk("2");
        return;

    }
    if (pids[3] < 0) {
        perror("Fork error");
    }

    initCustomers(); // start sending customers to sorter
    if (waitpid(pids[0], &status, 0) != -1) {
        if (WIFEXITED(status)) {
            printf("Sorter quitting\n\n");
        }

    }
    if (waitpid(pids[1], &status, 0) != -1) {
        if (WIFEXITED(status)) {
            printf("Clerk for new customers is quitting\n");
            getMsgFromClerk();
            printClerk(NEW);
        }

    }
    if (waitpid(pids[2], &status, 0) != -1) {
        if (WIFEXITED(status)) {
            printf("Clerk for upgrade customers is quitting\n");
            getMsgFromClerk();
            printClerk(UPGRADE);
        }

    }
    if (waitpid(pids[3], &status, 0) != -1) {
        if (WIFEXITED(status)) {
            printf("Clerk for repair customers is quitting\n");
            getMsgFromClerk();
            printClerk(REPAIR);
        }

    }


    free(sw);
    free(clerkList);
    msgctl(arrivalID, IPC_RMID, NULL);


}


int main(int argc, char *argv[]) {
    startSim();
    return 0;
}