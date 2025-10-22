#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];
    
    // Disable buffering for immediate output
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        // Convert to uppercase (ignoring newline)
        for (int i = 0; buffer[i] != '\0' && buffer[i] != '\n'; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        
        // Output result (will be redirected to Child2)
        printf("%s", buffer);
        fflush(stdout);
    }
    
    return 0;
}