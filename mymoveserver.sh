#!/bin/sh

echo "Moving mymoves.conf to right place if it exists..."
[[ -e /home/user/MyDocs/.moves/mymoves.conf ]] && cat /home/user/MyDocs/.moves/mymoves.conf > /home/user/.config/mymoves.conf
[[ -d /home/user/MyDocs/.moves ]] && rm -rf /home/user/MyDocs/.moves

mmconf=/home/user/.config/mymoves.conf

if [ -f $mmconf ]
then
    if [ `head -n1 $mmconf` = "0" ]
    then
        echo "Updating mymoves.conf to the newest version..."
        # Update mymoves.conf to the version 1 format
        echo 1 > $mmconf.new
        head -5 $mmconf | tail -4 >> $mmconf.new
        echo "d4######" >> $mmconf.new
        echo "d5######" >> $mmconf.new
        echo "d6######" >> $mmconf.new
        head -7 $mmconf | tail -1 | sed -e 's/d5###/d7###/' >> $mmconf.new
        echo "d8######" >> $mmconf.new
        echo "d9######" >> $mmconf.new
        echo "d10######" >> $mmconf.new
        head -6 $mmconf | tail -1 | sed -e 's/d4###/d11###/' >> $mmconf.new

        head -11 $mmconf | tail -4 >> $mmconf.new
        echo "t4######" >> $mmconf.new
        echo "t5######" >> $mmconf.new
        echo "t6######" >> $mmconf.new
        head -13 $mmconf | tail -1 | sed -e 's/t5###/t7###/'>> $mmconf.new
        echo "t8######" >> $mmconf.new
        echo "t9######" >> $mmconf.new
        echo "t10######" >> $mmconf.new
        head -12 $mmconf | tail -1 | sed -e 's/t4###/t11###/'>> $mmconf.new

        rm $mmconf
        mv $mmconf.new $mmconf
    fi
fi

echo "Starting mymoveserver"

exec /opt/mymoves/bin/mymoveserver
