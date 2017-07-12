// Main.c for converting tet4 to tet10

#include <stdio.h>
#include <time.h>

#include "hex8.h"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		printf("usage: ./toHex27 msh_file.msh\n");
		return 0;
	}
	else
	{
		clock_t begin = clock();
		toHex27( argv[1] );
		clock_t end = clock();
		double time = (double)(end - begin) / CLOCKS_PER_SEC;
		//printf("Time: %f\n", time);
	}

	return 0;
}