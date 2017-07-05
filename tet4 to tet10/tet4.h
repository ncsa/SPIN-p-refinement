#ifndef TET4_H
#define TET4_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void toTet10( const char* msh_file );
double avg( double a, double b );
char* checkForRepeat( char* new_coord, char* new_nodes[], int len );
char* optimize( char* nodes[], int used_nodes[][2], char* used_nodes_coords[], int first_node, int second_node, int third_node, int fourth_node );

#endif