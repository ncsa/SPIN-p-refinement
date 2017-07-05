#include "tet4.h"

// gcc tet4.c main.c -o toTet10
// Program to convert tet4 elements to tet10

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

	char* new_nodes[ num_elements * 6 ];
	char* elements[ num_elements ];

	// Read element lines and store in elements[]
	clock_t begin = clock();
	for(int i = 0; i < num_elements; i++)
	{
		fgets(buff, 255, msh);
		int elem_id;
		int num_tags;
		char* elem = strdup(buff);
		elements[i] = strdup(buff);
		//fscanf(msh, "%s ", buff);
		//fscanf(msh, "%d %s ", &elem_id, buff);
		//sscanf( elem, "%d %d %d %s", (int*)NULL, &elem_id, &num_tags, (char*)NULL );
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

			char new_coord[255];
			double x1, x2, y1, y2, z1, z2;

			// VTK ELEMENT NEW NODE ORDERING
			// [ (1,2) (2,3) (3,1) (1,4) (2,4) (3,4) ]

			// [First, Second]
			sscanf( nodes[first_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[second_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( new_coord, 255, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );
			if( i != 0 )
			{
				char* repeat = checkForRepeat( new_coord, new_nodes, i * 6 );
				//char* repeat = lfind()
				if( repeat == NULL )
				{
					new_nodes[i * 6] = strdup(new_coord);
				}
				else
				{
					new_nodes[i * 6] = strdup(repeat);
				}
			}
			else
			{
				new_nodes[i * 6] = strdup(new_coord);
			}

			// [Second, Third]
			sscanf( nodes[second_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[third_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );
	
			snprintf( new_coord, 255, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );
			if( i != 0 )
			{
				char* repeat = checkForRepeat( new_coord, new_nodes, i * 6 + 1);
				if( repeat == NULL )
				{
					new_nodes[i * 6 + 1] = strdup(new_coord);
				}
				else
				{
					new_nodes[i * 6 + 1] = strdup(repeat);
				}
			}
			else
			{
				new_nodes[i * 6 + 1] = strdup(new_coord);
			}

			// [First, Third]
			sscanf( nodes[first_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[third_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( new_coord, 255, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );
			if( i != 0 )
			{
				char* repeat = checkForRepeat( new_coord, new_nodes, i * 6 + 2 );
				if( repeat == NULL )
				{
					new_nodes[i * 6 + 2] = strdup(new_coord);
				}	
				else
				{
					new_nodes[i * 6 + 2] = strdup(repeat);
				}	
			}
			else
			{
				new_nodes[i * 6 + 2] = strdup(new_coord);
			}	

			// [First, Fourth]
			sscanf( nodes[first_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[fourth_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( new_coord, 255, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );
			if( i != 0 )
			{
				char* repeat = checkForRepeat( new_coord, new_nodes, i * 6 + 3 );
				if( repeat == NULL )
				{
					new_nodes[i * 6 + 3] = strdup(new_coord);
				}
				else
				{
					new_nodes[i * 6 + 3] = strdup(repeat);
				}
			}
			else
			{
				new_nodes[i * 6 + 3] = strdup(new_coord);
			}

			// [Second, Fourth]
			sscanf( nodes[second_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[fourth_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( new_coord, 255, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );
			if( i != 0 )
			{
				char* repeat = checkForRepeat( new_coord, new_nodes, i * 6 + 4);
				if( repeat == NULL )
				{
					new_nodes[i * 6 + 4] = strdup(new_coord);
				}
				else
				{
					new_nodes[i * 6 + 4] = strdup(repeat);
				}
			}
			else
			{
				new_nodes[i * 6 + 4] = strdup(new_coord);
			}

			// [Third, Fourth]
			sscanf( nodes[third_node - 1], "%lf %lf %lf", &x1, &y1, &z1 );
			sscanf( nodes[fourth_node - 1], "%lf %lf %lf", &x2, &y2, &z2 );

			snprintf( new_coord, 255, "%lf %lf %lf", avg(x1,x2), avg(y1,y2), avg(z1,z2) );
			if( i != 0 )
			{
				char* repeat = checkForRepeat( new_coord, new_nodes, i * 6 + 5);
				if( repeat == NULL )
				{
					new_nodes[i * 6 + 5] = strdup(new_coord);
				}
				else
				{
					new_nodes[i * 6 + 5] = strdup(repeat);
				}
			}
			else
			{
				new_nodes[i * 6 + 5] = strdup(new_coord);
			}
		}
		else
		{
			for(int j = 0; j < 6; j++)
			{
				new_nodes[i * 6 + j] = "placeholder";
			}
		}
	}
	clock_t end = clock();
	printf("Elem time: %f\n", (double)(end-begin)/CLOCKS_PER_SEC);

	fclose( msh );
	
	// Put old nodes and new nodes in one list 
	// Not efficient, will change in future (concat in writing stage)
	begin = clock();
	char* all_nodes[num_nodes + (num_elements * 6)];

	for(int i = 0; i < num_nodes; i++)
	{
		char coord[255];
		snprintf( coord, 255, "%d %s", (i+1), nodes[i] );
		all_nodes[i] = strdup(coord);
	}
	int j = num_nodes;
	for(int i = 0; i < num_elements * 6; i++)
	{
		// If not a repeat node
		if( new_nodes[i][0] != 'r' && new_nodes[i][0] != 'p' )
		{
			char coord[255];
			snprintf( coord, 255, "%d %s\n", (j+1), new_nodes[i] );
			all_nodes[j] = strdup(coord);
			j++;
			char all_nodes_id[10];
			snprintf( all_nodes_id, 10, "a%d", j );
			new_nodes[i] = strdup(all_nodes_id);
		}
	}
	end = clock();
	printf("Concating nodes: %f\n", (double)(end-begin)/CLOCKS_PER_SEC);

	int num_nodes_new = j;

	begin = clock();
	// Get new elements
	for(int i = 0; i < num_elements; i++)
	{
		char new_element[255];
		int n[6];

		char * orig_elem = strdup(elements[i]);
		int elem_id;
		sscanf( orig_elem, "%*d %d %*s", &elem_id );

		// If original element is TET4
		if(elem_id == 4)
		{
			// Get new node ids
			for(int j = 0; j < 6; j++)
			{
				int new_node_id;
				if( new_nodes[(i * 6) + j][0] == 'r' )
				{
					int repeat_id;
					sscanf( new_nodes[(i * 6) + j], "r%d", &repeat_id );
					sscanf( new_nodes[repeat_id], "a%d", &new_node_id);
					n[j] = new_node_id;
				}
				else
				{
					sscanf( new_nodes[(i * 6) + j], "a%d", &new_node_id);
					n[j] = new_node_id;
				}
			}

			// Rewrite element in elements[] to TET10 nodes
			char* elem = elements[i];
			int num_tags;
			sscanf( elem, "%*d %*d %d %[^\t\n]", &num_tags, elem );
			for(int j = 0; j < num_tags; j++)
			{
				sscanf(elem, "%*d %[^\t\n] ", elem);
			}
			snprintf( new_element, 255, "%d 11 2 0 0 %s %d %d %d %d %d %d\n", (i+1), elem, n[0], n[1], n[2], n[3], n[4], n[5] );
			elements[i] = strdup(new_element);
		}
	}
	end = clock();
	printf("Getting new elems: %f\n", (double)(end-begin)/CLOCKS_PER_SEC);

	// All new elements and nodes collected
	// Rewrite to msh file and convert to vtk

	char* _msh_file = strdup(msh_file);
	_msh_file[ strlen(_msh_file) - 4] = '\0';
	char new_msh_file[255];
	snprintf( new_msh_file, 255, "%s_tet10.msh", _msh_file );
	FILE * new_msh = fopen( new_msh_file, "w+" );

	fputs("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n$Nodes\n", new_msh);
	fprintf(new_msh, "%d\n", num_nodes_new);

	for(int i = 0; i < num_nodes_new; i++)
	{
		fprintf(new_msh, "%s", all_nodes[i]);
	}
	fputs("$EndNodes\n$Elements\n", new_msh);
	fprintf(new_msh, "%d\n", num_elements);

	for(int i = 0; i < num_elements; i++)
	{
		fprintf(new_msh, "%s", elements[i]);
	}

	fputs("$EndElements\n", new_msh);
	
	fclose( new_msh );
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




