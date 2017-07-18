// Main.c for converting tet4 to tet10

#include <stdio.h>
#include <time.h>

#include "tet4_parallel.h"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		printf("usage: ./toTet10 msh_file.msh\n");
		return 0;
	}
	else
	{
		clock_t begin = clock();
		toTet10( argv[1] );
		clock_t end = clock();
		double time = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Time: %f\n", time);
	}

	return 0;
}