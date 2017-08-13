#ifndef HEX64_H
#define HEX64_H

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
typedef struct edge {
	int node_id[2];  /**< Node IDs unique to each edge */
	// Node 1
	double x1;       /**< X coordinate of first node in edge */
	double y1;		 /**< Y coordinate of first node in edge */
	double z1;		 /**< Z coordinate of first node in edge */
	// Node 2
	double x2;		 /**< X coordinate of second node in edge */
	double y2;		 /**< Y coordinate of second node in edge */
	double z2;		 /**< Z coordinate of second node in edge */
	int inUse;		 /**< inUse bit to represent whether the current object has been constructed */
} edge_t;

/**
 * @brief Face object
 *
 * Face object contains universal node id and 3 double values representing the face's cartesian coordinates
 */
typedef struct face {
	int node_id[4];  /**< Node IDs unique to each face */
	// Node 1
	double x1;		 /**< X coordinate of first node in face */
	double y1;		 /**< Y coordinate of first node in face */
	double z1;		 /**< Z coordinate of first node in face */
	// Node 2
	double x2;		 /**< X coordinate of second node in face */
	double y2;		 /**< Y coordinate of second node in face */
	double z2; 		 /**< Z coordinate of second node in face */
	// Node 3
	double x3;		 /**< X coordinate of third node in face */
	double y3; 		 /**< Y coordinate of third node in face */
	double z3;		 /**< Z coordinate of third node in face */
	// Node 4
	double x4;		 /**< X coordinate of fourth node in face */
	double y4;		 /**< Y coordinate of fourth node in face */
	double z4; 		 /**< Z coordinate of fourth node in face */
	int inUse;		 /**< inUse bit to represent whether the current object has been constructed */
} face_t;

/**
 * @brief Internal object
 *
 * Internal object contains universal node id and 3 double values representing the internal's cartesian coordinates
 */
typedef struct internal {
	int node_id[8];  /**< Node IDs unique to each internal */
	// Node 1
	double x1;		 /**< X coordinate of first node in internal */
	double y1;		 /**< Y coordinate of first node in internal */
	double z1;		 /**< Z coordinate of first node in internal */
	// Node 2
	double x2;		 /**< X coordinate of second node in internal */
	double y2;		 /**< Y coordinate of second node in internal */
	double z2;		 /**< Z coordinate of second node in internal */
	// Node 3
	double x3;		 /**< X coordinate of third node in internal */
	double y3;		 /**< Y coordinate of third node in internal */
	double z3;		 /**< Z coordinate of third node in internal */
	// Node 4
	double x4;		 /**< X coordinate of fourth node in internal */
	double y4;		 /**< Y coordinate of fourth node in internal */
	double z4;  	 /**< Z coordinate of fourth node in internal */
	// Node 5
	double x5;		 /**< X coordinate of fifth node in internal */
	double y5;		 /**< Y coordinate of fifth node in internal */
	double z5;		 /**< Z coordinate of fifth node in internal */
	// Node 6
	double x6;		 /**< X coordinate of sixth node in internal */
	double y6;		 /**< Y coordinate of sixth node in internal */
	double z6;		 /**< Z coordinate of sixth node in internal */
	// Node 7
	double x7;		 /**< X coordinate of seventh node in internal */
	double y7;		 /**< Y coordinate of seventh node in internal */
	double z7;		 /**< Z coordinate of seventh node in internal */
	// Node 8
	double x8;		 /**< X coordinate of eigth node in internal */
	double y8;		 /**< Y coordinate of eigth node in internal */
	double z8;		 /**< Z coordinate of eigth node in internal */

	int inUse;		 /**< inUse bit to represent whether the current object has been constructed */
} internal_t;

/**
* Method to be called from main function that does all reading, constructing, and printing of new elements
* @author Dan Gross
* @date 30 July 2017
* @param msh_file .msh file to read data from
*/
void toHex64( const char* msh_file );
/**
* Method that takes all necessary objects of a certain element and constructs a refined element represented by a char*
* @author Dan Gross
* @date 30 July 2017
* @param elem_frag Fragment of original element that remains unchanged in refined element (first 8 nodes)
* @param elem_id Element ID
* @param num_nodes Total number of original nodes in mesh
* @param edges 2-D array storing all edge objects
* @param faces 2-D array storing all face objects
* @param num_elements Number of elements in mesh
* @param internals Array storing all internal objects
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 Node 1 in Hex-8 element
* @param n2 Node 2 in Hex-8 element
* @param n3 Node 3 in Hex-8 element
* @param n4 Node 4 in Hex-8 element
* @param n5 Node 5 in Hex-8 element
* @param n6 Node 6 in Hex-8 element
* @param n7 Node 7 in Hex-8 element
* @param n8 Node 8 in Hex-8 element
* @return char* representation of refined element
*/
char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t** faces, int num_elements, internal_t** internals, char* all_coords[num_elements * 19], char ** nodes, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8 );
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
* Method that returns the face object represented by the two nodes n1 and n2
* @author Dan Gross
* @date 30 July 2017
* @param n1 First node associated with face
* @param n2 Second node associated with face
* @param faces 2-D array storing all face objects
* @return Face associated with the two nodes
*/
face_t getFace( int n1, int n2, face_t ** faces );
/**
* Method that first searches to see if the edge associated with Nodes n1 and n2 exists, and if not constructs the new edge
* @author Dan Gross
* @date 30 July 2017
* @param elem_id Element ID
* @param num_nodes Total number of original nodes in mesh
* @param edges 2-D array storing all edge objects
* @param num_elements Number of elements in mesh
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 First node associated with edge
* @param n2 Second node associated with edge
* @return Edge Node ID for use in new element
*/
char* getEdgeNodeId( int elem_id, int num_nodes, edge_t** edges, int num_elements, char* all_coords[num_elements*56], char ** nodes, int n1, int n2 );
/**
* Method that first searches to see if the face associated with Nodes n1, n2, n3, and n4 exists, and if not constructs the new face
* @author Dan Gross
* @date 30 July 2017
* @param elem_id Element ID
* @param num_nodes Total number of original nodes in mesh
* @param faces 2-D array storing all face objects
* @param num_elements Number of elements in mesh
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 First node associated with face
* @param n2 Second node associated with face
* @param n3 Third node associated with face
* @param n4 Fourth node associated with face
* @return Face Node ID for use in new element
*/
char* getFaceNodeId( int elem_id, int num_nodes, face_t** faces, int num_elements, char* all_coords[num_elements*56], char ** nodes, int n1, int n2, int n3, int n4 );
/**
* Method that constructs the internal node associated with the element
* @author Dan Gross
* @date 30 July 2017
* @param elem_id Element ID
* @param num_nodes Total number of original nodes in mesh
* @param internals 2-D array storing all internal objects
* @param num_elements Number of elements in mesh
* @param all_coords Array storing all new coordinates generated
* @param nodes Array storing all original nodes in mesh
* @param n1 First node associated with internal
* @param n2 Second node associated with internal
* @return Internal Node ID for use in new element
*/
char* getInternalNodeId( int elem_id, int num_elements, internal_t** internals, char* all_coords[num_elements*56], char** nodes, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8 );
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
* Math function that computes the distance one third of the way between a and b
* @author Dan Gross
* @date 30 July 2017
* @param a First arg
* @param b Second arg
* @return The 'third' distance from a to b
*/ 
double thirds( double a, double b );

#endif