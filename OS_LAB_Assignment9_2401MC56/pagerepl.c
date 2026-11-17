#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_FRAMES 10
#define MAX_REFS   100

// Helper to print frame buffer contents
static void
print_frames(int *frames, int num_frames)
{
  printf(1, "[");
  int i;
  for(i = 0; i < num_frames; i++){
    if(frames[i] == -1)
      printf(1, "  -");
    else if(frames[i] < 10)
      printf(1, "  %d", frames[i]);
    else
      printf(1, " %d", frames[i]);
    if(i < num_frames - 1)
      printf(1, ",");
  }
  printf(1, " ]");
}

// Helper to print percentage without floating point
static void
print_pct(int num, int den)
{
  if(den == 0){
    printf(1, " 0.0%%");
    return;
  }
  int whole = (num * 100) / den;
  int frac = ((num * 1000) / den) % 10;
  if(whole < 10)
    printf(1, "  %d.%d%%", whole, frac);
  else if(whole < 100)
    printf(1, " %d.%d%%", whole, frac);
  else
    printf(1, "%d.%d%%", whole, frac);
}

// FIFO Page Replacement Algorithm
static int
run_fifo(int *refs, int n_refs, int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int i;
  for(i = 0; i < num_frames; i++)
    frames[i] = -1;

  int faults = 0;
  int hits = 0;
  int fifo_victim = 0; // Pointer to next frame to replace

  if(verbose){
    printf(1, "\n--------------------------------------------------------\n");
    printf(1, "FIFO Algorithm (%d Frames, %d References)\n", num_frames, n_refs);
    printf(1, "--------------------------------------------------------\n");
    printf(1, "Step | Ref | Frames Display ");
    for(i = 0; i < num_frames; i++) printf(1, " ");
    printf(1, "| Status\n");
    printf(1, "-----+-----+----------------+--------\n");
  }

  for(i = 0; i < n_refs; i++){
    int ref = refs[i];
    int found = 0;
    int j;
    for(j = 0; j < num_frames; j++){
      if(frames[j] == ref){
        found = 1;
        break;
      }
    }

    if(found){
      hits++;
      if(verbose){
        if(i + 1 < 10) printf(1, "  %d  |  %d  | ", i + 1, ref);
        else           printf(1, " %d  |  %d  | ", i + 1, ref);
        print_frames(frames, num_frames);
        printf(1, " | HIT\n");
      }
    } else {
      faults++;
      int empty_slot = -1;
      for(j = 0; j < num_frames; j++){
        if(frames[j] == -1){
          empty_slot = j;
          break;
        }
      }

      int victim_idx;
      if(empty_slot != -1){
        victim_idx = empty_slot;
        frames[victim_idx] = ref;
      } else {
        victim_idx = fifo_victim;
        frames[victim_idx] = ref;
        fifo_victim = (fifo_victim + 1) % num_frames;
      }

      if(verbose){
        if(i + 1 < 10) printf(1, "  %d  |  %d  | ", i + 1, ref);
        else           printf(1, " %d  |  %d  | ", i + 1, ref);
        print_frames(frames, num_frames);
        printf(1, " | FAULT\n");
      }
    }
  }

  if(verbose){
    printf(1, "-----+-----+----------------+--------\n");
    printf(1, "Total Page Faults: %d\n", faults);
    printf(1, "Total Page Hits  : %d\n", hits);
    printf(1, "Hit Ratio        : ");
    print_pct(hits, n_refs);
    printf(1, "\n");
  }

  return faults;
}

// LRU (Least Recently Used) Page Replacement Algorithm
static int
run_lru(int *refs, int n_refs, int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int last_used[MAX_FRAMES];
  int i;
  for(i = 0; i < num_frames; i++){
    frames[i] = -1;
    last_used[i] = -1;
  }

  int faults = 0;
  int hits = 0;

  if(verbose){
    printf(1, "\n--------------------------------------------------------\n");
    printf(1, "LRU Algorithm (%d Frames, %d References)\n", num_frames, n_refs);
    printf(1, "--------------------------------------------------------\n");
    printf(1, "Step | Ref | Frames Display ");
    for(i = 0; i < num_frames; i++) printf(1, " ");
    printf(1, "| Status\n");
    printf(1, "-----+-----+----------------+--------\n");
  }

  for(i = 0; i < n_refs; i++){
    int ref = refs[i];
    int time_now = i + 1;
    int found_idx = -1;
    int j;
    for(j = 0; j < num_frames; j++){
      if(frames[j] == ref){
        found_idx = j;
        break;
      }
    }

    if(found_idx != -1){
      hits++;
      last_used[found_idx] = time_now;
      if(verbose){
        if(i + 1 < 10) printf(1, "  %d  |  %d  | ", i + 1, ref);
        else           printf(1, " %d  |  %d  | ", i + 1, ref);
        print_frames(frames, num_frames);
        printf(1, " | HIT\n");
      }
    } else {
      faults++;
      int empty_idx = -1;
      for(j = 0; j < num_frames; j++){
        if(frames[j] == -1){
          empty_idx = j;
          break;
        }
      }

      int victim_idx;
      if(empty_idx != -1){
        victim_idx = empty_idx;
      } else {
        // Find victim with earliest last_used time
        victim_idx = 0;
        int min_time = last_used[0];
        for(j = 1; j < num_frames; j++){
          if(last_used[j] < min_time){
            min_time = last_used[j];
            victim_idx = j;
          }
        }
      }

      frames[victim_idx] = ref;
      last_used[victim_idx] = time_now;

      if(verbose){
        if(i + 1 < 10) printf(1, "  %d  |  %d  | ", i + 1, ref);
        else           printf(1, " %d  |  %d  | ", i + 1, ref);
        print_frames(frames, num_frames);
        printf(1, " | FAULT\n");
      }
    }
  }

  if(verbose){
    printf(1, "-----+-----+----------------+--------\n");
    printf(1, "Total Page Faults: %d\n", faults);
    printf(1, "Total Page Hits  : %d\n", hits);
    printf(1, "Hit Ratio        : ");
    print_pct(hits, n_refs);
    printf(1, "\n");
  }

  return faults;
}

int
main(int argc, char *argv[])
{
  // Custom CLI invocation mode: pagerepl <num_frames> <ref1> <ref2> ...
  if(argc > 2){
    int num_frames = atoi(argv[1]);
    if(num_frames < 1 || num_frames > MAX_FRAMES){
      printf(1, "Error: Number of frames must be between 1 and %d\n", MAX_FRAMES);
      exit();
    }
    int n_refs = argc - 2;
    int refs[MAX_REFS];
    int i;
    for(i = 0; i < n_refs && i < MAX_REFS; i++){
      refs[i] = atoi(argv[i + 2]);
    }

    printf(1, "========================================================\n");
    printf(1, "   Custom Page Replacement Simulation (%d Frames)       \n", num_frames);
    printf(1, "========================================================\n");
    run_fifo(refs, n_refs, num_frames, 1);
    run_lru(refs, n_refs, num_frames, 1);
    exit();
  }

  // Standard assignment benchmark string (Tasks b and c)
  int default_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
  int n_refs = sizeof(default_refs) / sizeof(default_refs[0]);

  printf(1, "========================================================================\n");
  printf(1, "   Assignment 9 - Question 3: Page Replacement Algorithms (FIFO & LRU)  \n");
  printf(1, "========================================================================\n");
  printf(1, "Reference String (20 references):\n");
  printf(1, "  7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1\n");

  printf(1, "\n========================================================================\n");
  printf(1, "   Task b: Testing with 3 Frames (Expected: FIFO=15, LRU=12)            \n");
  printf(1, "========================================================================\n");

  int fifo_faults_3 = run_fifo(default_refs, n_refs, 3, 1);
  int lru_faults_3  = run_lru(default_refs, n_refs, 3, 1);

  printf(1, "--------------------------------------------------------\n");
  printf(1, "Task b Verification Results (3 Frames):\n");
  printf(1, "  FIFO Page Faults: %d (Expected: 15) -> %s\n",
         fifo_faults_3, (fifo_faults_3 == 15) ? "PASS" : "FAIL");
  printf(1, "  LRU  Page Faults: %d (Expected: 12) -> %s\n",
         lru_faults_3, (lru_faults_3 == 12) ? "PASS" : "FAIL");
  printf(1, "--------------------------------------------------------\n");

  printf(1, "\n========================================================================\n");
  printf(1, "   Task c: Comparative Summary Table for 3, 4, and 5 Frames             \n");
  printf(1, "========================================================================\n");
  printf(1, "+--------+-----------+-------------+-------------+--------+-----------+\n");
  printf(1, "| Frames | Algorithm | Total Refs  | Page Faults | Hits   | Hit Ratio |\n");
  printf(1, "+--------+-----------+-------------+-------------+--------+-----------+\n");

  int frame_tests[3] = {3, 4, 5};
  int f;
  for(f = 0; f < 3; f++){
    int nf = frame_tests[f];
    int f_faults = run_fifo(default_refs, n_refs, nf, 0);
    int f_hits = n_refs - f_faults;
    printf(1, "|   %d    | FIFO      |     %d      |     ", nf, n_refs);
    if(f_faults < 10) printf(1, " ");
    printf(1, "%d      |   ", f_faults);
    if(f_hits < 10) printf(1, " ");
    printf(1, "%d   |   ", f_hits);
    print_pct(f_hits, n_refs);
    printf(1, "   |\n");

    int l_faults = run_lru(default_refs, n_refs, nf, 0);
    int l_hits = n_refs - l_faults;
    printf(1, "|   %d    | LRU       |     %d      |     ", nf, n_refs);
    if(l_faults < 10) printf(1, " ");
    printf(1, "%d      |   ", l_faults);
    if(l_hits < 10) printf(1, " ");
    printf(1, "%d   |   ", l_hits);
    print_pct(l_hits, n_refs);
    printf(1, "   |\n");

    printf(1, "+--------+-----------+-------------+-------------+--------+-----------+\n");
  }

  printf(1, "\nKey Observations:\n");
  printf(1, "1. For 3 frames, LRU generates 12 faults (40.0%% hit ratio) vs FIFO 15 faults (25.0%% hit ratio).\n");
  printf(1, "2. For 4 frames, LRU achieves 8 faults (60.0%% hit ratio) vs FIFO 10 faults (50.0%% hit ratio).\n");
  printf(1, "3. For 5 frames, LRU achieves 7 faults (65.0%% hit ratio) vs FIFO 9 faults (55.0%% hit ratio).\n");
  printf(1, "4. Across all frame allocations, LRU consistently achieves higher hit ratios because\n");
  printf(1, "   it exploits temporal locality by replacing the page not accessed for the longest time.\n");

  printf(1, "\n========================================================================\n");
  printf(1, "                    PAGEREPL COMPLETED SUCCESSFULLY                     \n");
  printf(1, "========================================================================\n");

  exit();
}
