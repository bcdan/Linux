

#ifndef HW3_DIFFTRY_SIMULATION_H
#define HW3_DIFFTRY_SIMULATION_H


#include "pelecom.h"
#include <string.h>
#include <sys/wait.h>
#include "random.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>

#define NUMOFPROC 4

#define TRUE 1
#define FALSE 0

enum queueKeys {
    NEW = TYPE_NEW, UPGRADE = TYPE_UPGRADE, REPAIR = TYPE_REPAIR, QUIT = TYPE_QUIT, SORTER
};


struct qMessage {
    long msgType;
    int msg;
} quit;


struct clerk {
    long msgType;
    int id;
    long elapse;
    long work;
    long wait;
    int numOfCustomers;
    long avgWork;
    long avgWait;
};


struct clerk *clerkList;

int numOfCustomers = 0;
pid_t pids[NUMOFPROC] = {0};
key_t quitKey;
key_t finishKey;
key_t sorter;
key_t upgrade;
key_t repair;
key_t new;
key_t arrival;


void startSim();

void init();

void getMsgFromClerk();

void printClerk(int type);

void initCustomers();

void startClerk(char *type);

void startSorter();

void setConsts(double *avg, double *std, double *min);

char *toStringByType(int type);

key_t getKeyByType(int type);

void gatherInfo(customer *c, int type, int num);

void getRandomConstsByType(int customer_t, double *avg, double *std, double *min) ;

void setArriveConsts(double *avg, double *std, double *min) ;

void setRepair(double *avg, double *std, double *min) ;

void setUpgrade(double *avg, double *std, double *min) ;

void setNew(double *avg, double *std, double *min) ;

void generateCustomer(int *typeOfCustomer) ;

double initRandomByType(double avg, double std, double min) ;


#endif //HW3_DIFFTRY_SIMULATION_H







