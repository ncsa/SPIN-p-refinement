#ifndef TET4_OBJECT_H
#define TET4_OBJECT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct edge{
	int node_id;
	int num_elem;
	int elements[10]; // unsure about this line
	double x;
	double y;
	double z;
	int inUse;
} edge_t;

void toTet10( const char* msh_file );
double avg( double a, double b );
char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, int num_elem, char* all_coords[num_elem * 6], char ** nodes, int n1, int n2, int n3, int n4 );
edge_t getEdge( int n1, int n2, edge_t ** edges );
int getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*6], char ** nodes, int n1, int n2 );

#endif