#!/bin/bash

## comment out whichever is not needed (both are here for any case)

echo Buildling windows executable...
# g++ for windows (test with wine)
wg++ -std=c++17 -o3 -os -ofast -static -o todo-hunter.exe main.cpp
echo Done building windows executable.

echo Building linux executable...
# Using standard g++ for linux
g++ -std=c++17 -o3 -os -ofast -static -o todo-hunter main.cpp
echo Done building linux executable.

## Uncomment these for smaller executable
# strip todo-hunter
# strip todo-hunter.exe

echo Done!
