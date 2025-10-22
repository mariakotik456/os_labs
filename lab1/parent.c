#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#define BUFFER_SIZE 1024

void print_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main()
{
    int pipe1[2]; // parent -> child1
    int pipe2[2]; // child1 -> child2
    int pipe3[2]; // child2 -> parent

    pid_t pid1, pid2;

    printf("=== Lab 1. Variant 11 ===\n");
    printf("Chain: Parent -> Child1 -> Child2 -> Parent\n\n");

    // Create pipes
    printf("=== Creating pipes ===\n");
    if (pipe(pipe1) == -1)
        print_error("pipe Parent->Child1");
    if (pipe(pipe2) == -1)
        print_error("pipe Child1->Child2");
    if (pipe(pipe3) == -1)
        print_error("pipe Child2->Parent");

    // Create Child1 process
    printf("=== Creating Child1 ===\n");
    pid1 = fork();
    if (pid1 == -1)
        print_error("fork Child1");

    if (pid1 == 0)
    {
        // Child1 process
        close(pipe1[1]); // close write end of pipe1
        close(pipe2[0]); // close read end of pipe2
        close(pipe3[0]); // close read end of pipe3
        close(pipe3[1]); // close write end of pipe3

        // Redirect stdin to pipe1 read end
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        // Redirect stdout to pipe2 write end
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);

        // Execute child1 program
        execl("./child1", "child1", NULL);
        print_error("execl Child1");
    }

    printf("Child1 created with PID: %d\n", pid1);

    // Create Child2 process
    printf("=== Creating Child2 ===\n");
    pid2 = fork();
    if (pid2 == -1)
        print_error("fork Child2");

    if (pid2 == 0)
    {
        // Child2 process
        close(pipe1[0]); // close read end of pipe1
        close(pipe1[1]); // close write end of pipe1
        close(pipe2[1]); // close write end of pipe2
        close(pipe3[0]); // close read end of pipe3

        // Redirect stdin to pipe2 read end
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);

        // Redirect stdout to pipe3 write end
        dup2(pipe3[1], STDOUT_FILENO);
        close(pipe3[1]);

        // Execute child2 program
        execl("./child2", "child2", NULL);
        print_error("execl Child2");
    }

    printf("Child2 created with PID: %d\n", pid2);

    // Parent process - close unused pipe ends
    close(pipe1[0]); // close read end of pipe1
    close(pipe2[0]); // close read end of pipe2
    close(pipe2[1]); // close write end of pipe2
    close(pipe3[1]); // close write end of pipe3

    printf("\n=== Starting data processing ===\n");
    printf("Enter strings for processing (type 'exit' to quit):\n");

    char buffer[BUFFER_SIZE];
    char result[BUFFER_SIZE];
    int line_count = 0;

    while (1)
    {
        printf("> ");
        fflush(stdout);

        // Read user input
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            break;
        }

        // Remove newline
        buffer[strcspn(buffer, "\n")] = 0;

        // Check for exit
        if (strcmp(buffer, "exit") == 0)
        {
            break;
        }

        line_count++;
        printf("[Parent] Sending string #%d to Child1: '%s'\n", line_count, buffer);

        // Send to Child1 (WITH newline)
        strcat(buffer, "\n");
        ssize_t bytes_written = write(pipe1[1], buffer, strlen(buffer));
        if (bytes_written == -1)
        {
            perror("write to Child1");
            break;
        }

        // Read result from Child2
        ssize_t bytes_read = read(pipe3[0], result, BUFFER_SIZE - 1);
        if (bytes_read > 0)
        {
            result[bytes_read] = '\0';
            // Remove trailing newline for clean output
            result[strcspn(result, "\n")] = '\0';
            printf("[Parent] Final result from Child2: '%s'\n\n", result);
        }
        else if (bytes_read == -1)
        {
            perror("read from Child2");
            break;
        }
        else
        {
            printf("[Parent] No result received from Child2\n\n");
        }
    }

    // Cleanup
    printf("\n=== Shutting down ===\n");
    printf("Closing pipes to signal children to exit...\n");

    // Close write pipes - this will signal EOF to children
    close(pipe1[1]);
    close(pipe3[0]);

    // Wait for child processes
    printf("Waiting for children to exit...\n");

    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    printf("Child processes exited with codes: Child1=%d, Child2=%d\n",
           WEXITSTATUS(status1), WEXITSTATUS(status2));

    printf("Program finished successfully.\n");
    return 0;
}