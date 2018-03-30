#include "tet4_jk.h"


// Static vars
static int num_edges = 0;
static int edge_id = 0;
static int num_faces = 0;
static int num_internal = 0;

void readTET4( const char* msh_file, int* num_nodes, int* num_TET4, NODE** mynodes, EL_TET4** myTET4 )
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
			EL_TET4* tmp_TET4 = (EL_TET4 *) malloc(num_all_elements * sizeof(EL_TET4));
			*num_TET4 = 0;
			int elem_id;
			char str_IX[256];
			for (int i=0;i< num_all_elements; i++) {
				fgets(buff, 256, msh);
				sscanf(buff,"%*d %d",&elem_id);
				if (elem_id == 4) {
					sscanf(buff,"%*d %*d %*d %*d %*d %d %d %d %d",&tmp_TET4[*num_TET4].nodeID[0],
						&tmp_TET4[*num_TET4].nodeID[1],&tmp_TET4[*num_TET4].nodeID[2],&tmp_TET4[*num_TET4].nodeID[3]);
					*num_TET4 = *num_TET4 + 1;
				}
			}
			tmp_TET4 = (EL_TET4 *) realloc(tmp_TET4, *num_TET4*sizeof(EL_TET4));
			*myTET4 = tmp_TET4;
			printf("\t\tnum_TET4 = %d\n",*num_TET4);
		}
	}

	fclose(msh);
	double end = omp_get_wtime();
	printf("\t\tElapsed wall time: %lf sec\n\n", (end-begin));
}


// void Construct_Edges_HEX8( int* num_nodes, int* num_edges, int* num_HEX8, ED_HEX8** myedges, EL_HEX8** myHEX8 )
// {
// 	double begin = omp_get_wtime();
// 	printf("\tStart constructing edge objects from HEX8 elements:\n");

// 	int num_all_edges = *num_HEX8 * MAX_EDGES;			// The maximum number of edges including repeated edges
// 	ED_HEX8* tmp_edges = (ED_HEX8*) malloc(num_all_edges * sizeof(ED_HEX8));
// 	ED_HEX8* unique_edges;
// 	// 0-based CSR format
// 	int *nlink_edges, *link_edges_ptr, *link_edges_ind, *tmp_count;
// 	int *link_edges_ptr_unique, *link_edges_ind_unique;
// 	nlink_edges = (int*) malloc(*num_nodes * sizeof(int));
// 	link_edges_ptr = (int*) malloc( (*num_nodes +1) * sizeof(int));				// CSR pointer for link_edges
// 	link_edges_ind = (int*) malloc(num_all_edges * sizeof(int));					// Another node ID of the edge
// 	link_edges_ptr_unique = (int*) malloc( (*num_nodes +1) * sizeof(int));		// CSR pointer for link_edges_unique
// 	link_edges_ind_unique = (int*) malloc(num_all_edges * sizeof(int));			// Another node ID of the edge after donig unique()
// 	tmp_count = (int*) malloc(*num_nodes * sizeof(int));
// 	int sum_tmp = 0, sum_tmp2=0;
// 	int here;	

// 	#pragma omp parallel default(shared) private(here)
// 	{
// 		#pragma omp for  						// Filling nodeID of myedges 
// 		for (int i=0; i<*num_HEX8; i++) {
// 			// The 1st edge		
// 			tmp_edges[0+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[0];
// 			tmp_edges[0+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[1];
// 			// The 2nd edge
// 			tmp_edges[1+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[1];
// 			tmp_edges[1+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[2];
// 			// The 3rd edge
// 			tmp_edges[2+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[2];
// 			tmp_edges[2+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[3];
// 			// The 4th edge
// 			tmp_edges[3+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[3];
// 			tmp_edges[3+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[0];
// 			// The 5th edge
// 			tmp_edges[4+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[0];
// 			tmp_edges[4+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[4];
// 			// The 6th edge
// 			tmp_edges[5+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[1];
// 			tmp_edges[5+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[5];
// 			// The 7th edge
// 			tmp_edges[6+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[2];
// 			tmp_edges[6+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[6];
// 			// The 8th edge
// 			tmp_edges[7+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[3];
// 			tmp_edges[7+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[7];
// 			// The 9th edge
// 			tmp_edges[8+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[4];
// 			tmp_edges[8+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[5];
// 			// The 10th edge
// 			tmp_edges[9+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[5];
// 			tmp_edges[9+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[6];
// 			// The 11th edge
// 			tmp_edges[10+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[6];
// 			tmp_edges[10+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[7];
// 			// The 12th edge
// 			tmp_edges[11+i*MAX_EDGES].nodeID[0] = (*myHEX8)[i].nodeID[7];
// 			tmp_edges[11+i*MAX_EDGES].nodeID[1] = (*myHEX8)[i].nodeID[4];
// 		}

// 		#pragma omp for 					// Copying tmp_edges to ref_edges and then sorting nodeID of tmp_edges
// 		for (int i=0; i<*num_HEX8; i++) {
// 			for(int j=0; j<MAX_EDGES; j++) {
// 				sort_edge_node(&(tmp_edges[ j+i*MAX_EDGES ]), &((*myHEX8)[i].edgeDIR[j]) );
// 			}
// 		}	

// 		#pragma omp for 					// Initializing nlink_edges
// 		for (int i=0; i<*num_nodes; i++) {
// 			nlink_edges[i] = 0;
// 			tmp_count[i] = 0;
// 		}

// 		#pragma omp single 					
// 		{
// 			// Counting nlink_edges	
// 			for (int i=0; i<num_all_edges; i++) {
// 				nlink_edges[ tmp_edges[i].nodeID[0] - 1 ] = nlink_edges[ tmp_edges[i].nodeID[0] - 1 ] + 1;
// 			}
// 			// Calculating link_edges_ptr
// 			link_edges_ptr[0] = 0;
// 			for (int i=0; i<*num_nodes; i++) {
// 				link_edges_ptr[i+1] = link_edges_ptr[i] + nlink_edges[i];
// 			}

// 			// Filling link_edges_ind
// 			for (int i=0; i<num_all_edges; i++) {
// 				here = link_edges_ptr[ tmp_edges[i].nodeID[0] - 1 ] + tmp_count[ tmp_edges[i].nodeID[0] - 1 ];
// 				tmp_count[ tmp_edges[i].nodeID[0] - 1 ] = tmp_count[ tmp_edges[i].nodeID[0] - 1 ] + 1;
// 				link_edges_ind[here] = tmp_edges[i].nodeID[1];
// 			}
// 		}

// 		#pragma omp for 
// 		for (int i=0;i<*num_nodes; i++) {
// 			tmp_count[i] = nlink_edges[i];
// 			unique_int(&(link_edges_ind[ link_edges_ptr[i] ]),&tmp_count[i]);
// 		}

// 		#pragma omp single 					
// 		{
// 			// Calculating link_edges_ptr_unique
// 			link_edges_ptr_unique[0] = 0;
// 			for (int i=0; i<*num_nodes; i++) {
// 				link_edges_ptr_unique[i+1] = link_edges_ptr_unique[i] + tmp_count[i];
// 			}
// 			*num_edges = link_edges_ptr_unique[*num_nodes];
// 			unique_edges = (ED_HEX8*) malloc( *num_edges * sizeof(ED_HEX8));
// 			*myedges = unique_edges;
// 		}

// 		#pragma omp for 
// 		for (int i=0;i<*num_nodes; i++) {
// 			// Creating link_edges_ind_unique from link_edge_ind
// 			for (int j=0;j<tmp_count[i];j++) {
// 				link_edges_ind_unique[ link_edges_ptr_unique[i] + j ] = 
// 					link_edges_ind[link_edges_ptr[i] + j];
// 			}
// 		}

// 		#pragma omp for  					// Filling *myHEX8.edgeID
// 		for (int i=0;i<*num_HEX8;i++) {
// 			for (int j=0; j<MAX_EDGES; j++) {
// 				for (int k= link_edges_ptr_unique[ tmp_edges[ j+i*MAX_EDGES ].nodeID[0] - 1 ]; 
// 					k < link_edges_ptr_unique[ tmp_edges[ j+i*MAX_EDGES ].nodeID[0] ]; k++) {
// 						if (tmp_edges[j+i*MAX_EDGES].nodeID[1] ==link_edges_ind_unique[k]) {
// 							(*myHEX8)[i].edgeID[j] = k;		break;		
// 						}
// 				}
// 			}
// 		}

// 		#pragma omp for  					// Creating the unique edges
// 		for (int i=0;i<*num_nodes;i++) {
// 			for (int j=link_edges_ptr_unique[i]; j<link_edges_ptr_unique[i+1]; j++) {
// 				unique_edges[j].nodeID[0] = i+1;
// 				unique_edges[j].nodeID[1] = link_edges_ind_unique[j];
// 			}
// 		}

// 	}
// 	// Free used memory
// 	free(link_edges_ind);
// 	free(link_edges_ptr);
// 	free(link_edges_ind_unique);
// 	free(link_edges_ptr_unique);
// 	free(tmp_edges);
// 	free(nlink_edges);
// 	free(tmp_count);

// 	// for (int i=0; i <*num_edges; i++) {
// 	// 	printf (" %d : %d, %d \n",i,(*myedges)[i].nodeID[0],(*myedges)[i].nodeID[1]);
// 	// }
// 	// for (int i=0; i <*num_HEX8; i++) {
// 	// 	printf ("\n %d : ",i);
// 	// 	for (int j=0; j < MAX_EDGES; j++) {
// 	// 		printf(" (%d, %d)", (*myHEX8)[i].edgeID[j], (*myHEX8)[i].edgeDIR[j]);
// 	// 	}
// 	// }
// 	// printf("\n");

// 	double end = omp_get_wtime();
// 	printf("\t\tnum_edges = %d\n",*num_edges);
// 	printf("\t\tElapsed wall time: %lf sec\n\n", (end-begin));
// }



// void Refine_Edges( int* num_nodes, int* num_edges, int* num_HEX8, NODE** mynodes, ED_HEX8** myedges,
// 	EL_HEX8** myHEX8, int order, int* num_new_nodes_edges, NODE** mynewnodes_edge,int** new_IX_edge)
// {
// 	double begin = omp_get_wtime();
// 	printf("\tStart p-refinement from O(1) to O(%d) for edge objects:\n",order);

// 	int num_additional_nodes = (order - 1)* (*num_edges);
// 	*num_new_nodes_edges = num_additional_nodes;
// 	NODE* mynodes_new_edge = (NODE*) malloc(num_additional_nodes * sizeof(NODE));
// 	*mynewnodes_edge = mynodes_new_edge;
// 	int* additional_IX_edge = (int*) malloc( (order-1)*MAX_EDGES*(*num_HEX8) * sizeof(int));
// 	*new_IX_edge = additional_IX_edge;
// 	int here,edgeID;
// 	bool edgeDIR;

// 	#pragma omp parallel default(shared) private(here,edgeID,edgeDIR)
// 	{
// 		#pragma omp for 			// Create additinoal nodes
// 		for (int i=0; i< *num_edges; i++) {
// 			// printf( "  left: %f %f %f \n",(*mynodes)[ (*myedges)[i].nodeID[0] - 1 ].X[0]
// 			// 	,(*mynodes)[ (*myedges)[i].nodeID[0] - 1 ].X[1],(*mynodes)[ (*myedges)[i].nodeID[0] - 1 ].X[2]  );
// 			for (int j=0; j< order-1; j++) {
// 				here = j + i*(order-1);
// 				for (int k=0; k <NDM; k++) {
// 					mynodes_new_edge[here].X[k] = (*mynodes)[ (*myedges)[i].nodeID[0] - 1 ].X[k] + 
// 						( (*mynodes)[ (*myedges)[i].nodeID[1] - 1 ].X[k] - (*mynodes)[ (*myedges)[i].nodeID[0] - 1 ].X[k] )/order*(j+1);	
// 				}
// 				// printf( "  %d   : %f %f %f \n",j,mynodes_new_edge[here].X[0]
// 				// 	, mynodes_new_edge[here].X[1],mynodes_new_edge[here].X[2]  );
// 			}
// 			// printf( " right: %f %f %f \n",(*mynodes)[ (*myedges)[i].nodeID[1] - 1 ].X[0]
// 			// 	,(*mynodes)[ (*myedges)[i].nodeID[1] - 1 ].X[1],(*mynodes)[ (*myedges)[i].nodeID[1] - 1 ].X[2]  );
// 		}

// 		#pragma omp for 			// Create additional_IX_edge
// 		for (int i=0; i< *num_HEX8; i++) {
// 			for (int j=0; j< MAX_EDGES; j++) {
// 				edgeID = (*myHEX8)[i].edgeID[j];
// 				edgeDIR = (*myHEX8)[i].edgeDIR[j];
// 				// printf(" Nodes on edge %d, DIR %d : %d |",edgeID,edgeDIR,(*myedges)[edgeID].nodeID[0]);
// 				for (int k=0; k<order-1; k++) {
// 					here = k + j*(order-1) + i*MAX_EDGES*(order-1);
// 					if (edgeDIR) {
// 						additional_IX_edge[here] = *num_nodes + edgeID*(order-1) + k;
// 					} else {
// 						additional_IX_edge[here] = *num_nodes + (edgeID+1)*(order-1) - k - 1;
// 					}
// 					// printf(" %d ",additional_IX_edge[here]);
// 				}
// 				// printf(" | %d \n",(*myedges)[edgeID].nodeID[1]);
// 			}
// 		}




// 	}



// 	double end = omp_get_wtime();
// 	printf("\t\tnum_additional_nodes = %d\n",num_additional_nodes);
// 	printf("\t\tElapsed wall time: %lf sec\n\n", (end-begin));
// }


// void sort_edge_node( ED_HEX8* edge, bool* flag )
// {
// 	int tmp;

// 	if ((*edge).nodeID[0] > (*edge).nodeID[1]){
// 		tmp = (*edge).nodeID[0];
// 		(*edge).nodeID[0] = (*edge).nodeID[1];
// 		(*edge).nodeID[1] = tmp;
// 		*flag = false;
// 	} else {
// 		*flag = true;
// 	}

// }


// void unique_int( int* list, int* nlist )
// {
// 	int i,j, n_unique=0, tmp[*nlist];

// 	// Counting unique components
// 	for (i=0; i<*nlist; i++ ) {
// 		for (j=0; j<=i; j++) {
// 			if (list[i] == list[j]) break;
// 		}
// 		if (i==j) {
// 			tmp[n_unique] = list[i];
// 			n_unique = n_unique + 1;
// 		}
// 	}
// 	// Copying tmp to lint
// 	for (i=0; i<n_unique; i++) {
// 		list[i] = tmp[i];
// 	}
// 	// Updating *nlist
// 	*nlist = n_unique;
// }


