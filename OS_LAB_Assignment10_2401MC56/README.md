# Operating Systems — Assignment 10

**Roll Number:** 2401MC56  
**Assignment:** OS Assignment 10 — Memory Management, Virtual Memory, Paging & Page Replacement  

---

## 1. Directory Structure

```
2401MC56_Assignment10/
├── README.md               # Complete overview, compilation, and execution guide
├── Makefile                # Updated xv6 Makefile with all 4 user test programs
├── defs.h                  # Function prototypes for new kernel functions
├── kalloc.c                # Physical memory allocator with allocation/deallocation tracking
├── memtest.c               # User test program for Question 1
├── pagereplace.c           # User test program for Question 4 (FIFO, LRU, Optimal)
├── pagetest.c              # User test program for Question 3 (Page Protection)
├── q1_memtest.txt          # Terminal output log for Question 1
├── q2_vttest.txt           # Terminal output log for Question 2
├── q3_pagetest.txt         # Terminal output log for Question 3
├── q4_pagereplace.txt      # Terminal output log for Question 4
├── screenshots/            # Directory containing all output screenshots
│   ├── q1_memtest.png                     # Q1: Physical Memory Tracking output
│   ├── q2_vttest.png                      # Q2: Virtual-to-Physical Translation output
│   ├── q3_pagetest.png                    # Q3: Page Table Management & Write Protection output
│   ├── q4_pagereplace_fifo.png            # Q4: FIFO simulation (3 frames)
│   ├── q4_pagereplace_lru.png             # Q4: LRU simulation (3 frames)
│   ├── q4_pagereplace_opt_summary.png     # Q4: Optimal simulation & 3-frame summary
│   └── q4_pagereplace_table_analysis.png  # Q4: 3, 4, 5 frames comparison table & analysis
├── syscall.c               # Syscall table entries and dispatch wrappers
├── syscall.h               # Syscall numbers (SYS_memstats, SYS_vtop, SYS_pageinfo, SYS_setreadonly)
├── sysproc.c               # Syscall implementations in sysproc
├── trap.c                  # Updated trap handler for read-only page fault detection
├── user.h                  # User-space syscall prototypes
├── usys.S                  # Assembly stubs for syscalls
├── vm.c                    # Virtual memory functions (vtop, pageinfo, setreadonly)
└── vttest.c                # User test program for Question 2
```

---

## 2. Compilation and Execution Instructions

### A. Building xv6
To compile the kernel and build the filesystem image containing all test programs:
```bash
make clean
make
```

### B. Running xv6 in QEMU
To run xv6 in terminal mode:
```bash
make qemu-nox
```
*(Or run `make qemu` for graphical window mode).*

### C. Running Assignment Tests in xv6
Inside the xv6 `$ ` shell prompt, execute the following commands:

1. **Question 1 (Physical Memory Tracking):**
   ```bash
   memtest
   ```

2. **Question 2 (Virtual-to-Physical Translation):**
   ```bash
   vttest
   ```

3. **Question 3 (Page Table Inspection & Write Protection):**
   ```bash
   pagetest
   ```

4. **Question 4 (Page Replacement Algorithms: FIFO, LRU, Optimal):**
   ```bash
   pagereplace
   ```

### D. Exiting QEMU
- In terminal mode (`qemu-nox`): Press `Ctrl-A` then release and press `x`.
- In graphical mode (`qemu`): Close the QEMU window.

---

## 3. Implementation Summary by Question

### Question 1: Memory Management (Tracking Physical Memory Allocation)
* **Kernel Changes:**
  * `kalloc.c`: Added `total_allocated` and `total_freed` counters to `struct kmem`. Incremented `total_allocated` in `kalloc()` and `total_freed` in `kfree()` (using an `in_freerange` flag to prevent counting initial boot-time memory populating).
  * Implemented system call `int memstats(void)` which prints:
    1. Total Allocated Pages
    2. Total Freed Pages
    3. Currently Allocated Pages (`total_allocated - total_freed`)
    4. Available Physical Pages (`kmem.free_pages`)
* **Test Program (`memtest.c`):**
  * Displays baseline statistics via `memstats()`.
  * Allocates 10 pages using `sbrk(40960)` and writes to them.
  * Displays updated statistics, verifying `total_allocated` increased by 10 and `available` dropped by 10.
  * Releases memory using `sbrk(-40960)`.
  * Verifies `total_freed` increased by 10 and `available` physical pages returned to initial baseline.

---

### Question 2: Virtual Memory (Virtual-to-Physical Address Translation)
* **Kernel Changes:**
  * `vm.c`: Implemented `int vtop(uint va)`:
    * Validates that `va < KERNBASE` (belongs to user address space).
    * Uses `walkpgdir()` to locate the corresponding page-table entry (PTE).
    * Checks whether `pte` is valid and the Present bit (`PTE_P`) is set; returns `-1` if unmapped or invalid.
    * Combines the physical frame base address (`PTE_ADDR(*pte)`) with the lower 12-bit page offset (`va & 0xFFF`) using bitwise OR and returns the complete physical address.
* **Test Program (`vttest.c`):**
  * Translates Code (`&main`), Data (`&global_test_var`), and Stack (`&local_stack_var`) segments.
  * Allocates 3 heap pages with `sbrk()`, verifies translation for base addresses and addresses with offsets (`+0x2A4`, `+0x8BC`).
  * Demonstrates that invalid/unmapped addresses (unmapped heap, high VA `0x60000000`, kernel VA `0x80100000`, negative address, and deallocated heap) correctly return `-1`.

---

### Question 3: Paging (Page Table Management and Page Protection)
* **Kernel Changes:**
  * `vm.c`: Implemented `int pageinfo(uint va)`:
    * Checks if `va` is mapped.
    * Displays Physical Frame Number (`PFN = PTE_ADDR >> 12`), Physical Base Address, and Page Table Flags: Present (`P`), Writable (`W`), and User (`U`).
  * `vm.c`: Implemented `int setreadonly(uint va)`:
    * Clears write permission: `*pte &= ~PTE_W`.
    * Reloads `cr3` register (`lcr3`) to invalidate the CPU TLB cache so permission changes take effect immediately in hardware.
  * `trap.c`: Updated `case T_PGFLT` to detect write attempts on read-only pages (`pte && (*pte & PTE_P) && !(*pte & PTE_W) && (tf->err & 2)`), reporting `[PAGE FAULT] Attempted write to read-only address` and terminating the offending process.
* **Test Program (`pagetest.c`):**
  * Allocates 1 page using `sbrk(4096)` and writes initial content.
  * Inspects flags before protection via `pageinfo()` (`W=1`).
  * Calls `setreadonly()`.
  * Inspects flags after protection via `pageinfo()` (`W=0`).
  * Forks a child process: child successfully reads data, but when it attempts to write, the CPU triggers Trap 14 (Page Fault).
  * Kernel terminates child, and parent verifies page protection succeeded.
  * Tests error handling for unmapped addresses returning `-1`.

---

### Question 4: Page Replacement Algorithms (FIFO, LRU, Optimal)
* **Program (`pagereplace.c`):**
  * Implemented all three algorithms:
    * **FIFO:** Circular pointer evicting the oldest loaded page.
    * **LRU:** Timestamps tracking last access, evicting page unused for longest time.
    * **Optimal:** Looks forward into reference string, evicting page whose next use is farthest in the future (or never).
  * Tested on reference string (20 references):
    `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1`
  * Displays step-by-step frame contents and HIT/FAULT status for each reference.
  * Generates comparison table for 3, 4, and 5 frames:

| Frames | Algorithm | Total References | Page Faults | Page Hits | Hit Ratio | Fault Ratio |
| :---: | :--- | :---: | :---: | :---: | :---: | :---: |
| **3** | **FIFO** | 20 | 15 | 5 | 25.0% | 75.0% |
| **3** | **LRU** | 20 | 12 | 8 | 40.0% | 60.0% |
| **3** | **OPTIMAL** | 20 | 9 | 11 | 55.0% | 45.0% |
| **4** | **FIFO** | 20 | 10 | 10 | 50.0% | 50.0% |
| **4** | **LRU** | 20 | 8 | 12 | 60.0% | 40.0% |
| **4** | **OPTIMAL** | 20 | 8 | 12 | 60.0% | 40.0% |
| **5** | **FIFO** | 20 | 9 | 11 | 55.0% | 45.0% |
| **5** | **LRU** | 20 | 7 | 13 | 65.0% | 35.0% |
| **5** | **OPTIMAL** | 20 | 7 | 13 | 65.0% | 35.0% |

* **Analysis:**
  * **Optimal** performs best across all frame sizes (lowest faults: 9 on 3 frames, 8 on 4 frames, 7 on 5 frames) because it has future knowledge of page requests and evicts the page not needed for the longest time.
  * **LRU** closely approximates Optimal by exploiting temporal locality (12 faults on 3 frames).
  * **FIFO** performs worst (15 faults on 3 frames) because it ignores access patterns and evicts pages purely by arrival order.
