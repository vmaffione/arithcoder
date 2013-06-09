#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

struct CST
  {
	uint64_t* tree;
	unsigned int num_codepoints;
  };

struct CST* CST_create( int num_codepoints )
  {
	struct CST* p;
	if ( num_codepoints < 1 )
	  {
		errno = EINVAL;
		return NULL;
	  }
	p = malloc( sizeof( struct CST ) );
	if ( p == NULL )
	  {
		errno = ENOMEM;
		return NULL;
	  }
	p->num_codepoints = num_codepoints;
	p->tree = calloc( num_codepoints, sizeof( uint64_t ) );
	if ( p->tree == NULL )
	  {
		free( p );
		errno = ENOMEM;
		return NULL;
	  }
	return p;
  }

void CST_destroy( struct CST* CST_p )
  {
	free( CST_p->tree );
	free( CST_p );
  }

uint64_t cumulative_sums_lookup( struct CST* CST_p, unsigned int symbol )
  {
	uint64_t mask = 0x0000000000000001;
	uint64_t result = 0;
	uint64_t symbol_64;
	symbol_64 = ( uint64_t )symbol;
	symbol_64++;
	while ( symbol_64 )
	  {
		if ( mask & symbol_64 )
		  {
			result += CST_p->tree[ symbol_64 - 1 ];
			symbol_64 &= ~mask;
		  }
		mask <<= 1;
	  }
	return result;
  }

void counter_increment( struct CST* CST_p, unsigned int symbol )
  {
	uint64_t mask = 0x0000000000000001;
	uint64_t symbol_64;
	symbol_64 = ( uint64_t )symbol;
	symbol_64++;
	while ( symbol_64 <= CST_p->num_codepoints )
	  {
		if ( mask & symbol_64 )
		  {
			CST_p->tree[ symbol_64 - 1 ]++;
			symbol_64 += mask;
		  }
		mask <<= 1;
	  }
  }
