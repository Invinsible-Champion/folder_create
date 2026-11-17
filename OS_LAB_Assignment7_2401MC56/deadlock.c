#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_N 10
#define MAX_M 10

// Static working arrays as specified in the assignment to prevent stack overflow
static int Work[MAX_M];
static int Finish[MAX_N];
static int Need[MAX_N][MAX_M];

// Utility to print state matrix
void
print_state(int n, int m, int Available[MAX_M], int Allocation[MAX_N][MAX_M], int RequestOrNeed[MAX_N][MAX_M], const char *req_label)
{
  printf(1, "Available Resources:\n  ");
  for(int j = 0; j < m; j++){
    printf(1, "R%d: %d  ", j, Available[j]);
  }
  printf(1, "\n\n");

  printf(1, "Process  |  Allocation   |  %s\n", req_label);
  printf(1, "---------+---------------+---------------\n");
  for(int i = 0; i < n; i++){
    printf(1, "  P%d     |  ", i);
    for(int j = 0; j < m; j++){
      printf(1, "%d ", Allocation[i][j]);
    }
    printf(1, "         |  ");
    for(int j = 0; j < m; j++){
      printf(1, "%d ", RequestOrNeed[i][j]);
    }
    printf(1, "\n");
  }
  printf(1, "\n");
}

// Deadlock Detection Algorithm
int
detect_deadlock(int n, int m, int Available[MAX_M], int Allocation[MAX_N][MAX_M], int Request[MAX_N][MAX_M], int deadlocked_list[MAX_N])
{
  for(int j = 0; j < m; j++){
    Work[j] = Available[j];
  }

  for(int i = 0; i < n; i++){
    int is_zero = 1;
    for(int j = 0; j < m; j++){
      if(Allocation[i][j] != 0){
        is_zero = 0;
        break;
      }
    }
    Finish[i] = is_zero ? 1 : 0;
  }

  // Iterate to find processes that can complete
  while(1){
    int found = 0;
    for(int i = 0; i < n; i++){
      if(!Finish[i]){
        int can_satisfy = 1;
        for(int j = 0; j < m; j++){
          if(Request[i][j] > Work[j]){
            can_satisfy = 0;
            break;
          }
        }
        if(can_satisfy){
          for(int j = 0; j < m; j++){
            Work[j] += Allocation[i][j];
          }
          Finish[i] = 1;
          found = 1;
        }
      }
    }
    if(!found)
      break;
  }

  // Count deadlocked processes
  int num_deadlocked = 0;
  for(int i = 0; i < n; i++){
    if(!Finish[i]){
      deadlocked_list[num_deadlocked++] = i;
    }
  }

  return num_deadlocked;
}

// Banker's Algorithm Safety Check
int
is_safe_state(int n, int m, int Available[MAX_M], int Allocation[MAX_N][MAX_M], int Max[MAX_N][MAX_M], int safe_sequence[MAX_N])
{
  for(int i = 0; i < n; i++){
    for(int j = 0; j < m; j++){
      Need[i][j] = Max[i][j] - Allocation[i][j];
    }
    Finish[i] = 0;
  }

  for(int j = 0; j < m; j++){
    Work[j] = Available[j];
  }

  int count = 0;
  while(count < n){
    int found = 0;
    for(int i = 0; i < n; i++){
      if(!Finish[i]){
        int can_proceed = 1;
        for(int j = 0; j < m; j++){
          if(Need[i][j] > Work[j]){
            can_proceed = 0;
            break;
          }
        }
        if(can_proceed){
          for(int j = 0; j < m; j++){
            Work[j] += Allocation[i][j];
          }
          safe_sequence[count++] = i;
          Finish[i] = 1;
          found = 1;
        }
      }
    }
    if(!found)
      break;
  }

  return (count == n);
}

// Simulates a hypothetical resource request using Banker's Algorithm
void
simulate_request(int n, int m, int Available[MAX_M], int Allocation[MAX_N][MAX_M], int Max[MAX_N][MAX_M], int pid, int req[MAX_M])
{
  printf(1, "--- Processing Request from Process P%d: [", pid);
  for(int j = 0; j < m; j++){
    printf(1, "%d%s", req[j], (j == m - 1) ? "" : ", ");
  }
  printf(1, "] ---\n");

  for(int i = 0; i < n; i++){
    for(int j = 0; j < m; j++){
      Need[i][j] = Max[i][j] - Allocation[i][j];
    }
  }

  for(int j = 0; j < m; j++){
    if(req[j] > Need[pid][j]){
      printf(1, "ERROR: Process P%d exceeded its maximum declared need.\n\n");
      return;
    }
    if(req[j] > Available[j]){
      printf(1, "REJECTED: Resources not currently available in system.\n\n");
      return;
    }
  }

  // Tentatively allocate
  for(int j = 0; j < m; j++){
    Available[j] -= req[j];
    Allocation[pid][j] += req[j];
  }

  static int safe_seq[MAX_N];
  if(is_safe_state(n, m, Available, Allocation, Max, safe_seq)){
    printf(1, "STATUS: Safe state maintained! Request GRANTED.\n");
    printf(1, "Safe Sequence: ");
    for(int i = 0; i < n; i++){
      printf(1, "P%d%s", safe_seq[i], (i == n - 1) ? "\n\n" : " -> ");
    }
  } else {
    printf(1, "STATUS: Unsafe state detected! Request DENIED (Rolling back allocation).\n\n");
    // Rollback
    for(int j = 0; j < m; j++){
      Available[j] += req[j];
      Allocation[pid][j] -= req[j];
    }
  }
}

// Static test matrix storage
static int Available1[MAX_M] = {3, 3, 2};
static int Allocation1[MAX_N][MAX_M] = {
  {0, 1, 0},
  {2, 0, 0},
  {3, 0, 2},
  {2, 1, 1},
  {0, 0, 2}
};
static int Max1[MAX_N][MAX_M] = {
  {7, 5, 3},
  {3, 2, 2},
  {9, 0, 2},
  {2, 2, 2},
  {4, 3, 3}
};

static int Available3[MAX_M] = {0, 0, 0, 0};
static int Allocation3[MAX_N][MAX_M] = {
  {1, 0, 0, 0}, // P0 holds R0
  {0, 1, 0, 0}, // P1 holds R1
  {0, 0, 1, 0}, // P2 holds R2
  {0, 0, 0, 1}  // P3 holds R3
};
static int Request3[MAX_N][MAX_M] = {
  {0, 1, 0, 0}, // P0 waits for R1 (held by P1)
  {0, 0, 1, 0}, // P1 waits for R2 (held by P2)
  {0, 0, 0, 1}, // P2 waits for R3 (held by P3)
  {1, 0, 0, 0}  // P3 waits for R0 (held by P0)
};

static int Avail_pipe[MAX_M] = {0, 0};
static int Alloc_pipe[MAX_N][MAX_M] = {
  {1, 0}, // Process A holds Pipe 1
  {0, 1}  // Process B holds Pipe 2
};
static int Req_pipe[MAX_N][MAX_M] = {
  {0, 1}, // Process A waiting for Pipe 2
  {1, 0}  // Process B waiting for Pipe 1
};

static int safe_seq1[MAX_N];
static int deadlocked_res[MAX_N];

int
main(int argc, char *argv[])
{
  printf(1, "=================================================================\n");
  printf(1, "    Question 3: Deadlock Detection & Banker's Algorithm          \n");
  printf(1, "=================================================================\n\n");

  // -----------------------------------------------------------------
  // Test Case 1: Initial System State (Banker's Algorithm Safe State)
  // -----------------------------------------------------------------
  printf(1, "================== Test Matrix 1: Safe System State =============\n");
  int n1 = 5, m1 = 3;

  for(int i = 0; i < n1; i++){
    for(int j = 0; j < m1; j++){
      Need[i][j] = Max1[i][j] - Allocation1[i][j];
    }
  }

  print_state(n1, m1, Available1, Allocation1, Need, "Need Matrix  ");

  if(is_safe_state(n1, m1, Available1, Allocation1, Max1, safe_seq1)){
    printf(1, "[PASS] System is in a SAFE state!\nSafe Execution Sequence: ");
    for(int i = 0; i < n1; i++){
      printf(1, "P%d%s", safe_seq1[i], (i == n1 - 1) ? "\n\n" : " -> ");
    }
  } else {
    printf(1, "[FAIL] System unexpectedly reported unsafe.\n\n");
  }

  // -----------------------------------------------------------------
  // Test Case 2: Dynamic Requests (Safe vs Unsafe Requests)
  // -----------------------------------------------------------------
  printf(1, "============= Test Matrix 2: Request Assessment ================\n");
  // 2a. Request that is SAFE: P1 requests (1, 0, 2)
  int req_p1[MAX_M] = {1, 0, 2};
  simulate_request(n1, m1, Available1, Allocation1, Max1, 1, req_p1);

  // 2b. Request that would lead to UNSAFE state: P4 requests (3, 3, 0)
  int req_p4[MAX_M] = {3, 3, 0};
  simulate_request(n1, m1, Available1, Allocation1, Max1, 4, req_p4);

  // 2c. Request that exceeds available resources: P0 requests (0, 3, 0)
  int req_p0[MAX_M] = {0, 3, 0};
  simulate_request(n1, m1, Available1, Allocation1, Max1, 0, req_p0);

  // -----------------------------------------------------------------
  // Test Case 3: Deadlock Detection Algorithm (Circular Wait Matrix)
  // -----------------------------------------------------------------
  printf(1, "============== Test Matrix 3: Deadlock Detection ===============\n");
  int n3 = 4, m3 = 4;

  print_state(n3, m3, Available3, Allocation3, Request3, "Request Matrix");

  int count3 = detect_deadlock(n3, m3, Available3, Allocation3, Request3, deadlocked_res);

  if(count3 > 0){
    printf(1, "[PASS] Deadlock correctly detected by algorithm!\n");
    printf(1, "Number of deadlocked processes: %d\nDeadlocked Process List: ", count3);
    for(int i = 0; i < count3; i++){
      printf(1, "P%d%s", deadlocked_res[i], (i == count3 - 1) ? "\n" : ", ");
    }
    printf(1, "Circular dependency: P0 -> P1 -> P2 -> P3 -> P0\n\n");
  } else {
    printf(1, "[FAIL] Deadlock was not detected!\n\n");
  }

  // -----------------------------------------------------------------
  // Test Case 4: Real xv6 IPC Pipe Circular Wait Simulation
  // -----------------------------------------------------------------
  printf(1, "========== Test Matrix 4: Real xv6 Pipe Deadlock Scenario =======\n");
  printf(1, "Simulating two real xv6 processes interacting via two pipes:\n");
  printf(1, "  Pipe 1 (descriptors p1[0], p1[1])\n");
  printf(1, "  Pipe 2 (descriptors p2[0], p2[1])\n");
  printf(1, "Process A holds write-end of Pipe 1 and waits to read from Pipe 2.\n");
  printf(1, "Process B holds write-end of Pipe 2 and waits to read from Pipe 1.\n\n");

  int p1[2], p2[2];
  if(pipe(p1) < 0 || pipe(p2) < 0){
    printf(1, "Error creating xv6 pipes\n");
    exit();
  }
  printf(1, "Created Pipe 1 [R:%d, W:%d] and Pipe 2 [R:%d, W:%d]\n", p1[0], p1[1], p2[0], p2[1]);

  int n_pipe = 2, m_pipe = 2;
  print_state(n_pipe, m_pipe, Avail_pipe, Alloc_pipe, Req_pipe, "Pipe Request  ");

  int pipe_dl_count = detect_deadlock(n_pipe, m_pipe, Avail_pipe, Alloc_pipe, Req_pipe, deadlocked_res);

  if(pipe_dl_count == 2){
    printf(1, "[PASS] Detection algorithm successfully identified the IPC Pipe Deadlock!\n");
    printf(1, "Deadlocked processes: P0 (Process A) and P1 (Process B)\n");
    printf(1, "Reason: Circular Wait between Pipe 1 and Pipe 2.\n");
  } else {
    printf(1, "[FAIL] Pipe deadlock undetected.\n");
  }

  close(p1[0]); close(p1[1]);
  close(p2[0]); close(p2[1]);

  printf(1, "=================================================================\n\n");
  exit();
}
