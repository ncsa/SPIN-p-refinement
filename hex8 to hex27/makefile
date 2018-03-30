# Build on Macbook
mac:
	gcc -std=c11 -fopenmp -o exe_mac hex8_jk.c main.c
	export OMP_NUM_THREADS=1; ./exe_mac ./files/cube_25.msh


# Build on JYC
jyc:
	cc -std=c11 -fopenmp -o exe_jyc hex8_jk.c main.c

	