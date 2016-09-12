#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "spinlock.h"
#include "proc.h"
#include "kthread.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

struct{
  struct spinlock lock;
  struct kthread_mutex_t mutexes[MAX_MUTEXES];
}mtable;

static struct proc *initproc;

int nextpid = 1;
int nextThread=1 ;
extern void forkret(void);
extern void trapret(void);
extern void wakeupThreads(struct thread* t);
extern int last(struct thread* t);
static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
  initlock(&mtable.lock,"mtable");
}

struct thread* allocthread (struct proc* p)
{

  struct thread* t;
  char *sp;
  acquire(&ptable.lock);
  for(t = p->threads; t < &p->threads[NTHREAD]; t++){
    if(t->state == UNUSED)
      goto found;
  }
  release(&ptable.lock);
  return 0;

found:

  t->state = EMBRYO;
  t->id = nextThread++;    // must hold the lock to avoid duplicate id's
  t->list=0;
  release(&ptable.lock);
  t->process = p;
  // Allocate kernel stack.
  if((t->kstack = kalloc()) == 0){
    t->state = UNUSED;
    return 0;
  }
  sp = t->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *t->tf;
  t->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *t->context;
  t->context = (struct context*)sp;
  memset(t->context, 0, sizeof *t->context);
  t->context->eip = (uint)forkret;

  return t;
}
void execSignalToThreads(struct thread* t){
  struct thread* temp;
  int allDone;
  acquire(&t->process->lock);
  //t->process->executed=1;
  while(1){
    if(proc->killed){
      release(&t->process->lock);
      exit();
    }
    allDone=1;
    for(temp=t->process->threads;temp<&t->process->threads[NTHREAD];temp++){
      //unrelevent threads.no point waiting on them
        if(temp==t||temp->state==UNUSED)
          continue;

        if(temp->state==ZOMBIE){
          //thread was terminated
          temp->state=UNUSED;
          kfree(temp->kstack);
          temp->kstack=0;
          continue;
        }

        if(temp->state==SLEEPING){
          //wakeup in order for it to terminate;
          //temp->killed=1;
          temp->state=RUNNABLE;
          }
          temp->killed=1;
          release(&t->process->lock);
          kthread_join(temp->id);
          acquire(&t->process->lock);
        allDone=0;
      }
      if(allDone){
        t->process->executed=0;
        release(&t->process->lock);
        return;
      }
      //sleep(t->process,&t->process->lock); //go to sleep until the next one is done;

  }
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
// Must hold ptable.lock.
static struct proc*
allocproc(void)
{
  struct proc *p;
  struct thread* t;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  initlock(&p->lock,"proclock");
  for (t = p->threads ; t < &p->threads[NTHREAD] ; t++)
    t->state = UNUSED;
  
  
  release(&ptable.lock);
  t = allocthread(p);
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  p = allocproc();
  initproc = p;
 
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;


  struct thread* t;
  t=p->threads;
  memset(t->tf, 0, sizeof(*t->tf));
  t->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  t->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  t->tf->es = t->tf->ds;
  t->tf->ss = t->tf->ds;
  t->tf->eflags = FL_IF;
  t->tf->esp = PGSIZE;
  t->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");
  p->state=RUNNABLE;
  t->state = RUNNABLE;
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;

  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;
  struct thread* t = np->threads;
  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(t->kstack); 
    t->kstack = 0;
    np->state = UNUSED;
    t->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *t->tf = *thread->tf;           

  // Clear %eax so that fork returns 0 in the child.
  t->tf->eax = 0;                 

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  safestrcpy(np->name, proc->name, sizeof(proc->name));
 
  pid = np->pid;

  // lock to force the compiler to emit the np->state write last.
  acquire(&ptable.lock);
  t->state = RUNNABLE;  
  np->state = RUNNABLE;             // the proc is in state: Runnable, running or sleeping
  release(&ptable.lock);
  
  return pid;
}


// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{

  struct proc *p;
  struct thread* t;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);
  wakeup1(thread);
  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){

    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  for(t=proc->threads;t<&proc->threads[NTHREAD];t++){
      t->id=0;
      if(t->state!=UNUSED)
        t->state=ZOMBIE;
      t->chan=0;
      t->process=0;
    }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  thread->state=ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{

  struct proc *p;
  int havekids, pid;
  struct thread* t;
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
       
        for(t = p->threads ; t < &p->threads[NTHREAD] ; t++){
          if(t->state != UNUSED){
            if(t->state != ZOMBIE)
              panic("wait panic: not ZOMBIE or UNUSED");
            
            kfree(t->kstack);
            t->kstack = 0;
            t->state = UNUSED;
          }
        }
        freevm(p->pgdir);
        p->pid = 0;
        p->executed=0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;

        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed|| proc->executed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

void
scheduler(void)
{
  struct proc *p;
  struct thread* t;
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if (p->state != RUNNABLE)
        continue;

      for(t = p->threads; t < &p->threads[NTHREAD]; t++){
        if(t->state != RUNNABLE)
          continue;

        // Switch to chosen process.  It is the process's job
        // to release ptable.lock and then reacquire it
        // before jumping back to us.
        proc = p;
        thread = t;
        //cprintf("chosen tid:%d\n",t->id);
       
       // if(t->id==4)
         // cprintf("4\n");
        switchuvm(p);

        t->state = RUNNING;
        swtch(&cpu->scheduler, t->context);
        switchkvm();

        // Process is done running for now.
        // It should have changed its p->state before coming back.
        proc = 0;
        thread = 0;
      }
    }
    release(&ptable.lock);

  }
}


// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&thread->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  thread->state = RUNNABLE;

  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  thread->chan = chan;
  thread->state = SLEEPING;
  sched();

  // Tidy up.
  thread->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes and their threads sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;
  struct thread* t;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    for(t = p->threads ; t < &p->threads[NTHREAD] ; t++)
      if(t->state == SLEEPING && t->chan == chan)
        t->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;
  struct thread* t;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      for(t = p->threads ; t < &p->threads[NTHREAD] ; t++){
        if(t->state == SLEEPING)
          t->state = RUNNABLE;
      }
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}
  
void printppn(pde_t* pgdir){
  int i,j;
  char* write;
  pte_t* pgtable;
  for(i=0; i<1024; i++){
    if ((pgdir[i] & PTE_P) && (pgdir[i] & PTE_U)){
      pgtable = (pte_t*)P2V(PTE_ADDR(pgdir[i]));
      for (j=0; j<1024; j++){
        if (pgtable[j] & PTE_P && pgtable[j] & PTE_U){
          if (pgtable[j]& PTE_W){
            write="y";
           } 
          else
            write="n";
          cprintf("%d -> %d %s\n", ((i<<10) + j), (pgtable[j] >> 12), write);   
        }
      }
    }
  }
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;

  struct proc *p;
  char *state;
  uint pc[10];
  struct thread* t;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;  
      if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
          state = states[p->state];
        else
          state = "???";
        cprintf("%d %s %s\n", p->pid, state, p->name);
        printppn(p->pgdir); 
    for(t=p->threads;t<&p->threads[NTHREAD];t++)
    {
        
        if(t->state == SLEEPING){
          getcallerpcs((uint*)t->context->ebp+2, pc);
          for(i=0; i<10 && pc[i] != 0; i++)
            cprintf(" %p", pc[i]);
        }
        /*cprintf("\n");*/
      }
  }
}


void kthread_exit(){
  acquire(&ptable.lock);
  int i;
  int last = -1;
  for(i=0;i<NTHREAD;i++){
    if(proc->threads[i].id!=thread->id&&proc->threads[i].state!=ZOMBIE&&proc->threads[i].state!=UNUSED){
      last=i;
      break;
    }
  }

  if(last==-1){
    release(&ptable.lock);
    exit();
  }
  else{
    
    
    //thread->kstack=0; will conflict with kfree() at wait..
    thread->tf=0;
    //thread->id=0; was really bad idea
    thread->state=ZOMBIE;
    wakeupThreads(thread);
      // Jump into the scheduler, never to return.
    sched();
    panic("kthread-exit: error");
    
  }
}

int kthread_mutex_alloc(){
  struct kthread_mutex_t* mutex;
  int i;
  acquire(&mtable.lock);
  for(i=0;i<MAX_MUTEXES;i++){
    if(!mtable.mutexes[i].used){
      mtable.mutexes[i].used=1;
      release(&mtable.lock);
      mutex=&mtable.mutexes[i];
      mutex->locked=0;
      mutex->waiting=0;
      mutex->owner=0;
      mutex->num=0;
      initlock(&mutex->lock,"mutexlock");
      return i;
    }
  }
  //there is no free mutex
  release(&mtable.lock);
  return -1;

}
int kthread_mutex_dealloc(int mutex_id){
  struct kthread_mutex_t* mutex=&mtable.mutexes[mutex_id];

  acquire(&mtable.lock);
  if(!mutex->used){
    release(&mtable.lock);
    return -1;
  }
  if(mutex->locked==1){
    release(&mtable.lock);
    return -1;
  }
  if(mutex->waiting){
    release(&mtable.lock);
    return -1;
  }
  mutex->used=0;
  release(&mtable.lock);
  return 0;


}
int kthread_mutex_lock(int mutex_id){
  struct kthread_mutex_t* mutex=&mtable.mutexes[mutex_id];
  if(!mutex)
    return -1;
  acquire(&mutex->lock);
  if(!mutex->used){
    release(&mutex->lock);
    return -1;
  }
  if(mutex->locked){
    //our implementation of watining list.. using an thread field "list" to create linked list of waiting threads.
    thread->list=mutex->waiting;
    mutex->waiting=thread;
    mutex->num++;
    sleep(thread,&mutex->lock); //moving the thread to "blocked" state. sleeping on itself.
  }
  else{
    //LOCK IS MINE
    if(mutex->num>0)
      panic("number panic");
    mutex->locked=1;
    mutex->owner=thread;
  }
  release(&mutex->lock);

  return 0;
}

//FIFO STYLE
struct thread* getWaitingThread(struct kthread_mutex_t* mutex){
  struct thread* ans;
  struct thread* prev;
  ans=mutex->waiting;
  prev=ans;

  if(!ans)
    return 0;
  //getting the last link in this linked list.
  while(ans->list){
    prev=ans;
    ans=ans->list;
  }
  //Only one thread is wating
  if(ans==prev)
    mutex->waiting=0;
  else
    prev->list=0;
  return ans;

}

///CONTINUE HERE !! HOPEFULLY LOCK IS WORKING FINE
int kthread_mutex_unlock(int mutex_id){

    struct kthread_mutex_t* mutex=&mtable.mutexes[mutex_id];
    acquire(&mutex->lock);
    if(!mutex->locked){
      release(&mutex->lock);
      return -1;
    }
    /*if(mutex->owner!=thread){
      release(&mutex->lock);
      panic("a thread trying to unlock mutex he does not own"); // make sure that what Vadim meant. if not, just delete this if.
       return -1;

    }*/
    // we are actually doing fair FIFO mutex. 
    struct thread* t = getWaitingThread(mutex);

    if(t){
      mutex->owner=t;
      mutex->num--;
      wakeup(t);
    }
    else{
      // no thread is waiting for this lock.
      mutex->locked=0;
      mutex->owner=0;
      mutex->waiting=0;
    }
    release(&mutex->lock);
    return 0;

}
int kthread_mutex_num(int mutex_id){
  struct kthread_mutex_t* mutex=&mtable.mutexes[mutex_id];
  return mutex->num;
}