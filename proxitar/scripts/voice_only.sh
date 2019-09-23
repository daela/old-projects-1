#!/bin/bash

cd "$(dirname "$0")"

./start_pulseaudio.sh

./start_voice_server.sh &>/dev/null &


