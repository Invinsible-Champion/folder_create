# MA3104 Operating Systems Lab - Assignment 9
Roll Number: 2401MC56

## Overview

This assignment focuses on virtual memory management, lazy allocation, and page replacement in the xv6 operating system (x86). It is divided into three parts:

1. Question 1: Implementation of a system call `vmprint(void)` that inspects and displays the two-level page table of the calling process, ignoring the kernel address space.
2. Question 2: Implementation of lazy heap memory allocation by deferring page allocation from `sbrk()` to the page fault trap handler (`T_PGFLT`), including page fault counting via the `getfaults()` system call.
3. Question 3: Implementation of FIFO and LRU page replacement algorithms in a user program (`pagerepl.c`) to simulate page references, record hits and faults, and compare performance across various frame capacities.

---

## Directory Structure

- `defs.h`: Kernel function declarations, including `vmprint(void)`.
- `exec.c`: Resets per-process page fault counter upon loading a new binary.
- `faulttest.c`: Test program for Question 2 (allocates 32 pages lazily, writes to 5 pages, verifies `getfaults() == 5`, tests child crash on out-of-bounds access).
- `Makefile`: Build configuration updated to include `_pttest`, `_faulttest`, and `_pagerepl` in `UPROGS` and `EXTRA`.
- `pagerepl.c`: User program for Question 3 implementing FIFO and LRU algorithms, supporting both automated evaluation and custom CLI inputs.
- `proc.c`: Updates process allocation to initialize `pgfault_count = 0`.
- `proc.h`: Extends `struct proc` with `pgfault_count`.
- `pttest.c`: Test program for Question 1 (calls `vmprint()` at start-up and after `sbrk(4 * 4096)`).
- `q1_pttest.txt`: Terminal text output log of `pttest`.
- `q2_faulttest.txt`: Terminal text output log of `faulttest`.
- `q3_pagerepl.txt`: Terminal text output log of `pagerepl`.
- `screenshots/`: Terminal output screenshots demonstrating verified execution for all questions.
  - `q1_result.png`: Execution of `pttest` showing the page table before and after heap expansion.
  - `q2_results.png`: Execution of `faulttest` showing lazy allocation, 5 page faults, and child termination.
  - `q3_part1.png`: Execution of `pagerepl` showing FIFO step-by-step trace on 3 frames.
  - `q3_part2.png`: Execution of `pagerepl` showing LRU step-by-step trace on 3 frames.
  - `q3_part3.png`: Execution of `pagerepl` showing comparative summary table for 3, 4, and 5 frames.
- `syscall.c`: Syscall table and handler dispatching for `sys_vmprint` and `sys_getfaults`.
- `syscall.h`: Syscall numbers for `SYS_vmprint` (34) and `SYS_getfaults` (35).
- `sysproc.c`: Implementations of `sys_sbrk` (lazy allocation), `sys_vmprint`, and `sys_getfaults`.
- `trap.c`: Page fault trap handler (`T_PGFLT`) allocating physical memory on demand.
- `user.h`: User-space prototypes for `vmprint()` and `getfaults()`.
- `usys.S`: User-space syscall stubs for `vmprint` and `getfaults`.
- `vm.c`: Implementation of `vmprint(void)`, and safe handling in `deallocuvm()` and `copyuvm()` to skip unallocated pages.

---

## Compilation and Running Instructions

### 1. Build the xv6 Kernel and Disk Image
From the project root directory, run:
```bash
make clean
make
```

### 2. Launch xv6 in QEMU
To run in terminal mode without a graphical window:
```bash
make qemu-nox
```
(Alternatively, run `make qemu` for graphical mode).

### 3. Running the Test Programs Inside xv6

- To test Question 1 (Page Table Dump):
  ```text
  $ pttest
  ```

- To test Question 2 (Lazy Allocation and Page Faults):
  ```text
  $ faulttest
  ```

- To test Question 3 (Page Replacement Simulation):
  - Run standard assignment benchmark (Task b with 3 frames and Task c with 3, 4, 5 frames):
    ```text
    $ pagerepl
    ```
  - Run custom frame count and reference string:
    ```text
    $ pagerepl 3 7 0 1 2 0 3
    ```

### 4. Exiting QEMU
Press `Ctrl-A` followed by `X`.

---

## Detailed Implementation Summary

### Question 1: Printing the Page Table of a Process
- A new system call `int vmprint(void)` was implemented in `vm.c`.
- The function walks the current process's page directory `myproc()->pgdir` from entry `0` up to `PDX(KERNBASE)` (512), completely ignoring the kernel portion above `0x80000000`.
- For each present page directory entry (`pgdir[i] & PTE_P`), the page table count is incremented.
- For each of the 1024 page table entries in that table, `walkpgdir()` inspects whether `PTE_P` is set.
- If present, it prints the virtual address (`va`), physical frame address (`pa = PTE_ADDR(*pte)`), and the hardware permission flags `P`, `W`, and `U`.
- At the end of the walk, it outputs the total number of valid user pages and the number of page-table pages used.
- In `pttest.c`, calling `vmprint()` at start-up shows 3 user pages (code/data at `0x0`, stack guard page at `0x1000` with `U=0`, and stack at `0x2000`). After `sbrk(4 * 4096)`, four new consecutive pages (`0x3000`, `0x4000`, `0x5000`, `0x6000`) appear, bringing the total valid user pages to 7 while page-table pages remains 1.

### Question 2: Handling Page Faults (Lazy Allocation)
- In `sysproc.c`, `sys_sbrk()` was altered so that when $n > 0$, it only increments `myproc()->sz` without calling `growproc()` or mapping physical memory immediately.
- In `trap.c`, a handler for `T_PGFLT` was implemented:
  - The faulting address is read with `rcr2()`.
  - If `va < myproc()->sz` and the page is not yet mapped, the kernel allocates a 4 KB frame using `kalloc()`, zeroes it with `memset()`, maps it via `mappages()` with `PTE_W | PTE_U`, increments `myproc()->pgfault_count`, and resumes the process.
  - If `va >= myproc()->sz`, the kernel prints a diagnostic message (`pid %d: page fault on address 0x%x--kill proc`) and marks the process as killed (`curproc->killed = 1`).
- In `vm.c`, `deallocuvm()` and `copyuvm()` were modified to skip non-present PTEs instead of invoking `panic()`.
- A system call `int getfaults(void)` was implemented to return `myproc()->pgfault_count`.
- In `faulttest.c`, `sbrk(32 * 4096)` expands the heap by 32 pages lazily (`getfaults()` remains 0). Touching 5 pages raises exactly 5 page faults (`getfaults() == 5`). A child process trying to write beyond `sz` is safely terminated by the kernel without affecting the parent process.

### Question 3: Page Replacement Algorithms (FIFO and LRU)
- A standalone user program `pagerepl.c` was written in C.
- **FIFO**: Uses a fixed-size frame array with a circular replacement index (`fifo_victim`). On a fault with full frames, the oldest inserted frame is evicted and replaced.
- **LRU**: Maintains an array of access timestamps (`last_used`). Every page hit or insertion updates the timestamp of that frame. On a fault with full frames, the frame with the lowest timestamp (least recently referenced) is evicted.
- **Step-by-step trace**: Displays the reference, current frame array, and whether each reference was a `HIT` or a `FAULT`.
- **Benchmark Results on `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1` (20 references)**:
  - 3 Frames: FIFO = 15 faults (25.0% hit ratio), LRU = 12 faults (40.0% hit ratio).
  - 4 Frames: FIFO = 10 faults (50.0% hit ratio), LRU = 8 faults (60.0% hit ratio).
  - 5 Frames: FIFO = 9 faults (55.0% hit ratio), LRU = 7 faults (65.0% hit ratio).
- **Observations**:
  1. LRU outperforms FIFO for all tested frame counts because it exploits temporal locality.
  2. Increasing frame capacity monotonically reduces the fault count for both algorithms.
