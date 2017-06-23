#ifndef CONVERTER_H
#define CONVERTER_H

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <sstream>
#include "vector"

using namespace std;

class Converter{
	public:
		static void convert( string msh_file );
		static pair<int, int> getSize( ifstream &se, string line );
		static pair<int, string> getNodes( string line );
	private:
} ;

#endif