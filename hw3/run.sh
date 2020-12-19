#!/bin/bash
killall -9 clerk sorter sim
gcc random.c Clerk.c -lm -o clerk
gcc random.c Sorter.c -lm -o sorter
gcc stopwatch.c random.c Sim.c  -lm -o sim
./sim


