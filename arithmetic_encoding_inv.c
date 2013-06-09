/*
 *  Arithmetic encoding (inverse)
 *
 *  Copyright (C) 2012  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bitbuffer.h"
#include "cumulative_sums_tree.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define NUM_CODEPOINTS 257   // compatible with ASCII encoding
uint64_t total_count;  // current total symbols counter
uint64_t LOW, HIGH;


int arithmetic_encoding_inv(const char* file_to_encode, const char* encoded_file)
  {
	FILE* fin;
	uint8_t symbol_8;
	unsigned int symbol;
	int i;
	uint64_t tmp;
	uint64_t mask;
	uint64_t data;
	struct CST* cumulative_sums_tree_pointer;
	
	fin = fopen(file_to_encode, "r");
	if (fin == NULL)
	  {
		printf("%s: No such file\n", file_to_encode);
		return -1;
	  }
	
	struct InvertingBitBuffer* bbp = bit_inv_open(encoded_file, BIT_BUFFER_WRITE);
	if (bbp == NULL)
		return -1;
	
	cumulative_sums_tree_pointer = CST_create(NUM_CODEPOINTS);
	if (cumulative_sums_tree_pointer == NULL)
	  {
		printf("Error: CST creation failed\n");
		bit_inv_close(bbp);
		return -1;
	  }
	
	// we deal with the zero-frequency problem initializing all counters to 1 and consequently NUM_CODEPOINTS to total_count
	total_count = NUM_CODEPOINTS;
	for (i=1; i<NUM_CODEPOINTS; i++)
		counter_increment(cumulative_sums_tree_pointer, i);
	
	LOW = 0;
	HIGH = 0xFFFFFFFFFFFFFFFF;
	for (;;)
	  {
		i = fread(&symbol_8, 1, 1, fin);
		symbol = (i) ? ((unsigned int)symbol_8) : (NUM_CODEPOINTS - 1);
		
		tmp = ((HIGH - LOW) / total_count);
		HIGH = LOW + tmp * (cumulative_sums_lookup(cumulative_sums_tree_pointer, symbol + 1));
		LOW = LOW + tmp * cumulative_sums_lookup(cumulative_sums_tree_pointer, symbol);
		/*  deadlock resolution
			HIGH == 1000 0000 00...
			LOW  == 0111 1111 11...
		*/
		if ((HIGH & 0xC000000000000000) == 0x8000000000000000 && (LOW & 0xC000000000000000) == 0x4000000000000000)
		  {
			i = 2;
			mask = 0x2000000000000000;
			while (mask && (LOW & mask) && (HIGH & mask) == 0)
			  {
				i++;
				mask >>= 1;
			  }
			if (mask & LOW & HIGH)
				/*  1000..01..  =>  1000..01
					0111..11..  =>  1000..00  */
				data = 0x0000000000000001;
			else
				/*  1000..00..  =>  0111..11
					0111..10..  =>  0111..10
					or
					1000..01..  =>  0111..11  (1000..01)  si potrebbe continuare a scandire per scoprire qual è la scelta migliore
					0111..10..  =>  0111..10  (1000..00)
					  */
				data = ((uint64_t)1 << i) - 2;
			bit_inv_write(bbp, data, i);
			LOW &= ~mask;
			HIGH |= mask;
			HIGH <<= i;
			LOW <<= i;
		  }
		
		tmp = LOW ^ HIGH;
		i = 0;
		mask = 0x8000000000000000;
		data = 0x0000000000000000;
		while (mask && (tmp & mask) == 0)
		  {
			i++;
			data >>= 1;
			if (LOW & mask)
				data |= 0x8000000000000000;
			mask >>= 1;
		  }
		data >>= (sizeof(uint64_t) * 8 - i);
		bit_inv_write(bbp, data, i);
		LOW <<= i;
		HIGH <<= i;
		// adaptive update
		counter_increment(cumulative_sums_tree_pointer, symbol + 1);
		total_count++;
		
		if (symbol == NUM_CODEPOINTS-1)
			break;
	  }
	// final write
	i = 0;
	mask = 0x8000000000000000;
	data = 0x0000000000000000;
	while (LOW)
	  {
		i++;
		data >>= 1;
		if (LOW & mask)
		  {
			data |= 0x8000000000000000;
			LOW &= ~mask;
		  }
		mask >>= 1;
	  }
	data >>= (sizeof(uint64_t) * 8 - i);
	bit_inv_write(bbp, data, i);
	
	bit_inv_close(bbp);
	CST_destroy(cumulative_sums_tree_pointer);
	
	return 0;
  }
