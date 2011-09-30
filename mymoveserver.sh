#!/bin/sh

echo "Moving mymoves.conf to right place if it exists..."
[[ -e /home/user/MyDocs/.moves/mymoves.conf ]] && cat /home/user/MyDocs/.moves/mymoves.conf > /home/user/.config/mymoves.conf
[[ -d /home/user/MyDocs/.moves ]] && rm -rf /home/user/MyDocs/.moves

echo "Starting mymoveserver"

exec /opt/mymoves/bin/mymoveserver
