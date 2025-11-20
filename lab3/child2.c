#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define BUFFER_SIZE 1024
#define SHM_SIZE 4096

typedef struct
{
    char data[BUFFER_SIZE];
    int exit_flag;
} shared_data;

void print_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main()
{
    int shm_fd2 = shm_open("/shm_child1_child2", O_RDWR, 0666);
    int shm_fd3 = shm_open("/shm_child2_parent", O_RDWR, 0666);

    if (shm_fd2 == -1 || shm_fd3 == -1)
    {
        print_error("shm_open");
    }

    shared_data *shm2 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);
    shared_data *shm3 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd3, 0);

    if (shm2 == MAP_FAILED || shm3 == MAP_FAILED)
    {
        print_error("mmap");
    }

    sem_t *sem_child1_child2 = sem_open("/sem_c1c2", 0);
    sem_t *sem_child2_parent = sem_open("/sem_c2p", 0);

    if (sem_child1_child2 == SEM_FAILED || sem_child2_parent == SEM_FAILED)
    {
        print_error("sem_open");
    }

    close(shm_fd2);
    close(shm_fd3);

    while (1)
    {
        sem_wait(sem_child1_child2);

        if (shm2->exit_flag)
        {
            shm3->exit_flag = 1;
            sem_post(sem_child2_parent);
            break;
        }

        for (int i = 0; shm2->data[i] != '\0'; i++)
        {
            if (isspace(shm2->data[i]))
            {
                shm2->data[i] = '_';
            }
        }

        strncpy(shm3->data, shm2->data, BUFFER_SIZE - 1);
        shm3->data[BUFFER_SIZE - 1] = '\0';
        sem_post(sem_child2_parent);
    }

    munmap(shm2, SHM_SIZE);
    munmap(shm3, SHM_SIZE);
    sem_close(sem_child1_child2);
    sem_close(sem_child2_parent);

    return 0;
}