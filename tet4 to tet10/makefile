# Build on Macbook
mac:
	gcc -std=c11 -fopenmp -o exe_mac_tet4 tet4_jk.c main_tet4_jk.c
	export OMP_NUM_THREADS=1; ./exe_mac_tet4 ./files/cube_554.msh


# Build on JYC
jyc:
	cc -std=c11 -fopenmp -o exe_jyc_tet4 tet4_jk.c main_tet4_jk.c

	