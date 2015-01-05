#!/bin/bash
# Example test script for test a range of baud rates on serial interface
port=$1
serial=$2
testfile=$3
flow=$4

error=0
count=0
retry_count=0
flow=0

echo " SERIAL TEST "
echo -e " Protocol\t: $serial "
echo -e " Port\t\t: $1 " 
echo -e " Data\t\t: $(ls -ls $testfile | awk '{ print $6 }') bytes "
for baud in 115200 57600 38400 19200 9600 4800 2400 1200 600 300
do
	retry=3
	while (( retry != 0 )); do
		./serial_client $port 115200 "$(< $testfile)" $baud $serial $flow 
		retval=$?
		echo -en " Baud $baud\t:"
		if [ $retval -eq 1 ]; then 
			echo " application failure "
			retry=0
			break
		fi
		if [ $retval -eq 2 ]; then
			echo " write failure "
		fi
		if [ $retval -eq 3 ]; then
			echo " write retry failure "
		fi
		if [ $retval -eq 4 ]; then
			echo " write command failure "
		fi
		if [ $retval -eq 5 ]; then
			echo " read data compare failure "
		fi
		if [ $retval -eq 6 ]; then 
			echo " read data count failure "
		fi
		if [ $retval -ne 0 ]; then 
			((retry--))
			((retry_count++))
		else
			echo -e " OK " 
			break
		fi
		sleep 1
	done
	if [ $retry -eq 0 ]; then
		echo Serial comms test failure
		echo port $2 type $serial baud $baud loop $loop
		(( error++ ))
		break
	fi
	sleep 1
done

if [ $error -eq 0 ]; then
	echo " PASS"
else
	echo " FAIL"
fi
return $error

#EOF


