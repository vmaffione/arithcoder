#include <stdint.h>
#include <stdlib.h>

typedef enum BitMode{ BIT_BUFFER_READ = 0, BIT_BUFFER_WRITE } BitMode;

struct BitBuffer;

struct BitBuffer* bit_open( const char* file, BitMode mode );

ssize_t bit_write( struct BitBuffer* bbp, uint64_t data, int n_bit );

ssize_t bit_read( struct BitBuffer* bbp, uint64_t* data, int n_bit );

int bit_close( struct BitBuffer* bbp );


struct InvertingBitBuffer;

struct InvertingBitBuffer* bit_inv_open( const char* file, BitMode mode );

ssize_t bit_inv_write( struct InvertingBitBuffer* bbp, uint64_t data, int n_bit );

ssize_t bit_inv_read( struct InvertingBitBuffer* bbp, uint64_t* data, int n_bit );

int bit_inv_close( struct InvertingBitBuffer* bbp );
