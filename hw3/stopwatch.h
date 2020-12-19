/**
 *  stopwatch.h - header file for millisecond stopwatch
 */

#include <sys/time.h>

typedef struct stopwatch
{
    struct timeval tv;
} stopwatch;

void swstart( stopwatch *sw );
long swlap( stopwatch *sw );
