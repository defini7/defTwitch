#!/bin/bash

if !(-d "Bin")
then
    mkdir Bin
fi

clang++ -Wall -std=c++20 defTwitch.cpp Examples/Source.cpp -o Bin/Examples -l ws2_32