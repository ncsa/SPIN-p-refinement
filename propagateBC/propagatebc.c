#include "propagatebc.h"

void propagate( char* msh_file )
{
	FILE * msh = fopen( msh_file, "a+" );
	if( msh == NULL )
	{
		printf("Error opening %s\n", msh_file);
		return;
	}
	fseek(msh, 0, SEEK_SET);
	// get num nodes
	int num_nodes;
	char buff[255];

	while( strcmp(buff, "$Nodes\n") != 0 )
	{
		fgets( buff, 255, msh );
	}

	fgets( buff, 255, msh );
	num_nodes = atoi(buff);

	double* node_data[num_nodes];
	for(int i = 0; i < num_nodes; i++)
	{
		node_data[i] = NULL;
	}

	while( strcmp(buff, "$Elements\n") != 0 )
	{
		fgets(buff, 255, msh);
	}

	fgets(buff, 255, msh);
	int num_elements = atoi(buff);
	char* elements[num_elements];

	for(int i = 0; i < num_elements; i++)
	{
		fgets(buff, 255, msh);
		elements[i] = strdup(buff);
	}

	// Get BCs of original nodes
	while( strcmp(buff, "$NodeData\n") != 0 )
	{
		fgets(buff, 255, msh);
	}

	// Loop through etc. information
	fgets(buff, 255, msh);
	int num_tag = atoi(buff);
	for(int i = 0; i < num_tag; i++)
	{
		fgets(buff, 255, msh);
	}
	fgets(buff, 255, msh);
	num_tag = atoi(buff);
	for(int i = 0; i < num_tag; i++)
	{
		fgets(buff, 255, msh);
	}
	fgets(buff, 255, msh);
	num_tag = atoi(buff);
	for(int i = 0; i < num_tag; i++)
	{
		fgets(buff, 255, msh);
	}


	// Collect BC data and populate node_data[]
	fgets(buff, 255, msh);
	int orig_nodes = 0;
	while( (strcmp(buff, "$EndNodeData") != 0) && (strcmp(buff, "$EndNodeData\n") != 0)  )
	{
		double *bc_data = malloc(sizeof(double*));
		sscanf(buff, "%*d %lf", bc_data);

		node_data[orig_nodes] =  bc_data;
		orig_nodes++;

		fgets(buff, 255, msh);
	}

	int nodes[10];
	// Calculate new BC data
	for(int i = 0; i < num_elements; i++)
	{
		char* elem = elements[i];
		sscanf(elem, "%*s %*s %d %[^\t\n]", &num_tag, elem);
		for(int j = 0; j < num_tag; j++)
		{
			sscanf(elem, "%*s %[^\t\n]", elem);
		}

		sscanf(elem, "%d %d %d %d %d %d %d %d %d %d", &nodes[0], &nodes[1], &nodes[2], &nodes[3], &nodes[4], &nodes[5], &nodes[6], &nodes[7], &nodes[8], &nodes[9]);

		// Start at nodes[4] and calc
		if( node_data[nodes[4] - 1] == NULL )
		{
			node_data[nodes[4] - 1] = malloc(sizeof(double*));
			*node_data[nodes[4] - 1] = avg( *node_data[nodes[0] - 1], *node_data[nodes[1] - 1] );
		}
		if( node_data[nodes[5] - 1] == NULL )
		{
			node_data[nodes[5] - 1] = malloc(sizeof(double*));
			*node_data[nodes[5] - 1] = avg( *node_data[nodes[1] - 1], *node_data[nodes[2] - 1] );
		}
		if( node_data[nodes[6] - 1] == NULL )
		{
			node_data[nodes[6] - 1] = malloc(sizeof(double*));
			*node_data[nodes[6] - 1] = avg( *node_data[nodes[0] - 1], *node_data[nodes[2] - 1] );
		}
		if( node_data[nodes[7] - 1] == NULL )
		{
			node_data[nodes[7] - 1] = malloc(sizeof(double*));
			*node_data[nodes[7] - 1] = avg( *node_data[nodes[0] - 1], *node_data[nodes[3] - 1] );
		}
		if( node_data[nodes[8] - 1] == NULL )
		{
			node_data[nodes[8] - 1] = malloc(sizeof(double*));
			*node_data[nodes[8] - 1] = avg( *node_data[nodes[2] - 1], *node_data[nodes[3] - 1] );
		}
		if( node_data[nodes[9] - 1] == NULL )
		{
			node_data[nodes[9] - 1] = malloc(sizeof(double*));
			*node_data[nodes[9] - 1] = avg( *node_data[nodes[1] - 1], *node_data[nodes[3] - 1] );
		}
	}

	char* _msh_file = strdup(msh_file);
	_msh_file[ strlen(_msh_file) - 4] = '\0';
	char new_msh_file[255];
	snprintf( new_msh_file, 255, "%s_BC.msh", _msh_file );
	FILE * new_msh = fopen( new_msh_file, "w+" );

	fseek( msh, SEEK_SET, 0 );
	while(strcmp(buff, "$NodeData\n") != 0 )
	{
		fgets(buff, 255, msh);
		fprintf(new_msh, "%s", buff);
	}
	fgets(buff, 255, msh);
	fprintf(new_msh, "%s", buff);
	num_tag = atoi(buff);
	for(int i = 0; i < num_tag; i++)
	{
		fgets(buff, 255, msh);
		fprintf(new_msh, "%s", buff);
	}
	fgets(buff, 255, msh);
	fprintf(new_msh, "%s", buff);
	num_tag = atoi(buff);
	for(int i = 0; i < num_tag; i++)
	{
		fgets(buff, 255, msh);
		fprintf(new_msh, "%s", buff);
	}
	fgets(buff, 255, msh);
	fprintf(new_msh, "%s", buff);
	num_tag = atoi(buff);
	for(int i = 0; i < num_tag; i++)
	{
		fgets(buff, 255, msh);
		fprintf(new_msh, "%s", buff);
	}

	for(int i = 0; i < num_nodes; i++)
	{
		fprintf(new_msh, "%d %f\n", (i+1), *node_data[i]);
	}

	fprintf(new_msh, "$EndNodeData\n");

}

double avg( double a, double b )
{
	return (a+b) / 2.0;
}


