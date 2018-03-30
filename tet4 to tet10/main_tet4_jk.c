// Main.c for converting TET4 to TET-higher

#include "tet4_jk.h"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		printf("usage: ./toHex27 msh_file.msh\n");
		return 0;
	}
	else
	{
		double begin = omp_get_wtime();

		int num_nodes = 0;
		NODE** mynodes;
		mynodes = (NODE**) malloc(sizeof(NODE*));

		int num_TET4 = 0;
		EL_TET4** myTET4;
		myTET4 = (EL_TET4**) malloc(sizeof(EL_TET4*));

		// Reading the input file: serial process
		readTET4( argv[1],&num_nodes,&num_TET4,mynodes,myTET4 );

		int num_edges = 0;
		ED_TET4** myedges;
		myedges = (ED_TET4**) malloc(sizeof(ED_TET4*));
		// Constructing edges from TET4 elements
		Construct_Edges_TET4(&num_nodes, &num_edges, &num_TET4, myedges, myTET4);


		// Refinement of edges
		// int num_new_nodes_edges, here, edgeID, order;
		// bool edgeDIR;
		// NODE** mynewnodes_edge;
		// mynewnodes_edge = (NODE**) malloc(sizeof(NODE*));
		// int** new_IX_edge;
		// new_IX_edge = (int **) malloc(sizeof(int*));

		// order = 2;
		// Refine_Edges(&num_nodes, &num_edges, &num_TET4, mynodes, myedges, myTET4, order,
		// 								&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);

		// order = 3;
		// Refine_Edges(&num_nodes, &num_edges, &num_TET4, mynodes, myedges, myTET4, order,
		// 								&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);

		// order = 4;
		// Refine_Edges(&num_nodes, &num_edges, &num_TET4, mynodes, myedges, myTET4, order,
		// 								&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);

		// order = 5;
		// Refine_Edges(&num_nodes, &num_edges, &num_TET4, mynodes, myedges, myTET4, order,
		// 								&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);

		// order = 10;
		// Refine_Edges(&num_nodes, &num_edges, &num_TET4, mynodes, myedges, myTET4, order,
		// 								&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);


		// for (int i=0; i< num_edges; i++) {
		// 	printf( "  left: %f %f %f \n",(*mynodes)[ (*myedges)[i].nodeID[0] - 1 ].X[0]
		// 		,(*mynodes)[ (*myedges)[i].nodeID[0] - 1 ].X[1],(*mynodes)[ (*myedges)[i].nodeID[0] - 1 ].X[2]  );
		// 	for (int j=0; j< order-1; j++) {
		// 		here = j + i*(order-1);
		// 		printf( "  %d   : %f %f %f \n",j,(*mynewnodes_edge)[here].X[0]
		// 			, (*mynewnodes_edge)[here].X[1],(*mynewnodes_edge)[here].X[2]  );
		// 	}
		// 	printf( " right: %f %f %f \n",(*mynodes)[ (*myedges)[i].nodeID[1] - 1 ].X[0]
		// 		,(*mynodes)[ (*myedges)[i].nodeID[1] - 1 ].X[1],(*mynodes)[ (*myedges)[i].nodeID[1] - 1 ].X[2]  );
		// }


		// for (int i=0; i< num_TET4; i++) {
		// 	for (int j=0; j< MAX_EDGES; j++) {
		// 		edgeID = (*myTET4)[i].edgeID[j];
		// 		edgeDIR = (*myTET4)[i].edgeDIR[j];
		// 		printf(" Nodes on edge %d, DIR %d : %d |",edgeID,edgeDIR,(*myedges)[edgeID].nodeID[0]);
		// 		for (int k=0; k<order-1; k++) {
		// 			here = k + j*(order-1) + i*MAX_EDGES*(order-1);
		// 			printf(" %d ",(*new_IX_edge)[here]);
		// 		}
		// 		printf(" | %d \n",(*myedges)[edgeID].nodeID[1]);
		// 	}
		// }


		// for (int i=0; i <num_edges; i++) {
		// 	printf (" %d : %d, %d \n",i,(*myedges)[i].nodeID[0],(*myedges)[i].nodeID[1]);
		// }
		// for (int i=0; i <num_TET4; i++) {
		// 	printf ("\n %d : ",i);
		// 	for (int j=0; j < MAX_EDGES; j++) {
		// 		printf(" (%d, %d)", (*myTET4)[i].edgeID[j], (*myTET4)[i].edgeDIR[j]);
		// 	}
		// }
		// printf("\n");



		// printf("\n\n\nMain NODE:%d\n",num_nodes);
		// for (int i=num_nodes-1; i<num_nodes;i++) {
		// 	// printf("Main:%d:\n",i);
		// 	printf("Main NODE:%d %g %g %g\n",i+1,(*mynodes)[i].X[0],(*mynodes)[i].X[1],(*mynodes)[i].X[2]);
		// }

		// printf("Main ELEM:%d\n",num_TET4);
		// for (int i=num_TET4-1; i<num_TET4;i++) {
		// 	printf("Main ELEM: %d %d %d %d %d\n",i,(*myTET4)[i].nodeID[0],(*myTET4)[i].nodeID[1],(*myTET4)[i].nodeID[2],
		// 		(*myTET4)[i].nodeID[3]);
		// }


		double end = omp_get_wtime();
		double time = (end - begin);
		printf("\tTotal elapsted wall time: %f\n", time);
	}

	return 0;
}