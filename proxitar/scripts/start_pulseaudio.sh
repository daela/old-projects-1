#!/bin/bash

cd "$(dirname "$0")"

pulseaudio --kill
pulseaudio --start
while ! pulseaudio --check;
do
	sleep 1
done

./fix_mic_volume.sh

