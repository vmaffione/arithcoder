/*
 *  Arithmetic encoder/decoder main program
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "arithmetic_encoding.h"
#include "arithmetic_decoding.h"


void usage()
{
    fprintf(stderr, "USAGE: arith [e|d] INPUTFILE\n");
    exit(1);
}

struct timeval time_diff(struct timeval* t_finish_p, struct timeval* t_start_p)
{
    struct timeval result;
    if (t_finish_p->tv_sec == t_start_p->tv_sec)
    {
	result.tv_sec = 0;
	result.tv_usec = t_finish_p->tv_usec - t_start_p->tv_usec;
    }
    else if (t_finish_p->tv_usec >= t_start_p->tv_usec)
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

int main(int argc, char **argv)
{
    int result;
    char function;
    int len;
    char *outfile;
    struct timeval t_start;
    struct timeval t_finish;
    struct timeval delta_t;

    if (argc != 3)
	usage();
    
    function = *(argv[1]);
    if (strlen(argv[1]) != 1 || (function != 'd' && function != 'e'))
	usage();

    if (access(argv[2], F_OK) < 0) {
	fprintf(stderr, "%s: no such file\n", argv[2]);
	exit(1);
    }

    len = strlen(argv[2]);
    //if (argv[2][len-2] == '.' && argv[2][len-1] == 'e' && function == 'd')
    outfile = malloc(len + 2 + 1);
    memcpy(outfile, argv[2], len);
    outfile[len] = '.';
    outfile[len+1] = function;
    outfile[len+2] = '\0';

    gettimeofday(&t_start, NULL);
    if (function == 'e') {
	result = ENCODING_FUNCTION(argv[2], outfile);
    } else {/* if (function == 'd') */
	result = DECODING_FUNCTION(argv[2], outfile);
    }
    if (result == -1)
	return -1;
    gettimeofday(&t_finish, NULL);
    delta_t = time_diff(&t_finish, &t_start);
    printf("spent: %us + %uus\n", (unsigned int)delta_t.tv_sec, (unsigned int)delta_t.tv_usec);

    return 0;
}
