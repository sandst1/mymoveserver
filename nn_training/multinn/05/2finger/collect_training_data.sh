#!/bin/sh

amount=10
samples=15

#for x in `seq 0 $(($amount-1))`
for x in `seq 1 9`
do
    /opt/mymoveserver/bin/mymoveserver $x $amount $samples
    cat gesturedata$x >> ann_training_data
done

