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
    int shm_fd1 = shm_open("/shm_parent_child1", O_RDWR, 0666);
    int shm_fd2 = shm_open("/shm_child1_child2", O_RDWR, 0666);

    if (shm_fd1 == -1 || shm_fd2 == -1)
    {
        print_error("shm_open");
    }

    shared_data *shm1 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    shared_data *shm2 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);

    if (shm1 == MAP_FAILED || shm2 == MAP_FAILED)
    {
        print_error("mmap");
    }

    sem_t *sem_parent_child1 = sem_open("/sem_pc1", 0);
    sem_t *sem_child1_child2 = sem_open("/sem_c1c2", 0);

    if (sem_parent_child1 == SEM_FAILED || sem_child1_child2 == SEM_FAILED)
    {
        print_error("sem_open");
    }

    close(shm_fd1);
    close(shm_fd2);

    while (1)
    {
        sem_wait(sem_parent_child1);

        if (shm1->exit_flag)
        {
            shm2->exit_flag = 1;
            sem_post(sem_child1_child2);
            break;
        }

        for (int i = 0; shm1->data[i] != '\0'; i++)
        {
            shm1->data[i] = toupper(shm1->data[i]);
        }

        strncpy(shm2->data, shm1->data, BUFFER_SIZE - 1);
        shm2->data[BUFFER_SIZE - 1] = '\0';
        sem_post(sem_child1_child2);
    }

    munmap(shm1, SHM_SIZE);
    munmap(shm2, SHM_SIZE);
    sem_close(sem_parent_child1);
    sem_close(sem_child1_child2);

    return 0;
}