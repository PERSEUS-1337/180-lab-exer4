#!/bin/bash

# Open a new WSL terminal
cmd.exe /c start cmd.exe /c wsl.exe -c "gcc lab04_test.c -o test; ./test"
sleep 100

# read -rp "Press Enter to exit..."