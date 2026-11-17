#include "types.h"
#include "stat.h"
#include "user.h"

// Resource IDs (ordered for deadlock prevention)
#define SCANNER 0 // 1 instance
#define PRINTER 1 // 2 instances
#define DISK    2 // 2 instances

void process_work(int pid, int res1, int res2) {
    // Strategy: Resource Ordering
    // Always request the resource with the lower ID first to prevent circular wait
    int first = (res1 < res2) ? res1 : res2;
    int second = (res1 > res2) ? res1 : res2;
    
    char* res_names[] = {"Scanner", "Printer", "Disk"};
    
    for (int i = 0; i < 3; i++) { // Run for a few cycles
        printf(1, "P%d: Iteration %d. Requesting %s...\n", pid, i, res_names[first]);
        sem_down(first);
        printf(1, "P%d: Granted %s. Requesting %s...\n", pid, res_names[first], res_names[second]);
        
        sem_down(second);
        printf(1, "P%d: Granted %s. Starting work...\n", pid, res_names[second]);
        
        // Simulate work
        sleep(5);
        
        printf(1, "P%d: Releasing %s and %s.\n", pid, res_names[second], res_names[first]);
        sem_up(second);
        sem_up(first);
        
        // Wait before next iteration
        sleep(5);
    }
    exit();
}

int main() {
    printf(1, "=== Combined Synchronization & Deadlock Avoidance ===\n");
    printf(1, "Strategy used: Deadlock Prevention via Resource Ordering (Hierarchical Allocation).\n");
    printf(1, "Resources are strictly ordered: Scanner(0) -> Printer(1) -> Disk(2).\n");
    
    // Initialize semaphores
    sem_init(SCANNER, 1);
    sem_init(PRINTER, 2);
    sem_init(DISK, 2);
    
    // Process requirements (Needs 2 out of 3)
    int reqs[5][2] = {
        {SCANNER, PRINTER},
        {PRINTER, DISK},
        {SCANNER, DISK},
        {SCANNER, PRINTER},
        {PRINTER, DISK}
    };
    
    for (int i = 0; i < 5; i++) {
        int pid = fork();
        if (pid == 0) {
            process_work(i, reqs[i][0], reqs[i][1]);
        }
    }
    
    // Parent waits for all children
    for (int i = 0; i < 5; i++) {
        wait();
    }
    
    printf(1, "All processes completed successfully. No deadlock!\n");
    exit();
}
