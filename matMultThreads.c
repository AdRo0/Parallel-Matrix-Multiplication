#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int **NxM;
int **MxN;
int **R;
int N;
int cell_index = 0;

int random_range(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

void *cell_thread(void *arg)
{
    pthread_mutex_lock(&lock);
    int cell = cell_index++;
    pthread_mutex_unlock(&lock);

    int last_cell = (N * N) - 1;
    while (cell <= last_cell)
    {
        int row = cell / N;
        int col = cell % N;

        int result = 0;
        for (int k = 0; k < N; k++)
        {
            result += NxM[row][k] * MxN[k][col];
        }
        R[row][col] = result;

        pthread_mutex_lock(&lock);
        cell = cell_index++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void mult()
{
    pthread_t threads[1000];
    for (int k = 0; k < 1000; k++) // Crear 8 threads
        pthread_create(&threads[k], NULL, cell_thread, NULL);

    for (int k = 0; k < 1000; k++)
        pthread_join(threads[k], NULL);

    pthread_mutex_destroy(&lock);
}

int main()
{
    printf("N: ");
    scanf("%d", &N);

    srand(time(NULL));

    // Allocate mem
    NxM = malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++)
        NxM[i] = malloc(N * sizeof(int));

    MxN = malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++)
        MxN[i] = malloc(N * sizeof(int));

    R = malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++)
        R[i] = malloc(N * sizeof(int));

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            NxM[i][j] = random_range(1, 5);

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            MxN[i][j] = random_range(1, 5);

    double total_time = 0;
    int runs = 100;
    double durations[runs];
    struct timespec start, end;
    for (int i = 0; i < runs; i++)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        mult();
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        durations[i] = time_taken;
        total_time += time_taken;
    }
    double average_time = total_time / runs;
    printf("Promedio: %f\n", average_time);

    double suma_cuadrados = 0;
    for (int i = 0; i < runs; i++)
    {
        suma_cuadrados += (durations[i] - average_time) * (durations[i] - average_time);
    }
    double desv = sqrt(suma_cuadrados / runs) > 0 ? sqrt(suma_cuadrados / runs) : 0;
    printf("Desviacion estandar: %f\n", desv);

    for (int i = 0; i < N; i++)
        free(NxM[i]);
    free(NxM);

    for (int i = 0; i < N; i++)
        free(MxN[i]);
    free(MxN);

    for (int i = 0; i < N; i++)
        free(R[i]);
    free(R);

    return 0;
}
