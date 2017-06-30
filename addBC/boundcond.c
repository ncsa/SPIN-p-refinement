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
	fflush(stdin);

	int num_nodes = getNumNodes(vtk);
	
	fprintf(vtk, "\nPOINT_DATA        %d\n", num_nodes);
	fprintf(vtk, "SCALARS %s\n", type);
	fprintf(vtk, "LOOKUP_TABLE default\n");

	char nodes[100];
	float value;
	printf("\nEnter desired nodes and hit enter\n");
	printf("Then enter desired boundary condition value and hit enter again\n");
	printf("Repeat this in order until done, then hit CTRL-D\n\n");
	printf("EX:\nNodes: 0-2\n> 2.0\nNodes: 3-5\n> 1.0\n");

	printf("Nodes > ");

	while( scanf("%s", nodes) != EOF )
	{
		printf("Value > ");
		scanf("%f", &value);

		int first, second;
		sscanf( nodes, "%d-%d", &first, &second);
		printf("~Adding boundary conditions~\n");
		// Add nodes and values to VTK file
		addPointData( vtk, first, second, value );

		printf("Nodes (or CTRL-D) > ");
	}
}

int getNumNodes( FILE * vtk )
{
	char buff[100];
	rewind(vtk);
	fgets( buff, 100, vtk );
	while( strcmp(buff, "DATASET UNSTRUCTURED_GRID\n") != 0 )
	{
		
		fgets( buff, 100, vtk );
	}

	int num_nodes;
	fgets( buff, 100, vtk );
	sscanf(buff, "%*s %d %*s", &num_nodes);

	return num_nodes;
}

void addPointData( FILE* vtk, int first, int second, float val )
{
	int num_points = second - first;

	for(int i = 0; i <= num_points; i++)
	{
		fprintf(vtk, "%f\n", val);
	}
}