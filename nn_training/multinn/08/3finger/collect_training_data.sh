#!/bin/sh

amount=12
samples=15

#for x in `seq 0 $(($amount-1))`
for x in `seq 11 11`
do
    /opt/mymoves/bin/mymoveserver $x $amount $samples
    cat gesturedata$x >> ann_training_data
done

