#!/bin/bash
# make sure it's a number
volume_percent="$1"

if ! [ "$volume_percent" -eq "$volume_percent" 2>/dev/null ]; then
	echo "Invalid volume; using maximum"
	volume_percent=100
fi
# constrain value if invalid
if [ "$volume_percent" -lt "0" ]; then
	volume_amount=0
elif [ "$volume_percent" -gt "100" ]; then
	volume_amount=65535
else
	# within valid range
	# assume it's a percent and convert to amount if less than 100
	if [ "$volume_percent" -le "100" ]; then
		volume_amount="$((65535*$volume_percent/100))"
	else
		# it isn't a percent, use it as value
		volume_amount="$volume_percent"
	fi

fi
pacmd set-sink-volume null_mic $volume_amount
