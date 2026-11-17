#include <unistd.h>
#include <stdio.h>

int main() {
    // Because of 'p', we only need "ls". The OS looks in /bin, /usr/bin, etc.
    execlp("ls", "ls", "-l", (char *)NULL);
    
    perror("execlp failed");
    return 1;
}