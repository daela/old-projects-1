#!/bin/bash

cd "$(dirname "$0")"

./start_pulseaudio.sh

./start_voice_server.sh &>/dev/null &

./start_ventrilo.sh &

cd ..
WINEPREFIX="$HOME/.wine_bot" wine dfweb_win32.exe & 

# supposedly this should kill the child processes on SIGTERM. 
trap "kill 0" SIGINT SIGTERM EXIT

wait

