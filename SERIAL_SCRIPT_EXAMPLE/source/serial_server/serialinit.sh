#!/bin/bash
#Initialisation for AIM104-COM4

if ! lsmod | grep st16c554 ; then
	echo load module : 8250_exar_st16c554 
	modprobe 8250_exar_st16c554
else
	echo module loaded
fi

echo setserial /dev/ttyS4
setserial /dev/ttyS4 port 0x180 irq 5 uart 16550 
echo setserial /dev/ttyS5
setserial /dev/ttyS5 port 0x188 irq 7 uart 16550
echo setserial /dev/ttyS6
setserial /dev/ttyS6 port 0x190 irq 10 uart 16550
echo setserial /dev/ttyS7
setserial /dev/ttyS7 port 0x198 irq 11 uart 16550

#stty -F /dev/ttyS4 115200 clocal -crtscts raw 
stty -F /dev/ttyS4 115200 raw 
stty -F /dev/ttyS5 115200 raw
stty -F /dev/ttyS6 115200 -crtscts
stty -F /dev/ttyS7 115200 -crtscts
