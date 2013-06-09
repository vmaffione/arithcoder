/*
 *  Bitbuffer
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

#include <stdint.h>
#include <stdlib.h>

typedef enum BitMode{ BIT_BUFFER_READ = 0, BIT_BUFFER_WRITE } BitMode;

struct BitBuffer;

struct BitBuffer* bit_open(const char* file, BitMode mode);

ssize_t bit_write(struct BitBuffer* bbp, uint64_t data, int n_bit);

ssize_t bit_read(struct BitBuffer* bbp, uint64_t* data, int n_bit);

int bit_close(struct BitBuffer* bbp);


struct InvertingBitBuffer;

struct InvertingBitBuffer* bit_inv_open(const char* file, BitMode mode);

ssize_t bit_inv_write(struct InvertingBitBuffer* bbp, uint64_t data, int n_bit);

ssize_t bit_inv_read(struct InvertingBitBuffer* bbp, uint64_t* data, int n_bit);

int bit_inv_close(struct InvertingBitBuffer* bbp);
