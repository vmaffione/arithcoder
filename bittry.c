/*
 *  Testsuite for the bitbuffer module
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

#include <stdio.h>
#include <stdint.h>

#include "bitbuffer.h"

int main()
{
    uint64_t a = 0x0011223344556677,
	     b = 0x8899AABBCCDDEEFF,
	     d;
    uint8_t byte;
    FILE* fin;
    struct BitBuffer* bbp;
    bbp = bit_open("fifo", BIT_BUFFER_WRITE, NON_INVERTING);
    d = (a >> (64-5));
    printf("current=%016lX\n", d);
    bit_write(bbp, d, 5);
    d = (a >> (64-5-19));
    printf("current=%016lX\n", d);
    bit_write(bbp, d, 19);
    d = (a >> (64-5-19-1));
    printf("current=%016lX\n", d);
    bit_write(bbp, d, 1);
    d = (a >> (64-5-19-1-25));
    printf("current=%016lX\n", d);
    bit_write(bbp, d, 25);
    d = (a >> (64-5-19-1-25-2));
    printf("current=%016lX\n", d);
    bit_write(bbp, d, 2);
    d = (a >> (64-5-19-1-25-2-10));
    printf("current=%016lX\n", d);
    bit_write(bbp, d, 10);

    d = ((a << 5) | (b >> (64 - 5)));
    bit_write(bbp, d, 7);

    d = (b >> (64-5-18));
    bit_write(bbp, d, 18);
    d = (b >> (64-5-18-1));
    bit_write(bbp, d, 1);
    d = (b >> (64-5-18-1-23));
    bit_write(bbp, d, 23);
    d = (b >> (64-5-18-1-23-9));
    bit_write(bbp, d, 9);
    //d = (b >> (64-5-18-1-23-9-8)); bit_write(bbp, d, 8);
    bit_close(bbp);

    fin = fopen("fifo", "r");
    while (fread(&byte, 1, 1, fin))
    {
	printf("%02hX", byte);
    }
    /*fread(&d, 8, 1, fin);
      printf("(1)  %016lX\n", d);
      fread(&d, 8, 1, fin);
      printf("(2)  %016lX\n", d);*/
    printf("\n");
    fclose(fin);

    bbp = bit_open("fifo", BIT_BUFFER_READ, NON_INVERTING);
    bit_read(bbp, &d, 15);
    printf("%016lX\n", d);
    bit_read(bbp, &d, 9);
    printf("%016lX\n", d);
    bit_read(bbp, &d, 12);
    printf("%016lX\n", d);
    bit_read(bbp, &d, 20);
    printf("%016lX\n", d);
    bit_read(bbp, &d, 16);
    printf("%016lX\n", d);
    bit_read(bbp, &d, 4);
    printf("%016lX\n", d);
    bit_read(bbp, &d, 12);
    printf("%016lX\n", d);
    bit_read(bbp, &d, 20);
    printf("%016lX\n", d);
    bit_read(bbp, &d, 12);
    printf("%016lX\n", d);
    /*
       bit_read(bbp, &d, 64);
       printf("%016lX\n", d);
       bit_read(bbp, &d, 56);
       printf("%016lX\n", d); */
    bit_close(bbp);

    return 0;
}
