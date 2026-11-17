#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// --- Assignment 7: Counting Semaphores Implementation ---
#define MAX_SEM 10

// Semaphore structure using xv6 sleep/wakeup for blocking
struct semaphore {
  int value;             // Semaphore counter value (can be negative when processes wait)
  struct spinlock lk;    // Spinlock protecting this semaphore's internal state
  int active;            // 1 if semaphore is initialized and active, 0 otherwise
};

struct semaphore semaphores[MAX_SEM];
struct spinlock sem_array_lock; // Spinlock protecting the global semaphore array

// Initialize all semaphores to inactive state during system boot
void
seminit(void)
{
  initlock(&sem_array_lock, "sem_array_lock");
  int i;
  for(i = 0; i < MAX_SEM; i++) {
    initlock(&semaphores[i].lk, "semaphore");
    semaphores[i].value = 0;
    semaphores[i].active = 0;
  }
}

// System call helper to initialize a semaphore
int
sem_init(int sem_id, int value)
{
  if(sem_id < 0 || sem_id >= MAX_SEM || value < 0)
    return -1;

  acquire(&sem_array_lock);
  acquire(&semaphores[sem_id].lk);
  semaphores[sem_id].value = value;
  semaphores[sem_id].active = 1;
  release(&semaphores[sem_id].lk);
  release(&sem_array_lock);
  return 0;
}

// System call helper to decrement and block if value < 0
int
sem_wait(int sem_id)
{
  if(sem_id < 0 || sem_id >= MAX_SEM)
    return -1;

  struct semaphore *s = &semaphores[sem_id];
  acquire(&s->lk);
  if(!s->active){
    release(&s->lk);
    return -1;
  }

  s->value--;
  if(s->value < 0){
    sleep(s, &s->lk);
  }
  release(&s->lk);
  return 0;
}

// System call helper to increment and wake one waiting process if any (value <= 0)
int
sem_signal(int sem_id)
{
  if(sem_id < 0 || sem_id >= MAX_SEM)
    return -1;

  struct semaphore *s = &semaphores[sem_id];
  acquire(&s->lk);
  if(!s->active){
    release(&s->lk);
    return -1;
  }

  s->value++;
  if(s->value <= 0){
    wakeup_one(s);
  }
  release(&s->lk);
  return 0;
}

// Syscall handlers for semaphores
int
sys_sem_init(void)
{
  int id, val;
  if(argint(0, &id) < 0 || argint(1, &val) < 0)
    return -1;
  return sem_init(id, val);
}

int
sys_sem_wait(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  return sem_wait(id);
}

int
sys_sem_signal(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  return sem_signal(id);
}

// Backwards compatibility aliases for Assignment 5
int
sys_sem_down(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  return sem_wait(id);
}

int
sys_sem_up(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  return sem_signal(id);
}

// --- Assignment 7: Threads System Call Wrappers ---
int
sys_clone(void)
{
  int fcn, arg, stack;
  if(argint(0, &fcn) < 0 || argint(1, &arg) < 0 || argint(2, &stack) < 0)
    return -1;
  return clone((void(*)(void*))fcn, (void*)arg, (void*)stack);
}

int
sys_join(void)
{
  return join();
}

// --- Assignment 7: Priority Scheduling System Call Wrappers ---
int
sys_set_priority(void)
{
  int pid, priority;
  if(argint(0, &pid) < 0 || argint(1, &priority) < 0)
    return -1;
  return set_priority(pid, priority);
}

int
sys_get_priority(void)
{
  int pid;
  if(argint(0, &pid) < 0)
    return -1;
  return get_priority(pid);
}
// ----------------------------------------------------

extern void* shm_get(void);

// Wrapper syscall to call the kernel shm_get implementation
int
sys_shm_get(void)
{
  return (int)shm_get();
}
