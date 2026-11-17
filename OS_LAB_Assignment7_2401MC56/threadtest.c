#include "types.h"
#include "stat.h"
#include "user.h"

// Shared global counter accessed concurrently by multiple threads
volatile int counter = 0;
volatile int start_flag = 0;

// Thread worker function: increments shared counter 100,000 times
void
thread_worker(void *arg)
{
  int id = (int)arg;
  printf(1, "[Thread %d] Started (PID: %d)\n", id, getpid());

  // Wait until all threads are created by parent to maximize concurrency
  while(!start_flag)
    ;

  for(int i = 0; i < 100000; i++){
    // Read-modify-write without synchronization
    int temp = counter;
    for(volatile int d = 0; d < 50; d++)
      ;
    counter = temp + 1;
  }

  printf(1, "[Thread %d] Completed 100,000 iterations\n", id);
  exit();
}

int
main(int argc, char *argv[])
{
  printf(1, "========================================================\n");
  printf(1, "  Question 1: Kernel-Supported Threads (threadtest)     \n");
  printf(1, "========================================================\n");
  printf(1, "Creating 3 threads using clone()...\n");
  printf(1, "Each thread executes 100,000 increments on a shared counter.\n");
  printf(1, "Ideal expected total without race conditions: 300,000\n\n");

  counter = 0;
  start_flag = 0;

  // Allocate 4KB user-level stack for each thread
  void *stack1 = malloc(4096);
  void *stack2 = malloc(4096);
  void *stack3 = malloc(4096);

  if(!stack1 || !stack2 || !stack3){
    printf(1, "Error: Failed to allocate thread stacks\n");
    exit();
  }

  int tid1 = clone(thread_worker, (void*)1, stack1);
  if(tid1 < 0){
    printf(1, "Error: clone(1) failed\n");
    exit();
  }

  int tid2 = clone(thread_worker, (void*)2, stack2);
  if(tid2 < 0){
    printf(1, "Error: clone(2) failed\n");
    exit();
  }

  int tid3 = clone(thread_worker, (void*)3, stack3);
  if(tid3 < 0){
    printf(1, "Error: clone(3) failed\n");
    exit();
  }

  // Release all 3 threads concurrently
  start_flag = 1;

  // Parent waits for all 3 threads to terminate using join()
  for(int i = 0; i < 3; i++){
    int joined_pid = join();
    printf(1, "Parent successfully joined child thread PID %d\n", joined_pid);
  }

  printf(1, "\n------------------- Test Results -------------------\n");
  printf(1, "Final counter value: %d\n", counter);
  printf(1, "Expected value     : 300000\n");
  printf(1, "Lost updates       : %d\n", 300000 - counter);

  if(counter < 300000){
    printf(1, "Outcome: Race condition successfully demonstrated!\n");
    printf(1, "Because threads share the same address space without synchronization,\n");
    printf(1, "concurrent increments interleaved, leading to lost updates.\n");
  } else {
    printf(1, "Outcome: All increments registered (execution interleaved sequentially).\n");
  }
  printf(1, "========================================================\n\n");

  free(stack1);
  free(stack2);
  free(stack3);
  exit();
}
