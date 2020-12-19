

#ifndef HW3_SIM_H
#define HW3_SIM_H

#include "pelecom.h"
#include "string.h"
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0

enum queueKeys {
    NEW = TYPE_NEW, UPGRADE = TYPE_UPGRADE, REPAIR = TYPE_REPAIR, QUIT = TYPE_QUIT, SORTER
};


struct qMessage {
    long msgType;
    int msg;
} quit;

char * sorterCMD [] = {"./sorter",NULL};

// init sw
stopwatch sw[1];
customer *customersList;
int numOfCustomers = 0;
int finishTime = 0;
int status;
pid_t pid;
customer c;



#endif //HW3_SIM_H