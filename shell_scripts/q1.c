#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    pid_t pid = fork();
    if(pid<0){
        printf("Fork failed\n");
        exit(1);
    }
    else if(pid == 0){
        printf("Child process %d executed\n",getpid());
        exit(0);
    }
    else{
        wait(NULL);
        printf("I am parent %d\n",getpid());
    }
    return 0;
}
