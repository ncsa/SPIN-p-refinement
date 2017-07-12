#ifndef HEX8_H
#define HEX8_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct edge {
	int node_id;
	int num_elem;
	int elements[4];
	double x;
	double y;
	double z;
	int inUse;
} edge_t;

typedef struct face {
	int node_id;
	int num_elem;
	int elements[4];
	double x;
	double y;
	double z;
	int inUse;
} face_t;

typedef struct internal {
	int node_id;
	double x;
	double y;
	double z;
	int inUse;
} internal_t;

void toHex27( const char* msh_file );
double avg( double a, double b );
char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t*** edges, face_t*** faces, int num_elements, internal_t** internals, char* all_coords[num_elements * 19], char ** nodes, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8 );
edge_t* getEdge( int n1, int n2, int num_nodes, edge_t *** edges );
face_t * getFace( int n1, int n2, int num_nodes, face_t *** faces );
int getEdgeNodeId( int elem_id, int num_nodes, edge_t*** edges, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2 );
int getFaceNodeId( int elem_id, int num_nodes, face_t*** faces, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2, int n3, int n4 );
int getInternalNodeId( int elem_id, int num_elements, internal_t** internals, char* all_coords[num_elements*19], char** nodes, int n1, int n2 );

#endif