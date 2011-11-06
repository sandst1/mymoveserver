#!/bin/sh

amount=16
samples=15

#for x in `seq 0 $(($amount-1))`
for x in `seq 12 15`
do
    /opt/mymoves/bin/mymoveserver $x $amount $samples
    cat gesturedata$x >> ann_training_data
done

