#include "tet4.h"

// gcc tet4.c main.c -o toTet10
// Program to convert tet4 elements to tet10

static int last_id = 0;

void toTet10( const char* msh_file )
{
	FILE * msh = fopen( msh_file, "r+" );
	if( msh == NULL )
	{
		printf("Error opening %s\n", msh_file);
		return;
	}
	
	char buff[255];

	while( strcmp( buff, "$Nodes\n") != 0 )
	{
		fgets(buff, 255, msh);
	}

	fgets(buff, 255, msh);
	int num_nodes = atoi(buff);

	// Read node lines and store in nodes array
	char* nodes[num_nodes];
	for(int i = 0; i < num_nodes; i++)
	{
		fscanf(msh, "%s ", buff);
		fgets(buff, 255, msh);

		nodes[i] = strdup(buff);
	}

	for(int i = 0; i < 3; i++)
	{
		fgets(buff, 255, msh);
	}
	
	int num_elements = atoi(buff);

	char** new_nodes = malloc((num_elements * 6) * sizeof(char*));
	char** elements = malloc((num_elements) * sizeof(char*));

	int used_nodes[num_nodes * 10][2];
	char** used_nodes_coords = malloc((num_nodes * 10) * sizeof(char*));

	// Read element lines and store in elements[]
	for(int i = 0; i < num_elements; i++)
	{
		fgets(buff, 255, msh);
		int elem_id;
		int num_tags;
		char* elem = strdup(buff);
		elements[i] = strdup(buff);

		char elem_frag[255];
		sscanf( elem, "%*d %d %d %[^\t\n]", &elem_id, &num_tags, elem_frag );
		for(int j = 0; j < num_tags; j++)
		{
			sscanf(elem_frag, "%*d %[^\t\n]", elem_frag);
		}

		// If the current element is a TET4
		if(elem_id == 4)
		{
			int first_node, second_node, third_node, fourth_node;
			sscanf(elem_frag, "%d %d %d %d", &first_node, &second_node, &third_node, &fourth_node);

			// OPTIMIZE, returns "%d %d %d %d %d %d" <-- new node ids (need to incremented by num_nodes before pushing to file)
			char* new_elem = optimize( nodes, used_nodes, used_nodes_coords, first_node, second_node, third_node, fourth_node );

			// Construct new element and push onto elements array
			int fifth_id, sixth_id, seventh_id, eigth_id, ninth_id, tenth_id;
			sscanf( new_elem, "%d %d %d %d %d %d", &fifth_id, &sixth_id, &seventh_id, &eigth_id, &ninth_id, &tenth_id );\

			char* elem = elements[i];
			int num_tags;
			sscanf( elem, "%*d %*d %d %[^\t\n]", &num_tags, elem );
			for(int j = 0; j < num_tags; j++)
			{
				sscanf(elem, "%*d %[^\t\n] ", elem);
			}

			char final_elem[255];
			snprintf( final_elem, 255, "%d 11 2 0 0 %s %d %d %d %d %d %d\n", (i+1), elem, (fifth_id + num_nodes + 1), (sixth_id +  num_nodes + 1), (seventh_id +  num_nodes + 1), (eigth_id +  num_nodes + 1), (ninth_id +  num_nodes + 1), (tenth_id +  num_nodes + 1) );
			elements[i] = strdup(final_elem);

		}
	}

	// All new elements and nodes collected
	// Rewrite to msh file and convert to vtk

	char* _msh_file = strdup(msh_file);
	_msh_file[ strlen(_msh_file) - 4] = '\0';
	char new_msh_file[255];
	snprintf( new_msh_file, 255, "%s_tet10.msh", _msh_file );
	FILE * new_msh = fopen( new_msh_file, "w+" );

	fputs("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n$Nodes\n", new_msh);
	fprintf(new_msh, "%d\n", ( num_nodes + last_id ));
	for(int i = 0; i < num_nodes; i++)
	{
		fprintf(new_msh, "%d %s", (i+1), nodes[i]);
		free(nodes[i]);
	}
	for(int i = 0; i < last_id; i++)
	{
		fprintf(new_msh, "%d %s\n", (i + num_nodes + 1), used_nodes_coords[i]);
		free(used_nodes_coords[i]);
	}
	fputs("$EndNodes\n$Elements\n", new_msh);
	fprintf(new_msh, "%d\n", num_elements);

	for(int i = 0; i < num_elements; i++)
	{
		fprintf(new_msh, "%s", elements[i]);
		free(elements[i]);
	}

	fputs("$EndElements\n", new_msh);
	fgets(buff, 255, msh);

	while( fgets(buff, 255, msh) != NULL )
	{
		fprintf(new_msh, "%s", buff);
	}
	
	fclose( new_msh );
	fclose( msh );
}

// Returns string containing a char representation of the nodes to added to the new element
char* optimize( char* nodes[], int used_nodes[][2], char* used_nodes_coords[], int first_node, int second_node, int third_node, int fourth_node )
{
	int first[2], second[2], third[2], fourth[2], fifth[2], sixth[2];

		// [1,2]
		if( first_node < second_node )
		{
			first[0] = first_node;
			first[1] = second_node;
		}
		else
		{
			first[0] = second_node;
			first[1] = first_node;
		}

		// [2,3]
		if( second_node < third_node )
		{
			second[0] = second_node;
			second[1] = third_node;
		}
		else
		{
			second[0] = third_node;
			second[1] = second_node;
		}

		// [1,3]
		if( first_node < third_node )
		{
			third[0] = first_node;
			third[1] = third_node;
		}
		else
		{
			third[0] = third_node;
			third[1] = first_node;

		}

		// [1,4]
		if( first_node < fourth_node )
		{
			fourth[0] = first_node;
			fourth[1] = fourth_node;
		}
		else
		{
			fourth[0] = fourth_node;
			fourth[1] = first_node;
		}

		// [2,4]
		if( second_node < fourth_node )
		{
			fifth[0] = second_node;
			fifth[1] = fourth_node;
		}
		else
		{
			fifth[0] = fourth_node;
			fifth[1] = second_node;
		}

		// [3,4]
		if( third_node < fourth_node )
		{
			sixth[0] = third_node;
			sixth[1] = fourth_node;
		}
		else
		{
			sixth[0] = fourth_node;
			sixth[1] = third_node;
		}

		// Check used_nodes for node pairs (should be sorted)
		int num_found = 0;
		char *first_coord = NULL;
		char *second_coord = NULL;
		char *third_coord = NULL;
		char *fourth_coord = NULL;
		char *fifth_coord = NULL;
		char *sixth_coord = NULL;
		int first_id, second_id, third_id, fourth_id, fifth_id, sixth_id;

		// Change to NOT linear search
		for(int i = 0; i < last_id; i++)
		{
			if(num_found == 6)
			{
				break;
			}
			// if used_nodes[i] == first then first_coord == used_nodes_coords[i]
			if( used_nodes[i][0] == first[0] && used_nodes[i][1] == first[1] )
			{
				first_coord = strdup(used_nodes_coords[i]);
				num_found++;

				first_id = i;
			}
			if( used_nodes[i][0] == second[0] && used_nodes[i][1] == second[1] )
			{
				second_coord = strdup(used_nodes_coords[i]);
				num_found++;

				second_id = i;
			}
			if( used_nodes[i][0] == third[0] && used_nodes[i][1] == third[1] )
			{
				third_coord = strdup(used_nodes_coords[i]);
				num_found++;

				third_id = i;
			}
			if( used_nodes[i][0] == fourth[0] && used_nodes[i][1] == fourth[1] )
			{
				fourth_coord = strdup(used_nodes_coords[i]);
				num_found++;

				fourth_id = i;
			}
			if( used_nodes[i][0] == fifth[0] && used_nodes[i][1] == fifth[1] )
			{
				fifth_coord = strdup(used_nodes_coords[i]);
				num_found++;

				fifth_id = i;
			}
			if( used_nodes[i][0] == sixth[0] && used_nodes[i][1] == sixth[1] )
			{
				sixth_coord = strdup(used_nodes_coords[i]);
				num_found++;

				sixth_id = i;
			}
		}

		double x1, x2, y1, y2, z1, z2;
		// If not found in list, calculate and push on to array
		if( first_coord == NULL )
		{
			first_coord = malloc(100);
			sscanf( nodes[first_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[second_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( first_coord, 100, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );

			used_nodes[last_id][0] = first[0];
			used_nodes[last_id][1] = first[1];
			used_nodes_coords[last_id] = strdup(first_coord);
			first_id = last_id;

			last_id++;
		}
		if( second_coord == NULL )
		{
			second_coord = malloc(100);
			sscanf( nodes[second_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[third_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( second_coord, 100, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );

			used_nodes[last_id][0] = second[0];
			used_nodes[last_id][1] = second[1];
			used_nodes_coords[last_id] = strdup(second_coord);
			second_id = last_id;

			last_id++;
		}
		if( third_coord == NULL )
		{
			third_coord = malloc(100);

			sscanf( nodes[first_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[third_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( third_coord, 100, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );

			used_nodes[last_id][0] = third[0];
			used_nodes[last_id][1] = third[1];
			used_nodes_coords[last_id] = strdup(third_coord);
			third_id = last_id;

			last_id++;
		}
		if( fourth_coord == NULL )
		{
			fourth_coord = malloc(100);

			sscanf( nodes[first_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[fourth_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( fourth_coord, 100, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );

			used_nodes[last_id][0] = fourth[0];
			used_nodes[last_id][1] = fourth[1];
			used_nodes_coords[last_id] = strdup(fourth_coord);
			fourth_id = last_id;

			last_id++;
		}
		if( fifth_coord == NULL )
		{
			fifth_coord = malloc(100);

			sscanf( nodes[second_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[fourth_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( fifth_coord, 100, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );

			used_nodes[last_id][0] = fifth[0];
			used_nodes[last_id][1] = fifth[1];
			used_nodes_coords[last_id] = strdup(fifth_coord);
			fifth_id = last_id;

			last_id++;
		}
		if( sixth_coord == NULL )
		{
			sixth_coord = malloc(100);

			sscanf( nodes[third_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[fourth_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( sixth_coord, 100, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );

			used_nodes[last_id][0] = sixth[0];
			used_nodes[last_id][1] = sixth[1];
			used_nodes_coords[last_id] = strdup(sixth_coord);
			sixth_id = last_id;

			last_id++;
		}

		// Nodes fully populated, arrays have been updated; return element string

	char* elem[100];

	snprintf( elem, 100, "%d %d %d %d %d %d", first_id, second_id, third_id, fourth_id, fifth_id, sixth_id );

	free(first_coord);
	free(second_coord);
	free(third_coord);
	free(fourth_coord);
	free(fifth_coord);
	free(sixth_coord);

	return strdup(elem);
}

/* 	Checks to see if the newly calculated node is a repeat node.
*	Returns NULL if not repeat, else returns char array representing the repeated node
*	I.E. returns "r1" if the node is a repeat of node 1 (may change this)
*/
char* checkForRepeat( char* new_coord, char* new_nodes[], int len )
{
	for(int i = 0; i < len; i++)
	{
		if( strcmp( new_coord, new_nodes[i] ) == 0 )
		{
			char ret[50];
			snprintf( ret, 50, "r%d", i);
			return strdup(ret);
		}
	}

	return NULL;
}

double avg( double a, double b )
{
	return (a+b) / 2.0;
}




