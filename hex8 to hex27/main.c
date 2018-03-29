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





		printf("\n\n\nMain NODE:%d\n",num_nodes);
		for (int i=num_nodes-1; i<num_nodes;i++) {
			// printf("Main:%d:\n",i);
			printf("Main NODE:%d %g %g %g\n",i+1,(*mynodes)[i].X[0],(*mynodes)[i].X[1],(*mynodes)[i].X[2]);
		}

		printf("Main ELEM:%d\n",num_HEX8);
		for (int i=num_HEX8-1; i<num_HEX8;i++) {
			printf("Main ELEM: %d %d %d %d %d %d %d %d %d\n",i,(*myHEX8)[i].nodeID[0],(*myHEX8)[i].nodeID[1],(*myHEX8)[i].nodeID[2],
				(*myHEX8)[i].nodeID[3],(*myHEX8)[i].nodeID[4],(*myHEX8)[i].nodeID[5],(*myHEX8)[i].nodeID[6],(*myHEX8)[i].nodeID[7]);
		}


		double end = omp_get_wtime();
		double time = (end - begin);
		printf("Time: %f\n", time);
	}

	return 0;
}