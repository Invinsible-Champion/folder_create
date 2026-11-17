#include "types.h"
#include "stat.h"
#include "user.h"

#define N 5 // Number of processes
#define M 3 // Number of resource types

int Allocation[N][M] = {
    {0, 1, 0},
    {2, 0, 0},
    {3, 0, 2},
    {2, 1, 1},
    {0, 0, 2}
};

int Max[N][M] = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3}
};

int Available[M] = {3, 3, 2};
int Need[N][M];

void compute_need() {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            Need[i][j] = Max[i][j] - Allocation[i][j];
        }
    }
}

int is_safe(int print_seq) {
    int work[M];
    int finish[N];
    int safe_seq[N];
    int count = 0;

    for(int i = 0; i < M; i++) {
        work[i] = Available[i];
    }
    for(int i = 0; i < N; i++) {
        finish[i] = 0;
    }

    while(count < N) {
        int found = 0;
        for(int p = 0; p < N; p++) {
            if(finish[p] == 0) {
                int j;
                for(j = 0; j < M; j++) {
                    if(Need[p][j] > work[j]) {
                        break;
                    }
                }
                
                if(j == M) {
                    for(int k = 0; k < M; k++) {
                        work[k] += Allocation[p][k];
                    }
                    safe_seq[count++] = p;
                    finish[p] = 1;
                    found = 1;
                }
            }
        }
        
        if(found == 0) {
            return 0; // System is not in safe state
        }
    }
    
    if (print_seq) {
        printf(1, "System is in a safe state.\nSafe sequence: ");
        for(int i = 0; i < N; i++) {
            printf(1, "P%d ", safe_seq[i]);
        }
        printf(1, "\n");
    }
    return 1;
}

void request_resources(int pid, int request[M]) {
    printf(1, "\n--- Requesting resources for P%d: [%d, %d, %d] ---\n", pid, request[0], request[1], request[2]);
    
    // Check if request exceeds Need
    for(int i = 0; i < M; i++) {
        if(request[i] > Need[pid][i]) {
            printf(1, "Request denied — exceeds maximum need.\n");
            return;
        }
    }
    
    // Check if request exceeds Available
    for(int i = 0; i < M; i++) {
        if(request[i] > Available[i]) {
            printf(1, "Request denied — resources not available right now.\n");
            return;
        }
    }
    
    // Simulate allocation
    for(int i = 0; i < M; i++) {
        Available[i] -= request[i];
        Allocation[pid][i] += request[i];
        Need[pid][i] -= request[i];
    }
    
    // Check if the new state is safe
    if(is_safe(0)) {
        printf(1, "Request granted safely.\n");
        is_safe(1); // Print the new safe sequence
    } else {
        printf(1, "Request denied — would lead to unsafe state. Rolling back.\n");
        // Rollback
        for(int i = 0; i < M; i++) {
            Available[i] += request[i];
            Allocation[pid][i] -= request[i];
            Need[pid][i] += request[i];
        }
    }
}

int main() {
    printf(1, "=== Banker's Algorithm Simulation ===\n");
    
    compute_need();
    is_safe(1); // Initial check
    
    // Scenario 1: Request that can be granted safely
    // P1 requests (1, 0, 2)
    int req1[M] = {1, 0, 2};
    request_resources(1, req1);
    
    // Scenario 2: Request that would lead to unsafe state
    // P4 requests (3, 3, 0)
    int req2[M] = {3, 3, 0};
    request_resources(4, req2);
    
    // Scenario 3: Request that exceeds available
    // P0 requests (0, 2, 0)
    int req3[M] = {0, 2, 0};
    request_resources(0, req3);
    
    exit();
}
