#!bin/bash

#read modification date:

stat -c %y test.txt
for f in *.wav
do
    mv -n "$f" "$(date -r "$f" +"%Y.%m.%d_%H:%M:%S").wav"
done
