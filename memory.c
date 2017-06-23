/*
 * memory.c
 *
 *  Created on: 02.01.2017
 *      Author: root
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "main.h" // own
#include "memory.h" // own

#define	ASSERT_ADDRESS(addr) assert( (addr)/2 < MEMORY_SIZE)

void mem_init(memory_t *_this) {
	unsigned cellidx;
	for (cellidx = 0; cellidx < MEMORY_SIZE; cellidx++)
		_this->cell[cellidx] = ~MEMORY_DATA_MASK; // set bits 31..16
}

// is cell [addr] initialized?
int mem_is_valid(memory_t *_this, unsigned addr) {
	unsigned cellidx = addr / 2;
	ASSERT_ADDRESS(addr);
	if ((_this->cell[cellidx] & ~MEMORY_DATA_MASK) != 0)
		return 0; // some of bits 31..16 set: invalid
	else
		return 1;
}

/*
 * can set bytes at odd addresses
 */
void mem_put_byte(memory_t *_this, unsigned addr, unsigned b) {
	unsigned w;
	unsigned baseaddr = addr & ~1; // clear bit 0
	ASSERT_ADDRESS(addr);
	w = MEM_GET_WORD(_this, baseaddr);
	if (baseaddr == addr) // set lsb
		w = (w & 0xff00) | b;
	else
		w = (w & 0xff) | (b << 8);
	MEM_PUT_WORD(_this, addr, w);
}

/*
 * textfile with words separated by white string.
 * 1st word = address, following words data.
 * every char other than [0-7] interpreted as white space
 * Prints "hello world":
 | 002000: 012702 177564 012701 002032 112100 001405 110062 000002
 | 002020: 105712 100376 000771 000000 000763 062510 066154 026157
 | 002040: 073440 071157 062154 006441
 * Or:
 | deposit 002000 012702
 | deposit 002002 177564
 | deposit 002004 012701
 | deposit 002006 002032
 | deposit 002010 112100
 | deposit 002012 001405
 | deposit 002014 110062
 | deposit 002016 000002
 | deposit 002020 105712
 | deposit 002022 100376
 | deposit 002024 000771
 | deposit 002026 000000
 | deposit 002030 000763
 | deposit 002032 062510
 | deposit 002034 066154
 | deposit 002036 026157
 | deposit 002040 073440
 | deposit 002042 071157
 | deposit 002044 062154
 | deposit 002046 006441
 *
 */
void mem_addr_value_text(memory_t *_this, char *fname) {
	FILE *fin;
	char linebuff[1024];
	char *s, *token, *endptr;
	int n;
	unsigned val;
	unsigned addr;

	fin = fopen(fname, "r");
	if (!fin) {
		sprintf(linebuff, "Error opening file %s", fname);
		perror(linebuff);
		exit(1);
	}
	_this->entry_address = MEMORY_ADDRESS_INVALID; // not known

	while (fgets(linebuff, sizeof(linebuff), fin)) {
		// make everything ( including \t, \n ,..) but octal digits a whitespace
		for (s = linebuff; *s; s++)
			if (*s < '0' || *s > '7')
				*s = ' ';
		// parse all worlds, n = word index
		n = 0;
		// parse first token = addr
		token = strtok(linebuff, " ");
		/* walk through data tokens */
		while (token != NULL) {
			val = strtol(token, &endptr, 8);
			if (n == 0) // first token = address
				addr = val;
			else { // following tokens are data
				ASSERT_ADDRESS(addr);
				MEM_PUT_WORD(_this, addr, val);
				addr += 2; // inc by word
			}
			n++;
			token = strtok(NULL, " "); // get/test next
		}
	}
	fclose(fin);

}

/*
 * Load from a macro 11 listing
 |
 |M9312 'DD' BOOT prom for TU58 D	MACRO V05.05  Tuesday 23-Mar-99 00:02  Page 1
 |
 |
 |      1
 |      2						.title	M9312 'DD' BOOT prom for TU58 DECtapeII serial tape controller (REVISED)
 |      3
 |      4						; Original edition AK6DN Don North,
 |      5					    ; Further processed JH
 |      6
 |      7						; This source code is an M9312 boot PROM for the TU58 version 23-765B9.
 |      8						;
 |      9						; This boot PROM is for the TU58 DECtapeII serial tape controller.
 |     10						;
 |     11						; Multiple units and/or CSR addresses are supported via different entry points.
 |     12						;
 |     13						; Standard devices are 82S131, Am27S13, 74S571 or other compatible bipolar
 |     14						; PROMs with a 512x4 TriState 16pin DIP architecture. This code resides in
 |     15						; the low half of the device; the top half is blank and unused.
 |     16						;
 |     17						; Alternatively, 82S129 compatible 256x4 TriState 16pin DIP devices can be
 |     18						; used, as the uppermost address line (hardwired low) is an active low chip
 |     19						; select (and will be correctly asserted low).
 |     20
 |     21
 |     22						;VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 |     23						;
 |     24						; The original code in 23-765A9 is REALLY BROKEN when it comes to
 |     25						; supporting a non-std CSR other than 776500 in R1 on entry
 |     26						;
 |     27						; All the hard references to:  ddrbuf, ddxcsr, ddxbuf
 |     28						; have been changed to: 	2(R1),	4(R1),	6(R1)
 |     29						;
 |     30						; The one reference where 'ddrcsr' might have been used is '(R1)' instead
 |     31						; which is actually correct (but totally inconsistent with other usage).
 |     32						;
 |     33						;AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 |     34
 |     35
 |     36		176500 			ddcsr	=176500 			; std TU58 csrbase
 |     37
 |     38		000000 			ddrcsr	=+0				; receive control
 |     39		000002 			ddrbuf	=+2				; receive data
 |     40		000004 			ddxcsr	=+4				; transmit control
 |     41		000006 			ddxbuf	=+6				; transmit data
 |     42
 |     43	000000					.asect
 |     44		010000 				.=10000
 |     45
 |     46						; --------------------------------------------------
 |     47
 |     48	010000				start:
 |     49
 |     50	010000	000261 			dd0n:	sec				; boot std csr, unit zero, no diags
 |     51	010002	012700 	000000 		dd0d:	mov	#0,r0			; boot std csr, unit zero, with diags
 |     52	010006	012701 	176500 		ddNr:	mov	#ddcsr,r1		; boot std csr, unit <R0>
 |     53
 |     54
 |     55					; mov #101,r3	; transmit A
 |     56					; mov r3,@#176506
 |     57					; halt
 |^L M9312 'DD' BOOT prom for TU58 D	MACRO V05.05  Tuesday 23-Mar-99 00:02  Page 1-1
 |
 |
 |     58					;	 call	 xmtch			; transmit unit number
 |     59					; halt
 |     60					; mov #132,r3	; transmit A
 |     61					;	 call	 xmtch			; transmit unit number
 |     62					; halt
 |     63	010012	012706 	002000 		go:	mov	#2000,sp		; setup a stack
 |     64	010016	005004 				clr	r4			; zap old return address
 |     65	010020	005261 	000004 			inc	ddxcsr(r1)		; set break bit
 |     66	010024	005003 				clr	r3			; data 000,000
 |     67	010026	004767 	000050 			call	xmtch8			; transmit 8 zero chars
 |     68	010032	005061 	000004 			clr	ddxcsr(r1)		; clear break bit
 |     69	010036	005761 	000002 			tst	ddrbuf(r1)		; read/flush any stale rx char
 |     70	010042	012703 	004004 			mov	#<010*400>+004,r3	; data 010,004
 |     71	010046	004767 	000034 			call	xmtch2			; transmit 004 (init) and 010 (boot)
 |     72	010052	010003 				mov	r0,r3			; get unit number
 |     73	010054	004767 	000030 			call	xmtch			; transmit unit number
 |     74
 |     75	010060	005003 				clr	r3			; clear rx buffer ptr
 |     76	010062	105711 			2$:	tstb	(r1)			; wait for rcv'd char available
 |     77	010064	100376 				bpl	2$			; br if not yet
 |     78	010066	116123 	000002 			movb	ddrbuf(r1),(r3)+	; store the char in buffer, bump ptr
 |     79	010072	022703 	001000 			cmp	#1000,r3		; hit end of buffer (512. bytes)?
 |     80	010076	101371 				bhi	2$			; br if not yet
 |     81
 |     82					;	 halt
 |     83	010100	005007 				clr	pc			; jump to bootstrap at zero
 |     84
 |     85	010102				xmtch8: ; transmit 4x the two chars in r3
 |     86	010102	004717 				jsr	pc,(pc) 		; recursive call for char replication
 |     87	010104				xmtch4:
 |     88	010104	004717 				jsr	pc,(pc) 		; recursive call for char replication
 |     89	010106				xmtch2: ; transmit 2 chars in lower r3, upper r3
 |     90	010106	004717 				jsr	pc,(pc) 		; recursive call for char replication
 |     91	010110				xmtch:	; xmt char in lower r3, then swap r3
 |     92	010110	105761 	000004 			tstb	ddxcsr(r1)		; wait for xmit buffer available
 |     93	010114	100375 				bpl	xmtch			; br if not yet
 |     94	010116	110361 	000006 			movb	r3,ddxbuf(r1)		; send the char
 |     95	010122	000303 				swab	r3			; swap to other char
 |     96	010124	000207 				return ; rts	pc			; now recurse or return
 |     97
 |     98		000001 				.end
 |^L M9312 'DD' BOOT prom for TU58 D	MACRO V05.05  Tuesday 23-Mar-99 00:02  Page 1-2
 |Symbol table
 |
 |DDCSR = 176500   	DDRCSR= 000000   	DD0D    010002   	START   010000   	XMTCH4  010104
 |DDNR    010006   	DDXBUF= 000006   	DD0N    010000   	XMTCH   010110   	XMTCH8  010102
 |DDRBUF= 000002   	DDXCSR= 000004   	GO      010012   	XMTCH2  010106
 |
 |. ABS.	010126    000	(RW,I,GBL,ABS,OVR)
 |      	000000    001	(RW,I,LCL,REL,CON)
 |Errors detected:  0
 |
 |*** Assembler statistics
 |
 |
 |Work  file  reads: 0
 |Work  file writes: 0
 |Size of work file: 48 Words  ( 1 Pages)
 |Size of core pool: 15104 Words  ( 59 Pages)
 |Operating  system: RT-11
 |
 |Elapsed time: 00:00:09.30
 |DK:DDBOOT,DK:DDBOOT.LST=DK:DDBOOT


 So:
 - in first line, determine width of line numbers
 - strip off line number in every line
 - 1st octal is address, must be 16 bit
 - following octals are data
 - data may be 3 digits, then 8bit, or 6 digits, then 16 bit.
 - multiple data per line allowed
 *
 */

// remove trailing white space by setting a new \0
static void trim_trail(char *line) {
	char *s = line + strlen(line) - 1;

	while (s >= line && isspace(*s)) {
		*s = 0;
		s--;
	}
}

static int calc_lineno_width(char *line) {
	// determine end of
	char *s = line;
	while (*s && isspace(*s))
		s++;
	while (*s && isdigit(*s))
		s++;
	// s now on first char after first number
	return s - line;
}

void mem_load_macro11_listing(memory_t *_this, char *fname) {
	int lineno_fieldwidth = 0;
	FILE *fin;
	char linebuff[1024];
	char *tp; // token pointer, start of cur number
	int tokenidx;
	int ready;
	unsigned val;
	unsigned addr;
	char *endptr;
	int wlen;

	fin = fopen(fname, "r");
	if (!fin) {
		sprintf(linebuff, "Error opening file %s", fname);
		perror(linebuff);
		exit(1);
	}
	_this->entry_address = MEMORY_ADDRESS_INVALID; // not known

	while (fgets(linebuff, sizeof(linebuff), fin)) {
		// remove trailing spaces
		trim_trail(linebuff);

		if (strlen(linebuff) == 0)
			continue; // empty line: next line

		// parse only until "Symbol table" in column 1"
		if (!strncasecmp(linebuff, "Symbol table", 12))
			break; // stop parsing

		if (linebuff[0] != ' ' && linebuff[0] != 9)
			continue; // line starts not with white space: header, no numbered line

		if (lineno_fieldwidth == 0)
			lineno_fieldwidth = calc_lineno_width(linebuff); // only once
		tp = linebuff + lineno_fieldwidth; // skip line number, ptr to cur pos

		// process all octal digit strings
		tokenidx = 0; // # of number processed
		ready = 0;
		addr = 0;
		while (!ready) {
			while (*tp && isspace(*tp))
				tp++; // skip white space
			val = strtol(tp, &endptr, 8);
			wlen = endptr - tp; // len if the numeric word
			if (wlen == 0)
				ready = 1;
			else if (wlen == 6) { // 16 bit value
				if (tokenidx == 0)
					addr = val; // 1st number: address
				else {
					ASSERT_ADDRESS(addr);
					MEM_PUT_WORD(_this, addr, val);
					addr += 2; // inc by word
				}
				tokenidx++;
				tp = endptr; // skip this number
			} else if (wlen == 3) { // 8 bnt value
				ASSERT_ADDRESS(addr);
				mem_put_byte(_this, addr, val);
				addr++; // inc by byte
				tokenidx++;
				tp = endptr; // skip this number
			} else
				ready = 1; // terminate on any unexpected
		}
	}

	fclose(fin);
}

/*
 * Load a DEC Standard Absolute Papertape Image file
 */
void mem_load_papertape(memory_t *_this, char *fname) {
	FILE *fin;
	int b;
	int state = 0;
	int block_byte_idx = 0;
	int block_databyte_count = 0;
	int block_byte_size, sum = 0, addr;
	int stream_byte_index; // absolute index of byte in file

	fin = fopen(fname, "r");
	if (!fin) {
		char buff[1024];
		sprintf(buff, "Error opening file %s", fname);
		perror(buff);
		exit(1);
	}

	_this->entry_address = MEMORY_ADDRESS_INVALID; // not yet known

	stream_byte_index = 0;
	while (!feof(fin)) {
		b = fgetc(fin);
		// fprintf(stderr,
		// 		"[0x%04x] state=%d b=0x%02x sum=0x%02x block_byte_idx=%d\n",
		// 		stream_byte_index, state, b, sum, block_byte_idx);
		stream_byte_index++;
		switch (state) {
		case 0: // skip bytes until 0x01 is found
			sum = 0;
			if (b == 1) {
				state = 1; // valid header: initialize counter and checksum
				block_byte_idx = 1;
				sum += b;
				sum &= 0xff;
			}
			break;
		case 1: // 0x01 is found, check for following 0x00
			if (b != 0)
				state = 0; // invalid start, skip bytes
			else {
				state = 2;
				block_byte_idx++;
				sum += b;
				sum &= 0xff;
			}
			break;
		case 2:
			// read low count byte
			block_byte_size = b;
			state = 3;
			sum += b;
			sum &= 0xff;
			block_byte_idx++;
			break;
		case 3:
			// read high count byte
			block_byte_size = block_byte_size | (b << 8);
			// calculate data word count
			block_databyte_count = (block_byte_size - 6);
			state = 4;
			sum += b;
			sum &= 0xff;
			block_byte_idx++;
			break;
		case 4:
			// read addr low
			addr = b;
			sum += b;
			sum &= 0xff;
			state = 5;
			block_byte_idx++;
			break;
		case 5:
			// read addr high
			addr = addr | (b << 8);
			state = 6;
			sum += b;
			sum &= 0xff;
			block_byte_idx++;
			if (block_byte_idx > block_byte_size) {
				fprintf(stderr,
						"Skipping mis-sized block with addr = %06o, size=%d",
						addr, block_byte_size);
				state = 0;
			} else {
				// block range now known
				// if block size = 0: entry addr
				if (block_databyte_count == 0) {
					_this->entry_address = addr;
					if (opt_debug)
						fprintf(stderr,
								"Empty block with addr = %06o, block_byte_size=%d\n",
								addr, block_byte_size);
					state = 0; // empty block. no chksum ?
				}
				// else fprintf(stderr,
				// 			"Starting data block with addr = %06o, block_byte_size=%d, databyte_count=%d\n",
				// 			addr, block_byte_size, block_databyte_count);
			}
			break;
		case 6: // read data byte
			ASSERT_ADDRESS(addr);
			mem_put_byte(_this, addr, b);
			sum += b;
			sum &= 0xff;
			addr += 1;
			block_byte_idx++;
			if (block_byte_idx >= block_byte_size)
				state = 7; // block end
			break;
		case 7:
			// all words of block read, eval checksum
			sum += b;
			sum &= 0xff;
			if (sum != 0) {
				fprintf(stderr, "Checksum error chsum = %02X\n", sum);
				exit(1);
			}
			// else
			//   fprintf(stderr, "Block end with correct checksum\n");
			sum = 0;
			state = 0;
		}
	}
	// if (_this->entry_address != -1)
	//	fprintf(fout, "%06o\n", _this->entry_address);

	fclose(fin);
}

// show range, count ,start
void mem_info(memory_t *_this, FILE *f) {
	if (!f)
		return;
	unsigned first_addr, last_addr, wordcount;
	unsigned addr;

	first_addr = MEMORY_SIZE + 1; // invalid
	last_addr = 0;
	wordcount = 0;
	for (addr = 0; addr < 2 * MEMORY_SIZE; addr += 2)
		if (mem_is_valid(_this, addr)) {
			if (first_addr > MEMORY_SIZE)
				first_addr = addr;
			last_addr = addr;
			wordcount++;
		}
	if (wordcount == 0)
		fprintf(f, "memory empty\n");
	else {
		fprintf(f, "memory filled from %06o to %06o with %u words", first_addr,
				last_addr, wordcount);
		if (_this->entry_address >= 0)
			fprintf(f, ", entry addr = %06o", _this->entry_address);
		fprintf(f, "\n");
	}
}

void mem_dump(memory_t *_this, FILE *f) {
	unsigned addr;
	for (addr = 0; addr < 2 * MEMORY_SIZE; addr += 2)
		if (mem_is_valid(_this, addr))
			fprintf(f, "%06o %06o\n", addr, MEM_GET_WORD(_this, addr));
}
