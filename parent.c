// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <windows.h>

// #define BUFFER_SIZE 1024

// void print_error(const char *msg)
// {
//     fprintf(stderr, "Error: %s (code: %lu)\n", msg, GetLastError());
//     exit(EXIT_FAILURE);
// }

// int main()
// {
//     SECURITY_ATTRIBUTES saAttr;

//     HANDLE hParentToChild1_rd, hParentToChild1_wr;
//     HANDLE hChild1ToChild2_rd, hChild1ToChild2_wr;
//     HANDLE hChild2ToParent_rd, hChild2ToParent_wr;

//     PROCESS_INFORMATION pi1, pi2;
//     STARTUPINFO si1, si2;

//     printf("=== Lab 1. Variant 11 ===\n");
//     printf("Chain: Parent -> Child1 -> Child2 -> Parent\n\n");

//     // Setup security attributes
//     saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
//     saAttr.bInheritHandle = TRUE;
//     saAttr.lpSecurityDescriptor = NULL;

//     // Create pipes
//     printf("=== Creating pipes ===\n");
//     if (!CreatePipe(&hParentToChild1_rd, &hParentToChild1_wr, &saAttr, 0))
//         print_error("CreatePipe Parent->Child1");
//     if (!CreatePipe(&hChild1ToChild2_rd, &hChild1ToChild2_wr, &saAttr, 0))
//         print_error("CreatePipe Child1->Child2");
//     if (!CreatePipe(&hChild2ToParent_rd, &hChild2ToParent_wr, &saAttr, 0))
//         print_error("CreatePipe Child2->Parent");

//     // Create Child1 process
//     printf("=== Creating Child1 ===\n");
//     ZeroMemory(&pi1, sizeof(PROCESS_INFORMATION));
//     ZeroMemory(&si1, sizeof(STARTUPINFO));
//     si1.cb = sizeof(STARTUPINFO);
//     si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);
//     si1.hStdOutput = hChild1ToChild2_wr;
//     si1.hStdInput = hParentToChild1_rd;
//     si1.dwFlags |= STARTF_USESTDHANDLES;

//     char cmd1[] = "child1.exe";
//     if (!CreateProcess(NULL, cmd1, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1))
//         print_error("CreateProcess Child1");

//     printf("Child1 created with PID: %lu\n", pi1.dwProcessId);

//     // Create Child2 process
//     printf("=== Creating Child2 ===\n");
//     ZeroMemory(&pi2, sizeof(PROCESS_INFORMATION));
//     ZeroMemory(&si2, sizeof(STARTUPINFO));
//     si2.cb = sizeof(STARTUPINFO);
//     si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);
//     si2.hStdOutput = hChild2ToParent_wr;
//     si2.hStdInput = hChild1ToChild2_rd;
//     si2.dwFlags |= STARTF_USESTDHANDLES;

//     char cmd2[] = "child2.exe";
//     if (!CreateProcess(NULL, cmd2, NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2))
//         print_error("CreateProcess Child2");

//     printf("Child2 created with PID: %lu\n", pi2.dwProcessId);

//     // Close unused handles
//     CloseHandle(hParentToChild1_rd);
//     CloseHandle(hChild1ToChild2_rd);
//     CloseHandle(hChild1ToChild2_wr);
//     CloseHandle(hChild2ToParent_wr);

//     printf("\n=== Starting data processing ===\n");
//     printf("Enter strings for processing (type 'exit' to quit):\n");

//     char buffer[BUFFER_SIZE];
//     char result[BUFFER_SIZE];
//     DWORD bytes_written, bytes_read;
//     int line_count = 0;

//     while (1)
//     {
//         printf("> ");
//         fflush(stdout);

//         // Read user input
//         if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
//         {
//             break;
//         }

//         // Remove newline
//         buffer[strcspn(buffer, "\n")] = 0;

//         // Check for exit
//         if (strcmp(buffer, "exit") == 0)
//         {
//             break;
//         }

//         line_count++;
//         printf("[Parent] Sending string #%d to Child1: '%s'\n", line_count, buffer);

//         // Send to Child1 (WITH newline)
//         strcpy(buffer + strlen(buffer), "\n");
//         if (!WriteFile(hParentToChild1_wr, buffer, strlen(buffer), &bytes_written, NULL))
//         {
//             print_error("WriteFile to Child1");
//         }

//         // Small delay to ensure processing
//         Sleep(100);

//         // Read result from Child2
//         bytes_read = 0;
//         strcpy(result, "");

//         // Read until we get a complete line
//         while (1)
//         {
//             char chunk[BUFFER_SIZE];
//             DWORD chunk_read;

//             if (ReadFile(hChild2ToParent_rd, chunk, BUFFER_SIZE - 1, &chunk_read, NULL))
//             {
//                 if (chunk_read > 0)
//                 {
//                     chunk[chunk_read] = '\0';
//                     strcat(result, chunk);

//                     // Check if we have a complete line (ends with \n)
//                     if (strchr(chunk, '\n') != NULL)
//                     {
//                         break;
//                     }
//                 }
//                 else
//                 {
//                     break; // No more data
//                 }
//             }
//             else
//             {
//                 break; // Error or no data
//             }
//         }

//         // Remove trailing newline for clean output
//         result[strcspn(result, "\n")] = '\0';
//         printf("[Parent] Final result from Child2: '%s'\n\n", result);
//     }

//     // Cleanup
//     printf("\n=== Shutting down ===\n");

//     CloseHandle(hParentToChild1_wr);
//     CloseHandle(hChild2ToParent_rd);

//     WaitForSingleObject(pi1.hProcess, INFINITE);
//     WaitForSingleObject(pi2.hProcess, INFINITE);

//     CloseHandle(pi1.hProcess);
//     CloseHandle(pi1.hThread);
//     CloseHandle(pi2.hProcess);
//     CloseHandle(pi2.hThread);

//     printf("Program finished.\n");
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define BUFFER_SIZE 1024

void print_error(const char *msg)
{
    fprintf(stderr, "Error: %s (code: %lu)\n", msg, GetLastError());
    exit(EXIT_FAILURE);
}

int main()
{
    SECURITY_ATTRIBUTES saAttr;

    HANDLE hParentToChild1_rd, hParentToChild1_wr;
    HANDLE hChild1ToChild2_rd, hChild1ToChild2_wr;
    HANDLE hChild2ToParent_rd, hChild2ToParent_wr;

    PROCESS_INFORMATION pi1, pi2;
    STARTUPINFO si1, si2;

    printf("=== Lab 1. Variant 11 ===\n");
    printf("Chain: Parent -> Child1 -> Child2 -> Parent\n\n");

    // Setup security attributes
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create pipes
    printf("=== Creating pipes ===\n");
    if (!CreatePipe(&hParentToChild1_rd, &hParentToChild1_wr, &saAttr, 0))
        print_error("CreatePipe Parent->Child1");
    if (!CreatePipe(&hChild1ToChild2_rd, &hChild1ToChild2_wr, &saAttr, 0))
        print_error("CreatePipe Child1->Child2");
    if (!CreatePipe(&hChild2ToParent_rd, &hChild2ToParent_wr, &saAttr, 0))
        print_error("CreatePipe Child2->Parent");

    // Create Child1 process
    printf("=== Creating Child1 ===\n");
    ZeroMemory(&pi1, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si1, sizeof(STARTUPINFO));
    si1.cb = sizeof(STARTUPINFO);
    si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si1.hStdOutput = hChild1ToChild2_wr;
    si1.hStdInput = hParentToChild1_rd;
    si1.dwFlags |= STARTF_USESTDHANDLES;

    char cmd1[] = "child1.exe";
    if (!CreateProcess(NULL, cmd1, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1))
        print_error("CreateProcess Child1");

    printf("Child1 created with PID: %lu\n", pi1.dwProcessId);

    // Create Child2 process
    printf("=== Creating Child2 ===\n");
    ZeroMemory(&pi2, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si2, sizeof(STARTUPINFO));
    si2.cb = sizeof(STARTUPINFO);
    si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si2.hStdOutput = hChild2ToParent_wr;
    si2.hStdInput = hChild1ToChild2_rd;
    si2.dwFlags |= STARTF_USESTDHANDLES;

    char cmd2[] = "child2.exe";
    if (!CreateProcess(NULL, cmd2, NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2))
        print_error("CreateProcess Child2");

    printf("Child2 created with PID: %lu\n", pi2.dwProcessId);

    // Close unused handles
    CloseHandle(hParentToChild1_rd);
    CloseHandle(hChild1ToChild2_rd);
    CloseHandle(hChild1ToChild2_wr);
    CloseHandle(hChild2ToParent_wr);

    printf("\n=== Starting data processing ===\n");
    printf("Enter strings for processing (type 'exit' to quit):\n");

    char buffer[BUFFER_SIZE];
    char result[BUFFER_SIZE];
    DWORD bytes_written, bytes_read;
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
        strcpy(buffer + strlen(buffer), "\n");
        if (!WriteFile(hParentToChild1_wr, buffer, strlen(buffer), &bytes_written, NULL))
        {
            print_error("WriteFile to Child1");
        }

        // Read result from Child2
        bytes_read = 0;
        strcpy(result, "");

        // Read with timeout to avoid blocking forever
        DWORD start_time = GetTickCount();
        while (GetTickCount() - start_time < 5000)
        { // 5 second timeout
            char chunk[BUFFER_SIZE];
            DWORD chunk_read;

            if (ReadFile(hChild2ToParent_rd, chunk, BUFFER_SIZE - 1, &chunk_read, NULL))
            {
                if (chunk_read > 0)
                {
                    chunk[chunk_read] = '\0';
                    strcat(result, chunk);

                    // Check if we have a complete line (ends with \n)
                    if (strchr(chunk, '\n') != NULL)
                    {
                        break;
                    }
                }
                else
                {
                    break; // No more data
                }
            }
            else
            {
                DWORD error = GetLastError();
                if (error == ERROR_BROKEN_PIPE)
                {
                    printf("[Parent] Pipe broken - child process may have exited\n");
                    break;
                }
                // Continue waiting for other errors
            }
            Sleep(10); // Small delay to prevent busy waiting
        }

        // Remove trailing newline for clean output
        if (strlen(result) > 0)
        {
            result[strcspn(result, "\n")] = '\0';
            printf("[Parent] Final result from Child2: '%s'\n\n", result);
        }
        else
        {
            printf("[Parent] No result received from Child2\n\n");
        }
    }

    // Cleanup - signal children to exit by closing pipes
    printf("\n=== Shutting down ===\n");
    printf("Closing pipes to signal children to exit...\n");

    // Close write pipes - this will signal EOF to children
    CloseHandle(hParentToChild1_wr);
    CloseHandle(hChild2ToParent_rd);

    // Wait for child processes with timeout
    printf("Waiting for children to exit...\n");
    DWORD wait_result1 = WaitForSingleObject(pi1.hProcess, 3000); // 3 second timeout
    DWORD wait_result2 = WaitForSingleObject(pi2.hProcess, 3000);

    if (wait_result1 == WAIT_TIMEOUT)
    {
        printf("Child1 did not exit in time, terminating...\n");
        TerminateProcess(pi1.hProcess, 0);
    }
    if (wait_result2 == WAIT_TIMEOUT)
    {
        printf("Child2 did not exit in time, terminating...\n");
        TerminateProcess(pi2.hProcess, 0);
    }

    DWORD exit_code1 = 0, exit_code2 = 0;
    GetExitCodeProcess(pi1.hProcess, &exit_code1);
    GetExitCodeProcess(pi2.hProcess, &exit_code2);

    printf("Child processes exited with codes: Child1=%lu, Child2=%lu\n", exit_code1, exit_code2);

    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);

    printf("Program finished successfully.\n");
    return 0;
}