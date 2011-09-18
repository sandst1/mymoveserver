#!/bin/sh

amount=14
samples=10

#for x in `seq 0 21`
for x in `seq 0 $(($amount-1))`
do
    /opt/mymoveserver/bin/mymoveserver $x $amount $samples
    cat gesturedata$x >> ann_training_data
done

