#ifndef TET4_H
#define TET4_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void toTet10( const char* msh_file );
double avg( double a, double b );
char* checkForRepeat( char* new_coord, char* new_nodes[], int len );


#endif