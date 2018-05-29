#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>

#define NUM 1000000
#define X_MIN 0
#define X_MAX M_PI
#define Y_MIN 0
#define Y_MAX 1

sem_t sem;

double f(double x, double y)
{
	return x*y;
}

void* integr(void* arg)
{
	int seed = time(NULL);
	int n = *((int*) arg);
	double* sum = (double*)calloc(1, sizeof(long double));
	for (int i = 0; i < n; ++i)
	{
		double x = (double)rand_r(&seed)/RAND_MAX*((X_MAX-X_MIN) + X_MIN);
		double y = (double)rand_r(&seed)/RAND_MAX*((Y_MAX-Y_MIN) + Y_MIN);
		if (y <= sin(x))
			*sum += f(x,y);
	}
	pthread_exit((void*)sum);
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Too few arguments\n");
		return 1;
	}
	int n = atoi(argv[1]);
	pthread_t* ptr = (pthread_t*)calloc(n, sizeof(pthread_t));
    
    sem_init(&sem, 0, 1);
	struct timespec T1, T2;

	clock_gettime(CLOCK_REALTIME, &T1);	
	int* arg = (int*)calloc(n, sizeof(int));
	for (int i = 0; i < n; i++)
	{
		arg[i] = NUM/n + (i < NUM%n);
		pthread_create(&ptr[i], NULL, integr, &arg[i]);
	}

	double* s = NULL;
	double res = 0;
	for(int i = 0; i < n; i++)
	{
		pthread_join(ptr[i], (void *)&s);
		res+=*s;
		free(s);
	}
	clock_gettime(CLOCK_REALTIME, &T2);	
	double time = T2.tv_sec - T1.tv_sec + 1.0e-9*(T2.tv_nsec - T1.tv_nsec);
    clock_gettime(CLOCK_REALTIME, &T2);
    printf("Result = %f\nTime = %f\n", res*(X_MAX-X_MIN)*(Y_MAX-Y_MIN)/NUM, time);
	FILE* f = fopen("res3.txt", "a");
	fprintf(f, "%f", time);
	fclose(f);

	sem_destroy(&sem);
	free(ptr);
	free(arg);
	return 0;
}