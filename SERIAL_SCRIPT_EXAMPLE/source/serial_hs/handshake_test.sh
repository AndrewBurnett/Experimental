#!/bin/bash
port=$1
retval=0

./serial_hs $port RTS -v
retval=$?
if [ $retval -eq 1 ]; then
	echo -e "\n ERROR : application failure "
	return $retval
fi
if [ $retval -eq 2 ]; then 
	echo -e "\n ERROR : RTS -> CTS failure" 
fi

./serial_hs $port DTR -v
retval=$?
if [ $retval -eq 1 ]; then
	echo -e "\n ERROR : application failure "
	return $retval
fi
if [ $retval -eq 3 ]; then 
	echo -e "\n ERROR : DTR -> DCD failure" 
fi
if [ $retval -eq 4 ]; then 
	echo -e "\n ERROR : DTR -> DSR failure" 
fi
if [ $retval -eq 5 ]; then 
	echo -e "\n ERROR : DTR -> RI failure" 
fi

return $retval
