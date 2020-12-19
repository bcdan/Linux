

#ifndef HW3_CLERK_H
#define HW3_CLERK_H
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "random.h"
#include "pelecom.h"
#include <sys/shm.h>



enum queueKeys {NEW=TYPE_NEW,UPGRADE=TYPE_UPGRADE,REPAIR=TYPE_REPAIR,QUIT=TYPE_QUIT,SORTER};


customer c;

double initRandomByType(double avg,double std,double min );
void setNew(double * avg, double *std, double* min);
void setUpgrade(double * avg, double *std, double* min);
void setRepair(double * avg, double *std, double* min);
void getRandomConstsByType(int customer_t,double * avg, double *std, double* min);
void start(char * type);


#endif //HW3_CLERK_H
