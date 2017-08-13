#include "tet4_parallel.h"

/*
*   Program to convert tet4 elements in mesh to tet10 elements
*   Writes to new file name (msh_file)_tet10.msh
*   @para msh_file original .msh file containing mesh
*/
// gcc tet4.c main.c -o toTet10

static int num_edges = 0;
static int edge_id = 0;

void toTet10( const char* msh_file )
{
	double begin = omp_get_wtime();
	FILE * msh = fopen( msh_file, "r+" );
	if( msh == NULL )
	{
		printf("Error opening %s\n", msh_file);
		return;
	}
	
	char buff[255];

	while( strcmp( buff, "$Nodes\n") != 0 )
	{
		fgets(buff, 255, msh);
	}

	fgets(buff, 255, msh);
	int num_nodes = atoi(buff);

	edge_id = num_nodes + 1;

	// Read node lines and store in nodes array
	char** nodes = malloc(num_nodes * sizeof(char*));
	for(int i = 0; i < num_nodes; i++)
	{
		fscanf(msh, "%s ", buff);
		fgets(buff, 255, msh);

		nodes[i] = strdup(buff);
	}

	while( strcmp(buff, "$Elements\n") != 0 )
	{
		fgets(buff, 255, msh);
	}
	
	fgets(buff, 255, msh);
	int num_elements = atoi(buff);

	// ALLOCATE MEMORY FOR EDGE AND ELEMENTS

	char ** all_coords = malloc(num_elements * 6 * sizeof(char*));

	char** elements = malloc((num_elements) * sizeof(char*));

	edge_t** edges = (edge_t**) calloc(num_nodes, sizeof(edge_t));
	for(int i = 0; i < num_nodes; i++)
	{
		edges[i] = (edge_t*) calloc(num_nodes, sizeof(edge_t));
	}

	double end = omp_get_wtime();

	printf("Memory Set-up: %f\n", (double)(end-begin));

	begin = omp_get_wtime();

	int num_tet4 = 0;

	// This should probably be fixed
	// adds an extra unneccessary(?) iteration
	for(int i = 0; i < num_elements; i++)
	{
		fgets(buff, 255, msh);
		int elem_id;
		int num_tags;
		char* elem = strdup(buff);
		elements[i] = strdup(buff);

		char * elem_frag = (char*) malloc(255);
		sscanf( elem, "%*d %d %d %[^\t\n]", &elem_id, &num_tags, elem_frag );
		for(int j = 0; j < num_tags; j++)
		{
			sscanf(elem_frag, "%*d %[^\t\n]", elem_frag);
		}

		if(elem_id == 4)
		{
			num_tet4++;
		}
	}

	end = omp_get_wtime();
	printf("Elem Iter Time: %lf\n", (end-begin));

	begin = omp_get_wtime();
	// Read element lines and store in elements[]
	#pragma omp parallel for schedule(static) shared(edge_id, num_edges)
	for(int i = num_elements - num_tet4; i < num_elements; i++)
	{
		int elem_id;
		int num_tags;
		char* elem = elements[i];

		char elem_frag[255];
		sscanf( elem, "%*d %d %d %[^\t\n]", &elem_id, &num_tags, elem_frag );
		for(int k = 0; k < num_tags; k++)
		{
			sscanf(elem_frag, "%*d %[^\t\n]", elem_frag);
		}

		// If the current element is a TET4
		if(elem_id == 4)
		{
			int n1, n2, n3, n4;
			sscanf(elem_frag, "%d %d %d %d", &n1, &n2, &n3, &n4);

			char * new_elem = constructElem( elem_frag, i, num_nodes, edges, num_elements, all_coords, nodes, n1, n2, n3, n4 );
			elements[i] = malloc(strlen(new_elem) + 1);
			strcpy(elements[i], new_elem);
			// Fix this maybe ... inefficient
		}
	}
	end = omp_get_wtime();

	printf("Nodes: %d\n", num_nodes + num_edges);

	printf("Elem Construct Time: %lf\n", end-begin);

	// All new elements and nodes collected
	// Rewrite to msh file and convert to vtk

	char* _msh_file = strdup(msh_file);
	_msh_file[ strlen(_msh_file) - 4] = '\0';
	char new_msh_file[255];
	snprintf( new_msh_file, 255, "%s_tet10.msh", _msh_file );
	FILE * new_msh = fopen( new_msh_file, "w+" );

	fputs("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n$Nodes\n", new_msh);

	fprintf(new_msh, "%d\n", ( num_nodes + num_edges ));

	for(int i = 0; i < num_nodes; i++)
	{
		fprintf(new_msh, "%d %s", (i+1), nodes[i]);
		free(nodes[i]);
	}

	for(int i = num_nodes + 1; i < num_edges + num_nodes + 1; i++)
	{
		char * curr_node = all_coords[i];
		fprintf(new_msh, "%s", curr_node);
		free(curr_node);
	}
	fputs("$EndNodes\n$Elements\n", new_msh);
	fprintf(new_msh, "%d\n", num_elements);

	for(int i = 0; i < num_elements; i++)
	{
		fprintf(new_msh, "%s", elements[i]);
		free(elements[i]);
	}

	fputs("$EndElements\n", new_msh);
	fgets(buff, 255, msh);

	while( fgets(buff, 255, msh) != NULL )
	{
		fprintf(new_msh, "%s", buff);
	}
	
	fclose( new_msh );
	fclose( msh );
}

char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, int num_elem, char* all_coords[num_elem * 6], char ** nodes, int n1, int n2, int n3, int n4 )
{
	int ids[6];

	// Node ordering based off of .msh file format

	// Edge nodes
	// edge_5
	ids[0] = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n1, n2 );

	// edge_6
	ids[1] = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n2, n3 );

	// edge_7
	ids[2] = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n1, n3 );

	// edge_8
	ids[3] = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n1, n4 );

	// edge_9
	ids[4] = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n3, n4 );

	// edge_10
	ids[5] = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n2, n4 );

	char* new_elem = malloc(255);
	snprintf( new_elem, 255, "%d 11 2 0 0 %s %d %d %d %d %d %d\n", elem_id + 1, elem_frag, ids[0], ids[1], ids[2], ids[3], ids[4], ids[5] );

	return new_elem;
}

// NEED TO FIX RACE CONDITIONS
// need to lock only when element is not found so it can be created without another thread reading
// try and get rid of critical sections (reimplement?)
int getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*6], char ** nodes, int n1, int n2 )
{
	edge_t curr_edge;

		// If two threads read the same edge at the same time it is race, this is v. slow
		// Could try and insure no two threads every have same n1, n2
		curr_edge = getEdge( n1, n2, edges );
	
		if( curr_edge.inUse == 0 )
		{
			int retest = 0;
			// Race condition
			#pragma omp critical
			{
				// Retest edge
				curr_edge = getEdge( n1, n2, edges );
				if(curr_edge.inUse == 0)
				{
					curr_edge.node_id = edge_id;
					edge_id++;
					curr_edge.inUse = 1;
					// Store in edges
					if( n1 < n2 )
					{
						edges[n1 - 1][n2 - 1] = curr_edge;
					}
					else
					{
						edges[n2 - 1][n1 - 1] = curr_edge;
					}
				}
				else
				{
					retest = 1;
				}
			}

			if(retest == 1)
			{
				printf("found duplicate\n");
				return curr_edge.node_id;
			}

			#pragma omp atomic update
			num_edges++;

			// Set coords
			char* node_1 = nodes[n1 - 1];
			char* node_2 = nodes[n2 - 1];
			double x1, x2, y1, y2, z1, z2;
			sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
			sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
			curr_edge.x = avg(x1, x2);
			curr_edge.y = avg(y1, y2);
			curr_edge.z = avg(z1, z2);
			curr_edge.inUse = 1;

			// Store in edges
			if( n1 < n2 )
			{
				edges[n1 - 1][n2 - 1] = curr_edge;
			}
			else
			{
				edges[n2 - 1][n1 - 1] = curr_edge;
			}

			char * new_node = malloc(100);
			snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_edge.node_id, curr_edge.x, curr_edge.y, curr_edge.z);

			if(all_coords[curr_edge.node_id] != NULL)
				printf("overwriting %d\n", curr_edge.node_id);

			all_coords[curr_edge.node_id] = new_node;
		}

	return curr_edge.node_id;
}

// Lookup edge in edges
edge_t getEdge( int n1, int n2, edge_t ** edges )
{
	if( n1 < n2 )
	{
		return edges[n1 - 1][n2 - 1];
	}
	else
	{
		return edges[n2 - 1][n1 - 1];
	}
}

// Simple averaging function
double avg( double a, double b )
{
	return (a+b) / 2.0;
}



