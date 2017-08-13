#ifndef TET35_H
#define TET35_H

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
	int node_id[3];  /**< Node IDs unique to each node in edge */
	// First node
	double x1;       /**< X coordinate of first node in edge */
	double y1;       /**< Y coordinate of first node in edge */
	double z1;       /**< Z coordinate of first node in edge */
	// Second node
	double x2;       /**< X coordinate of second node in edge */
	double y2;  	 /**< Y coordinate of second node in edge */
	double z2;		 /**< Z coordinate of second node in edge */
	// Third node
	double x3;		 /**< X coordinate of third node in edge */
	double y3;  	 /**< Y coordinate of third node in edge */ 
	double z3;		 /**< Z coordinate of third node in edge */

	int inUse;		 /**< inUse bit to represent whether the current object has been constructed */
} edge_t;

/**
 * @brief Face object
 *
 * Face object contains universal node id and 3 double values representing the edge's cartesian coordinates
 */
typedef struct face {
	int node_id[3];  /**< Node IDs unique to each node in edge */
	// First node
	double x1;       /**< X coordinate of first node in edge */
	double y1;       /**< Y coordinate of first node in edge */
	double z1;       /**< Z coordinate of first node in edge */
	// Second node
	double x2;       /**< X coordinate of second node in edge */
	double y2;  	 /**< Y coordinate of second node in edge */
	double z2;		 /**< Z coordinate of second node in edge */
	// Third node
	double x3;		 /**< X coordinate of third node in edge */
	double y3;  	 /**< Y coordinate of third node in edge */ 
	double z3;		 /**< Z coordinate of third node in edge */

	int inUse;		 /**< inUse bit to represent whether the current object has been constructed */
} face_t;

/**
 * @brief Internal object
 *
 * Internal object contains universal node id and 3 double values representing the edge's cartesian coordinates
 */
typedef struct internal {
	int node_id;  /**< Node ID unique to each internal */
	double x;     /**< X coordinate of edge */
	double y;     /**< Y coordinate of edge */
 	double z;     /**< Z coordinate of edge */
} internal_t;

/**
* Method to be called from main function that does all reading, constructing, and printing of new elements
* @author Dan Gross
* @date 30 July 2017
* @param msh_file .msh file to read data from
*/
void toTet35( const char* msh_file );
/**
* Method that takes all necessary objects of a certain element and constructs a refined element represented by a char*
* @author Dan Gross
* @date 30 July 2017
* @param elem_frag Fragment of original element that remains unchanged in refined element (first 8 nodes)
* @param elem_id Element ID
* @param num_nodes Total number of original nodes in mesh
* @param edges 2-D array storing all edge objects
* @param faces 3-D array storing all face objects
* @param internals Array storing all internal objects
* @param num_elem Number of elements in mesh
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 Node 1 in Tet-4 element
* @param n2 Node 2 in Tet-4 element
* @param n3 Node 3 in Tet-4 element
* @param n4 Node 4 in Tet-4 element
* @return char* representation of refined element
*/
char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t * faces, internal_t** internals, int num_elem, char* all_coords[num_elem * 31], char ** nodes, int n1, int n2, int n3, int n4 );
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
* Method that returns the face object represented by the nodes n1, n2, and n3
* @author Dan Gross
* @date 30 July 2017
* @param n1 First node associated with face
* @param n2 Second node associated with face
* @oaram n3 Third node associated with face
* @param num_nodes Number of nodes in original mesh
* @param faces 2-D array storing all face objects
* @return Face associated with the three nodes
*/
face_t getFace( int n1, int n2, int n3, int num_nodes, face_t * faces );
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
* @return char* representation of Edge Node IDs for use in new element
*/
char* getEdgeNodeId( int elem_id, edge_t ** edges, int num_elem, char* all_coords[num_elem*31], char ** nodes, int n1, int n2 );
/**
* Method that first searches to see if the face associated with Nodes n1, n2, and n3 exists, and if not constructs the new edge
* @author Dan Gross
* @date 30 July 2017
* @param elem_id Element ID
* @param num_nodes Number of nodes in original mesh
* @param faces 2-D array storing all face objects
* @param num_elem Number of elements in mesh
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 First node associated with face
* @param n2 Second node associated with face
* @param n3 Third node associated with face
* @return char* representation of Face Node IDs for use in new element
*/
char* getFaceNodeId( int elem_id, int num_nodes, face_t * faces, int num_elem, char* all_coords[num_elem*31], char ** nodes, int n1, int n2, int n3 );
/**
* Method that constructs the internal node associated with element defined by nodes n1, n2, n3, and n4
* @author Dan Gross
* @date 30 July 2017
* @param elem_id Element ID
* @param num_nodes Total number of original nodes in mesh
* @param internals 2-D array storing all internal objects
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 First node associated with internal
* @param n2 Second node associated with internal
* @param n3 Third node associated with internal
* @param n4 Fourth node associated with internal
* @return Internal Node ID for use in new element
*/
int getInternalNodeId( int elem_id, int num_elem, internal_t** internals, char* all_coords[num_elem*31], char** nodes, int n1, int n2, int n3, int n4 );
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
/**
* Math function that returns the average value of three double values
* @author Dan Gross
* @date 30 July 2017
* @param a First arg
* @param b Second arg
* @param c Third arg
* @return Average value of a, b, and c
*/ 
double avg3( double a, double b, double c );
/**
* Math function that returns the average value of four double values
* @author Dan Gross
* @date 30 July 2017
* @param a First arg
* @param b Second arg
* @param c Third arg
* @param d Fourth arg
* @return Average value of a, b, c, and d
*/ 
double avg4( double a, double b, double c, double d );
/**
* Math function that returns the minimum of three int values
* @author Dan Gross
* @date 30 July 2017
* @param a First arg
* @param b Second arg
* @param c Third arg
* @return Minimum value of a, b, and c
*/ 
int min3( int a, int b, int c );
/**
* Math function that returns the middle value of three int values
* @author Dan Gross
* @date 30 July 2017
* @param a First arg
* @param b Second arg
* @param c Third arg
* @return Middle value of a, b, and c
*/ 
int snd_min( int a, int b, int c );
/**
* Math function that returns the maximum value of three int values
* @author Dan Gross
* @date 30 July 2017
* @param a First arg
* @param b Second arg
* @param c Third arg
* @return Maximum value of a, b, and c
*/ 
int max3( int a, int b, int c );
/**
* Math function that computes the distance one third of the way between a and b
* @author Dan Gross
* @date 30 July 2017
* @param a First arg
* @param b Second arg
* @return The 'third' distance from a to b
*/ 
double thirds( double a, double b );

#endif