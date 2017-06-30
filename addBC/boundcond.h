#ifndef BOUNDCOND_H
#define BOUNDCOND_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void addBC( const char* vtk_file );
void addFloatPointData( FILE* vtk, int first, int second, float val );
void addIntPointData( FILE* vtk, int first, int second, int val );
int getNumNodes( FILE* vtk );

#endif