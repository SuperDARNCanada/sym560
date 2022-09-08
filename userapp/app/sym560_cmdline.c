/* File : 	sym560_cmdline.c
 * Author: 	Bradley Krug
 * Modified:	Feb 21 (added some comments)
 * Description:	This is the main application created for manipulating the Symmetricom
 *		560-5908-U (GPS-PCI-2U) device.  This program uses functions defined in 
 *		the file sym560_functions.h.  It has two main operating modes: manual and
 *		automatic.  In manual mode, the user can look up information such as time
 *		position, and antenna or satellite status.  Manual mode also contains two 
 *		sub modes one for timestamping events and the other for handling the 
 *		generator output (to get a 10 MHz output signal for instance).  The second
 *		mode, automatic mode, is used to timestamp events without requiring any user
 *		input.  To run in automatic mode, supply the argument "auto".  To stop auto 
 *		mode, the script stopstamp.bash can be used.  Both the automated and manual
 *		timestamping functions make use of the event_cap process which should be 
 *		compiled alongside this application.
 */

#include "sym560_functions.h"

int main(int argc, char **argv)
{
	char ch[30];
	int fd, ret;
	char filename[32];	
	/* Get date and time for the filename */
	time_t rawtime;
	struct tm *ptm;
	time(&rawtime);
	ptm = gmtime(&rawtime);
	sprintf(filename, "%04d%02d%02d.%02d%02d.timestampdata",
			ptm->tm_year+1900,
			ptm->tm_mon+1,
			ptm->tm_mday,
			ptm->tm_hour,
			ptm->tm_min);
	
	/* open the device for read and write */
	fd = open("/dev/symgps0", O_RDWR);
	if(fd < 0) {
		printf("\nDevice not found.  Make sure driver has been loaded ( systemctl start sym560.service )\n");
		exit(1);
	}
	
	/* check command line arguments and if the first one is "auto" then call the auto function */
	if ((argc > 1) && (strcmp(argv[1],"auto") == 0)) {
		autostamp(fd, filename);
		close(fd);
		/* Do not call findpulse.pl automatically here, do it in cron or something */
		/* execl("/usr/bin/findpulse.pl", "findpulse", "auto", filename, NULL); */
		/* After execl is called, nothing below it is executed, since it replaces this 
		 * process image with the new process image */	
		return(0);
	}

	/* ask if GPS should be initialized */
	for(;;) {
		printf("\nInitialize GPS (y/n)?: ");
		scanf( "%1s", ch);
		jsw_flush();
		if (strcasecmp(ch, "y") == 0) {
			GPS_init(fd);
			break;
		}
		else if(strcasecmp(ch, "n") == 0) {
			break;
		}
		else {
			printf("\nPlease choose 'y' or 'n'\n");
		}
	}
	
	print_menu();
	
	/* infinite loop to get user input for main menu, exits on 'q' or 'quit' */
	for (;;) {
		/* %25 limits input received to 25 chars, jsw_flush removes extra input */
		printf("\nEnter Command: ");
		scanf( "%25s", ch);
		jsw_flush();
		
		/* check for valid input strings */
		if ((strcasecmp(ch, "help") == 0) || (strcasecmp(ch, "h") == 0)) {
			print_menu();
		}
		else if ((strcasecmp(ch, "quit") == 0) || (strcasecmp(ch, "q") == 0)) {
			close(fd);
			printf("\nGoodbye\n\n");
			return 0;
		}
		else if ((strcasecmp(ch, "antenna") == 0) || (strcasecmp(ch, "a") == 0)) {
			check_antenna(fd);
		}
		else if ((strcasecmp(ch, "init") == 0) || (strcasecmp(ch, "i") == 0)) {
			GPS_init(fd);
		}
		else if ((strcasecmp(ch, "event") == 0) || (strcasecmp(ch, "e") == 0)) {
			ret = event_capture_menu(fd);
			if (ret != 0) {
				printf("\nGoodbye\n\n");
				return 0;
			}
			print_menu();
		}
		else if ((strcasecmp(ch, "generator") == 0) || (strcasecmp(ch, "g") == 0)) {
			ret = rategen_menu(fd);
			if (ret != 0) {
				printf("\nGoodbye\n\n");
				return 0;
			}
			print_menu();
		}
		else if ((strcasecmp(ch, "position") == 0) || (strcasecmp(ch, "p") == 0)) {
			fetch_position(fd);
		}
		else if ((strcasecmp(ch, "signal") == 0) || (strcasecmp(ch, "s") == 0)) {
			satsig(fd);
		}
		else if ((strcasecmp(ch, "time") == 0) || (strcasecmp(ch, "t") == 0)) {
			fetch_time(fd);
		}
		else {
			printf("Command '%s' is not allowed\n", ch);
			printf("Enter 'h' for help\n");
		}
	}	
}
/* end of main */
