/*
 *  Arithmetic decoding (inverse)
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


#define min(a,b) ((a) < (b)) ? (a) : (b)

#define NUM_CODEPOINTS 257   // compatible with ASCII encoding
uint64_t total_count;  // current total symbols counter
uint64_t LOW, HIGH;


static int bs(struct CST* CST_p, uint64_t x)
{
    int l, m, h;
    uint64_t ncs_0, ncs_1, ratio;
    l = 0;
    h = NUM_CODEPOINTS;
    ratio = (HIGH - LOW) / total_count;
    for (;;)
    {
	m = (l + h) / 2;
	ncs_0 = cumulative_sums_lookup(CST_p, m) * ratio + LOW;
	ncs_1 = cumulative_sums_lookup(CST_p, m + 1) * ratio + LOW;
	if (m == NUM_CODEPOINTS-1)
	{
	    if (ncs_0 <= x)
		break;
	}
	else if (ncs_0 <= x && x < ncs_1)
	    break;
	if (x < ncs_0)
	    h = m;
	else
	    l = m + 1;
	if (l >= h)
	    break;
    }
    if (l >= h)
	return -1;
    return m;
}


int arithmetic_decoding_inv(const char* encoded_file, const char* decoded_file)
{
    FILE* fout;
    uint8_t byte;
    int i;
    int i_ss;
    int i_d;
    int n;
    int bitc = 0;
    uint64_t tmp, mask, stream_slice;
    int bits_to_read;
    struct CST* cumulative_sums_tree_pointer;

    struct InvertingBitBuffer* bbp = bit_inv_open(encoded_file, BIT_BUFFER_READ);
    if (bbp == NULL)
    {
	printf("%s: No such file\n", encoded_file);
	return -1;
    }

    cumulative_sums_tree_pointer = CST_create(NUM_CODEPOINTS);
    if (cumulative_sums_tree_pointer == NULL)
    {
	printf("Error: CST creation failed\n");
	bit_inv_close(bbp);
	return -1;
    }

    fout = fopen(decoded_file, "wb");
    if (fout == NULL)
    {
	printf("Error: cannot create output file %s\n", decoded_file);
	return -1;
    }

    // we deal with the zero-frequency problem initializing all counters to 1 and consequently NUM_CODEPOINTS to total_count
    total_count = NUM_CODEPOINTS;
    for (i=1; i<NUM_CODEPOINTS; i++)
	counter_increment(cumulative_sums_tree_pointer, i);

    LOW = 0;
    HIGH = 0xFFFFFFFFFFFFFFFF;
    bits_to_read = sizeof(uint64_t) * 8;
    stream_slice = 0x0000000000000000;
    for (;;)
    {
	n = bit_inv_read(bbp, &tmp, bits_to_read);
	if (n == -1)
	{
	    printf("Error occurred!!\n");
	    return -1;
	}
	// here n could be zero, but this is not a concern
	bitc+=n;

	i = 0;
	mask = 0x0000000000000001;
	while (i < bits_to_read)
	{
	    stream_slice <<= 1;
	    if (tmp & mask)
		stream_slice |= 0x0000000000000001;
	    mask <<= 1;
	    i++;
	}

	n = bs(cumulative_sums_tree_pointer, stream_slice);
	if (n == -1)
	    return -1;
	if (n == NUM_CODEPOINTS - 1)  // EOF symbol received
	    break;
	byte = (uint8_t)n;
	fwrite(&byte, 1, 1, fout);

	tmp = ((HIGH - LOW) / total_count);
	HIGH = LOW + tmp * (cumulative_sums_lookup(cumulative_sums_tree_pointer, byte + 1));
	LOW = LOW + tmp * cumulative_sums_lookup(cumulative_sums_tree_pointer, byte);

	// deadlock resolution
	if ((HIGH & 0xC000000000000000) == 0x8000000000000000 && (LOW & 0xC000000000000000) == 0x4000000000000000)
	{
	    i_d = 2;
	    mask = 0x2000000000000000;
	    while (mask && (LOW & mask) && (HIGH & mask) == 0)
	    {
		i_d++;
		mask >>= 1;
	    }
	    LOW &= ~mask;
	    HIGH |= mask;
	    HIGH <<= i_d;
	    LOW <<= i_d;
	}
	else
	    i_d = 0;

	// stream scaling
	tmp = LOW ^ HIGH;
	i_ss = 0;
	mask = 0x8000000000000000;
	while (mask && (tmp & mask) == 0)
	{
	    i_ss++;
	    mask >>= 1;
	}
	bits_to_read = i_ss + i_d;  // for the next cycle
	LOW <<= i_ss;
	HIGH <<= i_ss;

	// adaptive update
	counter_increment(cumulative_sums_tree_pointer, byte + 1);
	total_count++;
    }

    bit_inv_close(bbp);
    CST_destroy(cumulative_sums_tree_pointer);
    fclose(fout);

    return 0;
}
