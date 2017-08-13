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

	char** all_coords = malloc(num_elements * 117 * sizeof(char*));

	char ** elements = malloc((num_elements) * sizeof(char*));
	edge_t** edges = (edge_t**) calloc(num_nodes, sizeof(edge_t));
	for(int i = 0; i < num_nodes; i++)
	{
		edges[i] = (edge_t*) calloc(num_nodes, sizeof(edge_t));	
	}

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
	snprintf( new_msh_file, 255, "%s_hex125.msh", _msh_file );
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
	char new_elem[500];
	snprintf( new_elem, 500, "%d 92 2 0 0 %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n", elem_id, elem_frag, edge_1, edge_2, edge_3, edge_4, edge_5, edge_6, edge_7, edge_8, edge_9, edge_10, edge_11, edge_12, face_1, face_2, face_3, face_4, face_5, face_6, internal);

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
				curr_edge_rev.node_id[2] = edge_id;
				edge_id++;
				curr_edge.node_id[1] = edge_id;
				curr_edge_rev.node_id[1] = edge_id;
				edge_id++;
				curr_edge.node_id[2] = edge_id;
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
			char * buff = malloc(25);
			snprintf( buff, 25, "%d %d %d", curr_edge.node_id[0], curr_edge.node_id[1], curr_edge.node_id[2] );
			return buff;
		}

		#pragma omp atomic update
		num_edges += 3;

		// Set coords
		char* node_1 = nodes[n1 - 1];
		char* node_2 = nodes[n2 - 1];
		double x1, x2, y1, y2, z1, z2;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);

		double _x[3];
		double _y[3];
		double _z[3];

		_x[0] = avg( avg(x1,x2), x1 );
		_y[0] = avg( avg(y1,y2), y1 );
		_z[0] = avg( avg(z1,z2), z1 );
		_x[1] = avg( x1, x2 );
		_y[1] = avg( y1, y2 );
		_z[1] = avg( z1, z2 );
		_x[2] = avg( avg(x1, x2), x2 );
		_y[2] = avg( avg(y1, y2), y2 );
		_z[2] = avg( avg(z1, z2), z2 );

		// Must set coords to correct order
		for(int i = 0; i < 3; i++)
		{
			curr_edge.x[i] = _x[i];
			curr_edge.y[i] = _y[i];
			curr_edge.z[i] = _z[i];

			curr_edge_rev.x[i] = _x[2-i];
			curr_edge_rev.y[i] = _y[2-i];
			curr_edge_rev.z[i] = _z[2-i];
		}

		curr_edge.inUse = 1;

		// Store in edges
		edges[n1 - 1][n2 - 1] = curr_edge;
		edges[n2 - 1][n1 - 1] = curr_edge_rev;

		//printf("Adding node to all_coords[%d]\n", curr_edge->node_id);
		char * new_node1 = malloc(100);
		char * new_node2 = malloc(100);
		char * new_node3 = malloc(100);
		snprintf(new_node1, 100, "%d %lf %lf %lf\n", curr_edge.node_id[0], curr_edge.x[0], curr_edge.y[0], curr_edge.z[0]);
		snprintf(new_node2, 100, "%d %lf %lf %lf\n", curr_edge.node_id[1], curr_edge.x[1], curr_edge.y[1], curr_edge.z[1]);
		snprintf(new_node3, 100, "%d %lf %lf %lf\n", curr_edge.node_id[2], curr_edge.x[2], curr_edge.y[2], curr_edge.z[2]);
		all_coords[curr_edge.node_id[0]] = strdup(new_node1);
		all_coords[curr_edge.node_id[1]] = strdup(new_node2);
		all_coords[curr_edge.node_id[2]] = strdup(new_node3);
	}

	char * buff = malloc(25);
	snprintf( buff, 25, "%d %d", curr_edge.node_id[0], curr_edge.node_id[1], curr_edge.node_id[2] );
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
				for(int i = 0; i < 9; i ++)
				{
					curr_face.node_id[i] = edge_id;
					edge_id++;
				}
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
				char * buff = malloc(100);
				snprintf( buff, 100, "%d %d %d %d %d %d %d %d %d", curr_face_2.node_id[0], curr_face_2.node_id[1], curr_face_2.node_id[2], curr_face.node_id[3], curr_face.node_id[4], curr_face.node_id[5], curr_face.node_id[6], curr_face.node_id[7], curr_face.node_id[8] );
				return buff;
			}
			else
			{
				char * buff = malloc(100);
				snprintf( buff, 100, "%d %d %d %d %d %d %d %d %d", curr_face_2.node_id[0], curr_face_2.node_id[1], curr_face_2.node_id[2], curr_face.node_id[3], curr_face.node_id[4], curr_face.node_id[5], curr_face.node_id[6], curr_face.node_id[7], curr_face.node_id[8] );
				return buff;
			}
		}

		#pragma omp atomic update
		num_faces += 9;

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

		double _x[9];
		double _y[9];
		double _z[9];

		_x[0] = avg( avg(avg(x1,x2), x1), avg( avg( avg(x1,x2), x1), avg( avg(x4,x3), x4) ) );
		_y[0] = avg( avg(avg(y1,y2), y1), avg( avg( avg(y1,y2), y1), avg( avg(y4,y3), y4) ) );
		_z[0] = avg( avg(avg(z1,z2), z1), avg( avg( avg(z1,z2), z1), avg( avg(z4,z3), z4) ) );
		_x[1] = avg( avg(avg(x4,x3), x4), avg( avg( avg(x1,x2), x1), avg( avg(x4,x3), x4) ) );
		_y[1] = avg( avg(avg(y4,y3), y4), avg( avg( avg(y1,y2), y1), avg( avg(y4,y3), y4) ) );
		_z[1] = avg( avg(avg(z4,z3), z4), avg( avg( avg(z1,z2), z1), avg( avg(z4,z3), z4) ) );
		_x[2] = avg( avg(avg(x3,x4), x3), avg( avg( avg(x3,x4), x3), avg( avg(x2,x1), x2) ) );
		_y[2] = avg( avg(avg(y3,y4), y3), avg( avg( avg(y3,y4), y3), avg( avg(y2,y1), y2) ) );
		_z[2] = avg( avg(avg(z3,z4), z3), avg( avg( avg(z3,z4), z3), avg( avg(z2,z1), z2) ) );
		_x[3] = avg( avg(avg(x2,x1), x2), avg( avg( avg(x3,x4), x3), avg( avg(x2,x1), x2) ) );
		_y[3] = avg( avg(avg(y2,y1), y2), avg( avg( avg(y3,y4), y3), avg( avg(y2,y1), y2) ) );
		_z[3] = avg( avg(avg(z2,z1), z2), avg( avg( avg(z3,z4), z3), avg( avg(z2,z1), z2) ) );
		_x[4] = avg( avg(avg(x1,x2), x1), avg( avg(x4,x3), x4) );
		_y[4] = avg( avg(avg(y1,y2), y1), avg( avg(y4,y3), y4) );
		_z[4] = avg( avg(avg(z1,z2), z1), avg( avg(z4,z3), z4) );
		_x[5] = avg( avg(avg(x1,x4), x4), avg( avg(x2,x3), x3) );
		_y[5] = avg( avg(avg(y1,y4), y4), avg( avg(y2,y3), y3) );
		_z[5] = avg( avg(avg(z1,z4), z4), avg( avg(z2,z3), z3) );
		_x[6] = avg( avg(avg(x3,x4), x3), avg( avg(x1,x2), x2) );
		_y[6] = avg( avg(avg(y3,y4), y3), avg( avg(y1,y2), y2) );
		_z[6] = avg( avg(avg(z3,z4), z3), avg( avg(z1,z2), z2) );
		_x[7] = avg( avg(avg(x1,x4), x1), avg( avg(x2,x3), x2) );
		_y[7] = avg( avg(avg(y1,y4), y1), avg( avg(y2,y3), y2) );
		_z[7] = avg( avg(avg(z1,z4), z1), avg( avg(z2,z3), z2) );
		_x[8] = avg( avg(x1,x4), avg(x2, x3));
		_y[8] = avg( avg(y1,y4), avg(y2, y3));
		_z[8] = avg( avg(z1,z4), avg(z2, z3));

		for(int i = 0; i < 9; i++)
		{
			curr_face.x[i] = _x[i];
			curr_face.y[i] = _y[i];
			curr_face.z[i] = _z[i];
		}
 
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
		for(int i = 0; i < 27; i++)
		{
			curr_elem.node_id[i] = edge_id;
			edge_id++;
		}
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

	double _x[27];
	double _y[27];
	double _z[27];

	// Constants for calculations --- q12 -> Quarter distance between node 1 and 2 (closer to n1) -- q21 -> Quarter distance between node 1 and 2 (closer to n2)
	double q12[3], q21[3], q23[3], q32[3], q34[3], q43[3], q41[3], q14[3], q56[3], q65[3], q67[3], q76[3], q78[3], q87[3], q58[3], q85[3], q48[3], q84[3], q15[3], q51[3], q62[3], q26[3], q73[3], q37[3], h34[3], h26[3], h48[3], h15[0];
	q12[0] = avg(x1, avg(x1,x2));
	q12[1] = avg(y1, avg(y1,y2));
	q12[2] = avg(z1, avg(z1,z2));

	q21[0] = avg(x2, avg(x1,x2));
	q21[1] = avg(y2, avg(y1,y2));
	q21[2] = avg(z2, avg(z1,z2));

	q23[0] = avg(x2, avg(x2,x3));
	q23[1] = avg(y2, avg(y2,y3));
	q23[2] = avg(z2, avg(z2,z3));

	q32[0] = avg(x3, avg(x2,x3));
	q32[1] = avg(y3, avg(y2,y3));
	q32[2] = avg(z3, avg(z2,z3));

	q34[0] = avg(x3, avg(x3,x4));
	q34[1] = avg(y3, avg(y3,y4));
	q34[2] = avg(z3, avg(z3,z4));

	q43[0] = avg(x4, avg(x3,x4));
	q43[1] = avg(y4, avg(y3,y4));
	q43[2] = avg(z4, avg(z3,z4));

	q41[0] = avg(x4, avg(x1,x4));
	q41[1] = avg(y4, avg(y1,y4));
	q41[2] = avg(z4, avg(z1,z4));

	q14[0] = avg(x1, avg(x1,x4));
	q14[1] = avg(y1, avg(y1,y4));
	q14[2] = avg(z1, avg(z1,z4));

	q56[0] = avg(x5, avg(x5,x6));
	q56[1] = avg(y5, avg(y5,y6));
	q56[2] = avg(z5, avg(z5,z6));

	q65[0] = avg(x6, avg(x5,x6));
	q65[1] = avg(y6, avg(y5,y6));
	q65[2] = avg(z6, avg(z5,z6));

	q67[0] = avg(x6, avg(x6,x7));
	q67[1] = avg(y6, avg(y6,y7));
	q67[2] = avg(z6, avg(z6,z7));

	q76[0] = avg(x7, avg(x6,x7));
	q76[1] = avg(y7, avg(y6,y7));
	q76[2] = avg(z7, avg(z6,z7));

	q78[0] = avg(x7, avg(x7,x8));
	q78[1] = avg(y7, avg(y7,y8));
	q78[2] = avg(z7, avg(z7,z8));

	q87[0] = avg(x8, avg(x7,x8));
	q87[1] = avg(y8, avg(y7,y8));
	q87[2] = avg(z8, avg(z7,z8));

	q58[0] = avg(x5, avg(x5,x8));
	q58[1] = avg(y5, avg(y5,y8));
	q58[2] = avg(z5, avg(z5,z8));

	q85[0] = avg(x8, avg(x5,x8));
	q85[1] = avg(y8, avg(y5,y8));
	q85[2] = avg(z8, avg(z5,z8));

	q48[0] = avg(x4, avg(x4,x8));
	q48[1] = avg(y4, avg(y4,y8));
	q48[2] = avg(z4, avg(z4,z8));

	q84[0] = avg(x8, avg(x4,x8));
	q84[1] = avg(y8, avg(y4,y8));
	q84[2] = avg(z8, avg(z4,z8));

	q15[0] = avg(x1, avg(x1,x5));
	q15[1] = avg(y1, avg(y1,y5));
	q15[2] = avg(z1, avg(z1,z5));

	q51[0] = avg(x5, avg(x1,x5));
	q51[1] = avg(y5, avg(y1,y5));
	q51[2] = avg(z5, avg(z1,z5));

	q62[0] = avg(x6, avg(x2,x6));
	q62[1] = avg(y6, avg(y2,y6));
	q62[2] = avg(z6, avg(z2,z6));

	q26[0] = avg(x2, avg(x2,x6));
	q26[1] = avg(y2, avg(y2,y6));
	q26[2] = avg(z2, avg(z2,z6));

	q73[0] = avg(x7, avg(x3,x7));
	q73[1] = avg(y7, avg(y3,y7));
	q73[2] = avg(z7, avg(z3,z7));

	q37[0] = avg(x3, avg(x3,x7));
	q37[1] = avg(y3, avg(y3,y7));
	q37[2] = avg(z3, avg(z3,z7));

	h34[0] = avg(x3, x4);
	h34[1] = avg(y3, y4);
	h34[2] = avg(z3, z4);

	h26[0] = avg(x2, x6);
	h26[1] = avg(y2, y6);
	h26[2] = avg(z2, z6);

	h48[0] = avg(x4, x8);
	h48[1] = avg(y4, y8);
	h48[2] = avg(z4, z8);

	h15[0] = avg(x1, x5);
	h15[1] = avg(y1, y5);
	h15[2] = avg(z1, z5);

	// Node 1
	_x[0] = avg( avg(q12[0], avg(q12[0], q43[0]) ), avg( avg(q12[0], avg(q12[0],q43[0])), avg(q56[0], avg(q56[0],q87[0])) ) );
	_y[0] = avg( avg(q12[1], avg(q12[1], q43[1]) ), avg( avg(q12[1], avg(q12[1],q43[1])), avg(q56[1], avg(q56[1],q87[1])) ) );
	_z[0] = avg( avg(q12[2], avg(q12[2], q43[2]) ), avg( avg(q12[2], avg(q12[2],q43[2])), avg(q56[2], avg(q56[2],q87[2])) ) );
	// Node 2
	_x[1] = avg( avg(q23[0], avg(q23[0],q67[0])), avg( avg(q23[0], avg(q23[0],q67[0]), avg(q14[0], avg(q14[0],q58[0]))) ) );
	_y[1] = avg( avg(q23[1], avg(q23[1],q67[1])), avg( avg(q23[1], avg(q23[1],q67[1]), avg(q14[1], avg(q14[1],q58[1]))) ) );
	_z[1] = avg( avg(q23[2], avg(q23[2],q67[2])), avg( avg(q23[2], avg(q23[2],q67[2]), avg(q14[2], avg(q14[2],q58[2]))) ) );
	// Node 3
	_x[2] = avg( avg(q32[0], avg(q32[0],q76[0])), avg( avg(q32[0], avg(q32[0],q76[0])), avg(q41[0], avg(q41[0],q85[0])) ) );
	_y[2] = avg( avg(q32[1], avg(q32[1],q76[1])), avg( avg(q32[1], avg(q32[1],q76[1])), avg(q41[1], avg(q41[1],q85[1])) ) );
	_z[2] = avg( avg(q32[2], avg(q32[2],q76[2])), avg( avg(q32[2], avg(q32[2],q76[2])), avg(q41[2], avg(q41[2],q85[2])) ) );
	// Node 4
	_x[3] = avg( avg(q43[0], avg(q43[0],q87[0])), avg( avg(q12[0], avg(q12[0],q56[0])), avg(q43[0], avg(q43[0],q87[0])) ) );
	_y[3] = avg( avg(q43[1], avg(q43[1],q87[1])), avg( avg(q12[1], avg(q12[1],q56[1])), avg(q43[1], avg(q43[1],q87[1])) ) );
	_z[3] = avg( avg(q43[2], avg(q43[2],q87[2])), avg( avg(q12[2], avg(q12[2],q56[2])), avg(q43[2], avg(q43[2],q87[2])) ) );
	// Node 5
	_x[4] = avg( avg(q56[0], avg(q56[0],q87[0])), avg( avg(q12[0], avg(q12[0],q43[0])), avg(q56[0], avg(q56[0],q87[0]))) );
	_y[4] = avg( avg(q56[1], avg(q56[1],q87[1])), avg( avg(q12[1], avg(q12[1],q43[1])), avg(q56[1], avg(q56[1],q87[1]))) );
	_z[4] = avg( avg(q56[2], avg(q56[2],q87[2])), avg( avg(q12[2], avg(q12[2],q43[2])), avg(q56[2], avg(q56[2],q87[2]))) );
	// Node 6
	_x[5] = avg( avg(q67[0], avg(q67[0],q23[0])), avg( avg(q67[0], avg(q67[0],q23[0])), avg(q58[0], avg(q58[0], avg(q58[0],q14[0])))) );
	_y[5] = avg( avg(q67[1], avg(q67[1],q23[1])), avg( avg(q67[1], avg(q67[1],q23[1])), avg(q58[1], avg(q58[1], avg(q58[1],q14[1])))) );
	_z[5] = avg( avg(q67[2], avg(q67[2],q23[2])), avg( avg(q67[2], avg(q67[2],q23[2])), avg(q58[2], avg(q58[2], avg(q58[2],q14[2])))) );
	// Node 7
	_x[6] = avg( avg(q76[0], avg(q76[0],q32[0])), avg( avg(q73[0], avg(q73[0],q62[0])), avg(q84[0], avg(q84[0],q57[0])) ) );
	_y[6] = avg( avg(q76[1], avg(q76[1],q32[1])), avg( avg(q73[1], avg(q73[1],q62[1])), avg(q84[1], avg(q84[1],q57[1])) ) );
	_z[6] = avg( avg(q76[2], avg(q76[2],q32[2])), avg( avg(q73[2], avg(q73[2],q62[2])), avg(q84[2], avg(q84[2],q57[2])) ) );
	// Node 8
	_x[7] = avg( avg(q87[0], avg(q87[0],q43[0])), avg( avg(q56[0], avg(q56[0],q72[0])), avg(q87[0], avg(q87[0],q43[0])) ) );
	_y[7] = avg( avg(q87[1], avg(q87[1],q43[1])), avg( avg(q56[1], avg(q56[1],q72[1])), avg(q87[1], avg(q87[1],q43[1])) ) );
	_z[7] = avg( avg(q87[2], avg(q87[2],q43[2])), avg( avg(q56[2], avg(q56[2],q72[2])), avg(q87[2], avg(q87[2],q43[2])) ) );
	// Node 9
	_x[8] = avg( avg(q23[0], avg(q23[0],q67[0])), avg(q14[0], avg(q14[0],q58[0])) );
	_y[8] = avg( avg(q23[1], avg(q23[1],q67[1])), avg(q14[1], avg(q14[1],q58[1])) );
	_z[8] = avg( avg(q23[2], avg(q23[2],q67[2])), avg(q14[2], avg(q14[2],q58[2])) );
	// Node 10
	_x[9] = avg( avg(q12[0], avg(q12[0],q56[0])), avg(q43[0], avg(q43[0],q87[0])) );
	_y[9] = avg( avg(q12[1], avg(q12[1],q56[1])), avg(q43[1], avg(q43[1],q87[1])) );
	_z[9] = avg( avg(q12[2], avg(q12[2],q56[2])), avg(q43[2], avg(q43[2],q87[2])) );
	// Node 11
	_x[10] = avg( avg(q12[0], avg(q12[0],q43[0])), avg(q56[0], avg(q56[0],q87[0])) );
	_y[10] = avg( avg(q12[1], avg(q12[1],q43[1])), avg(q56[1], avg(q56[1],q87[1])) );
	_z[10] = avg( avg(q12[2], avg(q12[2],q43[2])), avg(q56[2], avg(q56[2],q87[2])) );
	// Node 12
	_x[11] = avg( avg(q34[0], avg(q34[0],q73[0])), avg(q21[0], avg(q21[0],q65[0])) );
	_y[11] = avg( avg(q34[1], avg(q34[1],q73[1])), avg(q21[1], avg(q21[1],q65[1])) );
	_z[11] = avg( avg(q34[2], avg(q34[2],q73[2])), avg(q21[2], avg(q21[2],q65[2])) );
	// Node 13
	_x[12] = avg( avg(q23[0],q67[0]), avg( avg(q23[0],q67[0]), avg(q14[0],q58[0]) ) );
	_y[12] = avg( avg(q23[1],q67[1]), avg( avg(q23[1],q67[1]), avg(q14[1],q58[1]) ) );
	_z[12] = avg( avg(q23[2],q67[2]), avg( avg(q23[2],q67[2]), avg(q14[2],q58[2]) ) );
	// Node 14
	_x[13] = avg( avg(q32[0], avg(q32[0],q76[0])), avg(q41[0], avg(q41[0],q85[0])) );
	_y[13] = avg( avg(q32[1], avg(q32[1],q76[1])), avg(q41[1], avg(q41[1],q85[1])) );
	_z[13] = avg( avg(q32[2], avg(q32[2],q76[2])), avg(q41[2], avg(q41[2],q85[2])) );
	// Node 15
	_x[14] = avg( avg(q32[0], avg(q32[0],q41[0])), avg(q76[0], avg(q76[0],q85[0])) );
	_y[14] = avg( avg(q32[1], avg(q32[1],q41[1])), avg(q76[1], avg(q76[1],q85[1])) );
	_z[14] = avg( avg(q32[2], avg(q32[2],q41[2])), avg(q76[2], avg(q76[2],q85[2])) );
	// Node 16
	_x[15] = avg( avg(q43[0],q87[0]), avg( avg(q12[0],q43[0]), avg(q56[0],q87[0])) );
	_y[15] = avg( avg(q43[1],q87[1]), avg( avg(q12[1],q43[1]), avg(q56[1],q87[1])) );
	_z[15] = avg( avg(q43[2],q87[2]), avg( avg(q12[2],q43[2]), avg(q56[2],q87[2])) );
	// Node 17
	_x[16] = avg( avg(q67[0], avg(q67[0],q23[0])), avg(q58[0], avg(q58[0],q14[0])) );
	_y[16] = avg( avg(q67[1], avg(q67[1],q23[1])), avg(q58[1], avg(q58[1],q14[1])) );
	_z[16] = avg( avg(q67[2], avg(q67[2],q23[2])), avg(q58[2], avg(q58[2],q14[2])) );
	// Node 18
	_x[17] = avg( avg(q56[0], avg(q56[0],q12[0])), avg(q87[0], avg(q87[0],q43[0])) );
	_y[17] = avg( avg(q56[1], avg(q56[1],q12[1])), avg(q87[1], avg(q87[1],q43[1])) );
	_z[17] = avg( avg(q56[2], avg(q56[2],q12[2])), avg(q87[2], avg(q87[2],q43[2])) );
	// Node 19
	_x[18] = avg( avg(q62[0], avg(q62[0],q51[0])), avg(q73[0], avg(q73[0],q84[0])) );
	_y[18] = avg( avg(q62[1], avg(q62[1],q51[1])), avg(q73[1], avg(q73[1],q84[1])) );
	_z[18] = avg( avg(q62[2], avg(q62[2],q51[2])), avg(q73[2], avg(q73[2],q84[2])) );
	// Node 20
	_x[19] = avg( avg(q73[0], avg(q73[0],q62[0])), avg(q84[0], avg(q84[0],q51[0])) );
	_y[19] = avg( avg(q73[1], avg(q73[1],q62[1])), avg(q84[1], avg(q84[1],q51[1])) );
	_z[19] = avg( avg(q73[2], avg(q73[2],q62[2])), avg(q84[2], avg(q84[2],q51[2])) );
	// Node 21
	_x[20] = avg( avg(q26[0],q37[0]), avg(q48[0],q15[0]) );
	_y[20] = avg( avg(q26[1],q37[1]), avg(q48[1],q15[1]) );
	_z[20] = avg( avg(q26[2],q37[2]), avg(q48[2],q15[2]) );
	// Node 22
	_x[21] = avg( avg(q23[0],q67[0]), avg(q14[0],q58[0]) );
	_y[21] = avg( avg(q23[1],q67[1]), avg(q14[1],q58[1]) );
	_z[21] = avg( avg(q23[2],q67[2]), avg(q14[2],q58[2]) );
	// Node 23
	_x[22] = avg( avg(q12[0],q43[0]), avg(q56[0],q87[0]) );
	_y[22] = avg( avg(q12[1],q43[1]), avg(q56[1],q87[1]) );
	_z[22] = avg( avg(q12[2],q43[2]), avg(q56[2],q87[2]) );
	// Node 24
	_x[23] = avg( avg(q34[0],q73[0]), avg(q21[0],q65[0]) );
	_y[23] = avg( avg(q34[1],q73[1]), avg(q21[1],q65[1]) );
	_z[23] = avg( avg(q34[2],q73[2]), avg(q21[2],q65[2]) );
	// Node 25
	_x[24] = avg( avg(q23[0],q76[0]), avg(q41[0],q85[0]) );
	_y[24] = avg( avg(q23[1],q76[1]), avg(q41[1],q85[1]) );
	_z[24] = avg( avg(q23[2],q76[2]), avg(q41[2],q85[2]) );
	// Node 26
	_x[25] = avg( avg(q73[0],q62[0]), avg(q84[0],q51[0]) );
	_y[25] = avg( avg(q73[1],q62[1]), avg(q84[1],q51[1]) );
	_z[25] = avg( avg(q73[2],q62[2]), avg(q84[2],q51[2]) );
	// Node 27
	_x[26] = avg( avg(h34[0],h26[0]), avg(h48[0],h15[0]) );
	_y[26] = avg( avg(h34[1],h26[1]), avg(h48[1],h15[1]) );
	_z[26] = avg( avg(h34[2],h26[2]), avg(h48[2],h15[2]) );

	for(int i = 0; i < 27; i++)
	{
		curr_elem->x[i] = _x[i];
		curr_elem->y[i] = _y[i];
		curr_elem->z[i] = _z[i];
	}

	internals[elem_id] = curr_elem;

	#pragma omp atomic update
	num_internal += 27;

	char * new_node1 = malloc(50);
	char * new_node2 = malloc(50);
	char * new_node3 = malloc(50);
	char * new_node4 = malloc(50);
	char * new_node5 = malloc(50);
	char * new_node6 = malloc(50);
	char * new_node7 = malloc(50);
	char * new_node8 = malloc(50);
	char * new_node9 = malloc(50);
	char * new_node10 = malloc(50);
	char * new_node11 = malloc(50);
	char * new_node12 = malloc(50);
	char * new_node13 = malloc(50);
	char * new_node14 = malloc(50);
	char * new_node15 = malloc(50);
	char * new_node16 = malloc(50);
	char * new_node17 = malloc(50);
	char * new_node18 = malloc(50);
	char * new_node19 = malloc(50);
	char * new_node20 = malloc(50);
	char * new_node21 = malloc(50);
	char * new_node22 = malloc(50);
	char * new_node23 = malloc(50);
	char * new_node24 = malloc(50);
	char * new_node25 = malloc(50);
	char * new_node26 = malloc(50);
	char * new_node27 = malloc(50);
	snprintf(new_node1, 50, "%d %lf %lf %lf\n", curr_elem->node_id[0], curr_elem->x[0], curr_elem->y[0], curr_elem->z[0]);
	snprintf(new_node2, 50, "%d %lf %lf %lf\n", curr_elem->node_id[1], curr_elem->x[1], curr_elem->y[1], curr_elem->z[1]);
	snprintf(new_node3, 50, "%d %lf %lf %lf\n", curr_elem->node_id[2], curr_elem->x[2], curr_elem->y[2], curr_elem->z[2]);
	snprintf(new_node4, 50, "%d %lf %lf %lf\n", curr_elem->node_id[3], curr_elem->x[3], curr_elem->y[3], curr_elem->z[3]);
	snprintf(new_node5, 50, "%d %lf %lf %lf\n", curr_elem->node_id[4], curr_elem->x[4], curr_elem->y[4], curr_elem->z[4]);
	snprintf(new_node6, 50, "%d %lf %lf %lf\n", curr_elem->node_id[5], curr_elem->x[5], curr_elem->y[5], curr_elem->z[5]);
	snprintf(new_node7, 50, "%d %lf %lf %lf\n", curr_elem->node_id[6], curr_elem->x[6], curr_elem->y[6], curr_elem->z[6]);
	snprintf(new_node8, 50, "%d %lf %lf %lf\n", curr_elem->node_id[7], curr_elem->x[7], curr_elem->y[7], curr_elem->z[7]);
	snprintf(new_node9, 50, "%d %lf %lf %lf\n", curr_elem->node_id[8], curr_elem->x[8], curr_elem->y[8], curr_elem->z[8]);
	snprintf(new_node10, 50, "%d %lf %lf %lf\n", curr_elem->node_id[9], curr_elem->x[9], curr_elem->y[9], curr_elem->z[9]);
	snprintf(new_node11, 50, "%d %lf %lf %lf\n", curr_elem->node_id[10], curr_elem->x[10], curr_elem->y[10], curr_elem->z[10]);
	snprintf(new_node12, 50, "%d %lf %lf %lf\n", curr_elem->node_id[11], curr_elem->x[11], curr_elem->y[11], curr_elem->z[11]);
	snprintf(new_node13, 50, "%d %lf %lf %lf\n", curr_elem->node_id[12], curr_elem->x[12], curr_elem->y[12], curr_elem->z[12]);
	snprintf(new_node14, 50, "%d %lf %lf %lf\n", curr_elem->node_id[13], curr_elem->x[13], curr_elem->y[13], curr_elem->z[13]);
	snprintf(new_node15, 50, "%d %lf %lf %lf\n", curr_elem->node_id[14], curr_elem->x[14], curr_elem->y[14], curr_elem->z[14]);
	snprintf(new_node16, 50, "%d %lf %lf %lf\n", curr_elem->node_id[15], curr_elem->x[15], curr_elem->y[15], curr_elem->z[15]);
	snprintf(new_node17, 50, "%d %lf %lf %lf\n", curr_elem->node_id[16], curr_elem->x[16], curr_elem->y[16], curr_elem->z[16]);
	snprintf(new_node18, 50, "%d %lf %lf %lf\n", curr_elem->node_id[17], curr_elem->x[17], curr_elem->y[17], curr_elem->z[17]);
	snprintf(new_node19, 50, "%d %lf %lf %lf\n", curr_elem->node_id[18], curr_elem->x[18], curr_elem->y[18], curr_elem->z[18]);
	snprintf(new_node20, 50, "%d %lf %lf %lf\n", curr_elem->node_id[19], curr_elem->x[19], curr_elem->y[19], curr_elem->z[19]);
	snprintf(new_node21, 50, "%d %lf %lf %lf\n", curr_elem->node_id[20], curr_elem->x[20], curr_elem->y[20], curr_elem->z[20]);
	snprintf(new_node22, 50, "%d %lf %lf %lf\n", curr_elem->node_id[21], curr_elem->x[21], curr_elem->y[21], curr_elem->z[21]);
	snprintf(new_node23, 50, "%d %lf %lf %lf\n", curr_elem->node_id[22], curr_elem->x[22], curr_elem->y[22], curr_elem->z[22]);
	snprintf(new_node24, 50, "%d %lf %lf %lf\n", curr_elem->node_id[23], curr_elem->x[23], curr_elem->y[23], curr_elem->z[23]);
	snprintf(new_node25, 50, "%d %lf %lf %lf\n", curr_elem->node_id[24], curr_elem->x[24], curr_elem->y[24], curr_elem->z[24]);
	snprintf(new_node26, 50, "%d %lf %lf %lf\n", curr_elem->node_id[25], curr_elem->x[25], curr_elem->y[25], curr_elem->z[25]);
	snprintf(new_node27, 50, "%d %lf %lf %lf\n", curr_elem->node_id[26], curr_elem->x[26], curr_elem->y[26], curr_elem->z[26]);
	all_coords[curr_elem->node_id[0]] = strdup(new_node1);
	all_coords[curr_elem->node_id[1]] = strdup(new_node2);
	all_coords[curr_elem->node_id[2]] = strdup(new_node3);
	all_coords[curr_elem->node_id[3]] = strdup(new_node4);
	all_coords[curr_elem->node_id[4]] = strdup(new_node5);
	all_coords[curr_elem->node_id[5]] = strdup(new_node6);
	all_coords[curr_elem->node_id[6]] = strdup(new_node7);
	all_coords[curr_elem->node_id[7]] = strdup(new_node8);
	all_coords[curr_elem->node_id[8]] = strdup(new_node9);
	all_coords[curr_elem->node_id[9]] = strdup(new_node10);
	all_coords[curr_elem->node_id[10]] = strdup(new_node11);
	all_coords[curr_elem->node_id[11]] = strdup(new_node12);
	all_coords[curr_elem->node_id[12]] = strdup(new_node13);
	all_coords[curr_elem->node_id[13]] = strdup(new_node14);
	all_coords[curr_elem->node_id[14]] = strdup(new_node15);
	all_coords[curr_elem->node_id[15]] = strdup(new_node16);
	all_coords[curr_elem->node_id[16]] = strdup(new_node17);
	all_coords[curr_elem->node_id[17]] = strdup(new_node18);
	all_coords[curr_elem->node_id[18]] = strdup(new_node19);
	all_coords[curr_elem->node_id[19]] = strdup(new_node20);
	all_coords[curr_elem->node_id[20]] = strdup(new_node21);
	all_coords[curr_elem->node_id[21]] = strdup(new_node22);
	all_coords[curr_elem->node_id[22]] = strdup(new_node23);
	all_coords[curr_elem->node_id[23]] = strdup(new_node24);
	all_coords[curr_elem->node_id[24]] = strdup(new_node25);
	all_coords[curr_elem->node_id[25]] = strdup(new_node26);
	all_coords[curr_elem->node_id[26]] = strdup(new_node27);

	char* buff = malloc(255);
	snprintf( buff, 255, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", curr_elem->node_id[0], curr_elem->node_id[1], curr_elem->node_id[2], curr_elem->node_id[3], curr_elem->node_id[4], curr_elem->node_id[5], curr_elem->node_id[6], curr_elem->node_id[7], curr_elem->node_id[8], curr_elem->node_id[9], curr_elem->node_id[10], curr_elem->node_id[11], curr_elem->node_id[12], curr_elem->node_id[13], curr_elem->node_id[14], curr_elem->node_id[15], curr_elem->node_id[16], curr_elem->node_id[17], curr_elem->node_id[18], curr_elem->node_id[19], curr_elem->node_id[20], curr_elem->node_id[21], curr_elem->node_id[22], curr_elem->node_id[23], curr_elem->node_id[24], curr_elem->node_id[25], curr_elem->node_id[26] );
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

