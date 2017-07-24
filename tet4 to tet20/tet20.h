#ifndef TET20_H
#define TET20_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

typedef struct edge{
	int node_id[2];
	// First node
	double x1;
	double y1;
	double z1;
	// Second node
	double x2;  
	double y2;
	double z2;

	int inUse;
} edge_t;

typedef struct face {
	int node_id;
	double x;
	double y;
	double z;
	int inUse;
} face_t;


void toTet20( const char* msh_file );
double avg( double a, double b );
char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t** faces, int num_elem, char* all_coords[num_elem * 6], char ** nodes, int n1, int n2, int n3, int n4 );
edge_t getEdge( int n1, int n2, edge_t ** edges );
char* getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*6], char ** nodes, int n1, int n2 );
int getFaceNodeId( int elem_id, face_t ** faces, int num_elem, char* all_coords[num_elem*6], char ** nodes, int n1, int n2, int n3 );
face_t getFace( int n1, int n2, int n3, face_t ** faces );
int min3( int a, int b, int c );
int snd_min( int a, int b, int c );

#endif