/**
 *  random.h - header file for random number generating routines
 */
#include <math.h>

void   initrand();
double nrand();
double pnrand( double avg, double std, double min );
double urand( double min, double max );

