#/bin/bash

rm 500gesturedata*
rm modgesturedata*
rm traindata*
rm trainingdata
rm testdata*

#for x in `seq 0 25`
for x in `seq 0 21`
do
    num=0
    while read line
    do
        num=$(($num+1))
        if [ $(($num % 2)) -eq 0 ]
        then
            #echo $line
            ./add_output_vectors.sh $line >> modgesturedata$x
        else
            echo $line >> modgesturedata$x
        fi
    done < gesturedata$x

    cat modgesturedata$x | cut -d' ' -f 1-500 > 500gesturedata$x

    cat 500gesturedata$x > traindata$x
    cat traindata$x >> trainingdata
done


