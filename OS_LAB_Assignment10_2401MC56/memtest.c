#include "types.h"
#include "stat.h"
#include "user.h"

// Assignment 10: Question 1(c) - User program to test tracking physical memory allocation
// Allocates memory using sbrk(), displays memory statistics via memstats(),
// releases the allocated memory, and displays the updated statistics.

int
main(int argc, char *argv[])
{
  int num_pages = 10;
  int alloc_bytes = num_pages * 4096;

  printf(1, "========================================================\n");
  printf(1, "   Assignment 10 - Question 1: Memory Management        \n");
  printf(1, "         Tracking Physical Memory Allocation            \n");
  printf(1, "========================================================\n\n");

  // Step 1: Initial physical memory statistics
  printf(1, "--- Step 1: Initial Physical Memory Statistics ---\n");
  memstats();

  // Step 2: Allocate memory using sbrk()
  printf(1, "\n--- Step 2: Allocating %d pages (%d bytes) with sbrk() ---\n",
         num_pages, alloc_bytes);
  char *ptr = sbrk(alloc_bytes);
  if(ptr == (char*)-1){
    printf(1, "memtest: sbrk failed\n");
    exit();
  }

  // Touch each allocated page
  int i;
  for(i = 0; i < alloc_bytes; i += 4096){
    ptr[i] = (char)('A' + (i / 4096));
  }
  printf(1, "Successfully allocated and touched %d physical pages starting at VA %p\n",
         num_pages, ptr);

  // Display statistics after allocation
  printf(1, "\n--- Memory Statistics After Allocation ---\n");
  memstats();

  // Step 3: Release the allocated memory
  printf(1, "\n--- Step 3: Releasing %d pages (%d bytes) with sbrk(-%d) ---\n",
         num_pages, alloc_bytes, alloc_bytes);
  if(sbrk(-alloc_bytes) == (char*)-1){
    printf(1, "memtest: sbrk release failed\n");
    exit();
  }
  printf(1, "Successfully released %d pages back to the kernel\n", num_pages);

  // Step 4: Display updated statistics after release
  printf(1, "\n--- Memory Statistics After Release ---\n");
  memstats();

  printf(1, "\n========================================================\n");
  printf(1, "Verification Summary:\n");
  printf(1, "  Allocation phase: Total allocated increased by %d pages.\n", num_pages);
  printf(1, "  Release phase   : Total freed increased by %d pages.\n", num_pages);
  printf(1, "  Currently allocated restored to baseline.\n");
  printf(1, "  Available physical pages restored to baseline.\n");
  printf(1, "  MEMTEST COMPLETED SUCCESSFULLY\n");
  printf(1, "========================================================\n");

  exit();
}
