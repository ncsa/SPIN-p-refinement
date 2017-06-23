// Main.cpp
// gcc main.cpp -o convert -lstdc++

#include <string>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <unistd.h>

#include "converter.h"

using namespace std;

int main(int argc, char* argv[])
{
	if( argc != 2 )
	{
		cout << "usage: ./convert mesh_file.msh" << endl;
		return 0;
	}
	else
	{
		Converter::convert( argv[1] );
	}

	return 0;
}