// Main.c for converting tet4 to tet10

#include <stdio.h>

#include "boundcond.h"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		printf("usage: ./addBC vtk_file.vtk\n");
		return 0;
	}
	else
	{
		addBC( argv[1] );
	}

	return 0;
}