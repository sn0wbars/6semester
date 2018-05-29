#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

double func(double x)
{
	return (4.0/(1 + x*x));
}

int main(int argc, char* argv[])
{
	int rank=-1, size=-1;
	double t1=-1, t2=-1, t3 =-1, t4=-1;
	int print = 0;
	if (argc > 2 && argv[2] == "-v")
		print = 1;
		

	int N = atoi(argv[1]);
	int a = 0, b = 1;
	double h = (double)b/N;
	
	int* grid = NULL;
	int* grid_counts = NULL;

	double* res = NULL;

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		double I0 = 0;
		int i;
		t1 = MPI_Wtime();
		for (i = 0; i < N; ++i)
			I0 += (func(h*i) + func(h*(i+1)))/2*h;
		if (print)
			printf("%.10f\n", I0);
		t2 = MPI_Wtime();

		double I1 = 0;

		int d = N/size;
		grid = (int*)calloc((size)*2, sizeof(int));
		grid_counts = (int*)calloc(size, sizeof(int));

		res = (double*)calloc(size, sizeof(double));
		for(i = 0; i < size; ++i)
			grid_counts[i] = d;
		for(i = 0; i < N%size; ++i)
			grid_counts[i]++;
		int pos = 0;
		for(i = 0; i < size; ++i)
		{
			grid[2*i] = pos;
			pos += grid_counts[i];
			grid[2*i+1] = pos;
		}
	}
	int rcv[2];
	t3 = MPI_Wtime();
	MPI_Scatter(grid, 2, MPI_INT, rcv, 2, MPI_INT, 0, MPI_COMM_WORLD);
	double Ik = 0;
	
	int i;
	for (i = rcv[0]; i < rcv[1]; ++i)
		Ik += (func(i*h) + func(h*(i+1)))/2*h;
	if (print)
	{
		printf("%d: %d - %d\n",rank, rcv[0], rcv[1]);
		printf("%d: %f\n", rank, Ik);	
	}
	MPI_Gather(&Ik, 1, MPI_DOUBLE, res, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	if (rank == 0)
	{
		int i;
		double I = 0;
		for (i = 0; i < size; ++i)
			I += res[i];
		if (print)
			printf("%.10f\n", I);
		t4 = MPI_Wtime();
		printf("%f", (t2-t1)/(t4-t3));
		free(grid);
		free(grid_counts);
	}
	MPI_Finalize();

	return 0;
}