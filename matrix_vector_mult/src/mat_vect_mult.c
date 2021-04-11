#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/* Prof version
void Mat_vect_mult(double *local_A, double *local_x,
					 double *local_y, int local_m, int n, 
					 int local_n, MPI_Comm comm) {
	double *x
	int local_ok = 1;

	x = malloc(n*sizeof(double));
	MPI_Allgather(local_x, local_n, MPI_DOUBLE, x, local_n, 
					MPI_DOUBLE, comm);

	for (int local_i=0; local_i < local_m; local_i++) {
		local_y[local_i] = 0.0;
		for (int j=0; j<n; j++) {
			local_y[local_i] += local_A[local_i*n+j]*x[j];
		}
	}
	free(x);
}*/

// Serial version
void Mat_vect_mult_ser(double *A, double *x, double *y, int m, int n) {
	for (int i=0; i<m; i++) {
		y[i] = 0.0;
		for (int j=0; j<n; j++) {
			y[i] += A[i*n+j] * x[j];
		}
	}
}

// My version
double Mat_vect_mult(double *A, double *x, int local_m, int n) {
	double res = 0;
	for (int i=0; i<n; i++) {
		res += x[i]*A[local_m*n+i];
	}
	return res;
}

void printM(double *A, int row, int col);
void printV(double *x, int col, int my_rank);

int main(int argc, char** argv) {

	int comm_sz;
	int my_rank;

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int m = comm_sz;	// Each process will elaborate a line
	int n = 5;
	int local_m = my_rank;

	double *A = (double*)malloc(m*n * sizeof(double));
	for (int i=0; i<m; i++) {
		for (int j=0; j<n; j++) {
			A[i*n+j] = i*n+j+1;
		}
	}
	double *x = (double*)malloc(n * sizeof(double));
	for (int j=0; j<n; j++) {
		x[j] = 2;
	}
	double *y = (double*)malloc(m * sizeof(double));

	if (my_rank == 0) {
		printf("I'm the master, process %d of %d!\n", my_rank, comm_sz);
		printM(A, m, n);
		printV(x, n, my_rank);
		printV(y, m, my_rank);
		printf("\nprocessing...\n");
		y[my_rank] = Mat_vect_mult(A, x, local_m, n);

		MPI_Allgather(&y[my_rank], 1, MPI_DOUBLE, y, 1, MPI_DOUBLE,
			 MPI_COMM_WORLD);
		
		//printf("%d got %lf\n", my_rank, y[m-1]);
		printV(y, m, my_rank);
	}
	else {
		y[my_rank] = Mat_vect_mult(A, x, local_m, n);

		MPI_Allgather(&y[my_rank], 1, MPI_DOUBLE, y, 1, MPI_DOUBLE,
			 MPI_COMM_WORLD);

		//printf("%d got %lf\n", my_rank, y[m-1]);
		printV(y, m, my_rank);
	}
	
	MPI_Finalize();
}

void printM(double *A, int row, int col) {
	printf("Matrix:");
	for (int i=0; i<row; i++) {
		printf("\t[");
		for (int j=0; j<col; j++) {
			printf(" %f", A[i*col+j]);
		}
		printf(" ]\n");
	}
}

void printV(double *x, int col, int my_rank) {
	printf("Me(%d): [", my_rank);
	for (int i=0; i<col; i++) {
		printf(" %f", x[i]);
	}
	printf(" ]\n");
}