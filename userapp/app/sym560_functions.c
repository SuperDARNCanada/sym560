/* File : 	sym560_functions.c
 * Author: 	Bradley Krug
 * Modified:	Feb 21 (added some comments)
 * Description:	This library contains the functions used by the sym560_cmdline application.
 *		These functions can be modified or used along with the information in Chapter 3 
 *		of the Symmetricom 560-5908-U user manual to create applications for the Symmetricom
 *		GPS device.
 */

#include "sym560_functions.h"

/* MACRO Definitions */
#define GREENTEXT(text) printf("\033[22;32m%s\033[22;30m",text)
#define REDTEXT(text)	printf("\033[01;31m%s\033[22;30m",text)
#define BLUETEXT(text)	printf("\033[22;34m%s\033[22;30m",text)
#define STARTGREEN()	printf("\033[22;32m");
#define STARTRED()	printf("\033[01;31m");
#define STARTBLUE()	printf("\033[22;34m");
#define STARTBLACK()	printf("\033[22;30m");

/*******************************************************************************/
/* Function   : print_menu
 * Inputs     : None
 * Returns    : Nothing
 * Description: Prints the main menu at startup and whenever "help" or "h" is pressed
 */
int print_menu() {
	printf("\n\n\n\n\n");
	printf("*********************************************\n");
	printf("Main Command Menu:\n\n");
	printf("    antenna, a : View antenna status\n");
	printf("     signal, s : View satellite signal strength\n");
	printf("   position, p : View antenna position\n");
	printf("       time, t : View onboard time\n");
	printf("       init, i : Initialize GPS synchronized generator\n\n");
	printf("      event, e : Event capture mode\n");
	printf("  generator, g : Rate generator mode\n\n");
	printf("       help, h : Prints this menu\n");
	printf("       quit, q : Quit program\n");
	printf("*********************************************\n\n");
}
/* end of function: print_menu */
/*******************************************************************************/


/*******************************************************************************/
/* Function   : print_event_menu
 * Inputs     : None
 * Returns    : Nothing
 * Description: Prints the event menu when "event" or "e" is pressed from the main menu
 *              or when "help" or "h" is pressed while in event capture mode.
 */
int print_event_menu() {
	printf("\n\n\n\n\n");
	printf("*********************************************\n");
	printf("Event Capture Command Menu:\n\n");
	printf("      setup, v : View event setup\n");
	printf("     source, o : Choose event source\n");
	printf("      start, s : Start event capture\n");
	printf("       data, d : View timestamp data\n\n");
	printf("       help, h : Prints this menu\n");
	printf("       back, b : Back to main menu\n");
	printf("       quit, q : Quit program\n");
	printf("*********************************************\n\n");
}
/* end of function: print_event_menu */
/*******************************************************************************/


/*******************************************************************************/
/* Function   : print_rategen_menu
 * Inputs     : None
 * Returns    : Nothing
 * Description: Prints the rate generator menu when "generator" or "g" is pressed 
 *		from the main menu or when "help" or "h" is pressed while in 
 *		rate generator mode.
 */
int print_rategen_menu() {
	printf("\n\n\n\n\n");
	printf("*********************************************\n");
	printf("Rate Generator Command Menu:\n\n");
	printf("      setup, v : View generator setup\n");
	printf("       rate, r : Choose rate\n");
	printf("     enable, e : Enable Rate Generator\n\n");
	printf("       help, h : Prints this menu\n");
	printf("       back, b : Back to main menu\n");
	printf("       quit, q : Quit program\n");
	printf("*********************************************\n\n");
}
/* end of function: print_rategen_menu */
/*******************************************************************************/


/*******************************************************************************/
/* Function   : read_pci
 * Inputs     : int fd - file descriptor for the GPS-PCI device
 *              off_t regoff - location to read from
 *              char *user_buff - user buffer that will contain the read data
                int nbytes - Number of bytes to read
 * Returns    : 0 on Success
 *             -1 on Failure
 * Description: Reads data from the GPS-PCI device into a user buffer by first setting
 *              the position with lseek and then reading.
 */
int read_pci(int fd, off_t regoff, char *user_buff, int nbytes) {
	int ret;
	
	/* set position to read from */
	ret = lseek(fd, regoff, SEEK_SET);
	if (ret == -1) {
		return -1;
	}
	
	/* read from device */
	ret = read(fd, user_buff, nbytes);
	if (ret == -1) {
		return -1;
	}
	
	return 0;
}
/* end of function: read_pci */
/*******************************************************************************/


/*******************************************************************************/
/* Function   : read_pci_verbose
 * Inputs     : int fd - file descriptor for the GPS-PCI device
 *              off_t regoff - location to read from
 *              char *user_buff - user buffer that will contain the read data
                int nbytes - Number of bytes to read
 * Returns    : 0 on Success
 *             -1 on Failure
 * Description: Reads data by calling the read_pci function (defined above) and then
 * 		prints the read data in both hex and binary format.
 */
int read_pci_verbose(int fd, off_t regoff, char *user_buff, int nbytes) {
	int ret, cnt;
	unsigned char binbuff[10];
	ret = read_pci(fd, regoff, user_buff, nbytes);
	if (ret == -1) {
		printf("\n\nError: Read failed\n");
		return -1;
	}
	
	/* print out contents of buffer in HEX and BINARY */
	printf("\n\nReading %d byte(s) starting at offset 0x%x:\n ", nbytes, regoff);
	for (cnt = 0; cnt < nbytes; cnt++) {
		char2bin(&user_buff[cnt], binbuff, 1);
		printf("  Byte %d = %x (%s)",cnt+1, user_buff[cnt], binbuff);
		printf("\n");
	}
	
	return 0;
}
/* end of function: read_pci_verbose */
/*******************************************************************************/


/*******************************************************************************/
/* Function   : write_pci
 * Inputs     : int fd - file descriptor for the GPS-PCI device
 *              off_t regoff - location to write to
 *              char *user_buff - user buffer containing the data to be written
                int nbytes - Number of bytes to write
 * Returns    : 0 on Success
 *             -1 on Failure
 * Description: Writes data to the GPS-PCI device from a user buffer by first setting
 *              the position and then writing.
 */
int write_pci(int fd, off_t regoff, char *user_buff, int nbytes) {
	int ret;
	
	/* set position to write to */
	ret = lseek(fd, regoff, SEEK_SET);
	if (ret == -1) {
		return -1;
	}
	
	/* write to device */
	ret = write(fd, user_buff, nbytes);
	if (ret == -1) {
		return -1;
	}
	
	return 0;
}
/* end of function: write_pci */
/*******************************************************************************/


/*******************************************************************************/
/* Function   : write_pci_verbose
 * Inputs     : int fd - file descriptor for the GPS-PCI device
 *              off_t regoff - location to write to
 *              char *user_buff - user buffer containing the data to be written
                int nbytes - Number of bytes to write
 * Returns    : 0 on Success
 *             -1 on Failure
 * Description: Writes data to the GPS-PCI device from a user buffer by first setting
 *              the position and then writing. Verbose for debugging purposes.
 */
int write_pci_verbose(int fd, off_t regoff, char *user_buff, int nbytes) {
	int ret, cnt;
	unsigned char binbuff[10];
	
	ret = write_pci(fd, regoff, user_buff, nbytes);
	if (ret == -1) {
		printf("\n\nERROR: Write failed\n");
		return -1;
	}
	
	/* print out contents of buffer in HEX and BINARY */
	printf("\n\nWrote %d byte(s) starting at offset 0x%x:\n ", nbytes, regoff);
	for (cnt = 0; cnt < nbytes; cnt++) {
		char2bin(&user_buff[cnt], binbuff, 1);
		printf("  Byte %d = %x (%s)",cnt+1, user_buff[cnt], binbuff);
		printf("\n");
	}
	
	return 0;
}
/* end of function: write_pci_verbose */
/*******************************************************************************/


/*******************************************************************************/
/* Function   : GPS_init
 * Inputs     : int fd - file descriptor for the GPS-PCI device
 * Returns    : 0 on Success
 *             -1 if GPS was not properly initialized
 * Description: Checks antenna for any shorts or open loads and then sets the PCI 
 *		card to run in synchronized generator mode.  Then checks in 20 sec
 *		intervals for a certain number of times to acquire a gps lock.
 */
int GPS_init(int fd) {
	int ret, cnt, gpslock;
	int maxtime = 15;
	unsigned char binbuff[10];
	char user_buff[4], tmp;
	
	/* Check the antenna status */
	printf("\n\nChecking GPS Antenna...\n");
	ret = read_pci(fd, REG_HARD_STATUS, user_buff, 1);
	user_buff[0] = user_buff[0] & 0x30;
	if (user_buff[0] != 0x30) {
		printf("WARNING");
		if (user_buff[0] == 0x10) {
			printf(" GPS Antenna is SHORTED\n");
		}
		if (user_buff[0] == 0x20) {
			printf(" GPS Antenna is OPEN\n");
		}
		if (user_buff[0] == 0x00) {
			printf(" GPS Antenna is shown as being both OPEN and SHORTED\n");
			printf("This has been known to happen if the antenna connection at the\n");
			printf("PCI card is removed and then reconnected without a power cycle\n");
		}
	}
	else {
		//printf("SUCCESS:");
		printf("SUCCESS:");
		printf(" No shorts or open loads detected\n");
	}
	
	/* start synchronized generator and use gps reference */
	printf("\nStarting synchronized generator with GPS reference\n");
	user_buff[0] = 0x21;
	ret = write_pci(fd, REG_CONFIG1_TSC, user_buff, 1);
	if (ret == -1) {
		return -1;
	}
	
	/* Wait until lock status bits are set */
	printf("\nChecking GPS Signal Status...\n");
	printf("This may take a while\n");
	gpslock = 0;
	for (cnt = 0; cnt < maxtime; cnt++) {
		ret = read_pci(fd, REG_SOFTTIME_LOCK, user_buff, 1);
		
		char2bin(user_buff, binbuff, 1);
		printf("\n  Contents of Software time lock register = ");
		//STARTBLUE();
		printf("%s", binbuff);
		//STARTBLACK();
		printf("\n");
		
		/* check status bits*/
		tmp = user_buff[0] & 0x70;
		if (tmp == 0x70) {
			cnt = maxtime;
			gpslock = 1;
		}
		else {
			sleep(20);
		}
	}
	
	if (gpslock == 1) {
		//printf("\nSUCCESS: ");
		printf("Input signal is valid\n");
		//printf("\nSUCCESS: ");
		printf("Phase locked to input ref\n");
		//printf("\nSUCCESS: ");
		printf("GPS has been locked\n");
		
		return 0;
	}
	
	/* print out signal lock status */
	tmp = user_buff[0] & 0x20;
	if (tmp == 0x20) {
		//printf("\nSUCCESS: ");
		printf("Input signal is valid\n");
	}
	else {
		printf("\nWARNING: ");
		printf("Input signal is invalid\n");
	}
	tmp = user_buff[0] & 0x40;
	if (tmp == 0x40) {
		//printf("\nSUCCESS: ");
		printf("Phase locked to input ref\n");
	}
	else {
		printf("\nWARNING: ");
		printf("Phase is NOT LOCKED\n");
	}
	tmp = user_buff[0] & 0x10;
	if (tmp == 0x10) {
		//printf("\nSUCCESS: ");
		printf("GPS has been locked\n");
	}
	else {
		printf("\nWARNING: ");
		printf("GPS is NOT LOCKED\n");
	}
	return -1;
}
/*  end of function: GPS_init */
/*******************************************************************************/


/*******************************************************************************
 * Function   : satsig
 * Inputs     : int fd - device file descriptor.
 * Returns    : 0 on success
 *         	1 if signal is being updated
 * Description: Checks and prints out the satellite signal strength for the six
 *              satellites that are locked.  If the GPS is not hooked up or the 
 *              satellite lock has not been achieved, then the strength will be
 *              0.
 */
int satsig(int fd) {
	unsigned char user_buff[4];
	unsigned char svnum_tens, svnum_unit, sig_tens, sig_unit, sig_tenths, sig_hunds;
	int cnt, ret, offset;

	printf("\n\n\n");
	
	/* Check satelite update status */
	ret = read_pci(fd, REG_SATSTAT, user_buff, 1);
	if (user_buff[0] != 0) {
		printf("Satellite signal status is being updated, try again\n");
		return 1;
	}
	
	printf("Satellite Signals (6 can be tracked at one time)\n\n");
	
	/* loop through all six satellite locks */
	for (cnt = 0; cnt < 6; cnt++) {
		offset = REG_SATSIG_SATA + (cnt*4);
		ret = read_pci(fd, offset, user_buff, 4);
		
		/* store proper data into variables */
		svnum_tens = user_buff[0] >> 4;
		svnum_unit = user_buff[0] & 0x0F;
		sig_tenths = user_buff[2] >> 4;
		sig_hunds = user_buff[2] & 0x0F;
		sig_tens = user_buff[3] >> 4;
		sig_unit = user_buff[3] & 0x0F;
		
		/* print satellite info */
		printf("  SV Number    = %d%d \n", svnum_tens, svnum_unit);
		printf("  Signal Level = ");
		if ((sig_tens >= 1) || (sig_unit >= 5)) {
			//STARTGREEN();
;
		}
		else {
			//STARTRED();
;
		}
		printf("%d%d.%d%d \n\n", sig_tens, sig_unit, sig_tenths, sig_hunds);
		//STARTBLACK();
		printf("\n");
		
		/* Check satelite update status */
		ret = read_pci(fd, REG_SATSTAT, user_buff, 1);
		if (user_buff[0] != 0) {
			printf("(data was being updated, try again)\n");
			return 1;
		}
	}
	return 0;
}
/* end of function: sat_sig */
/*******************************************************************************/


/*******************************************************************************
 * Function   : fetch_position
 * Inputs     : int fd - device file descriptor.
 * Returns    : 0 on success
 *             -1 on failure
 * Description: Read longitude, latitude and altitude from device and print it out
 */
int fetch_position(int fd) {
	int ret, cnt, good_data;
	/* buffers being used to fetch the data */
	unsigned char quad_1[2][4];
	unsigned char quad_2[2][4];
	unsigned char quad_3[2][4];
	unsigned char quad_4[2][4];
	/* variables to hold the latitude/longitude info */
	unsigned char lat_unit_deg, lat_tens_deg, lat_hund_deg;
	unsigned char lat_unit_min, lat_tens_min;
	unsigned char lat_NorS, lon_EorW;
	unsigned char lat_tenths_sec, lat_unit_sec, lat_tens_sec;
	unsigned char lon_unit_deg, lon_tens_deg, lon_hund_deg;
	unsigned char lon_unit_min, lon_tens_min;
	unsigned char lon_tenths_sec, lon_unit_sec, lon_tens_sec;
	/* variables to hold the altitude info */
	unsigned char alt_unit_km, alt_tens_km, alt_sign;
	unsigned char alt_tenths_m, alt_unit_m, alt_tens_m, alt_hund_m;
	
	/* read the position register twice (as recommended pg 30 of manual)*/
	for (cnt = 0; cnt < 2; cnt++) {
		ret = read_pci(fd, REG_ANT_POSITION, &quad_1[cnt][0], 4);
		if (ret == -1) {
			printf("\nFailed to read time capture register\n");
			return -1;
		}
		ret = read_pci(fd, REG_ANT_POSITION + 4, &quad_2[cnt][0], 4);
		if (ret == -1) {
			printf("\nFailed to read time capture register\n");
			return -1;
		}
		ret = read_pci(fd, REG_ANT_POSITION + 8, &quad_3[cnt][0], 4);
		if (ret == -1) {
			printf("\nFailed to read time capture register\n");
			return -1;
		}
		ret = read_pci(fd, REG_ANT_POSITION + 12, &quad_4[cnt][0], 4);
		if (ret == -1) {
			printf("\nFailed to read time capture register\n");
			return -1;
		}
	}
	
	/* verify the two readings are identical */
	good_data = 1;
	for (cnt = 0; cnt < 3; cnt++) {
		if ((quad_1[0][cnt] != quad_1[1][cnt]) && (quad_2[0][cnt] != quad_2[1][cnt])) {
			good_data = 0;
		}
		if ((quad_3[0][cnt] != quad_3[1][cnt]) && (quad_4[0][cnt] != quad_4[1][cnt])) {
			good_data = 0;
		}
	}
	if (good_data == 0) {
		printf("\n\nThere may be an error in position.  Try again\n");
		return -1;
	}
	
	/* assign the values */
	lat_unit_deg = quad_1[0][0] & 0x0F;
	lat_tens_deg = quad_1[0][0] >> 4;
	lat_hund_deg = quad_1[0][1] & 0x0F;
	lat_unit_min = quad_1[0][2] & 0x0F;
	lat_tens_min = quad_1[0][2] >> 4;
	lat_NorS = quad_1[0][3];
	
	lat_tenths_sec = quad_2[0][0] & 0x0F;
	lat_unit_sec = quad_2[0][1] & 0x0F;
	lat_tens_sec = quad_2[0][1] >> 4;
	lon_unit_deg = quad_2[0][2] & 0X0F;
	lon_tens_deg = quad_2[0][2] >> 4;
	lon_hund_deg = quad_2[0][3] & 0x0F;
	
	lon_unit_min = quad_3[0][0] & 0x0F;
	lon_tens_min = quad_3[0][0] >> 4;
	lon_EorW = quad_3[0][1];
	lon_tenths_sec = quad_3[0][2] & 0x0F;
	lon_unit_sec = quad_3[0][3] & 0x0F;
	lon_tens_sec = quad_3[0][3] >> 4;
	
	alt_unit_km = quad_4[0][0] & 0x0F;
	alt_tens_km = quad_4[0][0] >> 4;
	alt_sign = quad_4[0][1];
	alt_tenths_m = quad_4[0][2] & 0x0F;
	alt_unit_m = quad_4[0][2] >> 4;
	alt_tens_m = quad_4[0][3] & 0x0F;
	alt_hund_m = quad_4[0][3] >> 4;
	

	/* print out the position */
	printf("\n\nAntenna Position:\n");
	printf("   Latitude = %d%d%d° ", lat_hund_deg, lat_tens_deg, lat_unit_deg);
	printf("%d%d\' ", lat_tens_min, lat_unit_min);
	printf("%d%d.%d\" %c\n", lat_tens_sec, lat_unit_sec, lat_tenths_sec, lat_NorS);
	printf("   Longitude = %d%d%d° ", lon_hund_deg, lon_tens_deg, lon_unit_deg);
	printf("%d%d\' ", lon_tens_min, lon_unit_min);
	printf("%d%d.%d\" %c\n", lon_tens_sec, lon_unit_sec, lon_tenths_sec, lon_EorW);
	printf("   Altitude = %c%d%d%d%d%d.%dm\n",alt_sign, alt_tens_km, alt_unit_km, alt_hund_m, alt_tens_m, alt_unit_m, alt_tenths_m);
	
	return 0;
}
/* end of function: fetch_position */ 
/*******************************************************************************/	



/*******************************************************************************
 * Function   : fetch_time
 * Inputs     : int fd - device file descriptor.
 * Returns    : 0 on success
 *             -1 on failure
 * Description: Read time from 100s of nanoseconds to thousands of years from device
 *		and print it out.
 */
int fetch_time(int fd) {
	int ret;
	unsigned char quad_1[4];
	unsigned char quad_2[4];
	unsigned char quad_3[4];
	unsigned char unit_micro, tens_micro, hunds_micro, unit_milli, hunds_nano, tens_milli;
	unsigned char hunds_milli, unit_sec, tens_sec, unit_min, tens_min, unit_hr, tens_hr;
	unsigned char unit_day, tens_day, hunds_day, unit_yr, tens_yr, hunds_yr, thou_yr;
	
	quad_1[0] = 0xff;
	/* write any value to location 0xFC to update the time capture register*/
	ret = write_pci(fd, REG_SOFTTIME, quad_1, 1);
	if (ret == -1) {
		printf("\nFailed to read time capture register\n");
		return -1;
	}
	
	/* read the time register */
	ret = read_pci(fd, REG_SOFTTIME, quad_1, 4);
	if (ret == -1) {
		printf("\nFailed to read time capture register\n");
		return -1;
	}
	ret = read_pci(fd, REG_SOFTTIME + 4, quad_2, 4);
	if (ret == -1) {
		printf("\nFailed to read time capture register\n");
		return -1;
	}
	ret = read_pci(fd, REG_SOFTTIME + 8, quad_3, 4);
	if (ret == -1) {
		printf("\nFailed to read time capture register\n");
		return -1;
	}
	
	/* assign the values */
	unit_micro = quad_1[0] & 0x0F;
	tens_micro = quad_1[0] >> 4;
	hunds_micro = quad_1[1] & 0x0F;
	unit_milli = quad_1[1] >> 4;
	hunds_nano = quad_1[3] >> 4;
	
	tens_milli = quad_2[0] & 0x0F;
	hunds_milli = quad_2[0] >> 4;
	unit_sec = quad_2[1] & 0x0F;
	tens_sec = quad_2[1] >> 4;
	unit_min = quad_2[2] & 0x0F;
	tens_min = quad_2[2] >> 4;
	unit_hr = quad_2[3] & 0x0F;
	tens_hr = quad_2[3] >> 4;
	
	unit_day = quad_3[0] & 0x0F;
	tens_day = quad_3[0] >> 4;
	hunds_day = quad_3[1] & 0x0F;
	unit_yr = quad_3[2] & 0x0F;
	tens_yr = quad_3[2] >> 4;
	hunds_yr = quad_3[3] & 0x0F;
	thou_yr = quad_3[3] >> 4;
	
	/* print out the time */
	printf("\n\n DATE AND TIME:\n");
	printf("       YEAR = %d%d%d%d\n", thou_yr, hunds_yr, tens_yr, unit_yr);
	printf("        DAY = %d%d%d\n", hunds_day, tens_day, unit_day);
	printf("       TIME = %d%d:%d%d:%d%d UTC\n", tens_hr, unit_hr, tens_min, unit_min, tens_sec, unit_sec);
	printf("   millisec = %d%d%d\n", hunds_milli, tens_milli, unit_milli);
	printf("   microsec = %d%d%d\n", hunds_micro, tens_micro, unit_micro);
	printf("    nanosec = %d00\n", hunds_nano);
	
	return 0;
}
/* end of function: fetch_time */
/*******************************************************************************/


/*******************************************************************************
 * Function   : event_capture_menu
 * Inputs     : int fd - device file descriptor.
 * Returns    : 0 to indicate user wants to go back to previous menu
 *              1 to indicate user wants to quit program
 * Description: Provides menu input functionality for event capture mode
 */
int event_capture_menu(int fd) {
	char ch[30];
	int ret;

	print_event_menu();
	
	for (;;) {
		/* %25 limits input received to 25 chars, jsw_flush removes extra input */
		printf("\nEnter Event Capture Command: ");
		scanf( "%25s", ch);
		jsw_flush();
		
		/* check for valid input strings */
		if ((strcasecmp(ch, "help") == 0) || (strcasecmp(ch, "h") == 0)) {
			print_event_menu();
		}
		else if ((strcasecmp(ch, "back") == 0) || (strcasecmp(ch, "b") == 0)) {
			return 0;
		}
		else if ((strcasecmp(ch, "quit") == 0) || (strcasecmp(ch, "q") == 0)) {
			return 1;
		}
		else if ((strcasecmp(ch, "source") == 0) || (strcasecmp(ch, "o") == 0)) {
			ev_source(fd);
		}
		else if ((strcasecmp(ch, "start") == 0) || (strcasecmp(ch, "s") == 0)) {
			event_capture(fd);
		}
		else if ((strcasecmp(ch, "setup") == 0) || (strcasecmp(ch, "v") == 0)) {
			ev_view_setup(fd);
		}
		else if ((strcasecmp(ch, "data") == 0) || (strcasecmp(ch, "d") == 0)) {
			fetch_event_data(fd);
		}
		else {
			printf("Command '%s' is not allowed\n", ch);
			printf("Enter 'h' for help\n");
		}
	}		
}
/* end of function: event_capture_menu */
/*******************************************************************************/


/*******************************************************************************
 * Function   : ev_source
 * Inputs     : int fd - device file descriptor.
 * Returns    : nothing
 * Description: Gets user input to change the source for external events
 */
void ev_source(int fd) {
	int ret;
	int num = 0;
	unsigned char user_buff[4];
	
	/* print event source options */
	printf("\n\nEvent Sources:\n");
	printf("  Rising Edge\n");
	printf("  1) External Event\n");
	printf("  2) Rate Synthesizer\n");
	printf("  3) Rate Generator\n");
	printf("  4) Time Compare\n\n");
	printf("  Falling Edge\n");
	printf("  5) External Event\n");
	printf("  6) Rate Synthesizer\n");
	printf("  7) Rate Generator\n");
	printf("  8) Time Compare\n\n");
	printf("Enter Choice: \n");
	
	/* get user input */
	scanf( "%d", &num);
	jsw_flush();
	
	switch(num) {
		case 1:
			user_buff[0] = 0x04;
			break;
		case 2:
			user_buff[0] = 0x05;
			break;
		case 3:
			user_buff[0] = 0x06;
			break;
		case 4:
			user_buff[0] = 0x07;
			break;
		case 5:
			user_buff[0] = 0x00;
			break;
		case 6:
			user_buff[0] = 0x01;
			break;
		case 7:
			user_buff[0] = 0x02;
			break;
		case 8:
			user_buff[0] = 0x03;
			break;
		default:
			printf("\nERROR: ");
			printf("Invalid Choice.  Event source not changed\n");
			return;
	}
	
	/* write the chosen source to config2 register */
	ret = write_pci(fd, REG_CONFIG2_ETCC, user_buff, 1);
	if (ret == 0) {
		//printf("\nSUCCESS: ");
		printf("Event source changed\n");
	}
	else {
		printf("\nERROR: ");
		printf("\nCould not write to device. Event source not changed\n");
	}
	return;
}
/* end of function: ev_source
/*******************************************************************************/

/*******************************************************************************
 * Function   : ev_view_setup
 * Inputs     : int fd - device file descriptor.
 * Returns    : nothing
 * Description: View the current external event source and edge
 */
void ev_view_setup(int fd) {
	char event[25], edge[10];
	unsigned char user_buff[4];
	
	read_pci(fd, REG_CONFIG2_ETCC, user_buff, 1);
	
	/* the first two bits indicate the event source */
	if ((user_buff[0] & 0x03) == 0x00) {
		strcpy(event, "EXTERNAL EVENT");
	}
	else if ((user_buff[0] & 0x03) == 0x01) {
		strcpy(event, "RATE SYNTHESIZER");
	}
	else if ((user_buff[0] & 0x03) == 0x02) {
		strcpy(event, "RATE GENERATOR");
	}
	else if ((user_buff[0] & 0x03) == 0x03) {
		strcpy(event, "TIME COMPARE");
	}
	
	/* the 3rd bit indicates the edge */
	if ((user_buff[0] & 0x04) == 0x00) {
		strcpy(edge, "FALLING");
	}
	else {
		strcpy(edge, "RISING");
	}
	
	printf("\n Event Source = %s, Trigger Edge = %s\n", event, edge);
	
	return;
}
/* end of function: ev_view_setup
/*******************************************************************************/


/*******************************************************************************
 * Function   : event_capture
 * Inputs     : int fd - device file descriptor.
 * Returns    : 0 on success
 *             -1 on error
 * Description: Spawns a child process which executes the event_cap process.  The
 *		event_cap process enables event interrupts and then waits for events 
 *		to occur, saving the timestamps into a temporary binary file.
 *		Meanwhile the  main parent process waits until a user presses 
 *		any key followed by enter, at which point the event_cap process is killed.
 *		and interrupts are once again disabled. The binary file containing the 
 *		timestamps is read in and the binary coded decimal numbers representing the
 *		date and time are then rewritten to a plain text file named by the user.
 */
int event_capture(int fd) {
	int binfile, txtfile, ret, len;
	pid_t pid;
	int eof_check = 0;
	char fd_arg[24], binfile_arg[24];
	char *filename;
	unsigned char user_buff[4], ch[256], large_buff[12], txtbuff[512];
	unsigned char unit_us, tens_us, hund_us, unit_ms, tens_ms, hund_ms, unit_s, tens_s;
	unsigned char unit_min, tens_min, unit_hr, tens_hr, unit_day, tens_day, hund_day;
	unsigned char unit_yr, tens_yr, hund_yr, thou_yr, hund_nano;
	
	/* open temporary binary output file */
	binfile = open("interrupt_data", O_RDWR|O_CREAT|O_APPEND, 00644);
	
	
	/* Turn the file descriptors into char strings so they can be passed 
	 * via execl to the event_cap process */
	sprintf(fd_arg, "%d", fd);
	sprintf(binfile_arg, "%d", binfile);
	
	/* spawn the child process */
	pid = fork();
	if (pid == -1) {
		printf("\n\nFork error. Exiting.\n");
		close(binfile);
		remove("interrupt_data");
		return -1;
	}
	if (pid == 0) {
	/* if pid is 0 then this is the child process */
		if (execl("/usr/bin/event_cap", "event_cap", fd_arg, binfile_arg, NULL) == -1){
			printf("execl Error!\n");
			close(binfile);
			remove("interrupt_data");
			return -1;
		}
	}
	/* otherwise this is the parent and pid = id number of child process */
	sleep(1);
	
	/* this process will now suspend until any key is entered */
	printf("\nType any character followed by <enter> to stop event capture\n");
	scanf( "%2s", ch);
	jsw_flush();
	
	/*disable the interrupt and clear the status bits*/
	user_buff[0] = 0x01;
	write_pci(fd, REG_HARD_CTRL, user_buff, 1);
	
	/* kill the child process (event_cap) */
	kill(pid, SIGKILL);
	
	
	/* stream the binary file into a text file */
	/***********************************************/
	filename = readline("\nSave as file: ");
	
	/* remove trailing whitespace in title */
	len = strlen(filename);
	for (len = strlen(filename); len >= 0; len--) {
		if ((filename[len-1] != ' ') && (filename[len-1] != '\t')) {
			break;
		}
	}
	filename[len] = '\0';
	

	txtfile = open(filename, O_RDWR|O_CREAT|O_APPEND, 00644);
	free(filename); /* need to free any variable set by readline */
	
	/*start from the top of the binary file */
	lseek(binfile, 0x00, SEEK_SET);

	/* Loop through the binary file reading 12 bytes at a time 
	 * because each timestamp is 12 bytes long 
	 */
	while (eof_check == 0) {
		ret = read(binfile, large_buff, 12);
		if (ret == 0) {
			eof_check = 1;
			continue;
		}
		
		unit_us = large_buff[0] & 0x0F;
		tens_us = large_buff[0] >> 4;
		hund_us = large_buff[1] & 0x0F;
		unit_ms = large_buff[1] >> 4;
		tens_ms = large_buff[2] & 0x0F;
		hund_ms = large_buff[2] >> 4;
		unit_s = large_buff[3] & 0x0F;
		tens_s = large_buff[3] >> 4;
		unit_min = large_buff[4] & 0x0F;
		tens_min = large_buff[4] >> 4;
		unit_hr = large_buff[5] & 0x0F;
		tens_hr = large_buff[5] >> 4;
		unit_day = large_buff[6] & 0x0F;
		tens_day = large_buff[6] >> 4;
		hund_day = large_buff[7] & 0x0F;
		unit_yr = large_buff[8] & 0x0F;
		tens_yr = large_buff[8] >> 4;
		hund_yr = large_buff[9] & 0x0F;
		thou_yr = large_buff[9] >> 4;
		hund_nano = large_buff[10] >> 4;
		
		/* print the contents in readable text format to a text buffer */
		sprintf(txtbuff, "       YEAR = %d%d%d%d\n        DAY = %d%d%d\n       TIME = %d%d:%d%d UTC\n        SEC = %d%d.%d%d%d%d%d%d%d\n\n", thou_yr, hund_yr, tens_yr, unit_yr, hund_day, tens_day, unit_day, tens_hr, unit_hr, tens_min, unit_min, tens_s, unit_s, hund_ms, tens_ms, unit_ms, hund_us, tens_us, unit_us, hund_nano);
		/* write the plain text buffer to the text file */
		write(txtfile, txtbuff, strlen(txtbuff));
	}
	/***********************************************/
	close(binfile);
	close(txtfile);
	remove("interrupt_data");
	return 0;
}
/* end of function: event_capture
/*******************************************************************************/


/*******************************************************************************
 * Function   : char2bin
 * Inputs     : unsigned char *inbuff - buffer containing a string of characters
 *		unsigned char *outbuff - buffer that will hold the 1s and 0s
 *		nbytes - number of bytes (or chars) to convert.
 * Returns    : Nothing
 * Description: Takes a character (or string of characters) and converts each 
 *		char into a binary string of 1s and 0s.
 */
void char2bin(unsigned char *inbuff, unsigned char *outbuff, int nbytes)
{
	int i,j,cnt;
	unsigned char pos;
	
	for(i = 0; i < nbytes; i++) {
		cnt = 0;
		for(j=7; j >= 0; j--) {
			pos = pow(2,j);
			if ((inbuff[i] & pos) == 0)
				sprintf(outbuff, "0");
			else
				sprintf(outbuff, "1");
			outbuff++;
		}
	}
}
/* end of function: char2bin
/*******************************************************************************/


/*******************************************************************************
 * Function   : fetch_event_data
 * Inputs     : int fd - GPS PCI file descriptor
 * Returns    : 0 on success
 *	       -1 on failure
 * Description: Displays the contents of the chosen data file.
 * To do list : Make some sort of standard header so that the timestamp files
 *		can be identified.  Otherwise this function will print out 
 *		any file that is chosen.
 */
int fetch_event_data(int fd) {
	int txtfile, eof_check, ret, len;
	char *filename, buff[256];
	filename = readline("Enter File Name:");
	
	/* remove trailing whitespace in title */
	len = strlen(filename);
	for (len = strlen(filename); len >= 0; len--) {
		if ((filename[len-1] != ' ') && (filename[len-1] != '\t')) {
			break;
		}
	}
	filename[len] = '\0';
	
	/* open the chosen file */
	txtfile = open(filename, O_RDWR);
	if (txtfile == -1) {
		printf("\nCould not open file: %s\n", filename);
		return -1;
	}
	
	
	/* display the contents of file */
	eof_check = 0;
	while(eof_check == 0) {
		ret = read(txtfile, buff, 256);
		if (ret == 0) {
			eof_check = 1;
			continue;
		}
		
		printf("%s", buff);
	}
			
	free(filename);
	return 0;
}
/* end of function: fetch_event_data
/*******************************************************************************/


/*******************************************************************************
 * Function   : jsw_flush
 * Inputs     : none
 * Returns    : none
 * Description: Gets rid of any extra characters in the stdin buffer.  Such as
 *		when input is limited to a certain number of characters and the
 *		limit is exceeded.  Without jsw_flush the extra characters would
 *		remain in the buffer and would be read in on the next scanf or getchar
 *		call.
 */
/* from http://www.c-for-dummies.com/faq/ */
void jsw_flush ( void ) 
{ 
	int ch; /* getchar returns an int */ 

	/* Read characters until there are none left */ 
	do 
		ch = getchar(); 
	while ( ch != EOF && ch != '\n' ); 

	clearerr ( stdin ); /* Clear EOF state */ 
}
/* end of function: jsw_flush
/*******************************************************************************/


/*******************************************************************************
 * Function   : check_antenna
 * Inputs     : int fd - GSP-PCI device file descriptor
 * Returns    : 0 on success
 *	       -1 on failure
 * Description: Checks antenna for shorts or open loads.
 */
int check_antenna(int fd) {
	int ret;
	unsigned char shorted, open;
	unsigned char user_buff[4];
	
	user_buff[0] = 0x70;
	write_pci(fd, REG_HARD_STATUS, user_buff, 1);
	ret = read_pci(fd, REG_HARD_STATUS, user_buff, 1);

	/* bits 5 and 6 indicate open and short status (0 = short/open)*/
	shorted = (user_buff[0] & 0x20) >> 5;
	open = (user_buff[0] & 0x10) >> 4;
	
	if (shorted == 0) {
		printf("WARNING: GPS Antenna Shorted\n");
	}
	if (open == 0) {
		printf("WARNING: GPS Antenna Open\n");
	}
	if (shorted == 1 && open == 1) {
		printf("GPS Antenna is Good (no SHORTS or OPEN loads detected)\n");
	}
	return 0;
}
/* end of function: check_antenna
/*******************************************************************************/


/*******************************************************************************
 * Function   : rate_menu
 * Inputs     : int fd - device file descriptor.
 * Returns    : 0 to indicate user wants to go back to main menu
 *              1 to indicate user wants to quit program
 * Description: Provides menu input functionality for rate generator mode
 */
int rategen_menu(int fd) {
	char ch[30];
	int ret;

	print_rategen_menu();
	
	for (;;) {
		/* %25 limits input received to 25 chars, jsw_flush removes extra input */
		printf("\nEnter Rate Generator Command: ");
		scanf( "%25s", ch);
		jsw_flush();
		
		/* check for valid input strings */
		if ((strcasecmp(ch, "help") == 0) || (strcasecmp(ch, "h") == 0)) {
			print_rategen_menu();
		}
		else if ((strcasecmp(ch, "back") == 0) || (strcasecmp(ch, "b") == 0)) {
			return 0;
		}
		else if ((strcasecmp(ch, "quit") == 0) || (strcasecmp(ch, "q") == 0)) {
			return 1;
		}
		else if ((strcasecmp(ch, "rate") == 0) || (strcasecmp(ch, "r") == 0)) {
			rg_rate(fd);
		}
		else if ((strcasecmp(ch, "enable") == 0) || (strcasecmp(ch, "e") == 0)) {
			rg_enable(fd);
		}
		else if ((strcasecmp(ch, "setup") == 0) || (strcasecmp(ch, "v") == 0)) {
			rg_view_setup(fd);
		}
		else {
			printf("Command '%s' is not allowed\n", ch);
			printf("Enter 'h' for help\n");
		}
	}		
}
/* end of function: rategen_menu */
/*******************************************************************************/


/*******************************************************************************
 * Function   : rg_view_setup
 * Inputs     : int fd - device file descriptor.
 * Returns    : Nothing
 * Description: Checks and reports on:
 *			a) whether the generator is on
 *			b) at what rate it is set for
 *			c) whether the generator output is being sent to the 
 *			   CODE OUT BNC port.
 */
void rg_view_setup(int fd) {
	unsigned char user_buff[4];
	
	/***********************************************/
	/* Check Config #1 Register to see if the generator is turned on 
	 * and at what rate it is running */
	/***********************************************/
	read_pci(fd, REG_CONFIG1, user_buff, 4);
	printf("\n  Generator is: ");
	if ((user_buff[0] & 0x08) == 0x08) {
		printf("OFF");
		printf(" (type 'e' to enable Generator)\n");
	}
	else {
		//printf("ON\n");
		printf("ON\n");
	}
	
	/* see what rate it's at (only care about upper 4 bits of 4th byte) */
	printf("  Rate = ");
	user_buff[3] = user_buff[3] & 0xF0;
	if (user_buff[3] == 0x00) {
		printf("0 PPS");
	}
	else if(user_buff[3] == 0x10) {
		printf("10K PPS");
	}
	else if(user_buff[3] == 0x20) {
		printf("1K PPS");
	}
	else if(user_buff[3] == 0x30) {
		printf("100 PPS");
	}
	else if(user_buff[3] == 0x40) {
		printf("10 PPS");
	}
	else if(user_buff[3] == 0x50) {
		printf("1 PPS");
	}
	else if(user_buff[3] == 0x60) {
		printf("100K PPS");
	}
	else if(user_buff[3] == 0x70) {
		printf("1M PPS");
	}
	else if(user_buff[3] == 0x80) {
		printf("5M PPS");
	}
	else if(user_buff[3] == 0x90) {
		printf("10M PPS");
	}
	else {
		printf("UNKNOWN RATE");
	}
	/***********************************************/
	
	/* check output source for CODE OUT BNC port*/
	printf("\n  Output on: pin 7 of D9 port");
	read_pci(fd, REG_CONFIG2_BNC, user_buff, 1);
	if (user_buff[0] == 0x02) {
		printf(" AND the ");
		printf("CODE OUT BNC port\n");
	}
	else {
		printf("\n  type 'e' to enable the rate generator on the CODE OUT BNC port\n");
		printf("  Currently selected source for CODE OUT port is: ");
		if (user_buff[0] == 0x00) {
			printf("IRIG-B AM\n");
		}
		else if (user_buff[0] == 0x01) {
			printf("IRIG-B DC\n");
		}
		else if (user_buff[0] == 0x03) { 
			printf("Rate Synthesizer\n");
		}
		else if (user_buff[0] == 0x04) {
			printf("Time Compare\n");
		}
		else if (user_buff[0] == 0x05) {
			printf("1 PPS\n");
		}
		else
			printf("Unknown\n");
	}
}
/* end of function: rg_view_setup */
/*******************************************************************************/


/*******************************************************************************
 * Function   : rg_rate
 * Inputs     : int fd - device file descriptor.
 * Returns    : Nothing
 * Description: Changes generator rate, after prompting user	
 */
void rg_rate(int fd) {
	int ret;
	int num = 0;
	unsigned char user_buff[4];
	
	/* print event source options */
	printf("\n\nRates (in PPS):\n");
	printf("  1) Disabled\n");
	printf("  2) 1\n");
	printf("  3) 10\n");
	printf("  4) 100\n");
	printf("  5) 1K\n");
	printf("  6) 10K\n");
	printf("  7) 100K\n");
	printf("  8) 1M\n");
	printf("  9) 5M\n");
	printf(" 10) 10M\n\n");
	printf("Select Option: \n");
	
	/* get user input */
	scanf( "%d", &num);
	jsw_flush();
	
	switch(num) {
		case 1:
			user_buff[0] = 0x00;
			break;
		case 2:
			user_buff[0] = 0x50;
			break;
		case 3:
			user_buff[0] = 0x40;
			break;
		case 4:
			user_buff[0] = 0x30;
			break;
		case 5:
			user_buff[0] = 0x20;
			break;
		case 6:
			user_buff[0] = 0x10;
			break;
		case 7:
			user_buff[0] = 0x60;
			break;
		case 8:
			user_buff[0] = 0x70;
			break;
		case 9:
			user_buff[0] = 0x80;
			break;
		case 10:
			user_buff[0] = 0x90;
			break;
		default:
			printf("\nERROR: ");
			printf("Invalid Choice.  Rate left unchanged\n");
			return;
	}
	
	/* write the chosen source to config2 register */
	ret = write_pci(fd, REG_CONFIG1_RGC, user_buff, 1);
	if (ret == 0) {
		printf("\nSUCCESS: ");
		printf("Rate changed\n");
	}
	else {
		printf("\nERROR: ");
		printf("\nCould not write to device. Event source not changed\n");
	}
	return;
}
/* end of function: rg_rate */
/*******************************************************************************/


/*******************************************************************************
 * Function   : rg_enable
 * Inputs     : int fd - device file descriptor.
 * Returns    : Nothing
 * Description: Ensures generator is both on, and being output to the CODE OUT
 *		BNC port	
 */
void rg_enable(int fd) {
	unsigned char user_buff[4], tmp;
	
	printf("\nSetting the 'CODE OUT' BNC port to Rate Generator\n");
	user_buff[0] = 0x02;
	write_pci(fd, REG_CONFIG2_BNC, user_buff, 1);
	
	read_pci(fd, REG_CONFIG1, user_buff, 1);
	printf("Turning on generator\n");
	tmp = user_buff[0] & 0x08;
	if (tmp == 0x08) {
		user_buff[0] = user_buff[0] & 0xF7; /* clears 4th bit */
		write_pci(fd, REG_CONFIG1, user_buff, 1);
	}
}
/* end of function: rg_enable */
/*******************************************************************************/


/*******************************************************************************
 * Function   : autostamp
 * Inputs     : int fd - device file descriptor.
 * 	      : char * tsfilename - character buffer containing filename to write to
 * 	      : after interrupt data is complete
 * Returns    : 0 on success
 *	       -1 on failure
 * Description: This function is very similar to the event_capture function except
 *		that it does not require user input and thus can be run automatically.
 */
int autostamp(int fd, char *tsfilename) {
	int binfile, txtfile, len, ret;
	pid_t pid;
	int eof_check = 0;
	char fd_arg[24], binfile_arg[24];
	char filename[32];
	unsigned char user_buff[4], large_buff[12], txtbuff[512];
	unsigned char unit_us, tens_us, hund_us, unit_ms, tens_ms, hund_ms, unit_s, tens_s;
	unsigned char unit_min, tens_min, unit_hr, tens_hr, unit_day, tens_day, hund_day;
	unsigned char unit_yr, tens_yr, hund_yr, thou_yr, hund_nano;
	

	/* make sure drivers are loaded */
	
	/* attempt to initialize the GPS for at most 5 minutes */
	GPS_init(fd);
	
	/* setup the output file */

	binfile = open("interrupt_data", O_RDWR|O_CREAT|O_APPEND, 00644);
	
	
	/* Turn the file descriptors into char strings so they can be passed 
	* via execl to the event_cap process */
	sprintf(fd_arg, "%d", fd);
	sprintf(binfile_arg, "%d", binfile);
	
	/* spawn the child process */
	pid = fork();
	if (pid == -1) {
		printf("\n\nFork error. Exiting.\n");
		close(binfile);
		remove("interrupt_data");
		return -1;
	}
	if (pid == 0) {
		/* if pid is 0 then this is the child process */
		if (execl("/usr/bin/event_cap", "event_cap", fd_arg, binfile_arg, NULL) == -1){
			printf("execl Error!\n");
			close(binfile);
			remove("interrupt_data");
			return -1;
		}
	}
	
	printf("\nTimestamping external events\n");
	printf("Run 'stopstamp.bash' in another terminal to stop\n");
	/* will wait until the child process has been killed */
	wait();
	
	/*disable the interrupt and clear the status bits*/
	user_buff[0] = 0x01;
	write_pci(fd, REG_HARD_CTRL, user_buff, 1);
	
	/* kill the child process (event_cap) */
	kill(pid, SIGKILL);
	
	
	/* stream the binary file into a text file */
	/***********************************************/
//	strcpy(filename, "timestampdata.txt");
// REMOVED OCT 24 2013 by KK, see above for more specific filename.	
	strcpy(filename, tsfilename);
	txtfile = open(filename, O_RDWR|O_CREAT|O_APPEND, 00644);
	
	/*start from the top of the binary file */
	lseek(binfile, 0x00, SEEK_SET);

	/* Loop through the binary file reading 12 bytes at a time 
	* because each timestamp is 12 bytes long 
	*/
	while (eof_check == 0) {
		ret = read(binfile, large_buff, 12);
		if (ret == 0) {
			eof_check = 1;
			continue;
		}
		
		unit_us = large_buff[0] & 0x0F;
		tens_us = large_buff[0] >> 4;
		hund_us = large_buff[1] & 0x0F;
		unit_ms = large_buff[1] >> 4;
		tens_ms = large_buff[2] & 0x0F;
		hund_ms = large_buff[2] >> 4;
		unit_s = large_buff[3] & 0x0F;
		tens_s = large_buff[3] >> 4;
		unit_min = large_buff[4] & 0x0F;
		tens_min = large_buff[4] >> 4;
		unit_hr = large_buff[5] & 0x0F;
		tens_hr = large_buff[5] >> 4;
		unit_day = large_buff[6] & 0x0F;
		tens_day = large_buff[6] >> 4;
		hund_day = large_buff[7] & 0x0F;
		unit_yr = large_buff[8] & 0x0F;
		tens_yr = large_buff[8] >> 4;
		hund_yr = large_buff[9] & 0x0F;
		thou_yr = large_buff[9] >> 4;
		hund_nano = large_buff[10] >> 4;
		
		/* print the contents in readable text format to a text buffer */
		sprintf(txtbuff, "       YEAR = %d%d%d%d\n        DAY = %d%d%d\n       TIME = %d%d:%d%d UTC\n        SEC = %d%d.%d%d%d%d%d%d%d\n\n", thou_yr, hund_yr, tens_yr, unit_yr, hund_day, tens_day, unit_day, tens_hr, unit_hr, tens_min, unit_min, tens_s, unit_s, hund_ms, tens_ms, unit_ms, hund_us, tens_us, unit_us, hund_nano);
		/* write the plain text buffer to the text file */
		write(txtfile, txtbuff, strlen(txtbuff));
	}
	/***********************************************/
	close(binfile);
	close(txtfile);
	remove("interrupt_data");

	
	return 0;
}
/* end of function: autostamp */
/*******************************************************************************/
