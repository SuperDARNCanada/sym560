/* File : 	event_cap.c
 * Author: 	Bradley Krug
 * Modified:	Feb 21 (added some comments)
 * Description:	
 * This program is spawned by the cmdline_interface program and should not be executed manually 
 * It requires two input parameters 
 *	1 - the device file descriptor
 *      2 - the output file descriptor
 * Both of these file descriptors are opened within the cmdline_interface program, which also
 * initializes the GPS PCI device to accept event interrupts.
 * This program then enters an infinite loop calling the IO command 8008f800 which 
 * sleeps until an event occurs and passes the event data back in the user_buff.
 * The data is then written to a temporary file in binary format.
 * Once the cmdline_interface program sends the kill signal, this program terminates and then the 
 * cmdline_interface program takes care of rewritting the contents into a plain text format as
 * as well as closes the outputfile, and disables the interrupts.
 */ 
#include <stdio.h>
#include "sym560_functions.h"


int main(int argc, char **argv){
	int ret;
	int devfd; 	/*file descriptor passed as argument from parent process*/
	int outfd;	/*file descriptor for output txt file used to store raw data*/
	unsigned char user_buff[12];

	/* check arguments */
	if (argc != 3) {
		printf("Error event_cap should not be run manually.\n");
		printf("Its called automatically by the sym560_cmdline application.\n\n");
		return -1;
	}

	/* scan the arguments(both file descriptors) in as ints */
	sscanf(argv[1], "%d", &devfd);
	sscanf(argv[2], "%d", &outfd);
	
	/* device IO function that enables PCI card interrupts*/
	ioctl(devfd, 0x0000f803);
	
	/* enable event driven interrupts and clear event status bit */
	user_buff[0] = 0x09;
	write_pci(devfd, REG_HARD_CTRL, user_buff, 1);	
	
	/* infinite loop until kill signal is received by parent process */
	for (;;) {
		/* call the event capture device io function */
		ioctl(devfd, 0x8008f800, user_buff);
	//	printf("ioctl\n");
		/* write the raw data to an output file */
		write(outfd, user_buff, 12);
	}

	return 0;
}
