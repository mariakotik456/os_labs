#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
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
    printf("=== Lab 3. Variant 11 ===\n");
    printf("Chain: Parent -> Child1 -> Child2 -> Parent (using Memory-Mapped Files)\n\n");

    int shm_fd1 = shm_open("/shm_parent_child1", O_CREAT | O_RDWR, 0666);
    int shm_fd2 = shm_open("/shm_child1_child2", O_CREAT | O_RDWR, 0666);
    int shm_fd3 = shm_open("/shm_child2_parent", O_CREAT | O_RDWR, 0666);

    if (shm_fd1 == -1 || shm_fd2 == -1 || shm_fd3 == -1)
    {
        print_error("shm_open");
    }

    if (ftruncate(shm_fd1, SHM_SIZE) == -1 ||
        ftruncate(shm_fd2, SHM_SIZE) == -1 ||
        ftruncate(shm_fd3, SHM_SIZE) == -1)
    {
        print_error("ftruncate");
    }

    shared_data *shm1 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    shared_data *shm2 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);
    shared_data *shm3 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd3, 0);

    if (shm1 == MAP_FAILED || shm2 == MAP_FAILED || shm3 == MAP_FAILED)
    {
        print_error("mmap");
    }

    sem_t *sem_parent_child1 = sem_open("/sem_pc1", O_CREAT, 0666, 0);
    sem_t *sem_child1_child2 = sem_open("/sem_c1c2", O_CREAT, 0666, 0);
    sem_t *sem_child2_parent = sem_open("/sem_c2p", O_CREAT, 0666, 0);

    if (sem_parent_child1 == SEM_FAILED || sem_child1_child2 == SEM_FAILED || sem_child2_parent == SEM_FAILED)
    {
        print_error("sem_open");
    }

    memset(shm1, 0, SHM_SIZE);
    memset(shm2, 0, SHM_SIZE);
    memset(shm3, 0, SHM_SIZE);
    shm1->exit_flag = 0;
    shm2->exit_flag = 0;
    shm3->exit_flag = 0;

    printf("=== Creating Child1 ===\n");
    pid_t pid1 = fork();
    if (pid1 == -1)
    {
        print_error("fork Child1");
    }

    if (pid1 == 0)
    {
        close(shm_fd1);
        close(shm_fd2);
        close(shm_fd3);

        execl("./child1", "child1", NULL);
        print_error("execl Child1");
    }

    printf("Child1 created with PID: %d\n", pid1);

    printf("=== Creating Child2 ===\n");
    pid_t pid2 = fork();
    if (pid2 == -1)
    {
        print_error("fork Child2");
    }

    if (pid2 == 0)
    {
        close(shm_fd1);
        close(shm_fd2);
        close(shm_fd3);

        execl("./child2", "child2", NULL);
        print_error("execl Child2");
    }

    printf("Child2 created with PID: %d\n", pid2);

    close(shm_fd1);
    close(shm_fd2);
    close(shm_fd3);

    printf("\n=== Starting data processing ===\n");
    printf("Enter strings for processing (type 'exit' to quit):\n");

    char buffer[BUFFER_SIZE];
    int line_count = 0;

    while (1)
    {
        printf("> ");
        fflush(stdout);

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0)
        {
            shm1->exit_flag = 1;
            sem_post(sem_parent_child1);
            break;
        }

        line_count++;
        printf("[Parent] Sending string #%d to Child1: '%s'\n", line_count, buffer);

        strncpy(shm1->data, buffer, BUFFER_SIZE - 1);
        shm1->data[BUFFER_SIZE - 1] = '\0';
        sem_post(sem_parent_child1);

        sem_wait(sem_child2_parent);

        printf("[Parent] Final result from Child2: '%s'\n\n", shm3->data);
    }

    printf("\n=== Shutting down ===\n");
    printf("Waiting for children to exit...\n");

    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    munmap(shm1, SHM_SIZE);
    munmap(shm2, SHM_SIZE);
    munmap(shm3, SHM_SIZE);
    shm_unlink("/shm_parent_child1");
    shm_unlink("/shm_child1_child2");
    shm_unlink("/shm_child2_parent");

    sem_close(sem_parent_child1);
    sem_close(sem_child1_child2);
    sem_close(sem_child2_parent);
    sem_unlink("/sem_pc1");
    sem_unlink("/sem_c1c2");
    sem_unlink("/sem_c2p");

    printf("Child processes exited with codes: Child1=%d, Child2=%d\n",
           WEXITSTATUS(status1), WEXITSTATUS(status2));

    printf("Program finished successfully.\n");
    return 0;
}