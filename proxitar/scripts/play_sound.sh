#!/bin/bash
volume_percent="$1"
sound_file="$2"

if [ -r "$sound_file" ]; then
	# make sure it's a number
	if [ "$volume_percent" -eq "$volume_percent" 2>/dev/null ]; then
		echo "Invalid volume; using maximum"
		volume_percent=100
	fi
	# constrain value if invalid
	if [ "$volume_percent" -lt "0" ]; then
		volume_amount=0
	elif [ "$volume_percent" -gt "65536" ]; then
		volume_amount=65536
	else
		# within valid range
		# assume it's a percent and convert to amount if less than 100
		if [ "$volume_percent" -le "100" ]; then
			volume_amount="$((65536*$volume_percent/100))"
		else
			# it isn't a percent, use it as value
			volume_amount="$volume_percent"
		fi

	fi
	# finally, play the sound
	paplay --device="null_mic" --volume=$volume_amount "$sound_file"
fi
