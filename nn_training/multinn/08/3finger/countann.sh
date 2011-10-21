#!/bin/sh

cat ann_training_data |
(
    while read line
    do
        fieldnum=`echo $line | awk '{ print NF }'`
        if [ "$fieldnum" -gt "600" ]
        then
            echo $line | cut -f1-600 -d' '
        elif [ "$fieldnum" -lt "600" -a "$fieldnum" -gt "12" ]
        then
            addnum=$((600-$fieldnum))
            echo $line $(yes "0 " | head -n$addnum)
        else
            echo $line
        fi
    done
)

#awk '{
#         if (NF != 3 && NF != 12 && NF < 600)
#         {
#	    print NF "Test"
#          $0 = $0 "JEIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII";
#         }
#         else
#         {
 #             print $0;
#         }
#
#     }' ann_training_data
