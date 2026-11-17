#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_FRAMES 10
#define MAX_REFS   100

// Helper function to print frame contents formatted cleanly
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

// Helper function to print percentages with 1 decimal place (avoiding floating point)
static void
print_pct(int num, int den)
{
  if(den == 0){
    printf(1, "  0.0%%");
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

// -------------------------------------------------------------
// Algorithm 1: FIFO (First-In First-Out)
// -------------------------------------------------------------
static int
run_fifo(int *refs, int n_refs, int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int i;
  for(i = 0; i < num_frames; i++)
    frames[i] = -1;

  int faults = 0;
  int hits = 0;
  int fifo_victim = 0; // Circular pointer to oldest loaded frame

  if(verbose){
    printf(1, "\n--------------------------------------------------------\n");
    printf(1, "FIFO Algorithm Simulation (%d Frames, %d References)\n", num_frames, n_refs);
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
    printf(1, "\nPage Fault Ratio : ");
    print_pct(faults, n_refs);
    printf(1, "\n");
  }

  return faults;
}

// -------------------------------------------------------------
// Algorithm 2: LRU (Least Recently Used)
// -------------------------------------------------------------
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
    printf(1, "LRU Algorithm Simulation (%d Frames, %d References)\n", num_frames, n_refs);
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
        // Find victim frame whose last access was farthest in the past
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
    printf(1, "\nPage Fault Ratio : ");
    print_pct(faults, n_refs);
    printf(1, "\n");
  }

  return faults;
}

// -------------------------------------------------------------
// Algorithm 3: Optimal Page Replacement (OPT)
// -------------------------------------------------------------
static int
run_optimal(int *refs, int n_refs, int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int i;
  for(i = 0; i < num_frames; i++)
    frames[i] = -1;

  int faults = 0;
  int hits = 0;

  if(verbose){
    printf(1, "\n--------------------------------------------------------\n");
    printf(1, "Optimal Algorithm Simulation (%d Frames, %d References)\n", num_frames, n_refs);
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
      } else {
        // Find the page that will not be used for longest period in the future
        int farthest_time = -1;
        victim_idx = 0;

        for(j = 0; j < num_frames; j++){
          int next_use = 999999; // Represents infinity (never used again)
          int k;
          for(k = i + 1; k < n_refs; k++){
            if(refs[k] == frames[j]){
              next_use = k;
              break;
            }
          }

          if(next_use > farthest_time){
            farthest_time = next_use;
            victim_idx = j;
          }
        }
      }

      frames[victim_idx] = ref;

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
    printf(1, "\nPage Fault Ratio : ");
    print_pct(faults, n_refs);
    printf(1, "\n");
  }

  return faults;
}

// -------------------------------------------------------------
// Main: Runs Tasks (a), (b), and (c)
// -------------------------------------------------------------
int
main(int argc, char *argv[])
{
  // Custom CLI invocation support: pagereplace <frames> <r1> <r2> ...
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
    printf(1, "  Custom Page Replacement Simulation (%d Frames)         \n", num_frames);
    printf(1, "========================================================\n");
    run_fifo(refs, n_refs, num_frames, 1);
    run_lru(refs, n_refs, num_frames, 1);
    run_optimal(refs, n_refs, num_frames, 1);
    exit();
  }

  // Standard Assignment 10 Reference String:
  // 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
  int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
  int n_refs = sizeof(refs) / sizeof(refs[0]);

  printf(1, "========================================================================\n");
  printf(1, "      Assignment 10 - Question 4: Page Replacement Algorithms           \n");
  printf(1, "                      FIFO, LRU, and OPTIMAL                            \n");
  printf(1, "========================================================================\n\n");
  printf(1, "Reference String (20 references):\n");
  printf(1, "  7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1\n");

  // -------------------------------------------------------------
  // Task (b): Test all three algorithms with 3 frames
  // -------------------------------------------------------------
  printf(1, "\n========================================================================\n");
  printf(1, "  Task (a) & (b): Detailed Simulation with 3 Frames                     \n");
  printf(1, "========================================================================\n");

  int fifo_3 = run_fifo(refs, n_refs, 3, 1);
  int lru_3  = run_lru(refs, n_refs, 3, 1);
  int opt_3  = run_optimal(refs, n_refs, 3, 1);

  printf(1, "\n--------------------------------------------------------\n");
  printf(1, "Task (b) Summary (3 Frames):\n");
  printf(1, "  FIFO    Total Page Faults: %d (Expected: 15) -> %s\n",
         fifo_3, (fifo_3 == 15) ? "PASS" : "FAIL");
  printf(1, "  LRU     Total Page Faults: %d (Expected: 12) -> %s\n",
         lru_3, (lru_3 == 12) ? "PASS" : "FAIL");
  printf(1, "  OPTIMAL Total Page Faults: %d (Expected:  9) -> %s\n",
         opt_3, (opt_3 == 9) ? "PASS" : "FAIL");
  printf(1, "--------------------------------------------------------\n");

  // -------------------------------------------------------------
  // Task (c): Comparison table for 3, 4, and 5 frames
  // -------------------------------------------------------------
  printf(1, "\n========================================================================\n");
  printf(1, "  Task (c): Comparison Table for 3, 4, and 5 Frames                     \n");
  printf(1, "========================================================================\n");

  printf(1, "+--------+-----------+------------+-------------+-----------+-----------+-------------+\n");
  printf(1, "| Frames | Algorithm | Total Refs | Page Faults | Page Hits | Hit Ratio | Fault Ratio |\n");
  printf(1, "+--------+-----------+------------+-------------+-----------+-----------+-------------+\n");

  int frame_counts[3] = {3, 4, 5};
  int f;
  for(f = 0; f < 3; f++){
    int nf = frame_counts[f];

    // FIFO
    int f_faults = run_fifo(refs, n_refs, nf, 0);
    int f_hits = n_refs - f_faults;
    printf(1, "|   %d    | FIFO      |     %d     |     ", nf, n_refs);
    if(f_faults < 10) printf(1, " ");
    printf(1, "%d     |    ", f_faults);
    if(f_hits < 10) printf(1, " ");
    printf(1, "%d     |  ", f_hits);
    print_pct(f_hits, n_refs);
    printf(1, "  |   ");
    print_pct(f_faults, n_refs);
    printf(1, "   |\n");

    // LRU
    int l_faults = run_lru(refs, n_refs, nf, 0);
    int l_hits = n_refs - l_faults;
    printf(1, "|   %d    | LRU       |     %d     |     ", nf, n_refs);
    if(l_faults < 10) printf(1, " ");
    printf(1, "%d     |    ", l_faults);
    if(l_hits < 10) printf(1, " ");
    printf(1, "%d     |  ", l_hits);
    print_pct(l_hits, n_refs);
    printf(1, "  |   ");
    print_pct(l_faults, n_refs);
    printf(1, "   |\n");

    // OPTIMAL
    int o_faults = run_optimal(refs, n_refs, nf, 0);
    int o_hits = n_refs - o_faults;
    printf(1, "|   %d    | OPTIMAL   |     %d     |     ", nf, n_refs);
    if(o_faults < 10) printf(1, " ");
    printf(1, "%d     |    ", o_faults);
    if(o_hits < 10) printf(1, " ");
    printf(1, "%d     |  ", o_hits);
    print_pct(o_hits, n_refs);
    printf(1, "  |   ");
    print_pct(o_faults, n_refs);
    printf(1, "   |\n");

    printf(1, "+--------+-----------+------------+-------------+-----------+-----------+-------------+\n");
  }

  // Explanation of algorithm performance
  printf(1, "\n========================================================================\n");
  printf(1, "  Algorithm Performance Explanation & Analysis                          \n");
  printf(1, "========================================================================\n");
  printf(1, "1. Which algorithm performs best?\n");
  printf(1, "   The OPTIMAL (OPT) algorithm performs BEST across all frame sizes:\n");
  printf(1, "   - 3 Frames: OPT achieves lowest faults (9) vs LRU (12) and FIFO (15).\n");
  printf(1, "   - 4 Frames: OPT achieves 8 faults (tied with LRU) vs FIFO (10).\n");
  printf(1, "   - 5 Frames: OPT achieves 7 faults (tied with LRU) vs FIFO (9).\n\n");
  printf(1, "2. Why does OPTIMAL perform best?\n");
  printf(1, "   The Optimal algorithm has omniscient future knowledge and always replaces\n");
  printf(1, "   the page that will not be needed for the longest period of time.\n");
  printf(1, "   This mathematically guarantees the minimum possible page fault rate for\n");
  printf(1, "   any given frame capacity (Belady's Optimal Algorithm).\n\n");
  printf(1, "3. LRU vs FIFO Comparison:\n");
  printf(1, "   LRU closely approximates Optimal by using the past as a predictor of the\n");
  printf(1, "   future (temporal locality). FIFO performs worst because it evicts pages\n");
  printf(1, "   based purely on arrival order, frequently discarding hot/active pages.\n");

  printf(1, "\n========================================================================\n");
  printf(1, "                 PAGEREPLACE COMPLETED SUCCESSFULLY                     \n");
  printf(1, "========================================================================\n");

  exit();
}
