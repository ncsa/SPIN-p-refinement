#ifndef TET4_PARALLEL_H
#define TET4_PARALLEL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

/**
 * @brief Edge object
 *
 * Edge object contains universal node id and 3 double values representing the edge's cartesian coordinates
 */
typedef struct edge{
	int node_id; /**< Node ID unique to each edge */
	double x;    /**< X coordinate of edge */
	double y;    /**< Y coordinate of edge */
	double z;    /**< Z coordinate of edge */
	int inUse;   /**< inUse bit to represent whether the current object has been constructed */
} edge_t;

/**
* Method to be called from main function that does all reading, constructing, and printing of new elements
* @author Dan Gross
* @date 30 July 2017
* @param msh_file .msh file to read data from
*/
void toTet10( const char* msh_file );
/**
* Method that takes all necessary objects of a certain element and constructs a refined element represented by a char*
* @author Dan Gross
* @date 30 July 2017
* @param elem_frag Fragment of original element that remains unchanged in refined element (first 8 nodes)
* @param elem_id Element ID
* @param num_nodes Total number of original nodes in mesh
* @param edges 2-D array storing all edge objects
* @param num_elem Number of elements in mesh
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 Node 1 in Tet-4 element
* @param n2 Node 2 in Tet-4 element
* @param n3 Node 3 in Tet-4 element
* @param n4 Node 4 in Tet-4 element
* @return char* representation of refined element
*/
char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, int num_elem, char* all_coords[num_elem * 6], char ** nodes, int n1, int n2, int n3, int n4 );
/**
* Method that returns the edge object represented by the two nodes n1 and n2
* @author Dan Gross
* @date 30 July 2017
* @param n1 First node associated with edge
* @param n2 Second node associated with edge
* @param edges 2-D array storing all edge objects
* @return Edge associated with the two nodes
*/
edge_t getEdge( int n1, int n2, edge_t ** edges );
/**
* Method that first searches to see if the edge associated with Nodes n1 and n2 exists, and if not constructs the new edge
* @author Dan Gross
* @date 30 July 2017
* @param elem_id Element ID
* @param edges 2-D array storing all edge objects
* @param num_elem Number of elements in mesh
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 First node associated with edge
* @param n2 Second node associated with edge
* @return Edge Node ID for use in new element
*/
int getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*6], char ** nodes, int n1, int n2 );
// Math functions
/**
* Math function that returns average of two doubles
* @author Dan Gross
* @date 30 July 2017
* @param a First arg
* @param b Second arg
* @return Average of a and b
*/
double avg( double a, double b );

#endif