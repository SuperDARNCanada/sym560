/* File : 	sym560_functions.h
 * Author: 	Bradley Krug
 * Modified:	Feb 21 (added some comments)
 * Description:	Header file for the sym560_functions.c functions.  Contains 
 *		includes, global variables and function declarations.
 */

#ifndef SYM560_CMDLINE_H
#define SYM560_CMDLINE_H

/* tsc-user.c */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <readline/readline.h>
#include <time.h>

/********************************************************/
/*PCI CARD REGISTERS */
/* See chapter 3 in the Sym560 user manual for details */

/***************************************/
/*Configuration #1 Register(4 bytes)*/
#define REG_CONFIG1		0x118
/* BYTE 1: Time Source Control*/
#define REG_CONFIG1_TSC		0x118

/* BYTE 2: Timecode Control */
#define REG_CONFIG1_TCC		0x119

/* BYTE 4: Rate Generator Control */
#define REG_CONFIG1_RGC		0x11B
/***************************************/

/***************************************/
/*Configuration #2 Register(4 bytes)*/
#define REG_CONFIG2		0x12C
/* BYTE 1: Miscellaneous Control*/
#define REG_CONFIG2_MISC	0x12C

/* BYTE 2: Rate Synthesizer Control */
#define REG_CONFIG2_RSC		0x12D

/* BYTE 3: Event Time Capture Control */
#define REG_CONFIG2_ETCC	0x12E

/* BYTE 4: Code Out BNC (J1) Source Select */
#define REG_CONFIG2_BNC		0x12F
/***************************************/

/***************************************/
/*Hardware Control Register(1 byte)*/
#define REG_HARD_CTRL		0xF8
/***************************************/

/***************************************/
/*Hardware Status Register(1 byte)*/
#define REG_HARD_STATUS		0xFE
/***************************************/

/***************************************/
/*Satellite Signal Strength Register(24 bytes)*/
/* Bytes 1-4 */
#define REG_SATSIG_SATA		0x198

/* Bytes 5-8 */
#define REG_SATSIG_SATB		0x19C

/* Bytes 9-12 */
#define REG_SATSIG_SATC		0x1A0

/* Bytes 13-16 */
#define REG_SATSIG_SATD		0x1A4

/* Bytes 17-20 */
#define REG_SATSIG_SATE		0x1A8

/* Bytes 21-24 */
#define REG_SATSIG_SATF		0x1AC
/***************************************/

/***************************************/
/* Satellite Update Status Register (1 byte) */
#define REG_SATSTAT		0x1B0
/***************************************/

/***************************************/
/* Software Time Capture (12 bytes) */
#define REG_SOFTTIME		0xFC
#define REG_SOFTTIME_LOCK	0x105
/***************************************/

/***************************************/
/* Antenna Position Register */
#define REG_ANT_POSITION	0x108
/***************************************/

/* end of register definitions */
/********************************************************/

/* function declarations */
void char2bin(unsigned char*, unsigned char*, int);
int print_menu();
int read_pci(int fd, off_t regoff, char *user_buff, int nbytes);
int read_pci_verbose(int fd, off_t regoff, char *user_buff, int nbytes);
int write_pci(int fd, off_t regoff, char *user_buff, int nbytes);
int write_pci_verbose(int fd, off_t regoff, char *user_buff, int nbytes);
int GPS_init(int fd);
int fetch_position(int fd);
int fetch_time(int fd);
int satsig(int fd);
void ev_source(int fd);
void ev_view_setup(int fd);
int event_capture(int fd);
int fetch_event_data(int fd);
void jsw_flush();
int check_antenna(int fd);
int rategen_menu(int fd);
void rg_rate(int fd);
void rg_enable(int fd);
void rg_view_setup(int fd);
int autostamp(int fd, char * tsfilename);

#endif /* SYM560_CMDLINE_H */
