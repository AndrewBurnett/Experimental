/*
 * serial_slave.c
 * An echo loopback for serial testing between units. 
 * Receives a string and echoes back to the host. 
 *
 * Developed on the ISIS test fixture. 
 * The serial interface was an AIM104-COM4 with an AIM104-RELAY8 providing
 * isolation between RS232 and RS422/485. 
 *
 * The original location of this source (v0.1 9/17/01) is
 * http://www.embeddedlinuxinterfacing.com/chapters/06/querySerial.c
 * Copyright (C) 2001 by Craig Hollabaugh as set out under the GPLv2
 * Please do not contact the original author of this utility for support.
 *
 * gcc serial_slave.c -o serial_slave 
 *
 * 1.1 AB 18/12/14
 * - Initial release.
*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

/* AIM104-RELAY8 */
#define	BASE	0x300
#define	ENABLE	0x301

/* Global variables */
struct termios tio;
long int timeout;
int verbose=0;		
int hardware_control=0;
int fixture=1;

/* Functions */
void *stop_test (void *unused)
{
	usleep (timeout);
	exit(0);
	return NULL;
}

int setRTS(int fd, int level)
{
    int status;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror("setRTS(): TIOCMGET");
        return 0;
    }
    
	if (level)
        status |= TIOCM_RTS;
    else
        status &= ~TIOCM_RTS;
    
	if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("setRTS(): TIOCMSET");
        return 0;
    }
    return 1;
}

int setDTR(int fd, int level)
{
    int status;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror("setDTR(): TIOCMGET");
        return 0;
    }
    
	if (level)
        status |= TIOCM_DTR;
    else
        status &= ~TIOCM_DTR;
	
	if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("setDTR(): TIOCMSET");
        return 0;
    }
    return 1;
}

// RTS is inverted to ensure that UUT detects a short
int getCTS (int fd)
{
    int status;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror("getCTS(): TIOCMGET");
        return 0;
    }
	if (status & TIOCM_CTS)	{
		setRTS(fd, 0);
	}
	else {
		setRTS(fd, 1);
	}
	return 1;
}

// DTR is inverted to ensure that UUT detects a short
int getDCD (int fd)
{
    int status;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror("getCTS(): TIOCMGET");
        return 0;
    }
	if (status & TIOCM_CD)	{
		setDTR(fd, 0);
	}
	else {
		setDTR(fd, 1);
	}
	return 1;
}

// alternative printf
int WriteFormat (char *fmt, ...)
{
	va_list argptr;
	int cnt;

	if (verbose)
	{
		va_start(argptr, fmt);
		cnt = vprintf(fmt, argptr);
		va_end(argptr);
	}
	return cnt;
}

// start
int main(int argc, char *argv[])
{
	int fd, result, n;
	long baud;
	char *buffer;
	char *store;
	int start = 0;
	int retry = 0;
	int rts = 0;
	int delay = 0;
	long temp = 0;
	long temp2 = 0;
	int usb_delay = 0;
	static time_t t1, t2;

	for (n=2; n < argc; n++)
	{
		if (strstr(argv[n], "rs485") != NULL)
		{
			rts=1;
		}
		if (strstr(argv[n], "-fixture"))
		{
			// Eurotech ltd test fixture (Aludra/AIM104-COM4/AIM104-RELAY8)
			fixture=1;
		}
		if (strstr(argv[n], "-hfcnt"))
		{
			hardware_control=1;
		}
		if (strstr(argv[n], "-v"))
		{
			verbose=1;
		}
	}

	buffer = (char *)malloc(2048);
	if (buffer == 0)
	{
		printf("\n\n ERROR : failed to allocate buffer : buffer");
		exit (1);
	}
	store = (char *)malloc(2048);
	if (store == 0)
	{
		printf("\n\n ERROR : failed to allocate buffer : buffer");
		exit (1);
	}

	if ( argc < 3 )
	{
		printf("\n\nUsage: qs_rx [port] [ default speed] [-rs485] [-v]\n");
		printf("       port        Port you want to use, e.g. /dev/ttyS4\n");
		printf("       baud        Initial transmission speed (1200, 2400, 4800, 9600, 19200, 38400 or 115200\n");
		printf("       -rs485      Optional - RTS control for RS485\n");
		printf("       -fixture    Optional - Fixture based test\n");
		printf("       -hfcnt      Optional - Hardware flow control\n");
		printf("       -v          Optional - Verbose\n"); 
		exit( 1 );
	}

	if (fixture)
	{
		if (ioperm (BASE, 2, 1))
		{
			printf("\n Error assigning IO port 0x300");
			exit (1);
		}
	}

	if (!strcmp("300", argv[2]))
	{
		baud = B300;
	}
	if (!strcmp("600", argv[2]))
	{
		baud = B600;
	}
	else if (!strcmp("1200", argv[2]))
	{
		baud = B1200;
	}
	else if (!strcmp("2400", argv[2]))
	{
		baud = B2400;
	}
	else if (!strcmp("4800", argv[2]))
	{
		baud = B4800;
	}
	else if (!strcmp("9600", argv[2]))
	{
		baud = B9600;
	}
	else if (!strcmp("19200", argv[2]))
	{
		baud = B19200;
	}
	else if (!strcmp("38400", argv[2]))
	{
		baud = B38400;
	}
	else if (!strcmp("57600", argv[2]))
	{
		baud = B57600;
	}
	else if (!strcmp("115200", argv[2]))
	{
		baud = B115200;
	}
	else if (!strcmp("230400", argv[2]))
	{
		baud = B230400;
	}
	else
	{
		printf("Baud rate %s is not supported, ", argv[2]);
		printf("use 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200.\n");
		exit(1);
	}

	/* pthread example */
	//if(argc == 6 && !strcmp(argv[5], "-hfcnt"))
	//{
	//	pthread_create (&thread_id, NULL, &stop_test, NULL);
	//}
	
	if (rts)
	{
		WriteFormat("\n RTS control set");
	}

	/* open the serial port device file
	* O_NDELAY - tells port to operate and ignore the DCD line
	* O_NOCTTY - this process is not to become the controlling 
	*            process for the port. The driver will not send
	*            this process signals due to keyboard aborts, etc.
	*/
	//if ((fd = open(argv[1],O_RDWR | O_NDELAY | O_NOCTTY)) < 0)
	if ((fd = open(argv[1], O_RDWR | O_NONBLOCK )) < 0)
	{
		printf("Couldn't open %s\n",argv[1]);
		exit(1);
	}

	/* at the moment 
	* FTDI usb devices need delay after write to ensure write completes 
	*/
    if (strstr(argv[1], "USB"))
	{
		WriteFormat("\n USB DELAY enabled");
		usb_delay=1;	
	}

	fcntl(fd, F_SETFL, 0);
	/* we are not concerned about preserving the old serial port configuration
	* CS8, 8 data bits
	* CREAD, receiver enabled
	* CLOCAL, don't change the port's owner 
	*/
	//tio.c_cflag = baud | CLOCAL | CS8 | CREAD ;
	tio.c_cflag = baud | CS8 | CREAD;

	if (hardware_control)
	{
		printf("\n Hardware control");
		tio.c_cflag |= HUPCL;
		tio.c_cflag |= CRTSCTS;
	}
	else
	{
		tio.c_cflag |= CLOCAL;
	}

	/* clear the CLOCAL bit so modem status does not get ignored */
	//tio.c_cflag &= ~CLOCAL; 

	//tio.c_cflag &= ~PARENB;
	tio.c_lflag = 0;       /* set input flag non-canonical, no processing */
	tio.c_iflag = IGNPAR;  /* ignore parity errors */
	tio.c_oflag = 0;       /* set output flag non-canonical, no processing */
	tio.c_cc[VTIME] = 0;   /*  3 no time delay */
	tio.c_cc[VMIN]  = 0;   /* no char delay */
	tcflush(fd, TCIFLUSH); /* flush the buffer */
	tcsetattr(fd, TCSANOW, &tio); /* set the attributes */

	/* Set up for no delay, ie non-blocking reads will occur. 
	When we read, we'll get what's in the input buffer or nothing */
	//fcntl(fd, F_SETFL, FNDELAY);
	fcntl(fd, F_SETFL, 0);

	while (1)
	{
		WriteFormat("\n\n PORT %s", argv[1]); 
		WriteFormat("\n Wait...\n");
		
		if (rts && !hardware_control)	{
			WriteFormat("\n De-assert RTS");
			setRTS(fd, 0);
			usleep(1000*delay*1);
		}
		memset(buffer, 0, 2048);
		memset(store, 0, 2048);

		/* read the input buffer*/
		result=0;							
		start=1;
		retry=5;
		WriteFormat("\n Read data ...");
		while (start || result || retry)	{
			
			/* handshaking */
			/* read CTS and set RTS level, read CTS at master */
			/* read DCD and set DTR level, read DCD, DSR, RI at master */
			if (!rts && !hardware_control) {
				getCTS(fd);
				getDCD(fd);
			}
			
			result = read(fd, store, 1024);
			store[result] = 0;		// zero terminate so printf works
			if (result)
			{
				start=0;
				strcat(buffer, store); 
			}
			if (!start && !result)	// if transmission started and no char
			{
				retry-=1;
			}
			usleep(50000);			//Minimum of one char 1200 baud, 1000000
		}

		if (rts && !hardware_control)	{
			WriteFormat("\n Assert RTS");
			setRTS(fd, 1);
			usleep(1000*delay*1);
		}
		
		printf("\n Rx : size %d", strlen(buffer));
		
		// if receive data, write data 
		if (strlen(buffer) > 0)
		{
			printf("\n Write data ...");
			t1=time(0);
			result = write(fd, buffer, strlen(buffer));
			tcdrain(fd);
			t2=time(0);
		}

		// FTDI USB devices need to delay to ensure write completes
		if ( usb_delay == 1 )
		{
			temp=1000L*(long)delay*(long)result;
			temp2=(long)(t2-t1) * 1000000L;
			usleep (temp-temp2);
		}

		WriteFormat("\n Tx : size %d", result); 
		if (result < 0)
		{
			fputs("write failed\n", stderr);
			close(fd);
			exit(1);
		}

		// relay control commands
		// set relay COM2/ttyS1 RS485
		if (strstr(buffer, "CMDRS485"))
		{
			WriteFormat("\n %s", buffer);
			if (fixture)
			{
				outb(0x33, BASE);
				outb(0x01, BASE+1);
			}
		}

		// set relay COM2/ttyS1 RS422
		if (strstr(buffer, "CMDRS422"))
		{
			WriteFormat("\n %s", buffer);
			if (fixture)
			{
				outb(0xCF, BASE);
				outb(0x01, BASE+1);
			}
		}

		// set relay COM2/ttyS1 RS232
		if (strstr(buffer, "CMDRS232"))
		{
			WriteFormat("\n %s", buffer);
			if (fixture)
			{
				outb(0x00, BASE);
				outb(0x01, BASE+1);
			}
		}

		// baud commands		
		// set baud back to default
		if (baud != B115200 || strstr(buffer, "BAUD115200")) 
		{
			WriteFormat("\n %s", "BAUD115200");
			baud = B115200;
			delay=1;
		}
		if (strstr(buffer, "BAUD300"))
		{
			WriteFormat("\n %s", buffer);
			baud = B300;
			delay=36;
		}
		if (strstr(buffer, "BAUD600"))
		{
			WriteFormat("\n %s", buffer);
			baud = B600;
			delay=20;
		}
		if (strstr(buffer, "BAUD1200"))
		{
			WriteFormat("\n %s", buffer);
			baud = B1200;
			delay=12;
		}
		if (strstr(buffer, "BAUD2400"))
		{
			WriteFormat("\n %s", buffer);
			baud = B2400;
			delay=8;
		}
		if (strstr(buffer, "BAUD4800"))
		{
			WriteFormat("\n %s", buffer);
			baud = B4800;
			delay=6;
		}
		if (strstr(buffer, "BAUD9600"))
		{
			WriteFormat("\n %s", buffer);
			baud = B9600;
			delay=4;
		}
		if (strstr(buffer, "BAUD19200"))
		{
			WriteFormat("\n %s", buffer);
			baud = B19200;
			delay=2;
		}
		if (strstr(buffer, "BAUD38400"))
		{
			WriteFormat("\n %s", buffer);
			baud = B38400;
			delay=1;
		}
		if (strstr(buffer, "BAUD57600"))
		{
			WriteFormat("\n %s", buffer);
			baud = B57600;
			delay=1;
		}
		if (strstr(buffer, "BAUD230400"))
		{
			WriteFormat("\n BAUD 230400 assigned");
			WriteFormat("\n %s", buffer);
			baud = B230400;
			delay=1;
		}
		tio.c_cflag = baud | CLOCAL | CS8 | CREAD ;
		tcflush(fd, TCIFLUSH); /* flush the buffer */
		tcsetattr(fd, TCSANOW, &tio); /* set the attributes */
	}
	/* close the device file */
	close(fd);
	printf("\n");
}

//EOF
