#include <stdio.h>
#include <sys/time.h>

#include "arithmetic_encoding.h"
#include "arithmetic_decoding.h"


struct timeval time_diff( struct timeval* t_finish_p, struct timeval* t_start_p )
  {
	struct timeval result;
	if ( t_finish_p->tv_sec == t_start_p->tv_sec )
	  {
		result.tv_sec = 0;
		result.tv_usec = t_finish_p->tv_usec - t_start_p->tv_usec;
	  }
	else if ( t_finish_p->tv_usec >= t_start_p->tv_usec )
	  {
		result.tv_sec = t_finish_p->tv_sec - t_start_p->tv_sec;
		result.tv_usec = t_finish_p->tv_usec - t_start_p->tv_usec;
	  }
	else
	  {
		result.tv_sec = t_finish_p->tv_sec - t_start_p->tv_sec - 1;
		result.tv_usec = t_finish_p->tv_usec + 1000000 - t_start_p->tv_usec;
	  }
	return result;
  }

//#define INV

#ifdef INV
	#define ENCODING_FUNCTION arithmetic_encoding_inv
	#define DECODING_FUNCTION arithmetic_decoding_inv
#else
	#define ENCODING_FUNCTION arithmetic_encoding
	#define DECODING_FUNCTION arithmetic_decoding
#endif

int main()
  {
	int result;
	struct timeval t_start;
	struct timeval t_finish;
	struct timeval delta_t;
	
	gettimeofday( &t_start, NULL );
	result = ENCODING_FUNCTION( "original", "compressed" );
	if ( result == -1 )
		return -1;
	gettimeofday( &t_finish, NULL );
	delta_t = time_diff( &t_finish, &t_start );
	printf( "enc: %us + %uus\n", ( unsigned int )delta_t.tv_sec, ( unsigned int )delta_t.tv_usec );
	
	gettimeofday( &t_start, NULL );
	result = DECODING_FUNCTION(  "compressed", "decoded" );
	if ( result == -1 )
		return -1;
	gettimeofday( &t_finish, NULL );
	delta_t = time_diff( &t_finish, &t_start );
	printf( "dec: %us + %uus\n", ( unsigned int )delta_t.tv_sec, ( unsigned int )delta_t.tv_usec );
	
	return 0;
  }
