

#ifndef HW3_SIM_H
#define HW3_SIM_H

#include "pelecom.h"
#include "string.h"

#define NUMOFPROC 4
#define TRUE 1
#define FALSE 0

enum queueKeys {
    NEW = TYPE_NEW, UPGRADE = TYPE_UPGRADE, REPAIR = TYPE_REPAIR, QUIT = TYPE_QUIT, SORTER
};


// init sw
stopwatch sw[1];
customer *customersList;
int numOfCustomers = 0;
int finishTime = 0;
int status;
customer c;
pid_t pid[NUMOFPROC];


char *execFiles[][3] = {
        {"./clerk", "0",   NULL},
        {"./clerk", "1",   NULL},
        {"./clerk", "2",   NULL},
        {"./sorter", NULL, NULL}

};


#endif //HW3_SIM_H