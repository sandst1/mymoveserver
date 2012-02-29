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

    if [ `head -n1 $mmconf` = "1" ]
    then
        echo 2 > $mmconf.new
        head -5 $mmconf | tail -4 >> $mmconf.new
        echo "d12######" >> $mmconf.new
        echo "d13######" >> $mmconf.new
        echo "d14######" >> $mmconf.new
        echo "d15######" >> $mmconf.new
        head -17 $mmconf | tail -12 >> $mmconf.new
        echo "t12######" >> $mmconf.new
        echo "t13######" >> $mmconf.new
        echo "t14######" >> $mmconf.new
        echo "t15######" >> $mmconf.new
        head -25 $mmconf | tail -8 >> $mmconf.new

        rm $mmconf
        mv $mmconf.new $mmconf
    fi

    if [ `head -n1 $mmconf` = "2" ]
    then
        echo 3 > $mmconf.new
        head -9 $mmconf | tail -8 >> $mmconf.new
        rm $mmconf
        mv $mmconf.new $mmconf
        sed -i 's/d12###/d4###/g' $mmconf
        sed -i 's/d13###/d5###/g' $mmconf
        sed -i 's/d14###/d6###/g' $mmconf
        sed -i 's/d15###/d7###/g' $mmconf
    fi
fi

echo "Starting mymoveserver"

exec /opt/mymoves/bin/mymoveserver
