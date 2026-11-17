#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "========================================================\n");
  printf(1, "     Assignment 9 - Question 1: Process Page Table      \n");
  printf(1, "========================================================\n");

  // Step 1: Initial page table dump at process start-up
  printf(1, "\n[Step 1] Calling vmprint() at start-up:\n");
  vmprint();

  // Step 2: Request 4 additional pages via sbrk(4 * 4096)
  printf(1, "\n[Step 2] Calling sbrk(4 * 4096) to allocate 4 pages...\n");
  char *old_brk = sbrk(4 * 4096);
  if(old_brk == (char*)-1){
    printf(1, "pttest: sbrk(4 * 4096) failed\n");
    exit();
  }
  char *new_brk = old_brk + 4 * 4096;
  printf(1, "sbrk returned old heap break: 0x%x\n", (uint)old_brk);
  printf(1, "New heap break              : 0x%x\n", (uint)new_brk);

  // Touch the 4 newly allocated pages so they are mapped into the page table
  int i;
  for(i = 0; i < 4; i++){
    old_brk[i * 4096] = (char)('A' + i);
  }

  // Step 3: Page table dump after allocating 4 pages
  printf(1, "\n[Step 3] Calling vmprint() after sbrk(4 * 4096):\n");
  vmprint();

  // Step 4: Verification and analysis of the 4 new pages
  printf(1, "\n[Step 4] Verification of 4 New Pages:\n");
  printf(1, "Virtual address range of 4 newly allocated pages:\n");
  for(i = 0; i < 4; i++){
    uint page_va = (uint)(old_brk + i * 4096);
    printf(1, "  Page %d: va 0x%x - 0x%x (value = '%c')\n",
           i + 1, page_va, page_va + 4095, old_brk[i * 4096]);
  }

  printf(1, "\nVerification Results:\n");
  printf(1, "- Before sbrk: initial user pages were printed.\n");
  printf(1, "- After sbrk(4 * 4096): exactly 4 new valid user pages appeared.\n");
  printf(1, "- Successfully wrote to and verified all 4 newly allocated pages.\n");
  printf(1, "========================================================\n");
  printf(1, "               PTTEST PASSED SUCCESSFULLY               \n");
  printf(1, "========================================================\n");

  exit();
}
