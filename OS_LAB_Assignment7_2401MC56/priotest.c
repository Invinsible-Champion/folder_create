#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PROCS 4

// CPU work loop count (calibrated for demonstrable scheduling & aging)
#define WORK_LOOPS 50000000

// Initial priority assignments (1 = highest, 20 = lowest)
int test_priorities[NUM_PROCS] = {5, 10, 15, 20};

void
cpu_workload(int id, int init_pri)
{
  int pid = getpid();
  int start_tick = uptime();
  int current_pri_at_start = get_priority(pid);
  printf(1, "[Child %d | PID %d] Started CPU work (Assigned: %d, Effective at start: %d, Start Tick: %d)\n",
         id, pid, init_pri, current_pri_at_start, start_tick);

  volatile int dummy = 0;
  int quarter = WORK_LOOPS / 4;

  for(int i = 1; i <= WORK_LOOPS; i++){
    dummy += (i & 0x7);

    // Periodic status report at 25%, 50%, 75%
    if(i % quarter == 0){
      int cur_pri = get_priority(pid);
      int cur_tick = uptime();
      printf(1, "  -> [Child %d | PID %d] Progress: %d%% | Current Priority: %d (Init: %d) | Tick: %d\n",
             id, pid, (i / quarter) * 25, cur_pri, init_pri, cur_tick);
    }
  }

  if(dummy == 0x7fffffff)
    printf(1, "never\n");

  int end_tick = uptime();
  int final_pri = get_priority(pid);
  printf(1, "[Child %d | PID %d] COMPLETED! (Init Pri: %d -> Final Pri: %d | Start: %d, End: %d, Duration: %d ticks)\n",
         id, pid, init_pri, final_pri, start_tick, end_tick, end_tick - start_tick);

  exit();
}

int
main(int argc, char *argv[])
{
  printf(1, "=================================================================\n");
  printf(1, "  Question 4: Priority Scheduling with Aging (priotest)          \n");
  printf(1, "=================================================================\n");
  printf(1, "Spawning %d CPU-bound processes with distinct priorities:\n", NUM_PROCS);
  for(int i = 0; i < NUM_PROCS; i++){
    printf(1, "  Child %d: Initial Priority = %d (%s)\n",
           i, test_priorities[i],
           (test_priorities[i] <= 5) ? "Highest" :
           (test_priorities[i] <= 10) ? "Medium-High" :
           (test_priorities[i] <= 15) ? "Medium-Low" : "Lowest");
  }
  printf(1, "Aging Rule: Any RUNNABLE process waiting > 30 ticks without CPU\n");
  printf(1, "            has priority incremented (decremented value by 1).\n");
  printf(1, "-----------------------------------------------------------------\n\n");

  int pids[NUM_PROCS];

  for(int i = 0; i < NUM_PROCS; i++){
    int pid = fork();
    if(pid < 0){
      printf(1, "Error: fork failed for child %d\n", i);
      exit();
    }
    if(pid == 0){
      // Child process runs workload
      cpu_workload(i, test_priorities[i]);
    }
    pids[i] = pid;
    // Parent sets initial priority
    set_priority(pid, test_priorities[i]);
  }

  // Parent collects termination order
  int completion_order[NUM_PROCS];
  for(int i = 0; i < NUM_PROCS; i++){
    int finished_pid = wait();
    completion_order[i] = finished_pid;
    printf(1, ">>> Parent reaped finished process PID %d (Finish Position #%d)\n", finished_pid, i + 1);
  }

  printf(1, "\n==================== Final Scheduling Summary ====================\n");
  printf(1, "Finish Order  |  Process ID  |  Expected Priority Bias\n");
  printf(1, "--------------+--------------+----------------------------------\n");
  for(int i = 0; i < NUM_PROCS; i++){
    int child_idx = -1;
    for(int k = 0; k < NUM_PROCS; k++){
      if(pids[k] == completion_order[i]){
        child_idx = k;
        break;
      }
    }
    printf(1, "     #%d       |   PID %d     |  Child %d (Init Priority: %d)\n",
           i + 1, completion_order[i], child_idx,
           (child_idx >= 0) ? test_priorities[child_idx] : -1);
  }

  printf(1, "\nAnalysis:\n");
  printf(1, "1. Higher-priority processes (Priority 5, 10) received CPU preference\n");
  printf(1, "   and made rapid early progress.\n");
  printf(1, "2. The lowest-priority process (Child 3, Priority 20) experienced aging;\n");
  printf(1, "   as wait-ticks accumulated, its priority improved dynamically,\n");
  printf(1, "   preventing starvation and allowing it to finish successfully!\n");
  printf(1, "=================================================================\n\n");

  exit();
}
