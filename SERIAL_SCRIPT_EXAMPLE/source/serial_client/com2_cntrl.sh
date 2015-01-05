#!/bin/bash
#ISIS specific script for disabling and enabling serial com2/ttyS1 

if [ "$1" == "DISABLE" ]; then 
	echo COM2 disable
	./io-rmw -w 0x5D0 0x88 unmask
fi

if [ "$1" == "ENABLE" ]; then
	echo COM2 enable
	./io-rmw -w 0x5D0 0x77 mask
fi

echo Read register
./io-rmw -r 0x5D0 -v

#EOF
