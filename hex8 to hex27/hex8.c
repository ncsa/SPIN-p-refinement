#include "hex8.h"

// Static vars
static int num_edges = 0;
static int edge_id = 0;
static int num_faces = 0;
static int num_internal = 0;

void toHex27( const char* msh_file )
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

	char** all_coords = malloc(num_elements * 19 * sizeof(char*));

	int max_num_edges = num_elements * 12;
	int max_num_faces = num_elements * 6;

	// OPTIMIZE MEMORY ALLOCATION, SUPER SLOW

	char ** elements = malloc((num_elements) * sizeof(char*));
	//edge_t* edges[num_nodes][num_nodes];
	edge_t** edges = (edge_t**) calloc(num_nodes, sizeof(edge_t));
	for(int i = 0; i < num_nodes; i++)
	{
		edges[i] = (edge_t*) calloc(num_nodes, sizeof(edge_t));	
	}

	// calc / numnodes*numnodes
	//face_t* faces[num_nodes][num_nodes];
	face_t** faces = (face_t**) calloc(num_nodes, sizeof(face_t));
	for(int i = 0; i < num_nodes; i++)
	{
		faces[i] = (face_t*) calloc(num_nodes, sizeof(face_t));
	}

	internal_t** internals = malloc(num_elements * sizeof(internal_t*));

	double end = omp_get_wtime();

	printf("Memory Setup: %lf\n", (end-begin));

	begin = omp_get_wtime();

	int num_hex8 = 0;
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

		if(elem_id == 5)
		{
			num_hex8++;
		}
	}

	end = omp_get_wtime();
	printf("Elem Iter Time: %lf\n", (end-begin));

	begin = omp_get_wtime();

	#pragma omp parallel for schedule(static) shared(edge_id, num_edges)
	for(int i = num_elements - num_hex8; i < num_elements; i++)
	{
		int elem_id;
		int num_tags;
		char* elem = elements[i];

		char elem_frag[255];
		sscanf( elem, "%*d %d %d %[^\t\n]", &elem_id, &num_tags, elem_frag );
		for(int j = 0; j < num_tags; j++)
		{
			sscanf(elem_frag, "%*d %[^\t\n]", elem_frag);
		}

		// If the current element is a HEX8
		if(elem_id == 5)
		{
			//begin = clock();
			int n1, n2, n3, n4, n5, n6, n7, n8;
			sscanf(elem_frag, "%d %d %d %d %d %d %d %d", &n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8);

			char* new_elem = constructElem( elem_frag, i, num_nodes, edges, faces, num_elements, internals, all_coords, nodes, n1, n2, n3, n4, n5, n6, n7, n8 );
			elements[i] = malloc(strlen(new_elem) + 1);
			strcpy(elements[i], new_elem);
			//end = clock();
		}
	}

	end = omp_get_wtime();

	printf("Elem Construct Time: %lf\n", (end-begin));

	// Print to file
	char* _msh_file = strdup(msh_file);
	_msh_file[ strlen(_msh_file) - 4] = '\0';
	char new_msh_file[255];
	snprintf( new_msh_file, 255, "%s_hex27.msh", _msh_file );
	FILE * new_msh = fopen( new_msh_file, "w+" );

	fputs("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n$Nodes\n", new_msh);
	fprintf(new_msh, "%d\n", num_nodes + num_edges + num_faces + num_internal );

	for(int i = 0; i < num_nodes; i++)
	{
		fprintf(new_msh, "%d %s", (i+1), nodes[i]);
		free(nodes[i]);
	}

	printf("nodes: %d. edges: %d. faces: %d. internals: %d\n", num_edges + num_faces + num_nodes + num_internal, num_edges, num_faces, num_internal);
	for(int i = num_nodes + 1; i < num_edges + num_faces + num_nodes + num_internal + 1; i++)
	{
		char * curr_node = all_coords[i];
		//printf("%d\n", i);
		//printf("%s", curr_node);
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

	fclose(new_msh);
}

char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t** faces, int num_elements, internal_t** internals, char* all_coords[num_elements*19], char ** nodes, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8 )
{
	int ids[19];

	// Edge Nodes
	// edge_9
	ids[0] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n2 );

	// edge_10
	ids[1] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n4 );

	// edge_11 
	ids[2] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n5 );

	//edge_12
	ids[3] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n2, n3 );

	// edge_13
	ids[4] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n2, n6 );

	// edge_14
	ids[5] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n3, n4 );

	// edge_15
	ids[6] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n3, n7 );

	// edge_16
	ids[7] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n4, n8 );

	// edge_17
	ids[8] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n5, n6 );

	// edge_18
	ids[9] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n5, n8 );

	// edge_19
	ids[10] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n6, n7 );

	// edge_20
	ids[11] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n7, n8 );

	// Face Nodes
	// face_21	
	ids[12] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n2, n4, n1, n3 );

	// face_22
	ids[13] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n1, n6, n2, n5 );

	// face_23
	ids[14] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n5, n4, n8, n1 );

	// face_24
	ids[15] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n7, n2, n6, n3 );

	// face_25
	ids[16] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n7, n4, n3, n8 );

	// face_26
	ids[17] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n5, n7, n8, n6 );

	// Internal node
	ids[18] = getInternalNodeId( elem_id, num_elements, internals, all_coords, nodes, n2, n8 );

	// Create new element
	char new_elem[255];
	snprintf( new_elem, 255, "%d 12 2 0 0 %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", elem_id, elem_frag, ids[0], ids[1], ids[2], ids[3], ids[4], ids[5], ids[6], ids[7], ids[8], ids[9], ids[10], ids[11], ids[12], ids[13], ids[14], ids[15], ids[16], ids[17], ids[18]);

	//printf("%s", new_elem);

	return strdup(new_elem);
}

int getEdgeNodeId( int elem_id, int num_nodes, edge_t** edges, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2 )
{
	edge_t curr_edge = getEdge( n1, n2, edges );

	// if edge dne
	if( curr_edge.inUse == 0 )
	{
		int retest = 0;
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
			edges[n1 - 1][n2 - 1] = curr_edge;
		else
			edges[n2 - 1][n1 - 1] = curr_edge;

		//printf("Adding node to all_coords[%d]\n", curr_edge->node_id);
		char * new_node = malloc(50);
		snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_edge.node_id, curr_edge.x, curr_edge.y, curr_edge.z);
		all_coords[curr_edge.node_id] = strdup(new_node);
	}

	return curr_edge.node_id;
}

// Check both sets of nodes but always store at faces[n1][n2]/faces[n2][n1]
int getFaceNodeId( int elem_id, int num_nodes, face_t** faces, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2, int n3, int n4 )
{
	face_t curr_face = getFace( n1, n2, faces );
	face_t curr_face_2 = getFace( n3, n4, faces );
	// if edge dne
	if( curr_face.inUse == 0 && curr_face_2.inUse == 0 )
	{
		int retest = 0;
		// Race condition
		#pragma omp critical
		{
			curr_face = getFace( n1, n2, faces );
			curr_face_2 = getFace( n3, n4, faces );
			if( curr_face.inUse == 0 && curr_face_2.inUse == 0 )
			{
				curr_face.node_id = edge_id;
				edge_id++;
				curr_face.inUse = 1;
				// Store in faces
				if( n1 < n2 )
				{
					faces[n1 - 1][n2 - 1] = curr_face;
				}
				else
				{
					faces[n2 - 1][n1 - 1] = curr_face;
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
			if(curr_face.inUse == 0)
			{
				return curr_face_2.node_id;
			}
			else
			{
				return curr_face.node_id;
			}
		}

		#pragma omp atomic update
		num_faces++;

		// Set coords
		char* node_1 = strdup(nodes[n1 - 1]);
		char* node_2 = strdup(nodes[n2 - 1]);
		double x1, x2, y1, y2, z1, z2;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
		curr_face.x = avg(x1, x2);
		curr_face.y = avg(y1, y2);
		curr_face.z = avg(z1, z2);
		curr_face.inUse = 1;

		// Store in faces
		if( n1 < n2 )
		{
			faces[n1 - 1][n2 - 1] = curr_face;
		}
		else
		{
			faces[n2 - 1][n1 - 1] = curr_face;
		}

		//printf("Adding node to all_coords[%d]\n", curr_face.node_id);
		char * new_node = malloc(50);
		snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_face.node_id, curr_face.x, curr_face.y, curr_face.z);
		//printf("%s", new_node);
		all_coords[curr_face.node_id] = strdup(new_node);
	}
	else
	{
		if(curr_face.inUse == 0)
		{
			curr_face = curr_face_2;
		}
	}

	return curr_face.node_id;
}

int getInternalNodeId( int elem_id, int num_elements, internal_t** internals, char* all_coords[num_elements*19], char** nodes, int n1, int n2 )
{
	internal_t * curr_elem = malloc(sizeof(internal_t));

	#pragma omp critical
	{
		curr_elem->node_id = edge_id;
		edge_id++;
	}

	// Set coords
	char* node_1 = strdup(nodes[n1 - 1]);
	char* node_2 = strdup(nodes[n2 - 1]);
	double x1, x2, y1, y2, z1, z2;
	sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
	sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
	curr_elem->x = avg(x1, x2);
	curr_elem->y = avg(y1, y2);
	curr_elem->z = avg(z1, z2);

	internals[elem_id] = curr_elem;

	#pragma omp atomic update
	num_internal++;

	char new_node[100];
	snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_elem->node_id, curr_elem->x, curr_elem->y, curr_elem->z);
	all_coords[curr_elem->node_id] = strdup(new_node);

	return curr_elem->node_id;
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

face_t getFace( int n1, int n2, face_t ** faces )
{
	if( n1 < n2 )
	{
		return faces[n1 - 1][n2 - 1];
	}
	else
	{
		return faces[n2 - 1][n1 - 1];
	}
}

double avg( double a, double b )
{
	return (a+b) / 2.0;
}


