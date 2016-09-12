#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
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
    if(proc->killed){
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
int sys_kthread_create(void){
  void*(*start_func)();
  void* stack;
  int stack_size;

  if(argint(0, (int*)&start_func) < 0)
    return -1;
  if(argint(1, (int*)&stack) < 0)
    return -1;
  if(argint(2, (int*)&stack_size) < 0)
    return -1;

  return kthread_create(start_func, stack, stack_size);
}

int sys_kthread_id(void){
  return kthread_id();
}

int sys_kthread_exit(void){
  kthread_exit();
  return 0;
}

int sys_kthread_join(void){
  int kthread_id;
  if(argint(0, &kthread_id) < 0)
    return -1;
  kthread_join(kthread_id);
  return 0;
}
int sys_kthread_mutex_alloc(){
  return kthread_mutex_alloc();
}
int sys_kthread_mutex_dealloc(){
  int mutex_id;
  if(argint(0, &mutex_id) < 0)
    return -1;
  return kthread_mutex_dealloc(mutex_id);
}
int sys_kthread_mutex_lock(){
  int mutex_id;
  if(argint(0, &mutex_id) < 0)
    return -1;
  return kthread_mutex_lock(mutex_id);
}
int sys_kthread_mutex_unlock(){
  int mutex_id;
  if(argint(0, &mutex_id) < 0)
    return -1;
  return kthread_mutex_unlock(mutex_id);
}

int sys_kthread_mutex_num(){
  int mutex_id;
  if(argint(0, &mutex_id) < 0)
    return -1;
  return kthread_mutex_num(mutex_id);
}

int
sys_forkcow(void)
{
  return forkcow();
}

int
sys_waitcow(void)
{
  return waitcow();
}