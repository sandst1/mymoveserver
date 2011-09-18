#!/bin/bash

x=$1
zerosbefore=$(($x))
#zerosafter=$((25-$x))
zerosafter=$((21-$x))
for i in `seq $zerosbefore`; do printf '0 '; done
printf "1 "
for i in `seq $zerosafter`; do printf '0 '; done
printf '\n'
