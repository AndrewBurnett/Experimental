/*
 * serial_hs.c
 * 
 * Simple serial handshake test
 * Expects serial_server to be running on remote unit. The CTS, DSR,DCD,RI returns are inverted
 * to catch shorts, so a loopback will not work.  
 *
 * gcc serial_hs.c -o serial_hs -Wall
 *
 * Version
 * 1.1 AB 21/11/14
 * - Created
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


/* Global variables */
struct termios tio;
long int timeout;
int verbose=0;

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
		perror(" Unable set serial signal : setRTS(): TIOCMGET");
        exit(1);
    }
    if (level)
        status |= TIOCM_RTS;
    else
        status &= ~TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror(" Unable to set serial signal : setRTS(): TIOCMSET");
        exit(1);
    }
    return 0;
}

int setDTR(int fd, int level)
{
    int status;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror(" Unable to set serial signal : setDTR(): TIOCMGET");
        exit(1);
    }
    if (level)
        status |= TIOCM_DTR;
    else
        status &= ~TIOCM_DTR;
	if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror(" Unable to set serial signal : setDTR(): TIOCMSET");
		exit(1);
    }
    return 0;
}

int getCTS (int fd)
{
    int status;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror(" Unable to get serial status : getCTS(): TIOCMGET");
        exit (1);
    }
	if (status & TIOCM_CTS)	{
		if (verbose)
			printf(" CTS 1");
		return 1;
	}
	else {
		if (verbose)
			printf(" CTS 0");
		return 0;
	}
	return 2;
}

int getDCD (int fd)
{
    int status;

    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror(" Unable to get serial status : getDCD(): TIOCMGET");
        exit (1);
    }
	if (status & TIOCM_CD)	{
		if (verbose)
			printf(" DCD 1");
		return 1;
	}
	else {
		if (verbose)
			printf(" DCD 0");
		return 0;
	}
	return 2;
}

int getRI (int fd)
{
    int status;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror(" Unable to get serial status : getRNG(): TIOCMGET");
        exit (1);
    }
	if (status & TIOCM_RNG)	{
		if (verbose)
			printf(" RI 1");
		return 1;
	}
	else {
		if (verbose)
			printf(" RI 0");
		return 0;
	}
	return 2;
}

int getDSR (int fd)
{
    int status;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror(" Unable to get serial status : getDSR(): TIOCMGET");
        exit (1);
    }
	if (status & TIOCM_DSR)	{
		if (verbose)
			printf(" DSR 1");
		return 1;
	}
	else {
		if (verbose)
			printf(" DSR 0");
		return 0;
	}
	return 2;
}

int main(int argc, char *argv[])
{
	int fd;
	long baud;

	if ( argc != 3 && argc != 4)
	{
		printf("\n usage : qs_hs [port] [handshake control line RTS|CTS]  [-v verbose]");
		printf("\n example : qs_rx /dev/ttyS0 RTS");
		exit (1);
	}

	if (argc == 4 && strstr(argv[3], "-v"))
	{
		verbose=1;
	}
	
	baud = B115200;

	/* open the serial port device file
	* O_NDELAY - tells port to operate and ignore the DCD line
	* O_NOCTTY - this process is not to become the controlling 
	*            process for the port. The driver will not send
	*            this process signals due to keyboard aborts, etc.
	*/
	if ((fd = open(argv[1],O_RDWR | O_NDELAY | O_NOCTTY)) < 0)
	{
		printf("Couldn't open %s\n",argv[1]);
		exit(1);
	}

	fcntl(fd, F_SETFL, 0);
	/* we are not concerned about preserving the old serial port configuration
	* CS8, 8 data bits
	* CREAD, receiver enabled
	* CLOCAL, don't change the port's owner 
	*/
	tio.c_cflag = baud | CLOCAL | CS8 | CREAD ;

	tio.c_cflag |= CRTSCTS;

	/* clear the CLOCAL bit so modem status does not get ignored */
	//tio.c_cflag &= ~CLOCAL; 

	tio.c_cflag &= ~PARENB;
	tio.c_lflag = 0;       /* set input flag non-canonical, no processing */
	tio.c_iflag = IGNPAR;  /* ignore parity errors */
	tio.c_oflag = 0;       /* set output flag non-canonical, no processing */
	tio.c_cc[VTIME] = 0;   /* no time delay */
	tio.c_cc[VMIN]  = 0;   /* no char delay */
	tcflush(fd, TCIFLUSH); /* flush the buffer */
	tcsetattr(fd, TCSANOW, &tio); /* set the attributes */


	if (!strcmp(argv[2], "RTS"))
	{
		if (verbose)
			printf("\n RTS 0 ->");
		setRTS(fd, 0);	
		sleep(1);
		if (getCTS(fd) != 1) {
			return 2;
		}
		
		if (verbose)
			printf("\n RTS 1 ->");
		setRTS(fd, 1);
		sleep(1);
		if (getCTS(fd) != 0) {
			return 2;
		}
	}
	
	if (!strcmp(argv[2], "DTR"))
	{
		if (verbose)
			printf("\n DTR 0 ->");
		setDTR(fd, 0);
		sleep(1);
		if (getDCD(fd) != 1) {
			return 3;
		}
		if (getDSR(fd) != 1) {
			return 4;
		}
		if (getRI(fd) != 1) {
			return 5;
		}

		if (verbose)
			printf("\n DTR 1 ->");
		setDTR(fd, 1);
		sleep(1);
		if (getDCD(fd) != 0) {
			return 3;
		}
		if (getDSR(fd) != 0) {
			return 4;
		}
		if (getRI(fd) != 0) {
			return 5;
		}
	}
	printf("\n");
	return 0;
}

//EOF
