#include "tet4_parallel.h"

// gcc tet4.c main.c -o toTet10
// Program to convert tet4 elements to tet10

static int num_edges = 0;
static int edge_id = 0;

void toTet10( const char* msh_file )
{
	clock_t begin = clock();
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

	clock_t end = clock();

	printf("Memory Set-up: %f\n", (double)(end-begin)/CLOCKS_PER_SEC);

	begin = clock();

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

	end = clock();
	printf("Elem Iter Time: %lf\n", (double)(end-begin)/CLOCKS_PER_SEC);

	begin = clock();
	// Read element lines and store in elements[]
	#pragma omp parallel for schedule(static)
	for(int i = num_tet4; i < num_elements; i++)
	{
		//fgets(buff, 255, msh);
		int elem_id;
		int num_tags;
		char* elem = elements[i];

		char * elem_frag = (char*) malloc(255);
		sscanf( elem, "%*d %d %d %[^\t\n]", &elem_id, &num_tags, elem_frag );
		for(int j = 0; j < num_tags; j++)
		{
			sscanf(elem_frag, "%*d %[^\t\n]", elem_frag);
		}

		// If the current element is a TET4
		if(elem_id == 4)
		{
			//begin = clock();
			int n1, n2, n3, n4;
			sscanf(elem_frag, "%d %d %d %d", &n1, &n2, &n3, &n4);

			//printf("Element %d being created on thread %d\n", i, omp_get_thread_num());
			char * new_elem = constructElem( elem_frag, i, num_nodes, edges, num_elements, all_coords, nodes, n1, n2, n3, n4 );
			elements[i] = malloc(strlen(new_elem) + 1);
			strcpy(elements[i], new_elem);
			// Fix this maybe ... inefficient
			//end = clock();
			//printf("Elem Construct Time: %lf\n", (double)(end-begin)/CLOCKS_PER_SEC);
		}
		free(elem_frag);
	}
	end = clock();

	printf("AVG Elem Construct Time: %lf\n", ((double)(end-begin)/CLOCKS_PER_SEC));

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

	// worth it to parallelize this bit of code? only 6 iterations

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
	ids[4] = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n2, n4 );

	// edge_10
	ids[5] = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n3, n4 );

	char* new_elem = malloc(255);
	snprintf( new_elem, 255, "%d 11 2 0 0 %s %d %d %d %d %d %d\n", elem_id + 1, elem_frag, ids[0], ids[1], ids[2], ids[3], ids[4], ids[5] );

	return new_elem;
}

// NEED TO FIX RACE CONDITIONS
// need to lock only when element is not found so it can be created without another thread reading
int getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*6], char ** nodes, int n1, int n2 )
{
	edge_t curr_edge;
	#pragma omp critical
	{
		curr_edge = getEdge( n1, n2, edges );

		if( curr_edge.inUse == 0 )
		{
			//printf("creating edge\n");
			curr_edge.node_id = edge_id;
			// Following two lines can be problematic, if two threads are constructing same edge
			num_edges++;
			edge_id++;

			// Set coords
			char* node_1 = strdup(nodes[n1 - 1]);
			char* node_2 = strdup(nodes[n2 - 1]);
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

			char * new_node = malloc(50);
			snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_edge.node_id, curr_edge.x, curr_edge.y, curr_edge.z);
			// This can be problematic if node_id is incremented incorrectly
			all_coords[curr_edge.node_id] = strdup(new_node);
		}
	}

	return curr_edge.node_id;
}

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


double avg( double a, double b )
{
	return (a+b) / 2.0;
}




