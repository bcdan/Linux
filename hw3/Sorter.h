

#ifndef HW3_SORTER_H
#define HW3_SORTER_H

#include "pelecom.h"
#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include "random.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define NUMOFPROC 3
enum queueKeys {NEW=TYPE_NEW,UPGRADE=TYPE_UPGRADE,REPAIR=TYPE_REPAIR,QUIT=TYPE_QUIT,SORTER};

pid_t pids[NUMOFPROC];
customer c;


char *execFiles[][3] = {
        {"./clerk", "0",   NULL},
        {"./clerk", "1",   NULL},
        {"./clerk", "2",   NULL},

};


#endif //HW3_SORTER_H
