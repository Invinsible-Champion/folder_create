// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  int free_pages;
  int total_pages;
  // Assignment 10: Question 1(a) - Counters for allocated and freed pages
  int total_allocated;
  int total_freed;
} kmem;

// Assignment 10: Flag to distinguish allocator initialization (freerange) from runtime deallocations
static int in_freerange = 0;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  kmem.free_pages = 0;
  kmem.total_pages = 0;
  // Assignment 10: Question 1(a) - Initialize allocation and freed counters
  kmem.total_allocated = 0;
  kmem.total_freed = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  in_freerange = 1; // Mark that we are initializing free list
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE){
    kmem.total_pages++;
    kfree(p);
  }
  in_freerange = 0;
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.free_pages++;
  // Assignment 10: Question 1(a) - Track total freed pages (skip initial freerange population)
  if(!in_freerange)
    kmem.total_freed++;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.free_pages--;
    // Assignment 10: Question 1(a) - Track total allocated pages
    kmem.total_allocated++;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

// Get the current number of free physical pages
int
get_free_pages(void)
{
  int n;
  if(kmem.use_lock)
    acquire(&kmem.lock);
  n = kmem.free_pages;
  if(kmem.use_lock)
    release(&kmem.lock);
  return n;
}

// Get the total number of physical pages managed
int
get_total_pages(void)
{
  return kmem.total_pages;
}

// Assignment 10: Question 1(b) - System call int memstats(void)
// Displays total allocated pages, total freed pages, currently allocated pages,
// and available physical memory pages.
int
memstats(void)
{
  int tot_alloc, tot_freed, cur_alloc, avail;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  tot_alloc = kmem.total_allocated;
  tot_freed = kmem.total_freed;
  cur_alloc = kmem.total_allocated - kmem.total_freed;
  avail = kmem.free_pages;
  if(kmem.use_lock)
    release(&kmem.lock);

  cprintf("Memory Statistics:\n");
  cprintf("  Total Allocated Pages   : %d\n", tot_alloc);
  cprintf("  Total Freed Pages       : %d\n", tot_freed);
  cprintf("  Currently Allocated Pages: %d\n", cur_alloc);
  cprintf("  Available Physical Pages: %d\n", avail);

  return 0;
}


