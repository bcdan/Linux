/**
 * stopwatch - routines for registering wall-clock time 
 *
 * The routines defined here help keep track of elapsed time. They simulate a 
 * stopwatch with millisecond accuracy. The stopwatch itself is kept in a structure
 * named 'stopwatch' which must be allocated by the caller. 
 *
 * swstart - starts the stopwatch indicated in the argument.
 *
 * swlap - returns the number of milliseconds elapsed since swstart was called last
 *         on the given stopwatch. 
 */

#include <stdio.h>
#include <stdlib.h>
#include "stopwatch.h"




/*
 * swstart - starts the stopwatch
 */
void swstart( stopwatch *sw )
{
    gettimeofday( &sw->tv, NULL );
}


/* 
 * swlap() - returns the number of milliseconds since the stopwatch was started
 */
long swlap( stopwatch *sw )
{
    struct timeval tv1;
    long result;

    if( sw->tv.tv_sec == 0 )
    {
        printf( "stopwatch not initialized\n" );
        return( 0 );
    }

    gettimeofday( &tv1, NULL );
    result = ( tv1.tv_sec - sw->tv.tv_sec ) * 1000;
    result += ( tv1.tv_usec - sw->tv.tv_usec ) / 1000;
    return( result );
}



int test_sw()
{
    int  flag = 1;
    int  i = 0;
    char c[10];
    int  s;
    stopwatch sw[10];

    while( flag )
    {
        printf( "cmd> " );
        scanf( "%s", c );
        switch( c[0] )
        {
        case 'i':
        case 's':
            swstart( &sw[i] );
            printf( "started stopwatch %d\n", i );
            i++;
            break;
    
        case 'l':
            printf( "Enter stopwatch number: " );
            scanf( "%d", &s );
            if( ( s < 0 ) || ( s > 9 ) )
            {
                printf( "Illegal stopwatch number %d\n", s );
                break;
            }
            printf( "%ld\n", swlap( &sw[s] ) );
            break;

        case 'q':
            flag = 0;
            break;

        default:
            printf( "illegal command <%c>; enter one of [slq]\n", c[0] );
            break;
        }
    }
}

/*
int main( int argc, char *argv[] )
{
    test_sw();
}
*/

