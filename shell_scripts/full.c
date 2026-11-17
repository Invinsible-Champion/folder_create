#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // 1. Enforce strict argument count check
    if (argc != 3) {
        printf("Incorrect number of arguments\n");
        exit(1);
    }

    // 2. Fork a child process to handle the command
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
       
        execlp(argv[1], argv[1], argv[2], (char *)NULL);
        
        // If execlp returns, it means the command was not found or failed.
        perror("exec failed");
        exit(1); 
    } 
    else {
        // PARENT PROCESS
        int status;
        // Wait for the child to terminate and reap it
        wait(&status);
       
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Command successfully completed\n");
        } else {
            printf("Command failed during execution\n");
        }
    }

    return 0;
}