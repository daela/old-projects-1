#!/bin/bash
cd "$(dirname "$0")"
#voicenum="$1"
#shift
voicenum=3

open-sapi/src/unstable/osapi-cli.tcl -p "$voicenum" -t "$@" @@
