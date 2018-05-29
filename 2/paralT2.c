#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

#define k 1
#define l 1
#define u_x0 1
#define u_0t 0
#define u_lt 0
#define eps 1e-100

int t_n = 1000;
int N = 2000;
double h = -1;
double tau = -1;
double T = -1;

int PRINT = 0; 

struct dataInfo
{
	int start;
	int n_pp;
};

double u_next(double x, double y, double z)
{
	return y + tau*k*(z - 2*y + x)/h/h;
}

double f_series(int m, double x, double t)
{
	return exp(-k*M_PI*M_PI*(2*m + 1)*(2*m + 1)*t/l/l)/(2*m + 1)*sin(M_PI*(2*m + 1)*x/l);
}

struct dataInfo dist(int x, int size, int rank)
{
	int rest = x%size;
	int n_pp = x/size;
	int a = rank*n_pp;
	if (rank < rest)
	{
		a += rank;
		n_pp++;
	}
	else
		a += rest;
	struct dataInfo dI = {a, n_pp};
	return dI;
}

double solve(double x, double t)
{
	double res = 0;
	int i = 0;
	for(i; i < 10; ++i)
	{
		double val = f_series(i, x, t);
		if(fabs(val) < eps)
			break;
		res += val;
	}
	printf("%d\n", i);
	return 4*u_x0/M_PI * res;
}//number of steps

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	int i = 1;
	if (argc > i)
	{
		if (strcmp(argv[i++], "-v") == 0)
		{
			PRINT = 1;
		}
		if (argc > i)
			N = atoi(argv[i++]);
		if (argc > i)
			t_n = atoi(argv[i]);
	}
	h = 1.0*l/N;
	tau = h*h/k/2;
	T = (tau*t_n);

	int rank=-1, size=-1;
	double t1=-1, t2=-1, t3=-1, t4=-1;	

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int n = N - 1; // Number of points, where Value will be calculated
	struct dataInfo dI = dist(n, size, rank);
	int a = dI.start;
	int n_pp = dI.n_pp;

	if (rank == 0 && PRINT)
	{
		printf("h = %f, tau = %.10f, T = %f\n", h, tau, T);
		printf("Parallel algorithm\n");
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	t3 = MPI_Wtime();
	int u_size = n_pp + 2;
	double** u = (double**) calloc(t_n+1, sizeof(double*)); 
	u[0] = (double*) calloc(u_size, sizeof(double));
	for(int i = 1; i < u_size-1; ++i) //
		u[0][i] = u_x0;

	int ProcR = (rank < size-1)? rank + 1: MPI_PROC_NULL, ProcL = (rank > 0)? rank - 1: MPI_PROC_NULL;

	for(int t = 0; t < t_n; ++t)
	{		
		MPI_Sendrecv(&u[t][1], 1, MPI_DOUBLE, ProcL, 0, &u[t][u_size-1], 1, MPI_DOUBLE, ProcR, 0, MPI_COMM_WORLD,  MPI_STATUS_IGNORE);
		MPI_Sendrecv(&u[t][u_size-2], 1, MPI_DOUBLE, ProcR, 1, &u[t][0], 1, MPI_DOUBLE, ProcL, 1, MPI_COMM_WORLD,  MPI_STATUS_IGNORE);
		
		if (rank == 0)
			u[t][0] = u_0t;
		else if (rank == size - 1)
			u[t][u_size-1] = u_lt;

		u[t+1] = (double*) calloc(u_size, sizeof(double));
		for(int i = 1; i < u_size-1; ++i)
			u[t+1][i] = u_next(u[t][i-1],u[t][i],u[t][i+1]);
	}
	t4 = MPI_Wtime();

	if (PRINT)
	{
		for(int j = 0; j < size; ++j)
		{
			MPI_Barrier(MPI_COMM_WORLD);
			if (rank == j)
			{
				int i = (rank > 0)? 1 : 0;
				printf("%d: %d-%d\n [%f", rank, a, a+n_pp+(rank==size-1), u[t_n][i++]);
				for(i; i < u_size - (rank < size-1)?1:0; ++i)
					printf(" ,%f", u[t_n][i]);
				printf("]\n");
			}
		}
	}

	for(int t = 0; t < t_n+1; ++t)
		free(u[t]);
	free(u);

	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)
	{
		u_size = n+2;

		// Exact solution
		if (PRINT)
		{
			double y[u_size];
			for(int i = 0; i < u_size; ++i)
				y[i] = solve(i*h, T);

			if(PRINT)
			{
				printf("Exact solution\n");
				printf("[");
				for(int i = 0; i < u_size-1; i+=(u_size-1)/10)
					printf("%f, ", y[i]);
				printf("%f]\n", y[u_size-1]);
			}
		}

		// Sequential algorithm	
		t1 = MPI_Wtime();
		double** u = (double**) calloc(t_n+1, sizeof(double*)); 
		u[0] = (double*) calloc(u_size, sizeof(double));
		for(int i = 1; i < u_size-1; ++i) //
			u[0][i] = u_x0;

		for(int t = 0; t < t_n; ++t)
		{			
			u[t][0] = u_0t;
			u[t][u_size-1] = u_lt;
			u[t+1] = (double*) calloc(u_size, sizeof(double));
			for(int i = 1; i < u_size-1; ++i)
				u[t+1][i] = u_next(u[t][i-1],u[t][i],u[t][i+1]);
		}
		t2 = MPI_Wtime();

		if (PRINT)
		{
			printf("Sequential algorithm\n[");
			for(int j = 0; j < u_size-1; j+=(u_size-1)/10)
				printf("%f, ", u[t_n][j]);
			printf("%f]\n", u[t_n][u_size-1]);
		}

		for(int t = 0; t < t_n+1; ++t)
			free(u[t]);
		free(u);

		double s = (t2-t1)/(t4-t3);
		if(PRINT)
			printf("Speedup: %f\n", s);
		FILE* f = fopen("res2.txt", "a");
		fprintf(f, "%f", s);
		fclose(f);
	}
	MPI_Finalize();			
	return 0;
}