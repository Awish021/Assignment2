#ifndef KTHREAD_H
#define KTHREAD_H

#define NTHREAD 64
#define MAX_MUTEXES 16


int kthread_create(void* (start_func)(),void* stack,int stack_size);
int kthread_id();
void kthread_exit();
int kthread_join();
int kthread_mutex_alloc();
int kthread_mutex_dealloc(int mutex_id);
int kthread_mutex_lock(int mutex_id);
int kthread_mutex_unlock(int mutex_id);
int kthread_mutex_num(int mutex_id);


enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct thread{
  int id;
  char *kstack;                // Bottom of kernel stack for this Thread
  enum procstate state;        // Thread state
  struct proc* process;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;
  struct thread* list;
};




#endif