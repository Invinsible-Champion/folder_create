#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

// DFS variables
int visited[MAX_PROCESSES];
int recStack[MAX_PROCESSES];
int path[MAX_PROCESSES];
int cycle_length = 0;

int has_cycle(int wait_for[MAX_PROCESSES][MAX_PROCESSES], int num_processes, int v, int path_idx) {
    visited[v] = 1;
    recStack[v] = 1;
    path[path_idx] = v;

    for (int i = 0; i < num_processes; i++) {
        if (wait_for[v][i]) {
            if (!visited[i]) {
                if (has_cycle(wait_for, num_processes, i, path_idx + 1)) {
                    return 1;
                }
            } else if (recStack[i]) {
                // Cycle detected, extract the cycle
                cycle_length = 0;
                int j = path_idx;
                while (j >= 0 && path[j] != i) {
                    j--;
                }
                
                // Now path[j...path_idx] contains the cycle
                printf(1, "Deadlock detected! Cycle: ");
                for (int k = j; k <= path_idx; k++) {
                    printf(1, "P%d -> ", path[k]);
                }
                printf(1, "P%d\n", i);
                
                return 1;
            }
        }
    }

    recStack[v] = 0;
    return 0;
}

void detect_deadlock(int num_processes, int num_resources, int alloc[][MAX_RESOURCES], int req[][MAX_RESOURCES]) {
    int wait_for[MAX_PROCESSES][MAX_PROCESSES];
    
    // Initialize wait-for graph
    for (int i = 0; i < num_processes; i++) {
        for (int j = 0; j < num_processes; j++) {
            wait_for[i][j] = 0;
        }
    }
    
    // Construct wait-for graph
    // Edge from P_i to P_j if P_i is waiting for a resource R_k held by P_j
    for (int i = 0; i < num_processes; i++) {
        for (int k = 0; k < num_resources; k++) {
            if (req[i][k] > 0) {
                // Find who holds resource k
                for (int j = 0; j < num_processes; j++) {
                    if (alloc[j][k] > 0 && i != j) {
                        wait_for[i][j] = 1;
                    }
                }
            }
        }
    }
    
    printf(1, "Wait-for graph constructed.\n");
    
    // Run DFS to detect cycles
    for (int i = 0; i < num_processes; i++) {
        visited[i] = 0;
        recStack[i] = 0;
    }
    cycle_length = 0;
    
    int deadlock = 0;
    for (int i = 0; i < num_processes; i++) {
        if (!visited[i]) {
            if (has_cycle(wait_for, num_processes, i, 0)) {
                deadlock = 1;
                break; // Just print one cycle
            }
        }
    }
    
    if (!deadlock) {
        printf(1, "No deadlock detected (acyclic wait-for graph).\n");
    }
}

int main() {
    printf(1, "=== Scenario 1: No Deadlock ===\n");
    int num_processes1 = 3;
    int num_resources1 = 3;
    
    // Allocation matrix
    int alloc1[MAX_PROCESSES][MAX_RESOURCES] = {
        {1, 0, 0}, // P0 holds R0
        {0, 1, 0}, // P1 holds R1
        {0, 0, 1}  // P2 holds R2
    };
    
    // Request matrix
    int req1[MAX_PROCESSES][MAX_RESOURCES] = {
        {0, 1, 0}, // P0 wants R1
        {0, 0, 1}, // P1 wants R2
        {0, 0, 0}  // P2 wants nothing
    };
    
    detect_deadlock(num_processes1, num_resources1, alloc1, req1);
    
    printf(1, "\n=== Scenario 2: Deadlock (Circular wait >= 3 processes) ===\n");
    int num_processes2 = 4;
    int num_resources2 = 4;
    
    int alloc2[MAX_PROCESSES][MAX_RESOURCES] = {
        {1, 0, 0, 0}, // P0 holds R0
        {0, 1, 0, 0}, // P1 holds R1
        {0, 0, 1, 0}, // P2 holds R2
        {0, 0, 0, 1}  // P3 holds R3
    };
    
    int req2[MAX_PROCESSES][MAX_RESOURCES] = {
        {0, 1, 0, 0}, // P0 wants R1 (held by P1)
        {0, 0, 1, 0}, // P1 wants R2 (held by P2)
        {0, 0, 0, 1}, // P2 wants R3 (held by P3)
        {1, 0, 0, 0}  // P3 wants R0 (held by P0) -> cycle P0->P1->P2->P3->P0
    };
    
    detect_deadlock(num_processes2, num_resources2, alloc2, req2);
    
    exit();
}
