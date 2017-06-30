#ifndef BOUNDCOND_H
#define BOUNDCOND_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void addBC( const char* vtk_file );
void addPointData( FILE* vtk, int first, int second, float val );
int getNumNodes( FILE* vtk );

#endif