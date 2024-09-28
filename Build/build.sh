#!/bin/bash

if !(-d "Bin")
then
    mkdir Bin
fi

g++ defTwitch.cpp Examples/Source.cpp -o Bin/App -std=c++20 -Wall