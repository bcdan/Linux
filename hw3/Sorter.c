
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






void start() {
    int running = 1;
    key_t customerKey;
    int msgRecieveId = 0;
    int msgForwardID = 0;
    double avg = 0;
    double std = 0;
    double min = 0;
    double handleTime = 0; ///////////////////
    double countHandle = 0;///////////////
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
    key_t keys[] = {new, upgrade, repair};
    int msgIDUpgrade= msgget(upgrade, 0666|IPC_CREAT );
    int msgIDRepair=msgget(repair, 0666|IPC_CREAT );
    int msgIDNew=msgget(new, 0666|IPC_CREAT );
    printf("\n");
    sleep(2);
    c.c_id = 1;
    while (running) {
        if (msgrcv(msgRecieveId, &c, sizeof(c), 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        handleTime = initRandomByType(avg, std, min);
        countHandle += handleTime;
        usleep(handleTime); //todo: test this later
        switch (c.c_data.type) {
            case NEW:
                msgsnd(msgIDNew, &c, sizeof(c), 0);
                break;
            case UPGRADE:
                msgsnd(msgIDUpgrade, &c, sizeof(c), 0);
                break;
            case REPAIR:
                msgsnd(msgIDRepair, &c, sizeof(c), 0);
                break;
            case QUIT: {
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

        // sleep(2);

    }

    // sendTerminalMsgs();
    printf("Sorter: Im removing my queue\n");

    msgctl(msgRecieveId, IPC_RMID, NULL);

}

















//
//void start() {
//    int running = 1;
//    key_t customerKey;
//    int msgRecieveId = 0;
//    int msgForwardID = 0;
//    double avg = 0;
//    double std = 0;
//    double min = 0;
//    double handleTime = 0; ///////////////////
//    double countHandle = 0;///////////////
//    setConsts(&avg, &std, &min);
//    key_t sorter = ftok("Sorter.c", 's');
//    msgRecieveId = msgget(sorter, 0666 | IPC_CREAT);
//    if (msgRecieveId == -1) {
//        fprintf(stderr, "msgget failed , [%d]\n", errno);
//        exit(EXIT_FAILURE);
//    }
//    key_t upgrade = ftok("Clerk.c", 'u');
//    key_t repair = ftok("Clerk.c", 'r');
//    key_t new = ftok("Clerk.c", 'n');
//    key_t keys[] = {new, upgrade, repair};
//    printf("\n");
//    sleep(2);
//    c.c_id = 1;
//    while (running) {
//        if (msgrcv(msgRecieveId, &c, sizeof(c), 0, 0) == -1) {
//            fprintf(stderr, "msgrcv failed, %s\n", strerror(errno));
//            exit(EXIT_FAILURE);
//        }
//        handleTime = initRandomByType(avg, std, min);
//        countHandle += handleTime;
//        usleep(handleTime); //todo: test this later
//        switch (c.c_data.type) {
//            case NEW:
//                customerKey = new;
//                msgForwardID = msgget(customerKey, 0666 | IPC_CREAT);
//                printf("Sorter msgIDForward : %d\n", customerKey);
//                msgsnd(msgForwardID, &c, sizeof(c), 0);
//                break;
//            case UPGRADE:
//                customerKey = upgrade;
//                msgForwardID = msgget(customerKey, 0666 | IPC_CREAT);
//                printf("Sorter msgIDForward : %d\n", customerKey);
//                msgsnd(msgForwardID, &c, sizeof(c), 0);
//                break;
//            case REPAIR:
//                customerKey = repair;
//                msgForwardID = msgget(customerKey, 0666 | IPC_CREAT);
//                printf("Sorter msgIDForward : %d\n", customerKey);
//                msgsnd(msgForwardID, &c, sizeof(c), 0);
//                break;
//            case QUIT: {
//                for (int i = 0; i < 3; ++i) {
//                    c.c_id = 1;
//                    customerKey = keys[i];
//                    msgForwardID = msgget(customerKey, 0666 | IPC_CREAT);
//                    msgsnd(msgForwardID, &c, sizeof(c), 0);
//                }
//                running = 0;
//                break;
//            }
//
//
//        }
//
//        // sleep(2);
//
//    }
//
//    // sendTerminalMsgs();
//     msgctl(msgRecieveId, IPC_RMID, NULL);
//
//}

int main(int argc, char *argv[]) {
    start();
    printf("sorter returned\n");
    return 0;
}