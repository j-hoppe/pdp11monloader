/*
 * memory.h
 *
 *  Created on: 02.01.2017
 *      Author: root
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
