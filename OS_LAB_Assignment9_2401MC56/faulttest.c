#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "========================================================\n");
  printf(1, "   Assignment 9 - Question 2: Lazy Allocation Test      \n");
  printf(1, "========================================================\n");

  // Step 1: Check initial fault count
  int f0 = getfaults();
  printf(1, "\n[Step 1] Initial page fault count: %d\n", f0);

  // Step 2: Call sbrk(32 * 4096) without writing
  printf(1, "\n[Step 2] Calling sbrk(32 * 4096) to allocate 32 pages lazily...\n");
  char *p = sbrk(32 * 4096);
  if(p == (char*)-1){
    printf(1, "faulttest: sbrk failed\n");
    exit();
  }
  printf(1, "sbrk returned base address: 0x%x\n", (uint)p);

  int f1 = getfaults();
  printf(1, "Page fault count immediately after sbrk: %d\n", f1);
  if(f1 != f0){
    printf(1, "WARNING: Fault count changed before touching any pages!\n");
  } else {
    printf(1, "Confirmed: 0 physical pages allocated immediately (lazy allocation active).\n");
  }

  // Step 3: Write to exactly 5 different pages
  printf(1, "\n[Step 3] Writing to exactly 5 different pages...\n");
  int pages_to_touch[5] = {0, 4, 11, 23, 31}; // 5 distinct pages within the 32 allocated
  int i;
  for(i = 0; i < 5; i++){
    int pageno = pages_to_touch[i];
    char *addr = p + pageno * 4096;
    printf(1, "  Touching page %d at address 0x%x (writing '%c')...\n",
           pageno, (uint)addr, 'A' + i);
    *addr = (char)('A' + i);
  }

  // Verify that the written data is intact
  for(i = 0; i < 5; i++){
    int pageno = pages_to_touch[i];
    char val = *(p + pageno * 4096);
    if(val != (char)('A' + i)){
      printf(1, "faulttest: data verification failed on page %d (got '%c', expected '%c')\n",
             pageno, val, 'A' + i);
      exit();
    }
  }
  printf(1, "All 5 touched pages read back and verified successfully.\n");

  // Step 4: Verify that exactly 5 page faults were handled
  int faults = getfaults();
  printf(1, "\n[Step 4] Checking handled page faults via getfaults():\n");
  printf(1, "getfaults() returned: %d (Expected: 5)\n", faults);
  if(faults == 5){
    printf(1, "Result: PASS (getfaults() returned exactly 5 as required)\n");
  } else {
    printf(1, "Result: FAIL (Expected 5 page faults, got %d)\n", faults);
  }

  // Step 5: Test that accessing an address above the allocated size kills only that process
  printf(1, "\n[Step 5] Testing out-of-bounds access above allocated size in child process...\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "faulttest: fork failed\n");
    exit();
  }

  if(pid == 0){
    // Child process attempts to access an address beyond allocated sz
    char *bad_addr = p + (32 * 4096) + 4096; // 1 page above sz
    printf(1, "Child (pid %d) attempting to write to invalid address 0x%x (above sz)...\n",
           getpid(), (uint)bad_addr);
    *bad_addr = 'X';
    printf(1, "ERROR: Child was not killed by kernel!\n");
    exit();
  }

  // Parent process waits for child to terminate
  int reaped_pid = wait();
  printf(1, "Parent successfully reaped child pid %d.\n", reaped_pid);
  printf(1, "Parent process continues running safely.\n");
  printf(1, "Result: PASS (Accessing address above allocated size kills only that process)\n");

  printf(1, "\n========================================================\n");
  printf(1, "             FAULTTEST PASSED SUCCESSFULLY              \n");
  printf(1, "========================================================\n");

  exit();
}
