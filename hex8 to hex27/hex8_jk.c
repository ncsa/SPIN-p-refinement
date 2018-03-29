#include "hex8_jk.h"

#define MAX_EDGES 12

// Static vars
static int num_edges = 0;
static int edge_id = 0;
static int num_faces = 0;
static int num_internal = 0;

void readHEX8( const char* msh_file, int* num_nodes, int* num_HEX8, NODE** mynodes, EL_HEX8** myHEX8 )
{
	double begin = omp_get_wtime();
	printf("\n\tStart reading the msh file:\n");

	// Attempt to open .msh file, return if failure
	FILE * msh = fopen( msh_file, "r+" );
	if( msh == NULL )
	{
		printf("Error opening %s\n", msh_file);
		return;
	}

	char buff[256];

	while (fgets(buff, 256, msh) != 0) {
		//	Reading the nodal coordinates		
		if( strcmp(buff, "$Nodes\n") == 0 ) {
			fscanf(msh,"%d",num_nodes);
			printf("\t\tnum_nodes = %d\n",*num_nodes);
			NODE* tmp_NODE = (NODE*) malloc(*num_nodes * sizeof(NODE));
			*mynodes= tmp_NODE;
			for (int i=0; i< *num_nodes;i++) {
				fscanf(msh,"%*d %g %g %g",&tmp_NODE[i].X[0],&tmp_NODE[i].X[1],&tmp_NODE[i].X[2]);
			}
		}
		// Reading all element connectivity and then saving only HEX8 element connectivity
		if( strcmp(buff, "$Elements\n") == 0 ) {
			int num_all_elements;
			fscanf(msh,"%d",&num_all_elements);	fgets(buff, 256, msh);
			printf("\t\tnum_all_elements = %d\n",num_all_elements);
			EL_HEX8* tmp_HEX8 = (EL_HEX8 *) malloc(num_all_elements * sizeof(EL_HEX8));
			*num_HEX8 = 0;
			int elem_id;
			char str_IX[256];
			for (int i=0;i< num_all_elements; i++) {
				fgets(buff, 256, msh);
				sscanf(buff,"%*d %d",&elem_id);
				if (elem_id == 5) {
					sscanf(buff,"%*d %*d %*d %*d %*d %d %d %d %d %d %d %d %d",&tmp_HEX8[*num_HEX8].nodeID[0],
						&tmp_HEX8[*num_HEX8].nodeID[1],&tmp_HEX8[*num_HEX8].nodeID[2],&tmp_HEX8[*num_HEX8].nodeID[3],
						&tmp_HEX8[*num_HEX8].nodeID[4],&tmp_HEX8[*num_HEX8].nodeID[5],&tmp_HEX8[*num_HEX8].nodeID[6],
						&tmp_HEX8[*num_HEX8].nodeID[7]);
					*num_HEX8 = *num_HEX8 + 1;
				}
			}
			tmp_HEX8 = (EL_HEX8 *) realloc(tmp_HEX8, *num_HEX8*sizeof(EL_HEX8));
			*myHEX8 = tmp_HEX8;
			printf("\t\tnum_HEX8 = %d\n",*num_HEX8);
		}
	}

	fclose(msh);
	double end = omp_get_wtime();
	printf("\t\tElapsed wall time: %lf sec\n\n", (end-begin));
}


void Construct_Edges_HEX8( int* num_nodes, int* num_edges, int* num_HEX8, ED_HEX8** myedges, EL_HEX8** myHEX8 )
{
	double begin = omp_get_wtime();
	printf("\tStart constructing edge objects from HEX8 elements:\n");

	int num_all_edges = *num_HEX8 * MAX_EDGES;			// The maximum number of edges including repeated edges
	ED_HEX8* tmp_edges = (ED_HEX8*) malloc(num_all_edges * sizeof(ED_HEX8));
	ED_HEX8* unique_edges;
	// 0-based CSR format
	int *nlink_edges, *link_edges_ptr, *link_edges_ind, *tmp_count;
	int *link_edges_ptr_unique, *link_edges_ind_unique;
	nlink_edges = (int*) malloc(*num_nodes * sizeof(int));
	link_edges_ptr = (int*) malloc( (*num_nodes +1) * sizeof(int));				// CSR pointer for link_edges
	link_edges_ind = (int*) malloc(num_all_edges * sizeof(int));					// Another node ID of the edge
	link_edges_ptr_unique = (int*) malloc( (*num_nodes +1) * sizeof(int));		// CSR pointer for link_edges_unique
	link_edges_ind_unique = (int*) malloc(num_all_edges * sizeof(int));			// Another node ID of the edge after donig unique()
	tmp_count = (int*) malloc(*num_nodes * sizeof(int));
	int sum_tmp = 0, sum_tmp2=0;
	int here;	

	#pragma omp parallel default(shared)
	{
		#pragma omp for  						// Filling nodeID of myedges 
		for (int i=0; i<*num_HEX8; i++) {
			// The 1st edge		
			tmp_edges[0+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[0];
			tmp_edges[0+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[1];
			// The 2nd edge
			tmp_edges[1+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[1];
			tmp_edges[1+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[2];
			// The 3rd edge
			tmp_edges[2+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[2];
			tmp_edges[2+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[3];
			// The 4th edge
			tmp_edges[3+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[3];
			tmp_edges[3+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[0];
			// The 5th edge
			tmp_edges[4+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[0];
			tmp_edges[4+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[4];
			// The 6th edge
			tmp_edges[5+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[1];
			tmp_edges[5+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[5];
			// The 7th edge
			tmp_edges[6+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[2];
			tmp_edges[6+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[6];
			// The 8th edge
			tmp_edges[7+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[3];
			tmp_edges[7+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[7];
			// The 9th edge
			tmp_edges[8+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[4];
			tmp_edges[8+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[5];
			// The 10th edge
			tmp_edges[9+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[5];
			tmp_edges[9+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[6];
			// The 11th edge
			tmp_edges[10+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[6];
			tmp_edges[10+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[7];
			// The 12th edge
			tmp_edges[11+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[7];
			tmp_edges[11+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[4];
		}

		#pragma omp for 					// Copying tmp_edges to ref_edges and then sorting nodeID of tmp_edges
		for (int i=0; i<*num_HEX8; i++) {
			for(int j=0; j<MAX_EDGES; j++) {
				sort_edge_node(&(tmp_edges[ j+i*MAX_EDGES ]), &((*myHEX8)[i].edgeDIR[j]) );
			}
		}	

		#pragma omp for 					// Initializing nlink_edges
		for (int i=0; i<*num_nodes; i++) {
			nlink_edges[i] = 0;
			tmp_count[i] = 0;
		}

		#pragma omp single 					
		{
			// Counting nlink_edges	
			for (int i=0; i<num_all_edges; i++) {
				nlink_edges[ tmp_edges[i].nodeID[0] - 1 ] = nlink_edges[ tmp_edges[i].nodeID[0] - 1 ] + 1;
			}
			// Calculating link_edges_ptr
			link_edges_ptr[0] = 0;
			for (int i=0; i<*num_nodes; i++) {
				link_edges_ptr[i+1] = link_edges_ptr[i] + nlink_edges[i];
			}

			// Filling link_edges_ind
			for (int i=0; i<num_all_edges; i++) {
				here = link_edges_ptr[ tmp_edges[i].nodeID[0] - 1 ] + tmp_count[ tmp_edges[i].nodeID[0] - 1 ];
				tmp_count[ tmp_edges[i].nodeID[0] - 1 ] = tmp_count[ tmp_edges[i].nodeID[0] - 1 ] + 1;
				link_edges_ind[here] = tmp_edges[i].nodeID[1];
			}
		}

		#pragma omp for 
		for (int i=0;i<*num_nodes; i++) {
			tmp_count[i] = nlink_edges[i];
			unique_int(&(link_edges_ind[ link_edges_ptr[i] ]),&tmp_count[i]);
		}

		#pragma omp single 					
		{
			// Calculating link_edges_ptr_unique
			link_edges_ptr_unique[0] = 0;
			for (int i=0; i<*num_nodes; i++) {
				link_edges_ptr_unique[i+1] = link_edges_ptr_unique[i] + tmp_count[i];
			}
			*num_edges = link_edges_ptr_unique[*num_nodes];
			unique_edges = (ED_HEX8*) malloc( *num_edges * sizeof(ED_HEX8));
			*myedges = unique_edges;
		}

		#pragma omp for 
		for (int i=0;i<*num_nodes; i++) {
			// Creating link_edges_ind_unique from link_edge_ind
			for (int j=0;j<tmp_count[i];j++) {
				link_edges_ind_unique[ link_edges_ptr_unique[i] + j ] = 
					link_edges_ind[link_edges_ptr[i] + j];
			}
		}

		#pragma omp for  					// Filling *myHEX8.edgeID
		for (int i=0;i<*num_HEX8;i++) {
			for (int j=0; j<MAX_EDGES; j++) {
				for (int k= link_edges_ptr_unique[ tmp_edges[ j+i*MAX_EDGES ].nodeID[0] - 1 ]; 
					k < link_edges_ptr_unique[ tmp_edges[ j+i*MAX_EDGES ].nodeID[0] ]; k++) {
						if (tmp_edges[j+i*MAX_EDGES].nodeID[1] ==link_edges_ind_unique[k]) {
							(*myHEX8)[i].edgeID[j] = k;		break;		
						}
				}
			}
		}

		#pragma omp for  					// Creating the unique edges
		for (int i=0;i<*num_nodes;i++) {
			for (int j=link_edges_ptr_unique[i]; j<link_edges_ptr_unique[i+1]; j++) {
				unique_edges[j].nodeID[0] = i+1;
				unique_edges[j].nodeID[1] = link_edges_ind_unique[j];
			}
		}

	}
	// Free used memory
	free(link_edges_ind);
	free(link_edges_ptr);
	free(link_edges_ind_unique);
	free(link_edges_ptr_unique);
	free(tmp_edges);
	free(nlink_edges);
	free(tmp_count);

	// for (int i=0; i <*num_edges; i++) {
	// 	printf (" %d : %d, %d \n",i,(*myedges)[i].nodeID[0],(*myedges)[i].nodeID[1]);
	// }
	// for (int i=0; i <*num_HEX8; i++) {
	// 	printf ("\n %d : ",i);
	// 	for (int j=0; j < MAX_EDGES; j++) {
	// 		printf(" (%d, %d)", (*myHEX8)[i].edgeID[j], (*myHEX8)[i].edgeDIR[j]);
	// 	}
	// }
	// printf("\n");

	double end = omp_get_wtime();
	printf("\t\tnum_edges = %d\n",*num_edges);
	printf("\t\tElapsed wall time: %lf sec\n\n", (end-begin));
}



void sort_edge_node( ED_HEX8* edge, bool* flag )
{
	int tmp;

	if ((*edge).nodeID[0] > (*edge).nodeID[1]){
		tmp = (*edge).nodeID[0];
		(*edge).nodeID[0] = (*edge).nodeID[1];
		(*edge).nodeID[1] = tmp;
		*flag = false;
	} else {
		*flag = true;
	}

}


void unique_int( int* list, int* nlist )
{
	int i,j, n_unique=0, tmp[*nlist];

	// Counting unique components
	for (i=0; i<*nlist; i++ ) {
		for (j=0; j<=i; j++) {
			if (list[i] == list[j]) break;
		}
		if (i==j) {
			tmp[n_unique] = list[i];
			n_unique = n_unique + 1;
		}
	}
	// Copying tmp to lint
	for (i=0; i<n_unique; i++) {
		list[i] = tmp[i];
	}
	// Updating *nlist
	*nlist = n_unique;
}




void toHex27( const char* msh_file )
{
	double begin = omp_get_wtime();

	// Attempt to open .msh file, return if failure
	FILE * msh = fopen( msh_file, "r+" );
	if( msh == NULL )
	{
		printf("Error opening %s\n", msh_file);
		return;
	}

	char buff[255];

	/* Initialize file size to 1024 lines, dynamically reallocate as file is read
	*  When reallocation is needed, double size
	*  var file_size is equivalent to the capacity of the file_dat[] array
	*/
	int file_size = 32768;
	int node_start = -1;
	int elem_start = -1;
	int num_nodes = 0;
	int num_elements = 0;
	int reading_nodes = 0;
	int reading_elems = 0;
	char ** nodes;
	char ** elements;

	int node_idx = 0;
	int elem_idx = 0;

	char ** file_dat = malloc(sizeof(char*) * file_size);

	/* Read file line by line, storing each line in file_dat[] array
	*  Stores useful indices related to file_dat[], node_start and elem_start which indicate starting indices of node data and element data respectively
	*  Populates nodes[] and elements[] arrays which contain pointers to the respectice chars stored in file_dat[] array. No new memory is allocated
	*/
	int num_lines = 0;
	while( fgets(buff, 255, msh) != NULL )
	{
		//printf("Reading in line %d\n", num_lines);
		file_dat[num_lines] = strdup(buff);
		if( strcmp(buff, "$Nodes\n") == 0 )
		{
			node_start = num_lines + 2;
		}
		if( strcmp(buff, "$EndNodes\n") == 0 )
		{
			reading_nodes = 0;
		}
		if( strcmp(buff, "$Elements\n") == 0 )
		{
			elem_start = num_lines + 2;
		}
		if( strcmp(buff, "$EndElements") == 0 )
		{
			reading_elems = 0;
		}
		// Read nodes and populate nodes[] array
		if( num_lines == node_start - 1 )
		{
			num_nodes = atoi(file_dat[node_start - 1]);
		}
		if( num_lines == elem_start - 1 )
		{
			num_elements = atoi(file_dat[elem_start - 1]);
		}
		if( num_lines == node_start )
		{
			nodes = malloc(sizeof(char*) * num_nodes);
			reading_nodes = 1;
		}
		if( num_lines == elem_start )
		{
			elements = malloc(sizeof(char*) * num_elements);
			reading_elems = 1;
		}
		if(reading_nodes == 1)
		{
			sscanf( file_dat[num_lines], "%*d %[^\t\n]", file_dat[num_lines] );
			nodes[node_idx] = file_dat[num_lines];
			node_idx++;
		}
		if(reading_elems == 1)
		{
			elements[elem_idx] = file_dat[num_lines];
			elem_idx++;
		}

		num_lines++;
		if( num_lines == file_size - 1 )
		{
			file_size = file_size * 2;
			file_dat = realloc(file_dat, sizeof(char*) * file_size);
		}
	}

	double end = omp_get_wtime();
	printf("JK_part1: %lf\n", (end-begin));
	begin = omp_get_wtime();

	// edge_id: all new nodes created will start counting id's from num_nodes
	edge_id = num_nodes + 1;

	// Allocates the maximum amount of memory that could be required. Each element, if it is a HEX-8, could require an additional 19 nodes to create a HEX-27
	char** all_coords = malloc(num_elements * 19 * sizeof(char*));

	// Each HEX element has 12 edges, if all HEX elements are disconnected we have num_elements * 12 edges being constructed
	int max_num_edges = num_elements * 12;
	// Each HEX element has 6 faces, if all HEX elements are disconnected we have num_elements * 6 faces being constructed
	int max_num_faces = num_elements * 6;
 
	printf("num_nodes = %d \n",num_nodes);

	/* Initialization of edges[][], faces[][], and internals[][] arrays
	*  Arrays are NUM_NODES x NUM_NODES, and are indexed based on the nodes associated with the edge, face, etc.
	*  I.E.: The edge associated with nodes 1 and 2 is stored at edges[0][1] == edges[node1 - 1][node2 - 1]
	*  Always stored with the smaller node as the first index
	*/
	edge_t** edges = (edge_t**) calloc(num_nodes, sizeof(edge_t));
	for(int i = 0; i < num_nodes; i++)
	{
		edges[i] = (edge_t*) calloc(num_nodes, sizeof(edge_t));	
	}

	end = omp_get_wtime();
	printf("JK_part2: %lf\n", (end-begin));
	begin = omp_get_wtime();

	face_t** faces = (face_t**) calloc(num_nodes, sizeof(face_t));
	for(int i = 0; i < num_nodes; i++)
	{
		faces[i] = (face_t*) calloc(num_nodes, sizeof(face_t));
	}

	internal_t** internals = malloc(num_elements * sizeof(internal_t*));

	end = omp_get_wtime();
	printf("JK_part3: %lf\n", (end-begin));
	begin = omp_get_wtime();


	// Iterate through all elements and count the number of HEX-8 elements present
	int num_hex8 = 0;
	for(int i = 0; i < num_elements; i++)
	{
		int elem_id;
		int num_tags;

		char * elem_frag = (char*) malloc(255);
		sscanf( elements[i], "%*d %d %d %[^\t\n]", &elem_id, &num_tags, elem_frag );
		for(int j = 0; j < num_tags; j++)
		{
			sscanf( elem_frag, "%*d %[^\t\n]", elem_frag );
		}

		if( elem_id == 5 )
		{
			num_hex8++;
		}

		free(elem_frag);
	}

	end = omp_get_wtime();

	printf("Memory Setup: %lf\n", (end-begin));

	begin = omp_get_wtime();

	// OpenMP parallelized for loop that constructs the new elements
	#pragma omp parallel for schedule(static) shared(edge_id, num_edges)
	for(int i = num_elements - num_hex8; i < num_elements; i++)
	{
		int elem_id;
		int num_tags;
		char* elem = elements[i];

		char elem_frag[255];
		sscanf( elem, "%*d %d %d %[^\t\n]", &elem_id, &num_tags, elem_frag );
		for(int j = 0; j < num_tags; j++)
		{
			sscanf(elem_frag, "%*d %[^\t\n]", elem_frag);
		}

		// If the current element is a HEX8
		if(elem_id == 5)
		{
			int n1, n2, n3, n4, n5, n6, n7, n8;
			sscanf(elem_frag, "%d %d %d %d %d %d %d %d", &n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8);

			char* new_elem = constructElem( elem_frag, i, num_nodes, edges, faces, num_elements, internals, all_coords, nodes, n1, n2, n3, n4, n5, n6, n7, n8 );
			elements[i] = malloc( strlen(new_elem) + 1 );
			strcpy(elements[i], new_elem);
		}
	}

	end = omp_get_wtime();

	printf("Elem Construct Time: %lf\n", (end-begin));

	// Print to file
	char* _msh_file = strdup(msh_file);
	_msh_file[ strlen(_msh_file) - 4] = '\0';
	char new_msh_file[255];
	snprintf( new_msh_file, 255, "%s_hex27.msh", _msh_file );
	FILE * new_msh = fopen( new_msh_file, "w+" );

	fputs("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n$Nodes\n", new_msh);
	fprintf(new_msh, "%d\n", num_nodes + num_edges + num_faces + num_internal );

	for(int i = 0; i < num_nodes; i++)
	{
		fprintf(new_msh, "%d %s\n", (i+1), nodes[i]);
	}

	printf("nodes: %d. edges: %d. faces: %d. internals: %d\n", num_edges + num_faces + num_nodes + num_internal, num_edges, num_faces, num_internal);
	for(int i = num_nodes + 1; i < num_edges + num_faces + num_nodes + num_internal + 1; i++)
	{
		char * curr_node = all_coords[i];
		fprintf(new_msh, "%s", curr_node);
		free(all_coords[i]);
	}

	fputs("$EndNodes\n$Elements\n", new_msh);
	fprintf(new_msh, "%d\n", num_elements);

	for(int i = 0; i < num_elements; i++)
	{
		fprintf(new_msh, "%s", elements[i]);
	}

	fputs("$EndElements\n", new_msh);
	fgets(buff, 255, msh);

	while( fgets(buff, 255, msh) != NULL )
	{
		fprintf(new_msh, "%s", buff);
	}

	// Memory Freeing
	for(int i = 0; i < num_lines; i++)
	{
		free( file_dat[i] );
	}
	free(nodes);
	free(elements);
	free(all_coords);
	free(internals);

	fclose(new_msh);
}

char * constructElem( char* elem_frag, int elem_id, int num_nodes, edge_t** edges, face_t** faces, int num_elements, internal_t** internals, char* all_coords[num_elements*19], char ** nodes, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8 )
{
	int ids[19];

	// Edge Nodes
	// edge_9
	ids[0] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n2 );

	// edge_10
	ids[1] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n4 );

	// edge_11 
	ids[2] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n1, n5 );

	//edge_12
	ids[3] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n2, n3 );

	// edge_13
	ids[4] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n2, n6 );

	// edge_14
	ids[5] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n3, n4 );

	// edge_15
	ids[6] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n3, n7 );

	// edge_16
	ids[7] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n4, n8 );

	// edge_17
	ids[8] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n5, n6 );

	// edge_18
	ids[9] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n5, n8 );

	// edge_19
	ids[10] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n6, n7 );

	// edge_20
	ids[11] = getEdgeNodeId( elem_id, num_nodes, edges, num_elements, all_coords, nodes, n7, n8 );

	// Face Nodes
	// face_21	
	ids[12] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n2, n4, n1, n3 );

	// face_22
	ids[13] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n1, n6, n2, n5 );

	// face_23
	ids[14] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n5, n4, n8, n1 );

	// face_24
	ids[15] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n7, n2, n6, n3 );

	// face_25
	ids[16] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n7, n4, n3, n8 );

	// face_26
	ids[17] = getFaceNodeId( elem_id, num_nodes, faces, num_elements, all_coords, nodes, n5, n7, n8, n6 );

	// Internal node
	ids[18] = getInternalNodeId( elem_id, num_elements, internals, all_coords, nodes, n2, n8 );

	// Create new element
	char new_elem[255];
	snprintf( new_elem, 255, "%d 12 2 0 0 %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", elem_id, elem_frag, ids[0], ids[1], ids[2], ids[3], ids[4], ids[5], ids[6], ids[7], ids[8], ids[9], ids[10], ids[11], ids[12], ids[13], ids[14], ids[15], ids[16], ids[17], ids[18]);

	//printf("%s", new_elem);

	return strdup(new_elem);
}

int getEdgeNodeId( int elem_id, int num_nodes, edge_t** edges, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2 )
{
	edge_t curr_edge = getEdge( n1, n2, edges );

	// if edge dne
	if( curr_edge.inUse == 0 )
	{
		int retest = 0;
		#pragma omp critical
		{
			// Retest edge
			curr_edge = getEdge( n1, n2, edges );
			if(curr_edge.inUse == 0)
			{
				curr_edge.node_id = edge_id;
				edge_id++;
				curr_edge.inUse = 1;
				// Store in edges
				if( n1 < n2 )
				{
					edges[n1 - 1][n2 - 1] = curr_edge;
				}
				else
				{
					edges[n2 - 1][n1 - 1] = curr_edge;
				}
			}
			else
			{
				retest = 1;
			}	
		}

		if(retest == 1)
		{
			printf("found duplicate\n");
			return curr_edge.node_id;
		}

		#pragma omp atomic update
		num_edges++;

		// Set coords
		char* node_1 = strdup(nodes[n1 - 1]);
		char* node_2 = strdup(nodes[n2 - 1]);
		double x1, x2, y1, y2, z1, z2;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
		curr_edge.x = avg(x1, x2);
		curr_edge.y = avg(y1, y2);
		curr_edge.z = avg(z1, z2);
		curr_edge.inUse = 1;

		// Store in edges
		if( n1 < n2 )
			edges[n1 - 1][n2 - 1] = curr_edge;
		else
			edges[n2 - 1][n1 - 1] = curr_edge;

		//printf("Adding node to all_coords[%d]\n", curr_edge->node_id);
		char * new_node = malloc(50);
		snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_edge.node_id, curr_edge.x, curr_edge.y, curr_edge.z);
		all_coords[curr_edge.node_id] = strdup(new_node);
		free(new_node);
	}

	return curr_edge.node_id;
}

// Check both sets of nodes but always store at faces[n1][n2]/faces[n2][n1]
int getFaceNodeId( int elem_id, int num_nodes, face_t** faces, int num_elements, char* all_coords[num_elements*19], char ** nodes, int n1, int n2, int n3, int n4 )
{
	face_t curr_face = getFace( n1, n2, faces );
	face_t curr_face_2 = getFace( n3, n4, faces );
	// if edge dne
	if( curr_face.inUse == 0 && curr_face_2.inUse == 0 )
	{
		int retest = 0;
		// Race condition
		#pragma omp critical
		{
			curr_face = getFace( n1, n2, faces );
			curr_face_2 = getFace( n3, n4, faces );
			if( curr_face.inUse == 0 && curr_face_2.inUse == 0 )
			{
				curr_face.node_id = edge_id;
				edge_id++;
				curr_face.inUse = 1;
				// Store in faces
				if( n1 < n2 )
				{
					faces[n1 - 1][n2 - 1] = curr_face;
				}
				else
				{
					faces[n2 - 1][n1 - 1] = curr_face;
				}
			}
			else
			{
				retest = 1;
			}
		}

		if(retest == 1)
		{
			printf("found duplicate\n");
			if(curr_face.inUse == 0)
			{
				return curr_face_2.node_id;
			}
			else
			{
				return curr_face.node_id;
			}
		}

		#pragma omp atomic update
		num_faces++;

		// Set coords
		char* node_1 = strdup(nodes[n1 - 1]);
		char* node_2 = strdup(nodes[n2 - 1]);
		double x1, x2, y1, y2, z1, z2;
		sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
		sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
		curr_face.x = avg(x1, x2);
		curr_face.y = avg(y1, y2);
		curr_face.z = avg(z1, z2);
		curr_face.inUse = 1;

		// Store in faces
		if( n1 < n2 )
		{
			faces[n1 - 1][n2 - 1] = curr_face;
		}
		else
		{
			faces[n2 - 1][n1 - 1] = curr_face;
		}

		//printf("Adding node to all_coords[%d]\n", curr_face.node_id);
		char * new_node = malloc(50);
		snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_face.node_id, curr_face.x, curr_face.y, curr_face.z);
		//printf("%s", new_node);
		all_coords[curr_face.node_id] = strdup(new_node);
		free(new_node);
	}
	else
	{
		if(curr_face.inUse == 0)
		{
			curr_face = curr_face_2;
		}
	}

	return curr_face.node_id;
}

int getInternalNodeId( int elem_id, int num_elements, internal_t** internals, char* all_coords[num_elements*19], char** nodes, int n1, int n2 )
{
	internal_t * curr_elem = malloc(sizeof(internal_t));

	#pragma omp critical
	{
		curr_elem->node_id = edge_id;
		edge_id++;
	}

	// Set coords
	char* node_1 = strdup(nodes[n1 - 1]);
	char* node_2 = strdup(nodes[n2 - 1]);
	double x1, x2, y1, y2, z1, z2;
	sscanf(node_1, "%lf %lf %lf", &x1, &y1, &z1);
	sscanf(node_2, "%lf %lf %lf", &x2, &y2, &z2);
	curr_elem->x = avg(x1, x2);
	curr_elem->y = avg(y1, y2);
	curr_elem->z = avg(z1, z2);

	internals[elem_id] = curr_elem;

	#pragma omp atomic update
	num_internal++;

	char new_node[100];
	snprintf(new_node, 100, "%d %lf %lf %lf\n", curr_elem->node_id, curr_elem->x, curr_elem->y, curr_elem->z);
	all_coords[curr_elem->node_id] = strdup(new_node);

	return curr_elem->node_id;
}

edge_t getEdge( int n1, int n2, edge_t ** edges )
{
	if( n1 < n2 )
	{
		return edges[n1 - 1][n2 - 1];
	}
	else
	{
		return edges[n2 - 1][n1 - 1];
	}
}

face_t getFace( int n1, int n2, face_t ** faces )
{
	if( n1 < n2 )
	{
		return faces[n1 - 1][n2 - 1];
	}
	else
	{
		return faces[n2 - 1][n1 - 1];
	}
}

double avg( double a, double b )
{
	return (a+b) / 2.0;
}


