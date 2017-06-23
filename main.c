/* main.c: pdp11monloader user interface, global resources
 *
 * PDP-11 code loader over console monitor
 *
 * Copyright (c) 2017, Joerg Hoppe, j_hoppe@t-online.de, www.retrocmp.com
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  - Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  18-Jun-2017 	JH  V 1.0.0		cloned from tu58fs
 *
 */

#define _MAIN_C_

#define VERSION	"v1.0.0"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <strings.h>

#include "error.h"
#include "utils.h"
#include "getopt2.h"
#include "serial.h"

#include "memory.h"
#include "monitor.h"

#include "main.h"   // own

static char copyright[] = "(C) 2017 Joerg Hoppe <j_hoppe@t-online.de>\n";

static char version[] =
		"pdp11monloader - Load binary code into PDP-11 over console monitor.\n"
		"                 Version "VERSION", compile "__DATE__ " " __TIME__ ".";

#define OPT_GO_NOT 0
#define OPT_GO 1
#define OPT_GO_COMMITED 2

// global options
int opt_info = 0;
int opt_non_info = 0;

char opt_serial_port[256] = "";
int opt_serial_speed = 0; // line baud rate
int opt_serial_bitcount = 8; //
char opt_serial_parity = 'n'; // n, e, o
int opt_serial_stopbits = 1; // stop bits, 1 or 2
int opt_debug = 0; // set nonzero for debug output
int opt_usbdelay = 0; // extra delay of RS232 over USB adapters

char opt_filename[4096] = "";
memory_fileformat_t opt_fileformat = fileformat_none;

monitor_type_t opt_monitor = monitor_none;
int opt_go_address = MEMORY_ADDRESS_INVALID;
int opt_go = OPT_GO_NOT;

int opt_tty = 0;

int arg_menu_linewidth = 80;

// command line args
static getopt_t getopt_parser;

// memory data buffer
static memory_t the_memory;

/*
 * help()
 */
static char * examples[] =
		{
				"sudo ./" PROGNAME " -p /dev/ttyS2 -b 9600 -odt --octaltext pattern.bin\n", //
				"    The PDP-11 is connected to serial RS232 port \"ttyS1\" under Linux,\n",
				"    Baud rate is 9600, data format is \"8N1\" by default.\n",
				"    Access to serial line device requires \"sudo\".\n", //
				"    It is a QBUS LSI-11 (11/03,11/23,11/73) with ODT console monitor.\n", //
				"    The PDP-11 must have been halted to show the \"@\" prompt.\n", //
				"    Data is read from file \"pattern.bin\", in the form of \n",
				"    one octal address and one or more octal 16bits words per line.\n",
				"    The data is loaded into the PDP-11 memory over console \"Deposit\" commands.\n",
				"\n",
				"sudo ./" PROGNAME " -p /dev/ttyS2 -b 9600 -odt -ml dd.lst -go 1000 -tty\n", //
				"    Same PDP-11 and connection as before, booting a TU58 tape device:\n",
				"    "PROGNAME" eliminates the need for a Boot ROM.\n"
				"    The boot loader code is parsed from MACRO-11 listing file \"dd.lst\"\n",
				"    After loading the code program execution is started at address 1000,\n",
				"    then program switches mode and user can operate the running PDP-11 over a\n",
				"    simple terminal emulation.\n", "\n",
				PROGNAME ".exe -p 1 -b 9600 -odt --macro11listing cxcpag.lst -go 220 -tty\n", //
				"    Call when running under MS Windows, serial port COM1 is used for connection.\n",
				"    This time the diagnostic CXCPAG is executed at start address 000220.\n",
				"\n",
				PROGNAME ".exe -p 1 -b 9600 -odt -pt DEC-11-AJPB-PB.ptap -go -tty\n", //
				"    Code is loaded from an \"Absolute Papertape\" image, its the \n",
				"    \"Papertape BASIC\" from 1972. Execution is started.\n",
				"    The program start address is read from paper tape and not given on command line.\n",
				"\n", //
				PROGNAME ".exe -p 1 -b 4800 -fmt 7e1 -m9312 -pt DEC-11-AJPB-PB.ptap -go -tty\n", //
				"    BASIC is run on an UNIBUS PDP-11, which executes the console monitor from a\n",
				"    M9312 Boot Terminator card. The DL11 serial console on this PDP-11 is jumpered\n",
				"    to 38400 baud, serial line format is 7 bit with even parity.\n",
				"\n", //

				PROGNAME ".exe -p 7 -b 115200 --usbdelay 7 -m9312 -pt DEC-11-AJPB-PB.ptap -go -tty\n", //
				"    As before. An USB-to-RS232 adapter is used, plug'n'play assigns port number COM7.\n",
				"    The PDP-11 console DL11 hardware is tuned for 115200 baud, \n",
				"    this USB dongle is FTDI and needs 7 ms extra delay.\n",
				NULL };

void help() {
	char **s;
	fprintf(ferr, "\n");
	fprintf(ferr, "NAME\n");
	fprintf(ferr, "\n");
	fprintf(ferr, "%s\n", version);
	fprintf(ferr, "%s\n", copyright);

	fprintf(ferr, "\n");
	fprintf(ferr, "SYNOPSIS\n");
	fprintf(ferr, "\n");
//	fprintf(ferr, "Command line options are processed strictly left-to-right. \n\n");
	// getopt must be initialized to print the syntax
	getopt_help(&getopt_parser, stdout, arg_menu_linewidth, 10, PROGNAME);
	fprintf(ferr, "\n");
	fprintf(ferr, "EXAMPLES\n");
	fprintf(ferr, "\n");
	for (s = examples; *s; s++)
		fputs(*s, ferr);
	fprintf(ferr, "\n");
	/*
	 fprintf(ferr, "SEE ALSO\n");
	 fprintf(ferr, "\n");
	 fprintf(ferr, "Online docs: www.retrocmp.com/tools/pdp11monloader \n");
	 fprintf(ferr, "Repository: https://github.com/j-hoppe/pdp11monloader\n");
	 fprintf(ferr, "Contact: j_hoppe@t-online.de\n");
	 fprintf(ferr, "\n");
	 */
	exit(1);
}

// show error for one option
static void commandline_error() {
	fprintf(ferr, "Error while parsing commandline:\n");
	fprintf(ferr, "  %s\n", getopt_parser.curerrortext);
	exit(1);
}

// parameter wrong for currently parsed option
static void commandline_option_error(char *errtext, ...) {
	va_list args;
	fprintf(ferr, "Error while parsing commandline option:\n");
	if (errtext) {
		va_start(args, errtext);
		vfprintf(ferr, errtext, args);
		fprintf(ferr, "\nSyntax:  ");
		va_end(args);
	} else
		fprintf(ferr, "  %s\nSyntax:  ", getopt_parser.curerrortext);
	getopt_help_option(&getopt_parser, stdout, 96, 10);
	exit(1);
}

/* check whether the given device parameter configuration
 * my cause problems.
 */

/*
 * read commandline parameters into global "param_" vars
 * result: 0 = OK, 1 = error
 */
static void parse_commandline(int argc, char **argv) {
	int res;

	// define commandline syntax
	getopt_init(&getopt_parser, /*ignore_case*/1);

//	getopt_def(&getopt_parser, NULL, NULL, "hostname", NULL, NULL, "Connect to the Blinkenlight API server on <hostname>\n"
//		"<hostname> may be numerical or ar DNS name",
//		"127.0.0.1", "connect to the server running on the same machine.",
//		"raspberrypi", "connected to a RaspberryPi with default name.");

	// !!!1 Do not define any defaults... else these will be set very time!!!

	getopt_parser.ignore_case = 1;
	getopt_def(&getopt_parser, "?", "help", NULL, NULL, NULL, "Print help.",
	NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "v", "version", NULL, NULL, NULL,
			"Output version string.",
			NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "dbg", "debug", NULL, NULL, NULL,
			"Enable debug output to console.",
			NULL, NULL, NULL, NULL);
	//	getopt_def(&getopt_parser, "v", "verbose", NULL, NULL, NULL,
	//			"Enable verbose output to terminal.",
	//			NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "b", "baudrate", "baudrate", NULL, "38400",
			"Set serial line speed to 300..3000000 baud.",
			NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "f", "format", "bits_parity_stop", NULL, "8N1",
			"Set format parameters for serial line as a 3 char string <bitcount><parity><stopbits>\n"
					"<bitcount> maybe 7 or 8, <parity> is n (no), e (even) or o (odd), <stopbits> is 1 or 2.\n"
					"Set to special console params for --boot operation. leave default for device emulation.",
			"7e2", "Set for 7 bit even parity with 2 stop bits (--boot)", NULL,
			NULL);
	getopt_def(&getopt_parser, "p", "port", "serial_device", NULL, NULL,
			"Select serial port: \"COM<serial_device>:\" or <serial_device> is a node like \"/dev/ttyS1\"",
			NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "odt", "odt", NULL, NULL, NULL,
			"Set PDP-11 console monitor type to \"ODT\", for all QBUS machines",
			NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "m9312", "m9312", NULL, NULL, NULL,
			"Set PDP-11 console monitor type to \"M9312\".\n"
					"This BOOT-terminator card is used on many UNIBUS machines\n"
					"and is also build into the 11/44.",
			NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "m9301", "m9301", NULL, NULL, NULL,
			"Older variant of M9312 BOOT terminator.",
			NULL, NULL, NULL, NULL);

	getopt_def(&getopt_parser, "ot", "octaltext", "inputfile", NULL, NULL,
			"Input file is interpreted as text file containing octal address and data words.\n"
					"Words made of characters '0' to '7', all other chars are interpreted as white space.\n"
					"First words in each text line is address, next words are data.\n"
					"For example, the \"octaltext\" format allows to read SimH \"deposit\" scripts.",
			"hello.txt", "load memory from hello.txt",
			NULL, NULL);
	getopt_def(&getopt_parser, "ml", "macro11listing", "inputfile", NULL, NULL,
			"Input file is interpreted as MACRO-11 listing\n"
					"Code is parsed and DEPOSITed into memory.", "hello.lst",
			"load memory from hello.lst",
			NULL, NULL);
	getopt_def(&getopt_parser, "pt", "papertape", "inputfile", NULL, NULL,
			"Input file is interpeted as \"Standard Absolute Paper Tape\" image.",
			NULL, NULL, NULL, NULL);

	getopt_def(&getopt_parser, "go", "go", NULL, "start_addr", NULL,
			"Issue a \"<start_addr>G\" to start program execution.\n"
					"The RUN/HALT switch must be in RUN position.\n"
					"<start_addr> can be omitted, if code loaded by previous --ptap",
			"1000", "Start program execution at address 1000 (octal)", NULL,
			NULL);
	getopt_def(&getopt_parser, "gc", "gocomitted", NULL, "start_addr", NULL,
			"Same as \"go\", but the final \"G\" is not sent.\n"
					"Typical used if a terminal emulation is started next:\n"
					"User types a single \"G\" to start and no output is lost",
			"",
			"Start program execution at address set by papertape file (if any)",
			NULL, NULL);

	getopt_def(&getopt_parser, "t", "tty", NULL, NULL, NULL,
			"After operation a simple line based terminal remains active on the serial port,\n"
					"so you can operate the PDP-11 over serial console immediately.\n"
					"If not used after \"go\" you have to start a more comfortable terminal emulator\n"
					"and will probably loose some output of the started program.",
			NULL, NULL, NULL, NULL);

	getopt_def(&getopt_parser, "ud", "usbdelay", "milliseconds", NULL, NULL,
			"Specifies extra delay for console monitor protocol.\n"
					"Some USB-RS232 adapters have large delays when polling input (for example FTDIs).\n"
					"Other brands (Prolific) and non-USB RS232 ports should never need this.\n"
					"Experiment for an optimum between download speed and reliability, recommended range 5-20.",
			NULL, NULL, NULL, NULL);

	/*
	 // test options
	 getopt_def(&getopt_parser, "testfs", "testfs", "filename", NULL, NULL,
	 "Read an image, convert it to filesystem and test",
	 NULL, NULL, NULL, NULL);
	 */
	if (argc < 2)
		help(); // at least 1 required

	opt_serial_port[0] = 0;
	opt_monitor = monitor_none;

	res = getopt_first(&getopt_parser, argc, argv);
	while (res > 0) {
		if (getopt_isoption(&getopt_parser, "help")) {
			opt_info = 1;
			help();
		} else if (getopt_isoption(&getopt_parser, "version")) {
			opt_info = 1;
			info(version);
		} else if (getopt_isoption(&getopt_parser, "debug")) {
			opt_debug = 1;
		} else if (getopt_isoption(&getopt_parser, "baudrate")) {
			if (getopt_arg_i(&getopt_parser, "baudrate", &opt_serial_speed) < 0)
				commandline_option_error(NULL);
		} else if (getopt_isoption(&getopt_parser, "format")) {
			char formatstr[80];
			if (getopt_arg_s(&getopt_parser, "bits_parity_stop", formatstr,
					sizeof(formatstr)) < 0)
				commandline_option_error(NULL);
			if (serial_decode_format(formatstr, &opt_serial_bitcount,
					&opt_serial_parity, &opt_serial_stopbits))
				commandline_option_error("Illegal format");
		} else if (getopt_isoption(&getopt_parser, "port")) {
			opt_non_info = 1;
			if (getopt_arg_s(&getopt_parser, "serial_device", opt_serial_port,
					sizeof(opt_serial_port)) < 0)
				commandline_option_error(NULL);

		} else if (getopt_isoption(&getopt_parser, "odt")) {
			opt_non_info = 1;
			opt_monitor = monitor_odt;
		} else if (getopt_isoption(&getopt_parser, "m9312")) {
			opt_non_info = 1;
			opt_monitor = monitor_m9312;
		} else if (getopt_isoption(&getopt_parser, "m9301")) {
			opt_non_info = 1;
			opt_monitor = monitor_m9301;

		} else if (getopt_isoption(&getopt_parser, "octaltext")) {
			opt_non_info = 1;
			opt_fileformat = fileformat_addr_value_text;
			if (getopt_arg_s(&getopt_parser, "inputfile", opt_filename,
					sizeof(opt_filename)) < 0)
				commandline_option_error(NULL);
		} else if (getopt_isoption(&getopt_parser, "macro11listing")) {
			opt_non_info = 1;
			opt_fileformat = fileformat_macro11_listing;
			if (getopt_arg_s(&getopt_parser, "inputfile", opt_filename,
					sizeof(opt_filename)) < 0)
				commandline_option_error(NULL);
		} else if (getopt_isoption(&getopt_parser, "papertape")) {
			opt_non_info = 1;
			opt_fileformat = fileformat_papertape;
			if (getopt_arg_s(&getopt_parser, "inputfile", opt_filename,
					sizeof(opt_filename)) < 0)
				commandline_option_error(NULL);

		} else if (getopt_isoption(&getopt_parser, "go")) {
			opt_non_info = 1;
			opt_go = OPT_GO;
			if (getopt_arg_o(&getopt_parser, "start_addr", &opt_go_address) < 0)
				opt_go_address = MEMORY_ADDRESS_INVALID;

		} else if (getopt_isoption(&getopt_parser, "gocomitted")) {
			opt_non_info = 1;
			opt_go = OPT_GO_COMMITED;
			if (getopt_arg_o(&getopt_parser, "start_addr", &opt_go_address) < 0)
				opt_go_address = MEMORY_ADDRESS_INVALID;

		} else if (getopt_isoption(&getopt_parser, "tty")) {
			opt_non_info = 1;
			opt_tty = 1;

		} else if (getopt_isoption(&getopt_parser, "usbdelay")) {
			opt_non_info = 1;
			if (getopt_arg_i(&getopt_parser, "milliseconds", &opt_usbdelay) < 0)
				commandline_option_error(NULL);
		}
		res = getopt_next(&getopt_parser);
	}
	if (res == GETOPT_STATUS_MINARGCOUNT || res == GETOPT_STATUS_MAXARGCOUNT)
		// known option, but wrong number of arguments
		commandline_option_error(NULL);
	else if (res < 0)
		commandline_error();
}

/*
 * Download object code from 'memory' over monitor into PDP-11
 */
int cmd_deposit(memory_t *memory, serial_device_t *serialdevice,
		monitor_type_t monitor_type) {
	monitor_t monitor;
	unsigned addr;

	if (monitor_init(&monitor, serialdevice, monitor_type, stdout,
			opt_usbdelay))
		return error_set(ERROR_MONITOR, "monitor_init in bootloader_download");
	if (monitor_assert_prompt(&monitor))
		return error_set(ERROR_MONITOR,
				"Boot loader download: PDP-11 console prompt not found.");

	// dump out defined address/value pairs
	for (addr = 0; addr < 2 * MEMORY_SIZE; addr += 2)
		if (mem_is_valid(memory, addr)) {
			if (monitor_deposit(&monitor, addr, MEM_GET_WORD(memory, addr)))
				return error_set(ERROR_MONITOR, "bootloader_download");
		}
//monitor_trace_dump(stderr) ; //debug
	monitor_close(&monitor);
	return ERROR_OK;
}

int cmd_go(unsigned address, int no_go, serial_device_t *serialdevice,
		monitor_type_t monitor_type) {
	int res = ERROR_OK;
	monitor_t monitor;
	if (monitor_init(&monitor, serialdevice, monitor_type, stdout,
			opt_usbdelay))
		return error_set(ERROR_MONITOR, "monitor_init in bootloader_download");
	if (monitor_assert_prompt(&monitor))
		return error_set(ERROR_MONITOR,
				"Boot loader download: PDP-11 console prompt not found.");
	res = monitor_go(&monitor, address, no_go);

	monitor_close(&monitor);
	return res;
}

/* primitive teletype:
 * User "keyboard" input from stdin, display to stdout,
 * Interface to PDP-11 over pre-initialized "serialdevice".
 * Exit
 */
int cmd_teletype(serial_device_t *serialdevice, monitor_type_t monitor_type,
		char *userinfo) {
#define EXITKEY1 0x01
#define EXITKEY2 0x01

	monitor_t monitor;
	int ready = 0;
	int exitkey1_typed = 0; // 1 if 1st exit hotkey typed

	// no user echo to stdout by monitor_*() functions
	if (monitor_init(&monitor, serialdevice, monitor_type, NULL, opt_usbdelay))
		return error_set(ERROR_MONITOR, "monitor_init in teletype");

	// console is in RAW mode: so do own CR LF
	fprintf(stdout,
			"\r\nTU58FS: teletype session to PDP-11 console opened.\r\n");
	fprintf(stdout, "TU58FS: ^G sound is shown as \"<BEL>\"\r\n");
	fprintf(stdout,
			"TU58FS: Terminate teletype session with ^A ^A double key sequence.\r\n");
	if (userinfo)
		fprintf(stdout, "TU58FS: %s\r\n", userinfo);
	while (!ready) {
		int status;
		fd_set readfds;
		struct timeval timeout;

		/*
		 * 1. read char from PDP, echo to user
		 */
		char *s = monitor_gets(&monitor, 0);
		if (s && strlen(s)) {
			// show unprocessed on Linux console
			fputs(s, stdout);
			fflush(stdout);
		}
		/*
		 * 2. if user keyboard input, xmt to PDP-11 and check for exit hot key
		 */
		// http://stackoverflow.com/questions/26948723/checking-the-stdin-buffer-if-its-empty
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		timeout.tv_sec = timeout.tv_usec = 0; // zero timeout, just poll
		status = select(1, &readfds, NULL, NULL, &timeout); // rtfm
		if (status < 0) {
			error_set(ERROR_TTY, "Teletype keyboard polling,  errno=%u", errno);
		} else if (status > 0) {
			// single char input!
			char c = getchar();

			// test for exit hotkey sequence
			if (exitkey1_typed && c == EXITKEY2) {
				// terminate
				ready = 1;
			} else if (!exitkey1_typed && c == EXITKEY1) {
				exitkey1_typed = 1;
				// do not echo to PDP
			} else {
				char buff[10];
				exitkey1_typed = 0;
				if (c == 0x07)
					strcpy(buff, "<BEL>");
				else {
					// char in buff[0], send it to PDP.
					buff[0] = c;
					buff[1] = 0; // char -> string
				}
				monitor_puts(&monitor, buff);
			}
		}
	}

	monitor_close(&monitor);
	fprintf(stdout, "\r\nTU58FS: teletype session closed.\r\n");
	return ERROR_OK;
}

int main(int argc, char *argv[]) {

	int res;
	serial_device_t monitor_serial; // PDP-11 console

	error_clear();
	ferr = stdout; // ferr in Eclipse console not visible?

	parse_commandline(argc, argv);
	// returns only if everything is OK
	// Std options already executed

	// some debug info
	if (opt_debug) {
		int i ;
		info(version);
		info(copyright);
		fprintf(ferr, "Command line: ") ;
		for (i=0 ; i < argc ; i++)
			fprintf(ferr, "%s ", argv[i]) ;
			fprintf(ferr, "\n") ;
	}

	// load memory
	mem_init(&the_memory);
	switch (opt_fileformat) {
	case fileformat_none:
		if (opt_info && !opt_non_info) // -help/-info printed:  termiante silently
			exit(0);
		else
			fatal("No inputfile given");
		break;
	case fileformat_addr_value_text:
		mem_addr_value_text(&the_memory, opt_filename);
		break;
	case fileformat_macro11_listing:
		mem_load_macro11_listing(&the_memory, opt_filename);
		break;
	case fileformat_papertape:
		mem_load_papertape(&the_memory, opt_filename);
		break;
	default:
		fatal("Unknown file format %d", opt_fileformat);
	}
	if (opt_debug)
		mem_dump(&the_memory, stdout);

	if (opt_monitor == monitor_none) {
		if (opt_info && !opt_non_info) // -help/-info printed: terminate silent
			exit(0);
		else
			fatal("No PDP-11 console monitor seleceted.");
	}

	if (strlen(opt_serial_port) == 0) {
		if (opt_info && !opt_non_info) // -help/-info printed: terminate silent
			exit(0);
		else
			fatal("Serial port not set.");
	}

	// give some info
	info("Using serial port %s at %d baud with %d%c%d format.", opt_serial_port,
			opt_serial_speed, opt_serial_bitcount, opt_serial_parity,
			opt_serial_stopbits);

	serial_devinit(&monitor_serial, opt_serial_port, opt_serial_speed,
			opt_serial_bitcount, opt_serial_parity, opt_serial_stopbits);
	coninit(1); // host console, raw: no processing of PDP-11 I/O

	// download, and echo to console
	res = cmd_deposit(&the_memory, &monitor_serial, opt_monitor);

	if (!res && opt_go != OPT_GO_NOT) {
		// odt_go_address (now) valid ?
		if (opt_go_address == MEMORY_ADDRESS_INVALID)
			opt_go_address = the_memory.entry_address;
		if (opt_go_address == MEMORY_ADDRESS_INVALID) {
			fprintf(stdout, "Can not start, no start address set");
			help();
		}
		res = cmd_go(opt_go_address, opt_go == OPT_GO_COMMITED, &monitor_serial,
				opt_monitor);
		// remember result for tty
	}

	if (opt_tty) {
		if (res) {
			cmd_teletype(&monitor_serial, opt_monitor,
					"PDP-11 does not respond. Use this teletype session to verify console prompt.");
		} else
			// interactive tty terminal on PDP-11 monitor port
			cmd_teletype(&monitor_serial, opt_monitor, NULL);
	}

	// restore serial and console ports
	conrestore();
	serial_devrestore(&monitor_serial);

	return 0;
}
