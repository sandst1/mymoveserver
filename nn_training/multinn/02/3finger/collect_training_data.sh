#!/bin/sh

amount=4
samples=10

#for x in `seq 0 $(($amount-1))`
for x in `seq 2 3`
do
    /opt/mymoveserver/bin/mymoveserver $x $amount $samples
    cat gesturedata$x >> ann_training_data
done

