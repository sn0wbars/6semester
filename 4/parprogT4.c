#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

long double c = 1;
long double h = 0.1;
long double tau = 0.1;
long double T = 4;
long double X_MIN = 0;
long double X_MAX = 10;

int PRINT = 0; 

long double* u = NULL;
pthread_barrier_t barrier;

struct dataInfo
{
    int start;
    int n_pp;
};

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
};

long double g(long double x)
{
    if(x > 0 && x < 2)
        return x*(2 - x);
    else 
        return 0;
}

long double solve(long double x, long double t)
{
	return g(x-c*t);
}

void* u_next(void* arg)
{
    int start = ((struct dataInfo*)arg)->start;
    int n_pp = ((struct dataInfo*)arg)->n_pp;
    int N_t = roundl(T/tau);

    if (start == X_MIN)
    {
        u[start++] = 0;
        n_pp--;
    }
    for (int j=0; j<N_t; ++j)
    {
        long double u_left = u[start-1];
        int i = start;
        for (i; i<start+n_pp-1; ++i)
        {
            long double u_temp = u[i] - tau*c/h*(u[i]-u_left);
            u_left = u[i];
            u[i] = u_temp;
        }
        pthread_barrier_wait(&barrier);
        u[i] = u[i] - tau*c/h*(u[i]-u_left);
        pthread_barrier_wait(&barrier);
    }
}

int main(int argc, char* argv[]){
    if(argc < 2)
    {
        printf("Too few arguments\n");
        return 1;
    }
    int n = atoi(argv[1]);

    if (argc > 2)
        h /= atoi(argv[2]);

    if (argc > 3)
        PRINT = (strcmp(argv[3], "-v") == 0);


    int N_h = roundl((X_MAX-X_MIN)/h);
    u = (long double*)calloc(N_h, sizeof(long double));

    for(int i=0; i<N_h; ++i)
        u[i] = g(i*h);

    pthread_t* ptr = (pthread_t*)calloc(n, sizeof(pthread_t));
    struct dataInfo* stNumInfo = (struct dataInfo*)calloc(n, sizeof(struct dataInfo));

    pthread_barrier_init(&barrier, NULL, n);
    struct timespec T1, T2;
    clock_gettime(CLOCK_REALTIME, &T1);

    for(int i = 0; i < n; i++)
    {
        stNumInfo[i] = dist(N_h, n, i);
        pthread_create(&(ptr[i]), NULL, u_next, &stNumInfo[i]);
    }

    for(int i = 0; i < n; i++)
        pthread_join(ptr[i], NULL);

    clock_gettime(CLOCK_REALTIME, &T2);
    double time = T2.tv_sec - T1.tv_sec + 1.0e-9*(T2.tv_nsec - T1.tv_nsec);

    if(PRINT)
    {
        printf("Parallel Algorithm\n");
        printf("Time = %f\n", time);
        printf("[%Lf", u[0]);
        for(int i = 1; i<N_h; ++i)
            printf(", %Lf" ,u[i]);
        printf("]\n");
    
        printf("Sequential Algorithm\n");
        printf("[%Lf", solve(0, T));
        for(int i = 1; i<N_h; ++i)
            printf(", %Lf" ,solve(i*h, T));
        printf("]\n");
    }
    FILE* f = fopen("res4.txt", "a");
    fprintf(f, "%f", time); 
    fclose(f);
    pthread_barrier_destroy(&barrier);
    free(ptr);
    free(stNumInfo);

    return 0;
}
