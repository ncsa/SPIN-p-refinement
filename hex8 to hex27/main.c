// Main.c for converting hex8 to hex27

#include <stdio.h>
#include <time.h>

#include "hex8_jk.h"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		printf("usage: ./toHex27 msh_file.msh\n");
		return 0;
	}
	else
	{
		double begin = omp_get_wtime();
		toHex27( argv[1] );
		double end = omp_get_wtime();
		double time = (end - begin);
		printf("Time: %f\n", time);
	}

	return 0;
}