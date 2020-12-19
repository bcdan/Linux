/**
 *  random - a collection of functions returning random numbers
 *
 *  nrand( average, standard-deviation ) - returns a number drawn from a NORMAL 
 *         distribution having the given properties.
 *
 *  urand( minimum, maximum ) - returns a number drawn from a UNIFORM distribution
 *         between the given bounds.
 *
 *  It is advised to call initrand() before using any of the others to insure a fresh 
 *  set of random values. 
 */

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <sys/time.h>
#include "random.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double nrand01();
double urand01();


/**
 * initrand - initialize the random generator
 */
void initrand()
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    srand( tv.tv_sec );
}


/*
 * nrand - returns a number normally distribution with average 'avg' and std dev 'std'
 */
double nrand( double avg, double std )
{
    return( avg + std * nrand01() );
}


/*
 * nrand01 - returns normal distribution with average 0 and std dev of 1
 */
double nrand01() 
{
    return( sqrt( -2 * log( urand01() ) ) * cos( 2 * M_PI * urand01() ) );
}


/*
 * pnrand - returns a random number, normally distributed (with 'avg', 'std') but 
 *          no less than 'min'
 */
double pnrand( double avg, double std, double min )
{
    double v;

    while( ( v = nrand( avg, std ) ) < min )
        ;

    return( v );
}


/*
 * urand - returns a random number uniformly distributed in the range [min,max]
 */
double urand( double min, double max )
{
    return( min + urand01() * ( max - min ) );
}


/* 
 * urand01 - returns uniform distribution in the range (0..1]
 */
double urand01()
{
    return( rand() + 1.0 ) / ( RAND_MAX + 1.0 );
}


int test_random( int argc, char *argv[] )
{
    int i;
    int size = 1000;

    double avg;
    double std;
    double min = -INT_MAX;
    double cnt;
    double x;
    double sum = 0;
    double ssq = 0;

    if( argc < 3 )
    {
        printf( "Usage: %s <average> <std-dev> [sample-size] [minimum]\n", argv[0] );
        return( EXIT_SUCCESS );
    }

    avg = atof( argv[1] );
    std = atof( argv[2] );
    if( argc > 3 )
        size = atoi( argv[3] );
    cnt = (double) size;
    if( argc > 4 )
        min = atof( argv[4] );

    printf( "average=%8.2f     std-dev=%8.2f     size=%d   min=%6.0f\n", avg, std, size, min );

    for( i = 0; i < size; i++ )
    {
        x = pnrand( avg, std, min );
        sum += x;
        ssq += x * x;
    }

    avg = sum / cnt;
    std = sqrt( ssq / cnt - avg * avg );
    printf( "average=%8.2f     std-dev=%8.2f\n", avg, std );
}





/*
int main(  int argc, char *argv[] )
{
    test_random( argc, argv );
}
*/

