#include <unistd.h>
#include <stdio.h>

int main() {
    // Arguments are packed into an array of string pointers.
    // The first element is the command name, the last must be NULL.
    char *args[] = {"ls", "-l", NULL};
    
    // Pass the array. Excellent for dynamically built commands.
    execvp("ls", args);
    
    perror("execvp failed");
    return 1;
}