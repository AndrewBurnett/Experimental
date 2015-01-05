#!/bin/bash
# ISIS serial control 
# Set COM2/ttyS1 to RS232/RS422/RS485.
# For use in manual systems where an appropriate cable/interface is to be connected.

serial=$1		# type of serial communication RS232,RS485,RS422
relay=$2		# allow relay control, for use on test fixture, RELAY or 0 to ignore
port=$3			# command port for relay control, /dev/ttyS0 or 0 to ignore
delay=1			# delay after relay activation

# Set Serial port command strings
if [ $serial == "RS232" ]; then
	cmd=CMDRS232
fi
if [ $serial == "RS422" ]; then
	cmd=CMDRS422
fi
if [ $serial == "RS485" ]; then
	cmd=CMDRS485
fi

# Set COM2/ttyS1 serial port to high impedance to allow change to RS485/422
#if [ $serial != "RS232" ]; then
	./io-rmw -w 0x5D0 0x88 unmask
#fi

# if test fixture, Send relay command to slave
if [ "$relay" == "RELAY" ]; then
	echo " RELAY CONTROL"
	./serial_client $port 115200 $cmd 115200 $serial
	sleep $delay
fi

# Set COM2/ttyS1 to RS232
if [ $serial == "RS232" ]; then
	echo " Set RS232"
	./io-rmw -w 0x5D0 0x77 mask
fi

# Set port to respective interface type
if [ $serial == "RS422" ]; then
	echo " Set RS422"
	./io-rmw -w 0x5D0 0xF7 mask
	./io-rmw -w 0x5D0 0x80 unmask
fi

if [ $serial == "RS485" ]; then
	echo " Set RS485"
	./io-rmw -w 0x5D0 0x7F mask
	./io-rmw -w 0x5D0 0x08 unmask
fi

#EOF


