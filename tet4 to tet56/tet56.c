#include "tet56.h"

/*
*   Program to convert tet4 elements in mesh to tet10 elements
*   Writes to new file name (msh_file)_tet10.msh
*   @para msh_file original .msh file containing mesh
*/

static int num_edges = 0;
static int edge_id = 0;
static int num_faces = 0;
static int num_internal = 0;

void toTet56( const char* msh_file )
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

	char ** all_coords = malloc(num_elements * 52 * sizeof(char*));

	char ** elements = malloc((num_elements) * sizeof(char*));

	edge_t** edges = (edge_t**) calloc(num_nodes, sizeof(edge_t));
	for(int i = 0; i < num_nodes; i++)
	{
		edges[i] = (edge_t*) calloc(num_nodes, sizeof(edge_t));
	}

	#define faces(i,j,k) (faces[num_nodes*num_nodes*i + num_nodes*j + k])
	face_t * faces = (face_t*) calloc(num_nodes*num_nodes*num_nodes, sizeof(face_t));

	internal_t** internals = malloc(num_elements * sizeof(internal_t*));

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
	//#pragma omp parallel for schedule(static) shared(edge_id, num_edges)
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

			char * new_elem = constructElem( elem_frag, i, num_nodes, edges, faces, internals, num_elements, all_coords, nodes, n1, n2, n3, n4 );
			elements[i] = malloc(strlen(new_elem) + 1);
			strcpy(elements[i], new_elem);
			// Fix this maybe ... inefficient
		}
	}
	end = clock();

	printf("Nodes: %d\n", num_nodes + num_edges + num_faces + num_internal);

	printf("AVG Elem Construct Time: %lf\n", ((double)(end-begin)/CLOCKS_PER_SEC));

	// All new elements and nodes collected
	// Rewrite to msh file and convert to vtk

	char* _msh_file = strdup(msh_file);
	_msh_file[ strlen(_msh_file) - 4] = '\0';
	char new_msh_file[255];
	snprintf( new_msh_file, 255, "%s_tet56.msh", _msh_file );
	FILE * new_msh = fopen( new_msh_file, "w+" );

	fputs("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n$Nodes\n", new_msh);

	fprintf(new_msh, "%d\n", ( num_nodes + num_edges + num_faces + num_internal ));

	for(int i = 0; i < num_nodes; i++)
	{
		fprintf(new_msh, "%d %s", (i+1), nodes[i]);
		free(nodes[i]);
	}

	for(int i = num_nodes + 1; i < num_edges + num_nodes + num_faces + num_internal + 1; i++)
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

char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t* faces, internal_t** internals, int num_elem, char* all_coords[num_elem * 52], char ** nodes, int n1, int n2, int n3, int n4 )
{
	// Node ordering based off of .msh file format

	// Edge nodes
	// Edge_1
	char* edge_1 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n1, n2 );

	// Edge_2
	char* edge_2 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n2, n3 );

	// Edge_3
	char* edge_3 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n3, n1 );

	// Edge_4
	char* edge_4 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n4, n1 );

	// Edge_5
	char* edge_5 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n4, n3 );

	// Edge_6
	char* edge_6 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n4, n2 );

	// Face Nodes
	// Face_1
	char* face_1 = getFaceNodeId( elem_id, num_nodes, faces, num_elem, all_coords, nodes, n1, n2, n3 );

	// Face_2
	char* face_2 = getFaceNodeId( elem_id, num_nodes, faces, num_elem, all_coords, nodes, n1, n2, n4 );

	// Face_3
	char* face_3 = getFaceNodeId( elem_id, num_nodes, faces, num_elem, all_coords, nodes, n1, n3, n4 );

	// Face_4
	char* face_4 = getFaceNodeId( elem_id, num_nodes, faces, num_elem, all_coords, nodes, n2, n3, n4 );

	// Internal nodes
	char* internal = getInternalNodeId( elem_id, num_elem, internals, all_coords, nodes, n1, n2, n3, n4 );

	char* new_elem = malloc(255);
	snprintf( new_elem, 255, "%d 31 2 0 0 %s %s %s %s %s %s %s %s %s %s %s %d\n", elem_id + 1, elem_frag, edge_1, edge_2, edge_3, edge_4, edge_5, edge_6, face_1, face_2, face_3, face_4, internal );

	return new_elem;
}

// Gets and returns node id for two given nodes
char* getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*52], char ** nodes, int n1, int n2 )
{
	edge_t curr_edge;
	edge_t curr_edge_rev;

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
				for(int i = 0; i < 4; i++)
				{
					curr_edge.node_id[i] = edge_id;
					curr_edge_rev.node_id[3-i] = edge_id;
					edge_id++;
				}
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
		num_edges += 4;

		// Set coords
		char* node_1 = nodes[n1 - 1];
		char* node_2 = nodes[n2 - 1];
		double x1, x2, y1, y2, z1, z2;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);

		double _x[4];
		double _y[4];
		double _z[4];

		_x[0] = fifths(x1, x2);
		_y[0] = fifths(y1, y2);
		_z[0] = fifths(z1, z2);
		_x[1] = two_fifths(x1, x2);
		_y[1] = two_fifths(y1, y2);
		_z[1] = two_fifths(z1, z2);
		_x[2] = two_fifths(x2, x1);
		_y[2] = two_fifths(y2, y1);
		_z[2] = two_fifths(z2, z1);
		_x[3] = fifths(x2, x1);
		_y[3] = fifths(y2, y1);
		_z[3] = fifths(z2, z1);

		// Must set coords to correct order
		for(int i = 0; i < 4; i++)
		{
			curr_edge.x[i] = _x[i];
			curr_edge.y[i] = _y[i];
			curr_edge.z[i] = _z[i];

			curr_edge_rev.x[3-i] = _x[3-i];
			curr_edge_rev.y[3-i] = _y[3-i];
			curr_edge_rev.z[3-i] = _z[3-i];
		}

		edges[n1 - 1][n2 - 1] = curr_edge;
		edges[n2 - 1][n1 - 1] = curr_edge_rev;


		char * new_node1 = malloc(100);
		char * new_node2 = malloc(100);
		char * new_node3 = malloc(100);
		char * new_node4 = malloc(100);
		snprintf(new_node1, 100, "%d %lf %lf %lf\n", curr_edge.node_id[0], curr_edge.x[0], curr_edge.y[0], curr_edge.z[0]);
		snprintf(new_node2, 100, "%d %lf %lf %lf\n", curr_edge.node_id[1], curr_edge.x[1], curr_edge.y[1], curr_edge.z[1]);
		snprintf(new_node3, 100, "%d %lf %lf %lf\n", curr_edge.node_id[2], curr_edge.x[2], curr_edge.y[2], curr_edge.z[2]);
		snprintf(new_node4, 100, "%d %lf %lf %lf\n", curr_edge.node_id[3], curr_edge.x[3], curr_edge.y[3], curr_edge.z[3]);

		all_coords[curr_edge.node_id[0]] = new_node1;
		all_coords[curr_edge.node_id[1]] = new_node2;
		all_coords[curr_edge.node_id[2]] = new_node3;
		all_coords[curr_edge.node_id[3]] = new_node4;
	}

	char* buff = malloc(25);
	snprintf( buff, 10, "%d %d %d %d", curr_edge.node_id[0], curr_edge.node_id[1], curr_edge.node_id[2], curr_edge.node_id[3] );
	return buff;
}

// Gets and returns node id for two given nodes
char* getFaceNodeId( int elem_id, int num_nodes, face_t* faces, int num_elem, char* all_coords[num_elem*52], char ** nodes, int n1, int n2, int n3 )
{
	#define faces(i,j,k) (faces[num_nodes*num_nodes*i + num_nodes*j + k])

	face_t curr_face;

	// If two threads read the same edge at the same time it is race, this is v. slow
	// Could try and insure no two threads every have same n1, n2
	curr_face = getFace( n1, n2, n3, num_nodes, faces );
	printf("faces[%d][%d][%d]\n", n1, n2, n3);
	
	if( curr_face.inUse == 0 )
	{
		int retest = 0;
		// Race condition
		#pragma omp critical
		{
			// Retest edge
			curr_face = getFace( n1, n2, n3, num_nodes, faces );
			if(curr_face.inUse == 0)
			{
				for(int i = 0; i < 6; i++)
				{
					curr_face.node_id[i] = edge_id;
					edge_id++;
				}

				curr_face.inUse = 1;
				// Store in faces
				int id_1 = min3(n1, n2, n3);
				int id_2 = snd_min(n1, n2, n3);
				int id_3 = max3(n1, n2, n3);
				faces(id_1 - 1, id_2 - 1, id_3 - 1) = curr_face;
			}
			else
			{
				retest = 1;
			}
		}

		if(retest == 1)
		{
			printf("found duplicate\n");
			char * buff = malloc(50);
			snprintf( buff, 50, "%d %d %d %d %d %d", curr_face.node_id[0], curr_face.node_id[1], curr_face.node_id[2], curr_face.node_id[3], curr_face.node_id[4], curr_face.node_id[5] );
			return buff;
		}

		#pragma omp atomic update
		num_faces+= 6;

		// Set coords
		char* node_1 = strdup(nodes[n1 - 1]);
		char* node_2 = strdup(nodes[n2 - 1]);
		char* node_3 = strdup(nodes[n3 - 1]);
		double x1, x2, x3, y1, y2, y3, z1, z2, z3;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
		sscanf(node_3, "%lf %lf %lf", &x3, &y3, &z3);

		double _x[6];
		double _y[6];
		double _z[6];

		_x[0] = avg( two_fifths(x1,x2), two_fifths(x1,x3) );
		_y[0] = avg( two_fifths(y1,y2), two_fifths(y1,y3) );
		_z[0] = avg( two_fifths(z1,z2), two_fifths(z1,z3) );
		_x[1] = avg( two_fifths(x2,x1), two_fifths(x2,x3) );
		_y[1] = avg( two_fifths(y2,y1), two_fifths(y2,y3) );
		_z[1] = avg( two_fifths(z2,z1), two_fifths(z2,z3) );
		_x[2] = avg( two_fifths(x3,x1), two_fifths(x3,x2) );
		_y[2] = avg( two_fifths(y3,y1), two_fifths(y3,y2) );
		_z[2] = avg( two_fifths(z3,z1), two_fifths(z3,z2) );
		_x[3] = avg( fifths(x1,x3), fifths(x2,x3) );
		_y[3] = avg( fifths(y1,y3), fifths(y2,y3) );
		_z[3] = avg( fifths(z1,z3), fifths(z2,z3) );
		_x[4] = avg( fifths(x2,x1), fifths(x3,x1) );
		_y[4] = avg( fifths(y2,y1), fifths(y3,y1) );
		_z[4] = avg( fifths(z2,z1), fifths(z3,z1) );
		_x[5] = avg( fifths(x1,x2), fifths(x3,x2) );
		_y[5] = avg( fifths(y1,y2), fifths(y3,y2) );
		_z[5] = avg( fifths(z1,z2), fifths(z3,z2) );

		for(int i = 0; i < 6; i++)
		{
			curr_face.x[i] = _x[i];
			curr_face.y[i] = _y[i];
			curr_face.z[i] = _z[i];
		}

		curr_face.inUse = 1;

		// Store in faces
		int id_1 = min3(n1, n2, n3);
		int id_2 = snd_min(n1, n2, n3);
		int id_3 = max3(n1, n2, n3);
		faces(id_1 - 1, id_2 - 1, id_3 - 1) = curr_face;
		//printf("creating face %d %d %d\n", id_1, id_2, id_3);

		char * new_node1 = malloc(100);
		char * new_node2 = malloc(100);
		char * new_node3 = malloc(100);
		snprintf(new_node1, 100, "%d %lf %lf %lf\n", curr_face.node_id[0], curr_face.x1, curr_face.y1, curr_face.z1);
		snprintf(new_node2, 100, "%d %lf %lf %lf\n", curr_face.node_id[1], curr_face.x2, curr_face.y2, curr_face.z2);
		snprintf(new_node3, 100, "%d %lf %lf %lf\n", curr_face.node_id[2], curr_face.x3, curr_face.y3, curr_face.z3);

		all_coords[curr_face.node_id[0]] = new_node1;
		all_coords[curr_face.node_id[1]] = new_node2;
		all_coords[curr_face.node_id[2]] = new_node3;
	}
	else
	{
		printf("found face[%d][%d][%d]\n 	%s\n 	%s\n 	%s\n", n1, n2, n3, all_coords[curr_face.node_id[0]], all_coords[curr_face.node_id[1]], all_coords[curr_face.node_id[2]]);
	}

	char* buff = malloc(50);
	snprintf( buff, 50, "%d %d %d", curr_face.node_id[0], curr_face.node_id[1], curr_face.node_id[2] );
	return buff;
}

char* getInternalNodeId( int elem_id, int num_elem, internal_t** internals, char* all_coords[num_elem*52], char** nodes, int n1, int n2, int n3, int n4 )
{
	internal_t * curr_elem = malloc(sizeof(internal_t));

	#pragma omp critical
	{
		for(int i = 0; i < 4; i++)
		{
			curr_elem->node_id[i] = edge_id;
			edge_id++;
		}
	}

	printf("creating internal[%d][%d][%d][%d]\n", n1, n2, n3, n4);

	// Set coords
	char* node_1 = strdup(nodes[n1 - 1]);
	char* node_2 = strdup(nodes[n2 - 1]);
	char* node_3 = strdup(nodes[n3 - 1]);
	char* node_4 = strdup(nodes[n4 - 1]);
	double x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;
	sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
	sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
	sscanf(node_3, "%lf %lf %lf", &x3, &y3, &z3);
	sscanf(node_4, "%lf %lf %lf", &x4, &y4, &z4);

	double _x[4];
	double _y[4];
	double _z[4];

	_x[0] = 

	curr_elem->x = avg4(x1, x2, x3, x4);
	curr_elem->y = avg4(y1, y2, y3, y4);
	curr_elem->z = avg4(z1, z2, z3, z4);

	internals[elem_id] = curr_elem;

	#pragma omp atomic update
	num_internal += 4;

	char new_node[100];
	snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_elem->node_id, curr_elem->x, curr_elem->y, curr_elem->z);
	all_coords[curr_elem->node_id] = strdup(new_node);

	return curr_elem->node_id;
}

// Lookup edge in edges
edge_t getEdge( int n1, int n2, edge_t ** edges )
{
	return edges[n1 - 1][n2 - 1];
}

// Lookup face in faces. The correct face is stored in the lowest to indices possible. I.E. Face[1,2,3] is stored in faces[1][2]
face_t getFace( int n1, int n2, int n3, int num_nodes, face_t* faces )
{
	#define faces(i,j,k) (faces[num_nodes*num_nodes*i + num_nodes*j + k])

	int id_1 = min3( n1, n2, n3 );
	int id_2 = snd_min( n1, n2, n3 );
	int id_3 = max3( n1, n2, n3 );
	return faces(id_1 - 1, id_2 - 1, id_3 - 1);
}

// Simple averaging function
double avg( double a, double b )
{
	return (a+b) / 2.0;
}

double avg3( double a, double b, double c )
{
	return (a+b+c) / 3.0;
}

double avg4( double a, double b, double c, double d )
{
	return (a+b+c+d) / 4.0; 
}

// Simple min function with three parameters
int min3( int a, int b, int c )
{
	int min = a;
	if( b < min )
	{
		min = b;
	}
	if( c < min )
	{
		min = c;
	}

	return min;
}

// Returns the middle integer
int snd_min( int a, int b, int c )
{
	int min = min3( a, b, c );
	if(min == a)
	{
		return b < c ? b : c; 
	}
	else if(min == b)
	{
		return a < c ? a : c;
	}
	else
	{
		return a < b ? a : b;
	}
}

// Simple max function
int max3( int a, int b, int c )
{
	int max = a;
	if( b > max )
	{
		max = b;
	}
	if( c > max )
	{
		max = c;
	}

	return max;
}

// Get the third distance from point a
double thirds( double a, double b )
{
	double third_a = a * 2/3;
	double third_b = b * 1/3;
	double ret = third_a + third_b;
	return ret;
}

double fifths( double a, double b )
{
	double fifth_a = a * 4/5;
	double fifth_b = b * 1/5;
	double ret = fifth_a + fifth_b;
	return ret;
}

double two_fifths( double a, double b )
{
	double fifth_a = a * 3/5;
	double fifth_b = b * 2/5;
	double ret = fifth_a + fifth_b;
	return ret;
}


