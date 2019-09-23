#!/bin/bash
cd "$(dirname "$0")"
while [ 1 ];
do
	# wine actually respects PULSE_SINK
	export PULSE_SINK="null_mic"
	export WINEPREFIX="$HOME/.wine_bot"
	wine open-sapi/tools/tcl/bin/tclsh85.exe open-sapi/src/unstable/osapi-srv.tcl
	sleep 1
done
