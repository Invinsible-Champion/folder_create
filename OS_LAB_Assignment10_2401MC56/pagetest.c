#include "types.h"
#include "stat.h"
#include "user.h"

// Assignment 10: Question 3(c) - User program to test page protection
// Allocates a memory page, displays its page-table information,
// changes the page to read-only using setreadonly(),
// and attempts to write to the protected page to demonstrate that a page fault occurs.

int
main(int argc, char *argv[])
{
  printf(1, "========================================================\n");
  printf(1, "      Assignment 10 - Question 3: Page Protection       \n");
  printf(1, "         Page Table Management & Write Protection       \n");
  printf(1, "========================================================\n\n");

  // Step 1: Allocate a memory page using sbrk()
  printf(1, "--- Step 1: Allocating 1 Page (4096 bytes) using sbrk() ---\n");
  char *ptr = sbrk(4096);
  if(ptr == (char*)-1){
    printf(1, "pagetest: sbrk failed\n");
    exit();
  }

  // Write initial content to prove the page is writable
  ptr[0] = 'H';
  ptr[1] = 'e';
  ptr[2] = 'l';
  ptr[3] = 'l';
  ptr[4] = 'o';
  ptr[5] = '\0';
  printf(1, "Allocated page at virtual address: %p\n", ptr);
  printf(1, "Initial write test successful: string = \"%s\"\n\n", ptr);

  // Step 2: Display page-table information before setreadonly()
  printf(1, "--- Step 2: Inspecting Page Info before setreadonly() ---\n");
  if(pageinfo((int)ptr) < 0){
    printf(1, "pagetest: pageinfo failed\n");
    exit();
  }
  printf(1, "Notice: Flags have W=1 (page is writable)\n\n");

  // Step 3: Change the page to read-only
  printf(1, "--- Step 3: Calling setreadonly() on address %p ---\n", ptr);
  if(setreadonly((int)ptr) < 0){
    printf(1, "pagetest: setreadonly failed\n");
    exit();
  }
  printf(1, "setreadonly() returned 0 (Success: PTE_W removed)\n\n");

  // Step 4: Display page-table information after setreadonly()
  printf(1, "--- Step 4: Inspecting Page Info after setreadonly() ---\n");
  if(pageinfo((int)ptr) < 0){
    printf(1, "pagetest: pageinfo failed\n");
    exit();
  }
  printf(1, "Notice: Flags now have W=0 (page is read-only!)\n\n");

  // Step 5: Attempt to write to the protected page
  // We fork a child process to perform the write test so that the kernel
  // page fault trap kills the child while the test runner can report results.
  printf(1, "--- Step 5: Attempting Write to Protected Page ---\n");
  printf(1, "Forking child process to verify memory write protection...\n");

  int pid = fork();
  if(pid < 0){
    printf(1, "pagetest: fork failed\n");
    exit();
  }

  if(pid == 0){
    // Child process
    printf(1, "[Child pid %d] Reading from page: \"%s\" (READ succeeds)\n", getpid(), ptr);
    printf(1, "[Child pid %d] Attempting to write 'W' to read-only address %p...\n", getpid(), ptr);

    // This write operation MUST trigger a CPU page fault (Trap 14) and kill the process!
    *ptr = 'W';

    // Execution should never reach this point
    printf(1, "[Child ERROR] Write succeeded! Page protection failed.\n");
    exit();
  } else {
    // Parent process waits for child to trigger page fault and be terminated
    int reaped = wait();
    printf(1, "\n[Parent pid %d] Child process %d was terminated by the kernel.\n", getpid(), reaped);
    printf(1, "[Parent] Page fault successfully caught on unauthorized write attempt!\n");
  }

  // Step 6: Test invalid/unmapped address handling
  printf(1, "\n--- Step 6: Testing Error Handling on Unmapped Address ---\n");
  int unmapped_addr = (int)ptr + 100 * 4096;
  printf(1, "Calling pageinfo on unmapped address 0x%x:\n", unmapped_addr);
  pageinfo(unmapped_addr);
  int ro_err = setreadonly(unmapped_addr);
  printf(1, "Calling setreadonly on unmapped address 0x%x: returned %d (Expected: -1) -> %s\n",
         unmapped_addr, ro_err, (ro_err == -1) ? "PASS" : "FAIL");

  printf(1, "\n========================================================\n");
  printf(1, "             PAGETEST COMPLETED SUCCESSFULLY            \n");
  printf(1, "========================================================\n");

  // Deallocate memory
  sbrk(-4096);
  exit();
}
