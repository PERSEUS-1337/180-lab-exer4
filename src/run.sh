#!/bin/bash
clear;
gcc lab04_test.c -o ./exe/test -lm -pthread;
cd ./exe;
./test;

