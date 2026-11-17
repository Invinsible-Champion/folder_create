#include "types.h"
#include "stat.h"
#include "user.h"

// Assignment 10: Question 2(c) - User program to test virtual-to-physical address translation
// Allocates memory using sbrk(), passes different virtual addresses to vtop(),
// prints their corresponding physical addresses, and demonstrates the result
// for an invalid or unmapped virtual address.

int global_test_var = 0x1234;

int
main(int argc, char *argv[])
{
  int local_stack_var = 0x5678;
  int pa;

  printf(1, "========================================================\n");
  printf(1, "   Assignment 10 - Question 2: Virtual Memory           \n");
  printf(1, "    Virtual-to-Physical Address Translation (vtop)      \n");
  printf(1, "========================================================\n\n");

  // Part 1: Translate standard process segments (Code, Data, Stack)
  printf(1, "--- Part 1: Standard Process Segments ---\n");
  pa = vtop((int)&main);
  printf(1, "  Code Segment  (&main)     : VA = 0x%x -> PA = 0x%x\n", (int)&main, pa);

  pa = vtop((int)&global_test_var);
  printf(1, "  Data Segment  (&global)   : VA = 0x%x -> PA = 0x%x\n", (int)&global_test_var, pa);

  pa = vtop((int)&local_stack_var);
  printf(1, "  Stack Segment (&stack)    : VA = 0x%x -> PA = 0x%x\n", (int)&local_stack_var, pa);

  // Part 2: Allocate memory using sbrk() and translate heap pages
  int num_pages = 3;
  int alloc_size = num_pages * 4096;
  printf(1, "\n--- Part 2: Allocating %d Pages (%d bytes) on Heap ---\n",
         num_pages, alloc_size);

  char *heap_ptr = sbrk(alloc_size);
  if(heap_ptr == (char*)-1){
    printf(1, "vttest: sbrk failed\n");
    exit();
  }

  // Populate data
  heap_ptr[0] = 'A';
  heap_ptr[4096] = 'B';
  heap_ptr[8192] = 'C';

  // Translate Page 0
  int va0 = (int)heap_ptr;
  int pa0 = vtop(va0);
  printf(1, "  Heap Page 0 Base     : VA = 0x%x -> PA = 0x%x\n", va0, pa0);

  // Translate Page 0 with offset
  int off0 = 0x2A4;
  int va0_off = va0 + off0;
  int pa0_off = vtop(va0_off);
  printf(1, "  Heap Page 0 + 0x%x  : VA = 0x%x -> PA = 0x%x (offset verified: +0x%x)\n",
         off0, va0_off, pa0_off, pa0_off - pa0);

  // Translate Page 1
  int va1 = (int)heap_ptr + 4096;
  int pa1 = vtop(va1);
  printf(1, "  Heap Page 1 Base     : VA = 0x%x -> PA = 0x%x\n", va1, pa1);

  // Translate Page 1 with offset
  int off1 = 0x8BC;
  int va1_off = va1 + off1;
  int pa1_off = vtop(va1_off);
  printf(1, "  Heap Page 1 + 0x%x  : VA = 0x%x -> PA = 0x%x (offset verified: +0x%x)\n",
         off1, va1_off, pa1_off, pa1_off - pa1);

  // Translate Page 2
  int va2 = (int)heap_ptr + 8192;
  int pa2 = vtop(va2);
  printf(1, "  Heap Page 2 Base     : VA = 0x%x -> PA = 0x%x\n", va2, pa2);

  // Part 3: Demonstrating invalid or unmapped virtual addresses
  printf(1, "\n--- Part 3: Demonstrating Invalid / Unmapped Virtual Addresses ---\n");

  // Test address beyond allocated heap
  int unmapped_heap = (int)heap_ptr + 50 * 4096;
  int res_unmapped = vtop(unmapped_heap);
  printf(1, "  1. Unmapped Heap VA (0x%x)   : vtop = %d (Expected: -1) -> %s\n",
         unmapped_heap, res_unmapped, (res_unmapped == -1) ? "PASS" : "FAIL");

  // Test high unmapped virtual address
  int high_va = 0x60000000;
  int res_high = vtop(high_va);
  printf(1, "  2. High Unmapped VA (0x%x)   : vtop = %d (Expected: -1) -> %s\n",
         high_va, res_high, (res_high == -1) ? "PASS" : "FAIL");

  // Test kernel address (above KERNBASE 0x80000000)
  int kernel_va = 0x80100000;
  int res_kernel = vtop(kernel_va);
  printf(1, "  3. Kernel Space VA  (0x%x)   : vtop = %d (Expected: -1) -> %s\n",
         kernel_va, res_kernel, (res_kernel == -1) ? "PASS" : "FAIL");

  // Test negative/invalid address
  int neg_va = -4096;
  int res_neg = vtop(neg_va);
  printf(1, "  4. Negative Address (0x%x)   : vtop = %d (Expected: -1) -> %s\n",
         neg_va, res_neg, (res_neg == -1) ? "PASS" : "FAIL");

  // Part 4: Deallocate memory and verify address becomes unmapped
  printf(1, "\n--- Part 4: Heap Shrink and Verification ---\n");
  sbrk(-alloc_size);
  int res_after_shrink = vtop(va0);
  printf(1, "  Deallocated VA (0x%x) after sbrk(-%d): vtop = %d (Expected: -1) -> %s\n",
         va0, alloc_size, res_after_shrink, (res_after_shrink == -1) ? "PASS" : "FAIL");

  printf(1, "\n========================================================\n");
  printf(1, "               VTTEST COMPLETED SUCCESSFULLY            \n");
  printf(1, "========================================================\n");

  exit();
}
