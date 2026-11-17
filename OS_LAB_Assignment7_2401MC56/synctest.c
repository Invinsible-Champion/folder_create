#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_THREADS 3
#define ITERATIONS 100000

// Semaphore identifiers
#define SEM_COUNTER_MUTEX 0
#define SEM_PC_MUTEX       1
#define SEM_PC_EMPTY       2
#define SEM_PC_FULL        3

// Part 1: Shared counter
volatile int sync_counter = 0;

void
sync_thread_worker(void *arg)
{
  int id = (int)arg;
  printf(1, "[Sync-Thread %d] Started (PID: %d)\n", id, getpid());

  for(int i = 0; i < ITERATIONS; i++){
    // Enter critical section using sem_wait
    sem_wait(SEM_COUNTER_MUTEX);

    int temp = sync_counter;
    temp = temp + 1;
    sync_counter = temp;

    // Exit critical section using sem_signal
    sem_signal(SEM_COUNTER_MUTEX);
  }

  printf(1, "[Sync-Thread %d] Completed %d synchronized increments\n", id, ITERATIONS);
  exit();
}

// Part 2: Producer-Consumer variables
#define BUF_SIZE 5
#define TOTAL_ITEMS 15

int buffer[BUF_SIZE];
int buf_in = 0;
int buf_out = 0;
int current_count = 0;
int underflow_detected = 0;
int overflow_detected = 0;

void
producer_worker(void *arg)
{
  printf(1, "[Producer] Worker thread started\n");
  for(int i = 1; i <= TOTAL_ITEMS; i++){
    sem_wait(SEM_PC_EMPTY);  // Decrement empty slots (wait if buffer full)
    sem_wait(SEM_PC_MUTEX);  // Enter critical section

    if(current_count >= BUF_SIZE){
      overflow_detected = 1;
    }

    buffer[buf_in] = i;
    buf_in = (buf_in + 1) % BUF_SIZE;
    current_count++;
    printf(1, "[Producer] Produced item %d  | Buffer Occupancy: %d/5\n", i, current_count);

    sem_signal(SEM_PC_MUTEX); // Exit critical section
    sem_signal(SEM_PC_FULL);  // Increment full slots (wake consumer if waiting)
  }
  printf(1, "[Producer] Finished producing %d items\n", TOTAL_ITEMS);
  exit();
}

void
consumer_worker(void *arg)
{
  printf(1, "    [Consumer] Worker thread started\n");
  for(int i = 1; i <= TOTAL_ITEMS; i++){
    sem_wait(SEM_PC_FULL);   // Decrement full slots (wait if buffer empty)
    sem_wait(SEM_PC_MUTEX);  // Enter critical section

    if(current_count <= 0){
      underflow_detected = 1;
    }

    int item = buffer[buf_out];
    buf_out = (buf_out + 1) % BUF_SIZE;
    current_count--;
    printf(1, "    [Consumer] Consumed item %d  | Buffer Occupancy: %d/5\n", item, current_count);

    sem_signal(SEM_PC_MUTEX); // Exit critical section
    sem_signal(SEM_PC_EMPTY); // Increment empty slots (wake producer if waiting)
  }
  printf(1, "    [Consumer] Finished consuming %d items\n", TOTAL_ITEMS);
  exit();
}

int
main(int argc, char *argv[])
{
  printf(1, "=================================================================\n");
  printf(1, "  Question 2: Process/Thread Synchronization (synctest)          \n");
  printf(1, "=================================================================\n\n");

  // -------------------------------------------------------------
  // Part 1: Synchronized Counter Test
  // -------------------------------------------------------------
  printf(1, "---------------- Part 1: Synchronized Counter -------------------\n");
  printf(1, "Initializing binary semaphore %d with value 1...\n", SEM_COUNTER_MUTEX);
  if(sem_init(SEM_COUNTER_MUTEX, 1) < 0){
    printf(1, "Error: sem_init failed for counter mutex\n");
    exit();
  }

  sync_counter = 0;
  void *stacks[NUM_THREADS];
  for(int i = 0; i < NUM_THREADS; i++){
    stacks[i] = malloc(4096);
    if(!stacks[i]){
      printf(1, "Error: failed to allocate stack %d\n", i);
      exit();
    }
  }

  printf(1, "Spawning %d threads, each doing %d increments...\n", NUM_THREADS, ITERATIONS);
  for(int i = 0; i < NUM_THREADS; i++){
    int tid = clone(sync_thread_worker, (void*)(i + 1), stacks[i]);
    if(tid < 0){
      printf(1, "Error: clone failed for thread %d\n", i + 1);
      exit();
    }
  }

  for(int i = 0; i < NUM_THREADS; i++){
    int jpid = join();
    printf(1, "Parent joined sync-thread PID %d\n", jpid);
  }

  printf(1, "\nPart 1 Results:\n");
  printf(1, "Final counter value : %d\n", sync_counter);
  printf(1, "Expected target     : %d\n", NUM_THREADS * ITERATIONS);
  if(sync_counter == NUM_THREADS * ITERATIONS){
    printf(1, "[PASS] EXACT MATCH! Semaphores successfully eliminated the race condition.\n");
  } else {
    printf(1, "[FAIL] Counter mismatch! Got %d, expected %d\n", sync_counter, NUM_THREADS * ITERATIONS);
  }

  for(int i = 0; i < NUM_THREADS; i++){
    free(stacks[i]);
  }

  // -------------------------------------------------------------
  // Part 2: Classic Producer-Consumer Problem
  // -------------------------------------------------------------
  printf(1, "\n---------------- Part 2: Producer-Consumer ---------------------\n");
  printf(1, "Buffer size: %d | Total items: %d\n", BUF_SIZE, TOTAL_ITEMS);
  printf(1, "Initializing semaphores:\n");
  printf(1, "  SEM_MUTEX (ID %d) = 1 (Mutual exclusion)\n", SEM_PC_MUTEX);
  printf(1, "  SEM_EMPTY (ID %d) = %d (Counting empty slots)\n", SEM_PC_EMPTY, BUF_SIZE);
  printf(1, "  SEM_FULL  (ID %d) = 0 (Counting full slots)\n\n", SEM_PC_FULL);

  sem_init(SEM_PC_MUTEX, 1);
  sem_init(SEM_PC_EMPTY, BUF_SIZE);
  sem_init(SEM_PC_FULL, 0);

  void *p_stack = malloc(4096);
  void *c_stack = malloc(4096);
  if(!p_stack || !c_stack){
    printf(1, "Error allocating producer/consumer stacks\n");
    exit();
  }

  int p_tid = clone(producer_worker, 0, p_stack);
  int c_tid = clone(consumer_worker, 0, c_stack);

  if(p_tid < 0 || c_tid < 0){
    printf(1, "Error cloning producer/consumer threads\n");
    exit();
  }

  join();
  join();

  printf(1, "\nPart 2 Results:\n");
  printf(1, "Buffer Overflow Detected  : %s\n", overflow_detected ? "YES (BUG!)" : "NO");
  printf(1, "Buffer Underflow Detected : %s\n", underflow_detected ? "YES (BUG!)" : "NO");
  printf(1, "Final Buffer Occupancy    : %d/5\n", current_count);

  if(!overflow_detected && !underflow_detected && current_count == 0){
    printf(1, "[PASS] Producer-Consumer executed without any overflow or underflow!\n");
  } else {
    printf(1, "[FAIL] Producer-Consumer anomalies detected.\n");
  }

  free(p_stack);
  free(c_stack);

  printf(1, "=================================================================\n\n");
  exit();
}
