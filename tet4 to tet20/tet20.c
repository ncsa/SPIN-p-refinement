#include "tet20.h"

/*
*   Program to convert tet4 elements in mesh to tet10 elements
*   Writes to new file name (msh_file)_tet10.msh
*   @para msh_file original .msh file containing mesh
*/
// gcc tet4.c main.c -o toTet10

static int num_edges = 0;
static int edge_id = 0;
static int num_faces = 0;

void toTet20( const char* msh_file )
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

	char ** all_coords = malloc(num_elements * 16 * sizeof(char*));

	char ** elements = malloc((num_elements) * sizeof(char*));

	edge_t** edges = (edge_t**) calloc(num_nodes, sizeof(edge_t));
	for(int i = 0; i < num_nodes; i++)
	{
		edges[i] = (edge_t*) calloc(num_nodes, sizeof(edge_t));
	}

	#define faces(i,j,k) (faces[num_nodes*num_nodes*i + num_nodes*j + k])
	face_t * faces = (face_t*) calloc(num_nodes*num_nodes*num_nodes, sizeof(face_t));

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

			char * new_elem = constructElem( elem_frag, i, num_nodes, edges, faces, num_elements, all_coords, nodes, n1, n2, n3, n4 );
			elements[i] = malloc(strlen(new_elem) + 1);
			strcpy(elements[i], new_elem);
			// Fix this maybe ... inefficient
		}
	}
	end = clock();

	printf("Nodes: %d\n", num_nodes + num_edges + num_faces);

	printf("AVG Elem Construct Time: %lf\n", ((double)(end-begin)/CLOCKS_PER_SEC));

	// All new elements and nodes collected
	// Rewrite to msh file and convert to vtk

	char* _msh_file = strdup(msh_file);
	_msh_file[ strlen(_msh_file) - 4] = '\0';
	char new_msh_file[255];
	snprintf( new_msh_file, 255, "%s_tet20.msh", _msh_file );
	FILE * new_msh = fopen( new_msh_file, "w+" );

	fputs("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n$Nodes\n", new_msh);

	fprintf(new_msh, "%d\n", ( num_nodes + num_edges + num_faces ));

	for(int i = 0; i < num_nodes; i++)
	{
		fprintf(new_msh, "%d %s", (i+1), nodes[i]);
		free(nodes[i]);
	}

	for(int i = num_nodes + 1; i < num_edges + num_nodes + num_faces + 1; i++)
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

char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t* faces, int num_elem, char* all_coords[num_elem * 6], char ** nodes, int n1, int n2, int n3, int n4 )
{
	int ids[4];

	// Node ordering based off of .msh file format

	// Edge nodes
	// edge_5 and edge_6
	char* edge_1 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n1, n2 );

	// edge_7 and edge_8
	char* edge_2 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n2, n3 );

	// edge_9 and edge_10
	char* edge_3 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n3, n1 );

	// edge_11 and edge_12
	char* edge_4 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n4, n1 );

	// edge_13 and edge_14
	char* edge_5 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n4, n3 );

	// edge_15 and edge_16
	char* edge_6 = getEdgeNodeId( elem_id, edges, num_elem, all_coords, nodes, n4, n2 );

	// Face Nodes
	// face_17
	ids[0] = getFaceNodeId( elem_id, num_nodes, faces, num_elem, all_coords, nodes, n1, n2, n3 );

	// face_18
	ids[1] = getFaceNodeId( elem_id, num_nodes, faces, num_elem, all_coords, nodes, n1, n2, n4);

	// face_19
	ids[2] = getFaceNodeId( elem_id, num_nodes, faces, num_elem, all_coords, nodes, n1, n3, n4 );

	// face_20
	ids[3] = getFaceNodeId( elem_id, num_nodes, faces, num_elem, all_coords, nodes, n2, n3, n4 );

	char* new_elem = malloc(255);
	snprintf( new_elem, 255, "%d 29 2 0 0 %s %s %s %s %s %s %s %d %d %d %d\n", elem_id + 1, elem_frag, edge_1, edge_2, edge_3, edge_4, edge_5, edge_6, ids[0], ids[1], ids[2], ids[3] );

	return new_elem;
}

// Gets and returns node id for two given nodes
char* getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*6], char ** nodes, int n1, int n2 )
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
				curr_edge.node_id[0] = edge_id;
				curr_edge_rev.node_id[0] = edge_id;
				edge_id++;
				curr_edge.node_id[1] = edge_id;
				curr_edge_rev.node_id[1] = edge_id;
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
			char * buff = malloc(10);
			snprintf( buff, 10, "%d %d", curr_edge.node_id[0], curr_edge.node_id[1] );
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

		double _x1 = avg( avg(x1, x2), x1);
		double _y1 = avg( avg(y1, y2), y1);
		double _z1 = avg( avg(z1, z2), z1);
		double _x2 = avg( avg(x1, x2), x2);
		double _y2 = avg( avg(y1, y2), y2);
		double _z2 = avg( avg(z1, z2), z2);

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

		edges[n1 - 1][n2 - 1] = curr_edge;
		edges[n2 - 1][n1 - 1] = curr_edge_rev;


		char * new_node1 = malloc(100);
		char * new_node2 = malloc(100);
		snprintf(new_node1, 100, "%d %lf %lf %lf\n", curr_edge.node_id[0], curr_edge.x1, curr_edge.y1, curr_edge.z1);
		snprintf(new_node2, 100, "%d %lf %lf %lf\n", curr_edge.node_id[1], curr_edge.x2, curr_edge.y2, curr_edge.z2);
		
		//if(all_coords[curr_edge.node_id[0]] != NULL)
		//	printf("overwriting %d\n", curr_edge.node_id);

		all_coords[curr_edge.node_id[0]] = new_node1;
		all_coords[curr_edge.node_id[1]] = new_node2;
	}

	char* buff = malloc(10);
	snprintf( buff, 10, "%d %d", curr_edge.node_id[0], curr_edge.node_id[1] );
	return buff;
}

// Gets and returns node id for two given nodes
int getFaceNodeId( int elem_id, int num_nodes, face_t* faces, int num_elem, char* all_coords[num_elem*6], char ** nodes, int n1, int n2, int n3 )
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
				curr_face.node_id = edge_id;
				edge_id++;
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
			return curr_face.node_id;
		}

		#pragma omp atomic update
		num_faces++;

			// Set coords
		char* node_1 = strdup(nodes[n1 - 1]);
		char* node_2 = strdup(nodes[n2 - 1]);
		char* node_3 = strdup(nodes[n3 - 1]);
		double x1, x2, x3, y1, y2, y3, z1, z2, z3;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
		sscanf(node_3, "%lf %lf %lf", &x3, &y3, &z3);
		curr_face.x = avg3(x1, x2, x3);
		curr_face.y = avg3(y1, y2, y3);
		curr_face.z = avg3(z1, z2, z3);
		curr_face.inUse = 1;

		// Store in faces
		int id_1 = min3(n1, n2, n3);
		int id_2 = snd_min(n1, n2, n3);
		int id_3 = max3(n1, n2, n3);
		faces(id_1 - 1, id_2 - 1, id_3 - 1) = curr_face;
		printf("creating face %d %d %d\n", id_1, id_2, id_3);

		char * new_node = malloc(100);
		snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_face.node_id, curr_face.x, curr_face.y, curr_face.z);

		if(all_coords[curr_face.node_id] != NULL)
			printf("overwriting %d\n", curr_face.node_id);

		all_coords[curr_face.node_id] = new_node;
	}
	else
	{
		printf("face found: %d %d %d\n", n1, n2, n3);
	}

	return curr_face.node_id;
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

// Simple min function
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


