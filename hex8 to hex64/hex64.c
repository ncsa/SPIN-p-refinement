#include "hex64.h"

// Static vars
static int num_edges = 0;
static int edge_id = 0;
static int num_faces = 0;
static int num_internal = 0;

void toHex64( const char* msh_file )
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

	char** all_coords = malloc(num_elements * 56 * sizeof(char*));

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
	snprintf( new_msh_file, 255, "%s_hex64.msh", _msh_file );
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
	// Edge Nodes
	// Edge_1
	char* edge_1 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n2 );

	// Edge_2
	char* edge_2 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n4 );

	// Edge_3
	char* edge_3 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n5 );

	// Edge_4
	char* edge_4 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n2, n3 );

	// Edge_5
	char* edge_5 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n2, n6 );

	// Edge_6
	char* edge_6 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n3, n4 );

	// Edge_7
	char* edge_7 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n3, n7 );

	// Edge_8
	char* edge_8 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n4, n8 );

	// Edge_9
	char* edge_9 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n5, n6 );

	// Edge_10
	char* edge_10 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n5, n8 );

	// Edge_11
	char* edge_11 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n6, n7 );

	// Edge_12
	char* edge_12 = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n7, n8 );

	// Face Nodes
	// Face_1
	char* face_1 = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n1, n2, n3, n4 );

	// Face_2
	char* face_2 = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n1, n2, n6, n5 );

	// Face_3
	char* face_3 = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n1, n5, n8, n4 );

	// Face_4
	char* face_4 = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n2, n6, n7, n3 );

	// Face_5
	char* face_5 = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n3, n4, n8, n7);

	// Face_6
	char* face_6 = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n5, n6, n7, n8 );

	// Internal nodes
	char* internal = getInternalNodeId( elem_id, num_elements, internals, all_coords, nodes, n1, n2, n3, n4, n5, n6, n7, n8 );

	// Create new element
	char new_elem[255];
	snprintf( new_elem, 255, "%d 92 2 0 0 %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n", elem_id, elem_frag, edge_1, edge_2, edge_3, edge_4, edge_5, edge_6, edge_7, edge_8, edge_9, edge_10, edge_11, edge_12, face_1, face_2, face_3, face_4, face_5, face_6, internal);

	//printf("%s", new_elem);

	return strdup(new_elem);
}

char* getEdgeNodeId( int elem_id, int num_nodes, edge_t** edges, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2 )
{
	edge_t curr_edge;
	edge_t curr_edge_rev;

	curr_edge = getEdge( n1, n2, edges );

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
				curr_edge.node_id[0] = edge_id;
				curr_edge_rev.node_id[1] = edge_id;
				edge_id++;
				curr_edge.node_id[1] = edge_id;
				curr_edge_rev.node_id[0] = edge_id;
				edge_id++;
				curr_edge.inUse = 1;
				curr_edge_rev.inUse = 1;
				// Store in edges
				edges[n1 - 1][n2 - 1] = curr_edge;
				edges[n2 - 1][n1 - 1] = curr_edge_rev;
			}
			else
			{
				retest = 1;
			}	
		}

		if(retest == 1)
		{
			printf("found duplicate\n");
			char * buff = malloc(15);
			snprintf( buff, 15, "%d %d", curr_edge.node_id[0], curr_edge.node_id[1] );
			return buff;
		}

		#pragma omp atomic update
		num_edges += 2;

		// Set coords
		char* node_1 = nodes[n1 - 1];
		char* node_2 = nodes[n2 - 1];
		double x1, x2, y1, y2, z1, z2;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);

		double _x1 = thirds(x1, x2);
		double _y1 = thirds(y1, y2);
		double _z1 = thirds(z1, z2);
		double _x2 = thirds(x2, x1);
		double _y2 = thirds(y2, y1);
		double _z2 = thirds(z2, z1); 

		// Must set coords to correct order
		curr_edge.x1 = _x1;
		curr_edge.y1 = _y1;
		curr_edge.z1 = _z1;
		curr_edge.x2 = _x2;
		curr_edge.y2 = _y2;
		curr_edge.z2 = _z2;

		curr_edge_rev.x1 = _x2;
		curr_edge_rev.y1 = _y2;
		curr_edge_rev.z1 = _z2;
		curr_edge_rev.x2 = _x1;
		curr_edge_rev.y2 = _y1;
		curr_edge_rev.z2 = _z1;

		curr_edge.inUse = 1;

		// Store in edges
		edges[n1 - 1][n2 - 1] = curr_edge;
		edges[n2 - 1][n1 - 1] = curr_edge_rev;

		//printf("Adding node to all_coords[%d]\n", curr_edge->node_id);
		char * new_node1 = malloc(50);
		char * new_node2 = malloc(50);
		snprintf(new_node1, 50, "%d %lf %lf %lf\n", curr_edge.node_id[0], curr_edge.x1, curr_edge.y1, curr_edge.z1);
		snprintf(new_node2, 50, "%d %lf %lf %lf\n", curr_edge.node_id[1], curr_edge.x2, curr_edge.y2, curr_edge.z2);
		all_coords[curr_edge.node_id[0]] = strdup(new_node1);
		all_coords[curr_edge.node_id[1]] = strdup(new_node2);
	}

	char * buff = malloc(15);
	snprintf( buff, 15, "%d %d", curr_edge.node_id[0], curr_edge.node_id[1] );
	return buff;
}

// Check both sets of nodes but always store at faces[n1][n2]/faces[n2][n1]
char* getFaceNodeId( int elem_id, int num_nodes, face_t** faces, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2, int n3, int n4 )
{
	face_t curr_face = getFace( n1, n3, faces );
	face_t curr_face_2 = getFace( n2, n4, faces );
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
				curr_face.node_id[0] = edge_id;
				edge_id++;
				curr_face.node_id[1] = edge_id;
				edge_id++;
				curr_face.node_id[2] = edge_id;
				edge_id++;
				curr_face.node_id[3] = edge_id;
				edge_id++;
				curr_face.inUse = 1;
				// Store in faces
				if( n1 < n2 )
				{
					faces[n1 - 1][n3 - 1] = curr_face;
				}
				else
				{
					faces[n3 - 1][n1 - 1] = curr_face;
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
				char * buff = malloc(50);
				snprintf( buff, 50, "%d %d %d %d", curr_face_2.node_id[0], curr_face_2.node_id[1], curr_face_2.node_id[2], curr_face.node_id[3] );
				return buff;
			}
			else
			{
				char * buff = malloc(50);
				snprintf( buff, 50, "%d %d %d %d", curr_face.node_id[0], curr_face.node_id[1], curr_face.node_id[2], curr_face.node_id[3] );
				return buff;
			}
		}

		#pragma omp atomic update
		num_faces += 4;

		// Set coords
		char* node_1 = nodes[n1 - 1];
		char* node_2 = nodes[n2 - 1];
		char* node_3 = nodes[n3 - 1];
		char* node_4 = nodes[n4 - 1];
		double x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
		sscanf(node_3, "%lf %lf %lf", &x3, &y3, &z3);
		sscanf(node_4, "%lf %lf %lf", &x4, &y4, &z4);

		double _x1 = thirds( thirds(x1,x4), thirds(x2,x3) );
		double _y1 = thirds( thirds(y1,y4), thirds(y2,y3) );
		double _z1 = thirds( thirds(z1,z4), thirds(z2,z3) );
		double _x2 = thirds( thirds(x2,x3), thirds(x1,x4) );
		double _y2 = thirds( thirds(y2,y3), thirds(y1,y4) );
		double _z2 = thirds( thirds(z2,z3), thirds(z1,z4) );
		double _x3 = thirds( thirds(x3,x2), thirds(x4,x1) );
		double _y3 = thirds( thirds(y3,y2), thirds(y4,y1) );
		double _z3 = thirds( thirds(z3,z2), thirds(z4,z1) );
		double _x4 = thirds( thirds(x4,x1), thirds(x3,x2) );
		double _y4 = thirds( thirds(y4,y1), thirds(y3,y2) );
		double _z4 = thirds( thirds(z4,z1), thirds(z3,z2) );

		curr_face.x1 = _x1;
		curr_face.y1 = _y1;
		curr_face.z1 = _z1;
		curr_face.x2 = _x2;
		curr_face.y2 = _y2;
		curr_face.z2 = _z2;
		curr_face.x3 = _x3;
		curr_face.y3 = _y3;
		curr_face.z3 = _z3;
		curr_face.x4 = _x4;
		curr_face.y4 = _y4;
		curr_face.z4 = _z4;

		curr_face.inUse = 1;

		// Store in faces
		if( n1 < n2 )
		{
			faces[n1 - 1][n3 - 1] = curr_face;
		}
		else
		{
			faces[n3 - 1][n1 - 1] = curr_face;
		}

		//printf("Adding node to all_coords[%d]\n", curr_face.node_id);
		char * new_node1 = malloc(50);
		char * new_node2 = malloc(50);
		char * new_node3 = malloc(50);
		char * new_node4 = malloc(50);
		snprintf(new_node1, 50, "%d %lf %lf %lf\n", curr_face.node_id[0], curr_face.x1, curr_face.y1, curr_face.z1);
		snprintf(new_node2, 50, "%d %lf %lf %lf\n", curr_face.node_id[1], curr_face.x2, curr_face.y2, curr_face.z2);
		snprintf(new_node3, 50, "%d %lf %lf %lf\n", curr_face.node_id[2], curr_face.x3, curr_face.y3, curr_face.z3);
		snprintf(new_node4, 50, "%d %lf %lf %lf\n", curr_face.node_id[3], curr_face.x4, curr_face.y4, curr_face.z4);
		all_coords[curr_face.node_id[0]] = strdup(new_node1);
		all_coords[curr_face.node_id[1]] = strdup(new_node2);
		all_coords[curr_face.node_id[2]] = strdup(new_node3);
		all_coords[curr_face.node_id[3]] = strdup(new_node4);
	}
	else
	{
		if(curr_face.inUse == 0)
		{
			curr_face = curr_face_2;
		}
	}

	char* buff = malloc(50);
	snprintf( buff, 50, "%d %d %d %d", curr_face.node_id[0], curr_face.node_id[1], curr_face.node_id[2], curr_face.node_id[3] );
	return buff;
}

char* getInternalNodeId( int elem_id, int num_elements, internal_t** internals, char* all_coords[num_elements*19], char** nodes, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8 )
{
	internal_t * curr_elem = malloc(sizeof(internal_t));

	#pragma omp critical
	{
		curr_elem->node_id[0] = edge_id;
		edge_id++;
		curr_elem->node_id[1] = edge_id;
		edge_id++;
		curr_elem->node_id[2] = edge_id;
		edge_id++;
		curr_elem->node_id[3] = edge_id;
		edge_id++;
		curr_elem->node_id[4] = edge_id;
		edge_id++;
		curr_elem->node_id[5] = edge_id;
		edge_id++;
		curr_elem->node_id[6] = edge_id;
		edge_id++;
		curr_elem->node_id[7] = edge_id;
		edge_id++;
	}

	// Set coords
	char* node_1 = strdup(nodes[n1 - 1]);
	char* node_2 = strdup(nodes[n2 - 1]);
	char* node_3 = strdup(nodes[n3 - 1]);
	char* node_4 = strdup(nodes[n4 - 1]);
	char* node_5 = strdup(nodes[n5 - 1]);
	char* node_6 = strdup(nodes[n6 - 1]);
	char* node_7 = strdup(nodes[n7 - 1]);
	char* node_8 = strdup(nodes[n8 - 1]);
	double x1, x2, x3, x4, x5, x6, x7, x8, y1, y2, y3, y4, y5, y6, y7, y8, z1, z2, z3, z4, z5, z6, z7, z8;
	sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
	sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
	sscanf(node_3, "%lf %lf %lf", &x3, &y3, &z3);
	sscanf(node_4, "%lf %lf %lf", &x4, &y4, &z4);
	sscanf(node_5, "%lf %lf %lf", &x5, &y5, &z5);
	sscanf(node_6, "%lf %lf %lf", &x6, &y6, &z6);
	sscanf(node_7, "%lf %lf %lf", &x7, &y7, &z7);
	sscanf(node_8, "%lf %lf %lf", &x8, &y8, &z8);
	// thirds( thirds( thirds(), thirds() ), thirds( thirds(), thirds() ) );
	// Node 1
	double _x1 = thirds( thirds( thirds(x1,x4), thirds(x5,x8) ), thirds( thirds(x2,x3), thirds(x6,x7) ) );
	double _y1 = thirds( thirds( thirds(y1,y4), thirds(y5,y8) ), thirds( thirds(y2,y3), thirds(y6,y7) ) );
	double _z1 = thirds( thirds( thirds(z1,z4), thirds(z5,z8) ), thirds( thirds(z2,z3), thirds(z6,z7) ) );

	// Node 2
	double _x2 = thirds( thirds( thirds(x2,x3), thirds(x6,x7) ), thirds( thirds(x1,x4), thirds(x5,x8) ) );
	double _y2 = thirds( thirds( thirds(y2,y3), thirds(y6,y7) ), thirds( thirds(y1,y4), thirds(y5,y8) ) );
	double _z2 = thirds( thirds( thirds(z2,z3), thirds(z6,z7) ), thirds( thirds(z1,z4), thirds(z5,z8) ) );

	// Node 3
	double _x3 = thirds( thirds( thirds(x3,x2), thirds(x7,x6) ), thirds( thirds(x4,x1), thirds(x8,x5) ) );
	double _y3 = thirds( thirds( thirds(y3,y2), thirds(y7,y6) ), thirds( thirds(y4,y1), thirds(y8,y5) ) );
	double _z3 = thirds( thirds( thirds(z3,z2), thirds(z7,z6) ), thirds( thirds(z4,z1), thirds(z8,z5) ) );

	// Node 4
	double _x4 = thirds( thirds( thirds(x4,x1), thirds(x8,x5) ), thirds( thirds(x3,x2), thirds(x7,x6) ) );
	double _y4 = thirds( thirds( thirds(y4,y1), thirds(y8,y5) ), thirds( thirds(y3,y2), thirds(y7,y6) ) );
	double _z4 = thirds( thirds( thirds(z4,z1), thirds(z8,z5) ), thirds( thirds(z3,z2), thirds(z7,z6) ) );

	// Node 5
	double _x5 = thirds( thirds( thirds(x5,x8), thirds(x1,x4) ), thirds( thirds(x6,x7), thirds(x2,x3) ) );
	double _y5 = thirds( thirds( thirds(y5,y8), thirds(y1,y4) ), thirds( thirds(y6,y7), thirds(y2,y3) ) );
	double _z5 = thirds( thirds( thirds(z5,z8), thirds(z1,z4) ), thirds( thirds(z6,z7), thirds(z2,z3) ) );

	// Node 6
	double _x6 = thirds( thirds( thirds(x6,x7), thirds(x2,x3) ), thirds( thirds(x5,x8), thirds(x1,x4) ) );
	double _y6 = thirds( thirds( thirds(y6,y7), thirds(y2,y3) ), thirds( thirds(y5,y8), thirds(y1,y4) ) );
	double _z6 = thirds( thirds( thirds(z6,z7), thirds(z2,z3) ), thirds( thirds(z5,z8), thirds(z1,z4) ) );

	// Node 7
	double _x7 = thirds( thirds( thirds(x7,x6), thirds(x3,x2) ), thirds( thirds(x8,x5), thirds(x4,x1) ) );
	double _y7 = thirds( thirds( thirds(y7,y6), thirds(y3,y2) ), thirds( thirds(y8,y5), thirds(y4,y1) ) );
	double _z7 = thirds( thirds( thirds(z7,z6), thirds(z3,z2) ), thirds( thirds(z8,z5), thirds(z4,z1) ) );

	// Node 8
	double _x8 = thirds( thirds( thirds(x8,x5), thirds(x4,x1) ), thirds( thirds(x7,x6), thirds(x3,x2) ) );
	double _y8 = thirds( thirds( thirds(y8,y5), thirds(y4,y1) ), thirds( thirds(y7,y6), thirds(y3,y2) ) );
	double _z8 = thirds( thirds( thirds(z8,z5), thirds(z4,z1) ), thirds( thirds(z7,z6), thirds(z3,z2) ) );

	curr_elem->x1 = _x1;
	curr_elem->y1 = _y1;
	curr_elem->z1 = _z1;

	curr_elem->x2 = _x2;
	curr_elem->y2 = _y2;
	curr_elem->z2 = _z2;

	curr_elem->x3 = _x3;
	curr_elem->y3 = _y3;
	curr_elem->z3 = _z3;

	curr_elem->x4 = _x4;
	curr_elem->y4 = _y4;
	curr_elem->z4 = _z4;

	curr_elem->x5 = _x5;
	curr_elem->y5 = _y5;
	curr_elem->z5 = _z5;

	curr_elem->x6 = _x6;
	curr_elem->y6 = _y6;
	curr_elem->z6 = _z6;

	curr_elem->x7 = _x7;
	curr_elem->y7 = _y7;
	curr_elem->z7 = _z7;

	curr_elem->x8 = _x8;
	curr_elem->y8 = _y8;
	curr_elem->z8 = _z8;

	internals[elem_id] = curr_elem;

	#pragma omp atomic update
	num_internal += 8;

	char * new_node1 = malloc(50);
	char * new_node2 = malloc(50);
	char * new_node3 = malloc(50);
	char * new_node4 = malloc(50);
	char * new_node5 = malloc(50);
	char * new_node6 = malloc(50);
	char * new_node7 = malloc(50);
	char * new_node8 = malloc(50);
	snprintf(new_node1, 50, "%d %lf %lf %lf\n", curr_elem->node_id[0], curr_elem->x1, curr_elem->y1, curr_elem->z1);
	snprintf(new_node2, 50, "%d %lf %lf %lf\n", curr_elem->node_id[1], curr_elem->x2, curr_elem->y2, curr_elem->z2);
	snprintf(new_node3, 50, "%d %lf %lf %lf\n", curr_elem->node_id[2], curr_elem->x3, curr_elem->y3, curr_elem->z3);
	snprintf(new_node4, 50, "%d %lf %lf %lf\n", curr_elem->node_id[3], curr_elem->x4, curr_elem->y4, curr_elem->z4);
	snprintf(new_node5, 50, "%d %lf %lf %lf\n", curr_elem->node_id[4], curr_elem->x5, curr_elem->y5, curr_elem->z5);
	snprintf(new_node6, 50, "%d %lf %lf %lf\n", curr_elem->node_id[5], curr_elem->x6, curr_elem->y6, curr_elem->z6);
	snprintf(new_node7, 50, "%d %lf %lf %lf\n", curr_elem->node_id[6], curr_elem->x7, curr_elem->y7, curr_elem->z7);
	snprintf(new_node8, 50, "%d %lf %lf %lf\n", curr_elem->node_id[7], curr_elem->x8, curr_elem->y8, curr_elem->z8);
	all_coords[curr_elem->node_id[0]] = strdup(new_node1);
	all_coords[curr_elem->node_id[1]] = strdup(new_node2);
	all_coords[curr_elem->node_id[2]] = strdup(new_node3);
	all_coords[curr_elem->node_id[3]] = strdup(new_node4);
	all_coords[curr_elem->node_id[4]] = strdup(new_node5);
	all_coords[curr_elem->node_id[5]] = strdup(new_node6);
	all_coords[curr_elem->node_id[6]] = strdup(new_node7);
	all_coords[curr_elem->node_id[7]] = strdup(new_node8);

	char* buff = malloc(100);
	snprintf( buff, 100, "%d %d %d %d %d %d %d %d", curr_elem->node_id[0], curr_elem->node_id[1], curr_elem->node_id[2], curr_elem->node_id[3], curr_elem->node_id[4], curr_elem->node_id[5], curr_elem->node_id[6], curr_elem->node_id[7] );
	return buff;
}

edge_t getEdge( int n1, int n2, edge_t ** edges )
{
	return edges[n1 -1][n2 - 1];
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

// Simple average function 
double avg( double a, double b )
{
	return (a+b) / 2.0;
}

// Get the third distance from point a
double thirds( double a, double b )
{
	double third_a = a * 2/3;
	double third_b = b * 1/3;
	double ret = third_a + third_b;
	return ret;
}