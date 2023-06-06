#!/bin/bash
clear;
gcc lab04.c -o ./exe/main -lm -pthread;
cd ./exe;
./main < MASTER.in;

