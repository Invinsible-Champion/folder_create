#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
    printf(1, "=== Deadlock Prevention (resourceorder_fixed) ===\n");
    
    // Initialize two semaphores as mutexes (value = 1)
    // Let 0 be Lock1, 1 be Lock2
    sem_init(0, 1);
    sem_init(1, 1);
    
    int pid = fork();
    
    if (pid < 0) {
        printf(1, "Fork failed\n");
        exit();
    }
    
    if (pid == 0) {
        // Child: Process B
        printf(1, "Process B (Child) starting.\n");
        
        // FIX: Always acquire Lock1 before Lock2 to maintain global resource order
        printf(1, "Process B acquiring Lock1...\n");
        sem_down(0);
        printf(1, "Process B acquired Lock1.\n");
        
        sleep(10);
        
        printf(1, "Process B waiting for Lock2...\n");
        sem_down(1);
        printf(1, "Process B acquired Lock2.\n");
        
        // Critical section
        
        sem_up(1);
        sem_up(0);
        printf(1, "Process B finished.\n");
        exit();
    } else {
        // Parent: Process A
        printf(1, "Process A (Parent) starting.\n");
        
        // Process A also acquires Lock1 before Lock2
        printf(1, "Process A acquiring Lock1...\n");
        sem_down(0);
        printf(1, "Process A acquired Lock1.\n");
        
        sleep(10);
        
        printf(1, "Process A waiting for Lock2...\n");
        sem_down(1);
        printf(1, "Process A acquired Lock2.\n");
        
        // Critical section
        
        sem_up(1);
        sem_up(0);
        
        wait();
        printf(1, "Process A finished.\n");
    }
    
    exit();
}
