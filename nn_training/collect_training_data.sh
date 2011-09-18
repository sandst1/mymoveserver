#!/bin/sh
#/opt/mymoveserver/bin/mymoveserver 0
#/opt/mymoveserver/bin/mymoveserver 1
#/opt/mymoveserver/bin/mymoveserver 2
#/opt/mymoveserver/bin/mymoveserver 3
#/opt/mymoveserver/bin/mymoveserver 4
#/opt/mymoveserver/bin/mymoveserver 5
#/opt/mymoveserver/bin/mymoveserver 6
#/opt/mymoveserver/bin/mymoveserver 7
#/opt/mymoveserver/bin/mymoveserver 8
#/opt/mymoveserver/bin/mymoveserver 9
#/opt/mymoveserver/bin/mymoveserver 10
#/opt/mymoveserver/bin/mymoveserver 11
#/opt/mymoveserver/bin/mymoveserver 12
#/opt/mymoveserver/bin/mymoveserver 13
#/opt/mymoveserver/bin/mymoveserver 14
#/opt/mymoveserver/bin/mymoveserver 15
#/opt/mymoveserver/bin/mymoveserver 16
#/opt/mymoveserver/bin/mymoveserver 17
/opt/mymoveserver/bin/mymoveserver 18
/opt/mymoveserver/bin/mymoveserver 19
/opt/mymoveserver/bin/mymoveserver 20
/opt/mymoveserver/bin/mymoveserver 21
/opt/mymoveserver/bin/mymoveserver 22
/opt/mymoveserver/bin/mymoveserver 23
/opt/mymoveserver/bin/mymoveserver 24
/opt/mymoveserver/bin/mymoveserver 25

#for x in `seq 0 25`
for x in `seq 0 25`
do
    cat gesturedata$x >> ann_training_data
done

