#!/bin/sh

#for x in `seq 0 21`
for x in `seq 10 21`
do
    /opt/mymoveserver/bin/mymoveserver $x
    cat gesturedata$x >> ann_training_data
done

