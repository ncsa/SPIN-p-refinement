// Main.c for converting tet4 to tet10

#include <stdio.h>
#include <time.h>
#include <omp.h>

#include "tet20.h"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		printf("usage: ./toTet20 msh_file.msh\n");
		return 0;
	}
	else
	{
		double begin = omp_get_wtime();
		toTet20( argv[1] );
		double end = omp_get_wtime();
		double time = (end - begin);
		printf("Time: %f\n", time);
	}

	return 0;
}