#include "boundcond.h"

// Rewrite to write to msh files

void addBC( const char * vtk_file )
{
	FILE * vtk = fopen( vtk_file, "a+" );
	if( vtk == NULL )
	{
		printf("Error opening %s\n", vtk_file);
		return;
	}

	printf("Input type of BC (ex: \"Emat int\"): ");
	char type[25];
	char val_type[10];
	scanf( "%s", type );
	scanf("%s", val_type);
	fflush(stdin);

	int num_nodes = getNumNodes(vtk);
	
	fprintf(vtk, "\nPOINT_DATA        %d\n", num_nodes);
	fprintf(vtk, "SCALARS %s %s\n", type, val_type);
	fprintf(vtk, "LOOKUP_TABLE default\n");

	char nodes[100];
	float fvalue;
	int ivalue;
	printf("\nEnter desired nodes and hit enter\n");
	printf("Then enter desired boundary condition value and hit enter again\n");
	printf("Repeat this in order until done, then hit CTRL-D\n\n");
	printf("EX:\nNodes: 0-2\n> 2.0\nNodes: 3-5\n> 1.0\n");

	printf("Nodes > ");

	while( scanf("%s", nodes) != EOF )
	{
		int first, second;
		sscanf( nodes, "%d-%d", &first, &second);

		printf("Value > ");
		if( strcmp(val_type,"int") == 0)
		{
			scanf("%d", &ivalue);
			printf("~Adding boundary conditions~\n");
			// Add nodes and values to VTK file
			addIntPointData( vtk, first, second, ivalue );
		}
		else	
		{
			scanf("%f", &fvalue);
			printf("~Adding boundary conditions~\n");
			// Add nodes and values to VTK file
			addFloatPointData( vtk, first, second, fvalue );
		}


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

void addFloatPointData( FILE* vtk, int first, int second, float val )
{
	int num_points = second - first;

	for(int i = 0; i <= num_points; i++)
	{
		fprintf(vtk, "%f\n", val);
	}
}

void addIntPointData( FILE* vtk, int first, int second, int val )
{
	int num_points = second - first;

	for(int i = 0; i <= num_points; i++)
	{
		fprintf(vtk, "%d\n", val);
	}
}