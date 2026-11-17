# OS Lab Assignment 7 Submission: xv6 Threads, Semaphores, Deadlocks & Priority Scheduling

**Student Roll No:** 2401MC56  
**Assignment:** OS Lab Assignment 7 (xv6 Kernel Modification & User Space Programs)  
**Environment:** xv6 (x86 32-bit multiprocessor) running on QEMU  

---

## 1. Directory Structure

```text
2401MC56_Assignment7/
├── OS_Lab_Assignment_7_Report.pdf  # Final consolidated report in PDF format (with screenshots & analysis)
├── 2401MC56_Report.pdf             # Copy of PDF report named with Roll Number
├── OS_Lab_Assignment_7_Report.md   # Markdown source of the technical report
├── README.md                       # This file (submission overview & instructions)
│
├── [Modified Kernel Files]
│   ├── proc.c                      # clone, join, wakeup_one, scheduler, aging, priority syscalls
│   ├── proc.h                      # struct proc modified with is_thread, priority, waitticks
│   ├── sysproc.c                   # sem_init, sem_wait, sem_signal, priority syscall implementations
│   ├── syscall.c                   # System call dispatch table entries
│   ├── syscall.h                   # System call vector numbers
│   ├── defs.h                      # Kernel function prototypes
│   ├── trap.c                      # Timer interrupt hook for update_aging()
│   ├── user.h                      # User-space prototypes for new system calls
│   ├── usys.S                      # Assembly stubs for new system calls
│   ├── param.h                     # FSSIZE increased to 2000 for additional binaries
│   └── Makefile                    # Build rules for new user programs
│
├── [User Test Programs]
│   ├── threadtest.c                # Question 1: 3 cloned threads with race condition demonstration
│   ├── synctest.c                  # Question 2: Synchronized counter and Producer-Consumer with semaphores
│   ├── deadlock.c                  # Question 3: Deadlock detection & Banker's algorithm + IPC pipe deadlock
│   └── priotest.c                  # Question 4: 4 CPU-bound processes testing priority scheduling & aging
│
├── [Screenshots]
│   └── screenshots/
│       ├── q1_threadtest.png       # Q1 Terminal run showing lost updates
│       ├── q2_synctest.png         # Q2 Terminal run showing Producer-Consumer without underflow/overflow
│       ├── q3_deadlock_part1.png   # Q3 Safe state discovery & resource request evaluation
│       ├── q3_deadlock_part2.png   # Q3 Deadlock cycle detection & real xv6 IPC pipe circular wait
│       ├── q4_priotest_part1.png   # Q4 Priority scheduling execution of CPU-bound processes
│       └── q4_priotest_part2.png   # Q4 Aging progression and final completion summary table
│
└── [Execution Logs]
    ├── q1_threadtest.txt           # Verbatim terminal output for Question 1
    ├── q2_synctest.txt             # Verbatim terminal output for Question 2
    ├── q3_deadlock.txt             # Verbatim terminal output for Question 3
    └── q4_priotest.txt             # Verbatim terminal output for Question 4
```

---

## 2. Compilation & How to Run

From the main `xv6-public` root directory:

```bash
# 1. Clean and build kernel & file system image
make clean
make fs.img

# 2. Launch xv6 in QEMU terminal
make qemu-nox
```

Inside the xv6 shell (`$` prompt):

```bash
# Question 1: Test clone(), join(), and demonstrate race condition
$ threadtest

# Question 2: Test semaphores (synchronized counter & producer-consumer)
$ synctest

# Question 3: Test Banker's algorithm, deadlock detection, and IPC pipe deadlock
$ deadlock

# Question 4: Test priority scheduling with dynamic aging
$ priotest

# To exit QEMU:
# Press Ctrl-A then X
```

---

## 3. Question Summaries & Key Outcomes

### Question 1 — Kernel-Supported Threads (`clone` and `join`)
* **Implemented:** `clone(fcn, arg, stack)` and `join()`. Child thread shares parent address space (`pgdir`), duplicates open file descriptors, and executes on user-supplied stack with x86 cdecl argument passing.
* **Test Outcome:** Spawns 3 concurrent threads incrementing a shared counter 100,000 times each. Without synchronization, preemption causes **98,163 lost updates** (final counter `201,837` / `300,000`).

### Question 2 — Semaphores & Synchronization
* **Implemented:** Kernel counting semaphores with `sem_init(id, val)`, `sem_wait(id)`, `sem_signal(id)`, and single-thread wakeups via `wakeup_one(chan)`.
* **Test Outcome:**
  * Synchronized counter reaches exactly **300,000** (0 lost updates).
  * Producer-Consumer over 15 items in a buffer of size 5 completes with **zero overflow** and **zero underflow**.

### Question 3 — Deadlock Detection & Banker's Algorithm
* **Implemented:** Static array management, Banker's Algorithm safety check (`is_safe_state`), resource request evaluation (`simulate_request`), and Deadlock Detection Algorithm (`detect_deadlock`).
* **Test Outcome:**
  * Matrix 1: Finds safe execution sequence `P1 -> P3 -> P4 -> P0 -> P2`.
  * Matrix 2: Grants safe request `[1,0,2]`, rejects out-of-bounds request `[3,3,0]`, denies and rolls back unsafe request `[0,3,0]`.
  * Matrix 3: Accurately identifies 4-process circular dependency `P0 -> P1 -> P2 -> P3 -> P0`.
  * Matrix 4: Accurately identifies circular wait between real xv6 processes interacting via two IPC pipes.

### Question 4 — Priority Scheduling with Dynamic Aging
* **Implemented:** Added `priority` (1–20, default 10) and `waitticks` to `struct proc`. Modified `scheduler()` to pick runnable process with minimum numerical priority (ties broken in round-robin). Added dynamic aging in `trap.c` timer interrupt: runnable processes waiting $> 30$ ticks have priority decremented by 1 (`priority--`).
* **Test Outcome:** Evaluated with 4 CPU-bound processes (priorities 5, 10, 15, 20). Strict priority order observed for high-priority tasks (5 finishes first, then 10). Dynamic aging promoted Child 2 ($15 \rightarrow 14$) and Child 3 ($20 \rightarrow 19$), completely preventing CPU starvation.
