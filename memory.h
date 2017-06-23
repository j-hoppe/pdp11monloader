/* memory.c - reading and saving several memory file formats
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

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdio.h>

typedef enum {
	fileformat_none = 0,
	fileformat_addr_value_text =1,
	fileformat_macro11_listing =2,
	fileformat_papertape =3
} memory_fileformat_t;


#define MEMORY_ADDRESS_INVALID	0x7fffffff

#define MEMORY_SIZE	0x800000	//128 kwords, 18 bit addresses
#define MEMORY_DATA_MASK	0xffff	// lower 16bits are valid

typedef struct {
	// a cell is undefined, if (cell& MASK) != 0
	// cell idx IS NOT the addr, addr = idx + 2
	unsigned cell[MEMORY_SIZE];
	int entry_address ; // start address, if found. MEMORY_ADDRESS_INVALID = invalid
} memory_t;

#define MEM_GET_WORD(_this,addr) ( (_this)->cell[(addr)/2] )
#define MEM_PUT_WORD(_this, addr, w) ( (_this)->cell[(addr)/2] = (w) )

void mem_init(memory_t * this);
int mem_is_valid(memory_t *_this, unsigned addr);

void mem_addr_value_text(memory_t *_this, char *fname) ;
void mem_load_macro11_listing(memory_t *_this, char *fname) ;
void mem_load_papertape(memory_t *_this, char *fname) ;

void mem_info(memory_t *_this, FILE *f) ;
void mem_dump(memory_t *_this, FILE	*f) ;


#endif /* MEMORY_H_ */
