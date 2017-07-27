#ifndef HEX64_H
#define HEX64_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

typedef struct edge {
	int node_id[2];
	// Node 1
	double x1;
	double y1;
	double z1;
	// Node 2
	double x2;
	double y2;
	double z2;
	int inUse;
} edge_t;

typedef struct face {
	int node_id[4];
	// Node 1
	double x1;
	double y1;
	double z1;
	// Node 2
	double x2;
	double y2;
	double z2;
	// Node 3
	double x3;
	double y3;
	double z3;
	// Node 4
	double x4;
	double y4;
	double z4;
	int inUse;
} face_t;

typedef struct internal {
	int node_id[8];
	// Node 1
	double x1;
	double y1;
	double z1;
	// Node 2
	double x2;
	double y2;
	double z2;
	// Node 3
	double x3;
	double y3;
	double z3;
	// Node 4
	double x4;
	double y4;
	double z4;
	// Node 5
	double x5;
	double y5;
	double z5;
	// Node 6
	double x6;
	double y6;
	double z6;
	// Node 7
	double x7;
	double y7;
	double z7;
	// Node 8
	double x8;
	double y8;
	double z8;

	int inUse;
} internal_t;

void toHex64( const char* msh_file );
char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t** faces, int num_elements, internal_t** internals, char* all_coords[num_elements * 19], char ** nodes, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8 );
edge_t getEdge( int n1, int n2, edge_t ** edges );
face_t getFace( int n1, int n2, face_t ** faces );
char* getEdgeNodeId( int elem_id, int num_nodes, edge_t** edges, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2 );
char* getFaceNodeId( int elem_id, int num_nodes, face_t** faces, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2, int n3, int n4 );
char* getInternalNodeId( int elem_id, int num_elements, internal_t** internals, char* all_coords[num_elements*19], char** nodes, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8 );
// Math functions
double avg( double a, double b );
double thirds( double a, double b );

#endif