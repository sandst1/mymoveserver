#!/bin/bash
gcc -o train train.c `pkg-config --cflags --libs fann`
./train
gcc -o test test.c `pkg-config --cflags --libs fann`
./test 

