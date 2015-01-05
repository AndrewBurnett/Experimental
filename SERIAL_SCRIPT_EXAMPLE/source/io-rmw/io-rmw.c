/* 
 * Simple x86 io write and read using ioperm
 * AB 1.1	101014
*/

#include <stdio.h> 
#include <unistd.h>
#include <sys/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int main (int argc, char *argv[])
{
	bool write = false;
	bool read = false;
	int data = 0x00;
	int port = 0x00;
	bool set = false;
	bool unset = false;
	bool verbose = false;
	
	//check number of arguments
	if ((argc < 3) || (argc > 6))	
	{
		printf("\n io-rmw - x86 io byte read-modify-write");
		printf("\n read a port  : -r <address in hex>");
		printf("\n write a port : -w <address in hex> <data> <mask|unmask>");
		printf("\n verbose      : -v");
		printf("\n");
		exit (1);
	}

	//io read or write 
	if (strstr(argv[1], "-r"))	{
		read=true;
	}
	if (strstr(argv[1], "-w"))	{
		write=true;
		if (argc < 5)	{
			printf("\n mask|unmask command required for a write");
			printf("\n write a port : -w <address in hex> <data> <mask|unmask>\n");
			exit(1);
		}
	}

	//get and enable access to port address 
	sscanf(argv[2], "%x", &port);
	if (port >= 0x400)	{
		iopl(3);
	}
	else	{
		ioperm(port, 1, 1);
	}

	//get write data
	if (argc > 3) {
		if (strstr(argv[3], "-v"))	{
			verbose=true;
		}
		else {
			sscanf(argv[3], "%x", &data);
			if (data > 0xFF)	{
				printf("\n Write data out of range");
				printf("\n Expect between 0x00 and 0xFF\n");
				exit(1);
			}
		}
	}

	//Get mask
	if (argc > 4) {
		if (strstr(argv[4], "unmask"))	{
			set=true;
		}		
		else if (strstr(argv[4], "mask"))	{
			unset=true;
		}
		else {
			printf("\n mask option not found");
			exit(1);
		}
	}
	
	// get verbose mode
	if (argc > 5) {
		if (strstr(argv[5], "-v"))	{
			verbose=true;
		}
	}
		
	// read port
	if (read)	{
		data = inb(port);
		if (verbose)
			printf("\n read 0x%x : 0x%x \n", port, data);
	}

	//write port
	if (write)	{	
		data &= 0xFF;
		
		if (set)
			outb(inb(port) | data, port);
		
		if (unset)
			outb(inb(port) & data, port);
		
		usleep(1000);

		data = inb(port);
		if (verbose)
			printf("\n read 0x%x : 0x%x \n", port, data);
	}
	printf("\n");
	exit(0);
}

/* EOF */
