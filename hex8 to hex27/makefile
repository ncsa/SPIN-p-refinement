# Build on Macbook
mac:
	gcc -std=c11 -fopenmp -o exe_mac_hex8 hex8_jk.c main_hex8_jk.c
	export OMP_NUM_THREADS=1; ./exe_mac_hex8 ./files/cube_25.msh


# Build on JYC
jyc:
	cc -std=c11 -fopenmp -o exe_jyc_hex8 hex8_jk.c main_hex8_jk.c

	