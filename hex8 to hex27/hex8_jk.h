/**
 * @file hex8_jk.h
 * @author Dan Gross, JaeHyuk Kwack
 * @date 26 March 2018
 * @brief Header file for HEX-8 to HEX-27 refinement code
 *
 */
// gcc-7 -fopenmp main.c hex8.c -o toHex27

#ifndef HEX8_H
#define HEX8_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>

#define MAX_EDGES 12
#define NDM 3

/**
 * @brief NODE object
 *
 * NODE object contains 3 coordinates for x, y, and z-axis
 */
typedef struct nodes {
	float X[NDM]; 		/**< Nodal coordinates of this node */
} NODE;


/**
 * @brief HEX8 object
 *
 * HEX8 object contains 8 node indices, 12 edge indices and 6 face indices
 */
typedef struct elements {
	int nodeID[8]; 	/**< Node IDs contained in this element */
	int edgeID[12]; 	/**< Edge IDs contained in this element */
	bool edgeDIR[12];	/**< If the local edge in this element has the same direction as the global edge */
	int faceID[6];	   /**< Face IDs contained in this element */
	bool faceDIR[6];	/**< If the local face in this element has the same direction as the global face */
} EL_HEX8;

/**
 * @brief Edge object for HEX8
 *
 * Edge object for HEX8 contains 2 node indices
 */
typedef struct edges {
	int nodeID[2]; 	/**< Node IDs contained in this edge */
} ED_HEX8;


/**
 * @brief Face object for HEX8
 *
 * Face object for HEX8 contains 4 node indices and global facet ID
 */
typedef struct faces {
	int nodeID[4]; 	/**< Node IDs contained in this face */
	int myID;			/**< Global face ID of this face */
	int refID;			/**< Reference face ID that is the smallest face ID that has the same nodeIDs as this face */
} FA_HEX8;


/**
* Function to be called from main function that read a msh file
* @author Dan Gross, JaeHyuk Kwack
* @date 26 March 2018
* @param msh_file .msh file to read data from
* @param num_nodes number of nodes
* @param num_HEX8 number of HEX8 elements
* @param mynodes Node objects
* @param myHEX8 HEX8 objects
*/
void readHEX8( const char* msh_file, int* num_nodes, int* num_HEX8, NODE** mynodes, EL_HEX8** myHEX8 );


/**
* Function to be called from main function that constructs edges from HEX8 elements
* @author Dan Gross, JaeHyuk Kwack
* @date 27 March 2018
* @param num_nodes number of nodes
* @param num_edges number of edges
* @param num_nodes number of nodes
* @param num_HEX8 number of HEX8 elements
* @param myedges Edge objects
* @param myHEX8 HEX8 objects
*/
void Construct_Edges_HEX8( int* num_nodes, int* num_edges, int* num_HEX8, ED_HEX8** myedges, EL_HEX8** myHEX8 );

/**
* Function that sorts nodes in the edge
* @author Dan Gross, JaeHyuk Kwack
* @date 27 March 2018
* @param edge Edge object
* @param flag If edge nodes are already ordered, the flag is 'true'. Otherwise, the flag is 'false'.
*/
void sort_edge_node( ED_HEX8* edge, bool* flag );

/**
* Function that returns unique components
* @author Dan Gross, JaeHyuk Kwack
* @date 29 March 2018
* @param list the list possible containing repeated components
* @param nlist number of components in the list
*/
void unique_int( int* list, int* nlist );


/**
* Function to be called from main function that constructs edges from HEX8 elements
* @author Dan Gross, JaeHyuk Kwack
* @date 27 March 2018
* @param num_nodes number of nodes
* @param num_edges number of edges
* @param num_nodes number of nodes
* @param num_HEX8 number of HEX8 elements
* @param mynodes Node objects
* @param myedges Edge objects
* @param myHEX8 HEX8 objects
* @param order Order for p-refinement (e.g., 2 for HEX27, 3 for HEX64, and 4 for HEX125)
*/
void Refine_Edges( int* num_nodes, int* num_edges, int* num_HEX8, NODE** mynodes, ED_HEX8** myedges, EL_HEX8** myHEX8, int order);


/**
 * @brief Edge object
 *
 * Edge object contains universal node id and 3 double values representing the edge's cartesian coordinates
 */
typedef struct edge {
	int node_id; /**< Node ID unique to each edge */
	double x;    /**< X coordinate of edge */
	double y;    /**< Y coordinate of edge */
	double z;    /**< Z coordinate of edge */
	int inUse;   /**< inUse bit to represent whether the current object has been constructed */
} edge_t;

/**
 * @brief Face object
 *
 * Face object contains universal node id and 3 double values representing the face's cartesian coordinates
 */
typedef struct face {
	int node_id; /**< Node ID unique to each face */
	double x;    /**< X coordinate of face */
	double y;    /**< Y coordinate of face */
	double z;    /**< Z coordinate of face */
	int inUse;   /**< inUse bit to represent whether the current object has been constructed */
} face_t;

/**
 * @brief Internal object
 *
 * Internal object contains universal node id and 3 double values representing the internal's cartesian coordinates
 */
typedef struct internal {
	int node_id; /**< Node ID unique to each internal */
	double x;    /**< X coordinate of internal */
	double y;    /**< Y coordinate of internal */
 	double z;    /**< Z coordinate of internal */
	int inUse;   /**< inUse bit to represent whether the current object has been constructed */
} internal_t;

/**
* Method to be called from main function that does all reading, constructing, and printing of new elements
* @author Dan Gross
* @date 30 July 2017
* @param msh_file .msh file to read data from
*/
void toHex27( const char* msh_file );
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
/*
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
int getEdgeNodeId( int elem_id, int num_nodes, edge_t** edges, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2 );
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
int getFaceNodeId( int elem_id, int num_nodes, face_t** faces, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2, int n3, int n4 );
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
int getInternalNodeId( int elem_id, int num_elements, internal_t** internals, char* all_coords[num_elements*19], char** nodes, int n1, int n2 );
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