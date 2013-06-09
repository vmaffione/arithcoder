/*
 *  Bit buffer implementation
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

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <endian.h>

#include <stdio.h>


#define min(a,b) ((a) < (b)) ? (a) : (b)


#define BUFFER_LENGTH 2
struct BitBuffer
{
    int fd;
    int mode;
    uint64_t buffer[ BUFFER_LENGTH ];
    int write_bit_pointer;
    int read_bit_pointer;
};


struct BitBuffer* bit_open(const char* file, BitMode mode)
{
    int fd;
    struct BitBuffer* bbp;
    if (mode != BIT_BUFFER_READ && mode != BIT_BUFFER_WRITE)
    {
	errno = EINVAL;
	return NULL;
    }
    fd = open(file, ((mode == BIT_BUFFER_READ) ? O_RDONLY : O_WRONLY) | O_CREAT, 0644);
    if (fd == -1)
	return NULL;
    bbp = calloc(1, sizeof(struct BitBuffer));
    if (bbp == NULL)
    {
	close(fd);
	errno = ENOMEM;
	return NULL;
    }
    bbp->fd = fd;
    bbp->mode = mode;
    bbp->write_bit_pointer = 0;
    bbp->read_bit_pointer = 0;
    return bbp;
}


int bit_flush_if_full(struct BitBuffer* bbp)
{
    int n;
    int bytesToWrite = sizeof(uint64_t) * BUFFER_LENGTH;
    int offset = 0;
    if (bbp->write_bit_pointer == sizeof(uint64_t) * 8 * BUFFER_LENGTH)
    {
	for (n=0; n<BUFFER_LENGTH; n++)
	    bbp->buffer[n] = htobe64(bbp->buffer[n]);
	while (bytesToWrite)
	{
	    n = write(bbp->fd, ((uint8_t*)bbp->buffer) + offset, bytesToWrite);
	    if (n == -1)
		return -1;
	    offset += n;
	    bytesToWrite -= n;
	}
	// printf("%lu bytes written\n", (sizeof(uint64_t) * BUFFER_LENGTH));
	bbp->write_bit_pointer = 0;
    }
    return 0;
}


int bit_flush(struct BitBuffer* bbp)
{
    int bytesToWrite;
    int offset = 0;
    int n;
    bytesToWrite = bbp->write_bit_pointer / 8;
    if (bbp->write_bit_pointer % 8)
	bytesToWrite++;
    //printf("I'm gonna write %d bytes\n", bytesToWrite);
    for (n=0; n<=bytesToWrite/(sizeof(uint64_t)); n++)
	bbp->buffer[n] = htobe64(bbp->buffer[n]);
    while (bytesToWrite)
    {
	n = write(bbp->fd, ((uint8_t*)bbp->buffer) + offset, bytesToWrite);
	if (n == -1)
	    return -1;
	offset += n;
	bytesToWrite -= n;
    }
    //printf("%d bytes written\n", what?);
    bbp->write_bit_pointer = 0;
    return 0;
}


ssize_t bit_write(struct BitBuffer* bbp, uint64_t data, int n_bit)
{
    int offset;
    int r;
    int m;
    int bits_to_write;
    //uint8_t* p;
    if (bbp->mode != BIT_BUFFER_WRITE)
    {
	errno = EINVAL;
	return -1;
    }

    data <<= (64 - n_bit);  // preliminar shift
    bits_to_write = n_bit;
    offset = bbp->write_bit_pointer / (sizeof(uint64_t) * 8);
    r = bbp->write_bit_pointer % (sizeof(uint64_t) * 8);
    //printf("off=%d r=%d n_bit=%d, data=%016lX\n", offset, r, n_bit, data);
    if (r)
    {
	m = min(bits_to_write, (sizeof(uint64_t) * 8) - r);
	//bbp->buffer[ offset ] |= ((data & (((uint64_t)1 << m) - 1)) << r);
	bbp->buffer[ offset ] |= (data & ((((uint64_t)1 << m) - 1) << (64 - m))) >> r;
	//printf("b.0=%016lX\n", bbp->buffer[ offset ]);
	bbp->write_bit_pointer += m;
	bits_to_write -= m;
	data <<= m;  // flush m bit
    }
    m = bit_flush_if_full(bbp);
    if (m == -1)
	return -1;
    // if ((m = flush_if_full(bbp)) < 0) return -1;
    if (bits_to_write)
    {
	offset = bbp->write_bit_pointer / (sizeof(uint64_t) * 8);
	bbp->buffer[ offset ] = data;
	//printf("b.1=%016lX\n", bbp->buffer[ offset ]);
	bbp->write_bit_pointer += bits_to_write;
	m = bit_flush_if_full(bbp);
	if (m == -1)
	    return -1;
    }

    /*printf("BUF: ");
      p = (uint8_t*)bbp->buffer;
      for (m=0; m<bbp->write_bit_pointer/8+((bbp->write_bit_pointer%8)?1:0); m++)
      printf("%02hX", p[m]);
      printf("\ntot=%d\n", bbp->write_bit_pointer); */
    return n_bit;
}



ssize_t bit_read(struct BitBuffer* bbp, uint64_t* data, int n_bit)
{	
    int n;
    int offset;
    int r;
    int m;
    int bits_to_read;
    int i;
    if (bbp->mode != BIT_BUFFER_READ)
    {
	errno = EINVAL;
	return -1;
    }

    bits_to_read = n_bit;
    offset = bbp->read_bit_pointer / (sizeof(uint64_t) * 8);
    r = bbp->read_bit_pointer % (sizeof(uint64_t) * 8);
    //printf(" off=%d r=%d nbit=%d rp=%d wp=%d ", offset, r, n_bit, bbp->read_bit_pointer, bbp->write_bit_pointer);
    *data = 0x0000000000000000;
    if (r)
    {
	m = min(bits_to_read, (sizeof(uint64_t) * 8) - r);
	m = min(m, bbp->write_bit_pointer - bbp->read_bit_pointer);
	//printf("m=%d mask=%lX", m, ((((uint64_t)1 << m) - 1) << r));
	//*data |= ((bbp->buffer[ offset ] & ((((uint64_t)1 << m) - 1) << r)) >> r);
	*data = (bbp->buffer[ offset ] & ((((uint64_t)1 << m) - 1) << (64 - r - m))) << r;
	bbp->read_bit_pointer += m;
	bits_to_read -= m;
    }
    else
	m = 0;
    if (bbp->read_bit_pointer + bits_to_read > bbp->write_bit_pointer)
    {
	n = read(bbp->fd, bbp->buffer, sizeof(uint64_t) * BUFFER_LENGTH);
	for (i=0; i<BUFFER_LENGTH; i++)
	    bbp->buffer[i] = be64toh(bbp->buffer[i]);
	if (n == -1)
	    return -1;
	bbp->write_bit_pointer = n * 8;
	bbp->read_bit_pointer = 0;
    }
    // buffer-aligned read
    if (bits_to_read && (bbp->write_bit_pointer - bbp->read_bit_pointer) > 0)
    {
	offset = bbp->read_bit_pointer / (sizeof(uint64_t) * 8);
	n = min(bits_to_read, (bbp->write_bit_pointer - bbp->read_bit_pointer));
	//printf("n=%d ", n);
	if (n == sizeof(uint64_t) * 8)
	    *data = bbp->buffer[ offset ];
	else
	    *data |= ((bbp->buffer[ offset ] & ((((uint64_t)1 << n) - 1) << (64 - n))) >> m);
	bbp->read_bit_pointer += n;
	bits_to_read -= n;
    }
    else
	n = 0;
    //printf("\n");
    *data >>= (64 - n - m);

    return n_bit - bits_to_read;
}


int bit_close(struct BitBuffer* bbp)
{
    int m;
    if (bbp->mode == BIT_BUFFER_WRITE)
	m = bit_flush(bbp);
    else
	m = 0;
    close(bbp->fd);
    free(bbp);
    return m;
}



// INVERTED BIT BUFFER

struct InvertingBitBuffer
{
    int fd;
    int mode;
    uint64_t buffer[ BUFFER_LENGTH ];
    int write_bit_pointer;
    int read_bit_pointer;
};


struct InvertingBitBuffer* bit_inv_open(const char* file, BitMode mode)
{
    int fd;
    struct InvertingBitBuffer* bbp;
    if (mode != BIT_BUFFER_READ && mode != BIT_BUFFER_WRITE)
    {
	errno = EINVAL;
	return NULL;
    }
    fd = open(file, ((mode == BIT_BUFFER_READ) ? O_RDONLY : O_WRONLY) | O_CREAT, 0644);
    if (fd == -1)
	return NULL;
    bbp = calloc(1, sizeof(struct InvertingBitBuffer));
    if (bbp == NULL)
    {
	close(fd);
	errno = ENOMEM;
	return NULL;
    }
    bbp->fd = fd;
    bbp->mode = mode;
    bbp->write_bit_pointer = 0;
    bbp->read_bit_pointer = 0;
    return bbp;
}


int bit_inv_flush_if_full(struct InvertingBitBuffer* bbp)
{
    int n;
    int bytesToWrite = sizeof(uint64_t) * BUFFER_LENGTH;
    int offset = 0;
    if (bbp->write_bit_pointer == sizeof(uint64_t) * 8 * BUFFER_LENGTH)
    {
	for (n=0; n<BUFFER_LENGTH; n++)
	    bbp->buffer[n] = htole64(bbp->buffer[n]);
	while (bytesToWrite)
	{
	    n = write(bbp->fd, ((uint8_t*)bbp->buffer) + offset, bytesToWrite);
	    if (n == -1)
		return -1;
	    offset += n;
	    bytesToWrite -= n;
	}
	// printf("%lu bytes written\n", (sizeof(uint64_t) * BUFFER_LENGTH));
	bbp->write_bit_pointer = 0;
    }
    return 0;
}


int bit_inv_flush(struct InvertingBitBuffer* bbp)
{
    int bytesToWrite;
    int offset = 0;
    int n;
    bytesToWrite = bbp->write_bit_pointer / 8;
    if (bbp->write_bit_pointer % 8)
	bytesToWrite++;
    //printf("I'm gonna write %d bytes\n", bytesToWrite);
    for (n=0; n<=bytesToWrite/(sizeof(uint64_t)); n++)
	bbp->buffer[n] = htole64(bbp->buffer[n]);
    while (bytesToWrite)
    {
	n = write(bbp->fd, ((uint8_t*)bbp->buffer) + offset, bytesToWrite);
	if (n == -1)
	    return -1;
	offset += n;
	bytesToWrite -= n;
    }
    //printf("%d bytes written\n", what?);
    bbp->write_bit_pointer = 0;
    return 0;
}


ssize_t bit_inv_write(struct InvertingBitBuffer* bbp, uint64_t data, int n_bit)
{
    int offset;
    int r;
    int m;
    int bits_to_write;
    //uint8_t* p;
    if (bbp->mode != BIT_BUFFER_WRITE)
    {
	errno = EINVAL;
	return -1;
    }

    bits_to_write = n_bit;
    offset = bbp->write_bit_pointer / (sizeof(uint64_t) * 8);
    r = bbp->write_bit_pointer % (sizeof(uint64_t) * 8);
    //printf("off=%d r=%d n_bit=%d, data=%016lX\n", offset, r, n_bit, data);
    if (r)
    {
	m = min(bits_to_write, (sizeof(uint64_t) * 8) - r);
	bbp->buffer[ offset ] |= ((data & (((uint64_t)1 << m) - 1)) << r);
	bbp->write_bit_pointer += m;
	bits_to_write -= m;
	data = data >> m;  // flush m bit
    }
    m = bit_inv_flush_if_full(bbp);
    if (m == -1)
	return -1;
    // if ((m = flush_if_full(bbp)) < 0) return -1;
    if (bits_to_write)
    {
	offset = bbp->write_bit_pointer / (sizeof(uint64_t) * 8);
	bbp->buffer[ offset ] = data;
	bbp->write_bit_pointer += bits_to_write;
	m = bit_inv_flush_if_full(bbp);
	if (m == -1)
	    return -1;
    }
    /*printf("BUF: ");
      p = (uint8_t*)bbp->buffer;
      for (m=0; m<bbp->write_bit_pointer/8+((bbp->write_bit_pointer%8)?1:0); m++)
      printf("%02hX", p[m]);
      printf("\ntot=%d\n", bbp->write_bit_pointer); */
    return n_bit;
}


ssize_t bit_inv_read(struct InvertingBitBuffer* bbp, uint64_t* data, int n_bit)
{	
    int n;
    int offset;
    int r;
    int m;
    int bits_to_read;
    int i;
    if (bbp->mode != BIT_BUFFER_READ)
    {
	errno = EINVAL;
	return -1;
    }

    bits_to_read = n_bit;
    offset = bbp->read_bit_pointer / (sizeof(uint64_t) * 8);
    r = bbp->read_bit_pointer % (sizeof(uint64_t) * 8);
    //printf(" off=%d r=%d nbit=%d rp=%d wp=%d ", offset, r, n_bit, bbp->read_bit_pointer, bbp->write_bit_pointer);
    *data = 0x0000000000000000;
    if (r)
    {
	m = min(bits_to_read, (sizeof(uint64_t) * 8) - r);
	m = min(m, bbp->write_bit_pointer - bbp->read_bit_pointer);
	//printf("m=%d mask=%lX", m, ((((uint64_t)1 << m) - 1) << r));
	*data |= ((bbp->buffer[ offset ] & ((((uint64_t)1 << m) - 1) << r)) >> r);
	bbp->read_bit_pointer += m;
	bits_to_read -= m;
    }
    else
	m = 0;
    if (bbp->read_bit_pointer + bits_to_read > bbp->write_bit_pointer)
    {
	n = read(bbp->fd, bbp->buffer, sizeof(uint64_t) * BUFFER_LENGTH);
	for (i=0; i<BUFFER_LENGTH; i++)
	    bbp->buffer[i] = le64toh(bbp->buffer[i]);
	// for (i=0; i<BUFFER_LENGTH; i++) printf("%016lX", bbp->buffer[i]); printf("  ");
	//printf(" - buffer reload +%d - ", n*8);
	if (n == -1)
	    return -1;
	bbp->write_bit_pointer = n * 8;
	bbp->read_bit_pointer = 0;
    }
    // buffer-aligned read
    if (bits_to_read && (bbp->write_bit_pointer - bbp->read_bit_pointer) > 0)
    {
	offset = bbp->read_bit_pointer / (sizeof(uint64_t) * 8);
	n = min(bits_to_read, (bbp->write_bit_pointer - bbp->read_bit_pointer));
	//printf("n=%d ", n);
	if (n == sizeof(uint64_t) * 8)
	    *data = bbp->buffer[ offset ];
	else
	    *data |= ((bbp->buffer[ offset ] & (((uint64_t)1 << n) - 1)) << m);
	bbp->read_bit_pointer += n;
	bits_to_read -= n;
    }
    //printf("\n");

    return n_bit - bits_to_read;
}


int bit_inv_close(struct InvertingBitBuffer* bbp)
{
    int m;
    if (bbp->mode == BIT_BUFFER_WRITE)
	m = bit_inv_flush(bbp);
    else
	m = 0;
    close(bbp->fd);
    free(bbp);
    return m;
}
