#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

sem_t *sem;
int (*NxM)[0];
int (*MxN)[0];
int (*R)[0];
int N;
int cell_index = 0;

int random_range(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

void cell_process()
{
    int last_cell = N * N - 1;

    while (1)
    {
        sem_wait(sem);
        int cell = cell_index++;
        sem_post(sem);

        if (cell > last_cell)
            break;

        int row = cell / N;
        int col = cell % N;

        int result = 0;
        for (int k = 0; k < N; k++)
        {
            result += NxM[row][k] * MxN[k][col];
        }
        R[row][col] = result;
    }
}

void mult()
{
    int num_processes = 1000;
    pid_t pids[num_processes];

    for (int i = 0; i < num_processes; i++)
    {
        pids[i] = fork();
        if (pids[i] < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (pids[i] == 0)
        {
            cell_process();
            exit(0);
        }
    }

    for (int i = 0; i < num_processes; i++)
        wait(NULL);
}

int main()
{
    printf("N: ");
    scanf("%d", &N);

    srand(time(NULL));

    NxM = mmap(NULL, sizeof(int[N][N]), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    MxN = mmap(NULL, sizeof(int[N][N]), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    R = mmap(NULL, sizeof(int[N][N]), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(sem, 1, 1);

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            NxM[i][j] = random_range(1, 5);

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            MxN[i][j] = random_range(1, 5);

    double total_time = 0;
    int runs = 30;
    double durations[runs];
    struct timespec start, end;

    for (int r = 0; r < runs; r++)
    {
        cell_index = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);
        mult();
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time_taken = (end.tv_sec - start.tv_sec) +
                            (end.tv_nsec - start.tv_nsec) / 1e9;
        durations[r] = time_taken;
        total_time += time_taken;
    }

    double average_time = total_time / runs;
    printf("Promedio: %f\n", average_time);

    double suma_cuadrados = 0;
    for (int i = 0; i < runs; i++)
        suma_cuadrados += (durations[i] - average_time) * (durations[i] - average_time);

    double desv = sqrt(suma_cuadrados / runs);
    printf("Desviacion estandar: %f\n", desv);

    munmap(NxM, sizeof(int[N][N]));
    munmap(MxN, sizeof(int[N][N]));
    munmap(R, sizeof(int[N][N]));
    sem_destroy(sem);
    munmap(sem, sizeof(sem_t));

    return 0;
}
