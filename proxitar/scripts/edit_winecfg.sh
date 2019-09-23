#!/bin/bash
cd "$(dirname "$0")"
while [ 1 ];
do
	WINEPREFIX="$HOME/.wine_bot" winecfg
	sleep 1
done
