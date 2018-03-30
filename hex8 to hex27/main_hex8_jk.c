// Main.c for converting hex8 to hex27

#include "hex8_jk.h"

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

		int num_HEX8 = 0;
		EL_HEX8** myHEX8;
		myHEX8 = (EL_HEX8**) malloc(sizeof(EL_HEX8*));

		// Reading the input file: serial process
		readHEX8( argv[1],&num_nodes,&num_HEX8,mynodes,myHEX8 );

		int num_edges = 0;
		ED_HEX8** myedges;
		myedges = (ED_HEX8**) malloc(sizeof(ED_HEX8*));
		// Constructing edges from HEX8 elements
		Construct_Edges_HEX8(&num_nodes, &num_edges, &num_HEX8, myedges, myHEX8);

		// int num_faces = 0;
		// FA_HEX8** myfaces;
		// myfaces = (FA_HEX8**) malloc(sizeof(FA_HEX8*));
		// // Constructing faces from HEX8 elements
		// Construct_Faces_HEX8(&num_nodes, &num_faces, &num_HEX8, myfaces, myHEX8);





		// Refinement of edges
		int num_new_nodes_edges, here, edgeID, order;
		bool edgeDIR;
		NODE** mynewnodes_edge;
		mynewnodes_edge = (NODE**) malloc(sizeof(NODE*));
		int** new_IX_edge;
		new_IX_edge = (int **) malloc(sizeof(int*));

		order = 2;
		Refine_Edges(&num_nodes, &num_edges, &num_HEX8, mynodes, myedges, myHEX8, order,
										&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);

		order = 3;
		Refine_Edges(&num_nodes, &num_edges, &num_HEX8, mynodes, myedges, myHEX8, order,
										&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);

		order = 4;
		Refine_Edges(&num_nodes, &num_edges, &num_HEX8, mynodes, myedges, myHEX8, order,
										&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);

		order = 5;
		Refine_Edges(&num_nodes, &num_edges, &num_HEX8, mynodes, myedges, myHEX8, order,
										&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);

		order = 10;
		Refine_Edges(&num_nodes, &num_edges, &num_HEX8, mynodes, myedges, myHEX8, order,
										&num_new_nodes_edges, mynewnodes_edge, new_IX_edge);


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


		// for (int i=0; i< num_HEX8; i++) {
		// 	for (int j=0; j< MAX_EDGES; j++) {
		// 		edgeID = (*myHEX8)[i].edgeID[j];
		// 		edgeDIR = (*myHEX8)[i].edgeDIR[j];
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
		// for (int i=0; i <num_HEX8; i++) {
		// 	printf ("\n %d : ",i);
		// 	for (int j=0; j < MAX_EDGES; j++) {
		// 		printf(" (%d, %d)", (*myHEX8)[i].edgeID[j], (*myHEX8)[i].edgeDIR[j]);
		// 	}
		// }
		// printf("\n");



		// printf("\n\n\nMain NODE:%d\n",num_nodes);
		// for (int i=num_nodes-1; i<num_nodes;i++) {
		// 	// printf("Main:%d:\n",i);
		// 	printf("Main NODE:%d %g %g %g\n",i+1,(*mynodes)[i].X[0],(*mynodes)[i].X[1],(*mynodes)[i].X[2]);
		// }

		// printf("Main ELEM:%d\n",num_HEX8);
		// for (int i=num_HEX8-1; i<num_HEX8;i++) {
		// 	printf("Main ELEM: %d %d %d %d %d %d %d %d %d\n",i,(*myHEX8)[i].nodeID[0],(*myHEX8)[i].nodeID[1],(*myHEX8)[i].nodeID[2],
		// 		(*myHEX8)[i].nodeID[3],(*myHEX8)[i].nodeID[4],(*myHEX8)[i].nodeID[5],(*myHEX8)[i].nodeID[6],(*myHEX8)[i].nodeID[7]);
		// }


		double end = omp_get_wtime();
		double time = (end - begin);
		printf("\tTotal elapsted wall time: %f\n", time);
	}

	return 0;
}