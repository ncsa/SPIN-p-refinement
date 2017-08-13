// Main.c for converting tet4 to tet10

#include <stdio.h>
#include <time.h>
#include <omp.h>

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
		double begin = omp_get_wtime();
		toTet10( argv[1] );
		double end = omp_get_wtime();
		double time = (end - begin);
		printf("Time: %f\n", time);
	}

	return 0;
}