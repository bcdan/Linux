

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
#define MSGBUFF 50

enum queueKeys {NEW=TYPE_NEW,UPGRADE=TYPE_UPGRADE,REPAIR=TYPE_REPAIR,QUIT=TYPE_QUIT,SORTER};

//struct message {
//    long msgType;
//    char msgText[MSGBUFF];
//    customer c;
//} message;

customer c;
//char *customerTypes[] = {
//        "NEW",
//        "UPGRADE",
//        "REPAIR",
//        "QUIT"
//};



#endif //HW3_SORTER_H
