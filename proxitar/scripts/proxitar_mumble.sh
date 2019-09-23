#!/bin/bash

cd "$(dirname "$0")"

./start_pulseaudio.sh

./start_voice_server.sh &>/dev/null &

cd ..
# this starts mumble for us
./dfweb_lin32 & 

# supposedly this should kill the child processes on SIGTERM. 
trap "kill 0" SIGINT SIGTERM EXIT

wait

