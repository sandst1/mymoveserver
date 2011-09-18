#!/bin/sh

amount=14
samples=15


#for x in `seq 0 $(($amount-1))`
for x in `seq 10 13`
do
    /opt/mymoveserver/bin/mymoveserver $x $amount $samples
    cat gesturedata$x >> ann_training_data
done

