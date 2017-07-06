// Main.c for propagating BC conditions from *_tet10.msh files

#include <stdio.h>

#include "propagatebc.h"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		printf("usage: ./propBC msh_file_tet10.msh\n");
		return 0;
	}
	else
	{
		propagate( argv[1] );
	}

	return 0;
}