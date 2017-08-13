#ifndef TET4_H
#define TET4_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/**
* Method to be called from main function that does all reading, constructing, and printing of new elements
* @author Dan Gross
* @date 30 July 2017
* @param msh_file .msh file to read data from
*/
void toTet10( const char* msh_file );
/**
* Method that reads through previously constructed nodes and checks for a reapeat
* @author Dan Gross
* @date 30 July 2017
* @param new_coord newly constructed node
* @param new_nodes array of new nodes
* @param len offset to check for node
* @return char* that represents a found repeat node or NULL representing node not found
*/
char* checkForRepeat( char* new_coord, char* new_nodes[], int len );
/**
* Method that refines the current element represented by first_node, second_node, third_node, and fourth_node
* @author Dan Gross
* @date 30 July 2017
* @param nodes array containing all nodes from original mesh
* @param used_nodes array containing all used nodes in new mesh
* @param used_nodes_coords array containing all coordinates in used nodes
* @param first_node ID of first node associated with the TET-4 element
* @param second_node ID of second node associated with the TET-4 element
* @param third_node ID of third node associated with the TET-4 element
* @param fourth_node ID of fourth node associated with the TET-4 element
* @rerurn char* that represents a found repeat node or NULL representing node not found
*/
char* optimize( char* nodes[], int used_nodes[][2], char* used_nodes_coords[], int first_node, int second_node, int third_node, int fourth_node );
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