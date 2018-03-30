/**
 * @file tet4_jk.h
 * @author Dan Gross, JaeHyuk Kwack
 * @date 30 March 2018
 * @brief Header file for TET-4 to TET-higher refinement code
 *
 */

#ifndef TET4_H
#define TET4_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>

#define MAX_EDGES 6
#define MAX_FACES 4
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
 * @brief TET4 object
 *
 * TET4 object contains 4 node indices, and 6 edge indices
 */
typedef struct elements {
	int nodeID[4]; 	/**< Node IDs contained in this element */
	int edgeID[6]; 	/**< Edge IDs contained in this element */
	bool edgeDIR[6];	/**< If the local edge in this element has the same direction as the global edge */
} EL_TET4;

/**
 * @brief Edge object for TET4
 *
 * Edge object for TET4 contains 2 node indices
 */
typedef struct edges {
	int nodeID[2]; 	/**< Node IDs contained in this edge */
} ED_TET4;


/**
* Function to be called from main function that read a msh file
* @author Dan Gross, JaeHyuk Kwack
* @date 30 March 2018
* @param msh_file .msh file to read data from
* @param num_nodes number of nodes
* @param num_TET4 number of TET4 elements
* @param mynodes Node objects
* @param myTET4 TET4 objects
*/
void readTET4( const char* msh_file, int* num_nodes, int* num_TET4, NODE** mynodes, EL_TET4** myTET4 );


/**
* Function to be called from main function that constructs edges from TET4 elements
* @author Dan Gross, JaeHyuk Kwack
* @date 30 March 2018
* @param num_nodes number of nodes
* @param num_edges number of edges
* @param num_nodes number of nodes
* @param num_TET4 number of TET4 elements
* @param myedges Edge objects
* @param myTET4 TET4 objects
*/
void Construct_Edges_TET4( int* num_nodes, int* num_edges, int* num_TET4, ED_TET4** myedges, EL_TET4** myTET4 );

/**
* Function that sorts nodes in the edge
* @author Dan Gross, JaeHyuk Kwack
* @date 30 March 2018
* @param edge Edge object
* @param flag If edge nodes are already ordered, the flag is 'true'. Otherwise, the flag is 'false'.
*/
void sort_edge_node( ED_TET4* edge, bool* flag );

/**
* Function that returns unique components
* @author Dan Gross, JaeHyuk Kwack
* @date 30 March 2018
* @param list the list possible containing repeated components
* @param nlist number of components in the list
*/
void unique_int( int* list, int* nlist );


/**
* Function to be called from main function that constructs edges from TET4 elements
* @author Dan Gross, JaeHyuk Kwack
* @date 30 March 2018
* @param num_nodes number of nodes
* @param num_edges number of edges
* @param num_nodes number of nodes
* @param num_TET4 number of TET4 elements
* @param mynodes Node objects
* @param myedges Edge objects
* @param myTET4 TET4 objects
* @param order Order for p-refinement (e.g., 2 for O(2), 3 for O(3) and n for O(n) )
*/
void Refine_Edges( int* num_nodes, int* num_edges, int* num_TET4, NODE** mynodes,
	ED_TET4** myedges, EL_TET4** myTET4, int order, int* num_new_nodes_edges,
	NODE** mynewnodes_edge,int** new_IX_edge);


#endif