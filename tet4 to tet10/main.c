// Main.c for converting tet4 to tet10

#include <stdio.h>

#include "tet4.h"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		printf("usage: ./toTet10 msh_file.msh\n");
		return 0;
	}
	else
	{
		toTet10( argv[1] );
	}

	return 0;
}