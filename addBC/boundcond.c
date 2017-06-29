#include "boundcond.h"

void addBC( const char * vtk_file )
{
	FILE * vtk = fopen( vtk_file, "a+" );
	if( vtk == NULL )
	{
		printf("Error opening %s\n", vtk_file);
		return;
	}

	printf("Input type of BC (ex: \"Emat int\"): ");
	char type[100];
	scanf( "%[^\t\n]", type );

	char nodes[100];
	char value[100];
	printf("\nEnter desired nodes and hit enter\n");
	printf("Then enter desired boundary condition value and hit enter again\n");
	printf("Repeat this in order until done, then hit CTRL-D\n\n");
	printf("EX:\nNodes: 0-2\n> 2.0\nNodes: 3-5\n> 1.0\n");

	while( !EOF )
	{
		scanf("%s", nodes);
		scanf("%s", value);
	}
}