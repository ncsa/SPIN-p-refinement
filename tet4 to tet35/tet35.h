#ifndef TET35_H
#define TET35_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

typedef struct edge{
	int node_id[3];
	// First node
	double x1;
	double y1;
	double z1;
	// Second node
	double x2;  
	double y2;
	double z2;
	// Third node
	double x3;
	double y3;
	double z3;

	int inUse;
} edge_t;

typedef struct face {
	int node_id[3];
	// First node
	double x1;
	double y1;
	double z1;
	// Second node
	double x2;
	double y2;
	double z2;
	// Third node
	double x3;
	double y3;
	double z3;

	int inUse;
} face_t;

typedef struct internal {
	int node_id;
	double x;
	double y;
	double z;
} internal_t;


void toTet35( const char* msh_file );
char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t * faces, internal_t** internals, int num_elem, char* all_coords[num_elem * 31], char ** nodes, int n1, int n2, int n3, int n4 );
edge_t getEdge( int n1, int n2, edge_t ** edges );
face_t getFace( int n1, int n2, int n3, int num_nodes, face_t * faces );
char* getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*31], char ** nodes, int n1, int n2 );
char* getFaceNodeId( int elem_id, int num_nodes, face_t * faces, int num_elem, char* all_coords[num_elem*31], char ** nodes, int n1, int n2, int n3 );
int getInternalNodeId( int elem_id, int num_elem, internal_t** internals, char* all_coords[num_elem*31], char** nodes, int n1, int n2, int n3, int n4 );
// Math functions
double avg( double a, double b );
double avg3( double a, double b, double c );
double avg4( double a, double b, double c, double d );
int min3( int a, int b, int c );
int snd_min( int a, int b, int c );
int max3( int a, int b, int c );
double thirds( double a, double b );

#endif