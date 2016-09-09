#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "spinlock.h"
#include "proc.h"
#include "kthread.h"


void wakeupThreads(void* chan){
	struct thread* t;
	for(t=proc->threads;t<&proc->threads[NTHREAD];t++){
		if(t->state==SLEEPING&&t->chan==chan)
			t->state=RUNNABLE;
	}
}

int last(struct thread* t){
	struct proc* p =t->process;
	struct thread* temp;
	for(temp=p->threads;temp<&p->threads[NTHREAD];t++)
		if(temp!=t&&temp->state!=ZOMBIE&&temp->state!=UNUSED)
			return 0;
	return 1;
		
	
}

int kthread_create(void* (start_func)(),void* stack,int stack_size){
	struct thread* new;
	new =allocthread(proc);
	if(!new||stack<=0)
		return -1;
	cprintf("here\n");
	*new->tf=*thread->tf;

	new->tf->eip=(int)start_func;
	new->tf->esp=(int) (stack)+stack_size;
	acquire(&new->process->lock);
	new->state=RUNNABLE;
	release(&new->process->lock);

	return new->id;
}

int kthread_id(){
	return thread->id;
}

void kthread_exit(){
	if(last(thread))
		exit();
	else{
		thread->state=ZOMBIE;
		wakeupThreads(thread);
		
	}
}


int kthread_join(int thread_id){
	struct thread* t;
	int exists=0;
	acquire(&proc->lock);
	for(t = proc->threads; t < &proc->threads[NTHREAD]; t++){
		if(t->id==thread_id){
			exists=1;
			break;
		}
	}
	if(!exists){
		release(&proc->lock);
		return -1;
	}
	if(t->state==UNUSED){
  	release(&proc->lock);
  	return 0;
  }
	else if(t->state == ZOMBIE){
		free_and_return:
    kfree(t->kstack);
    t->kstack = 0;
    t->state = UNUSED;
    release(&proc->lock);
    return 0;
  }
  else{
  	while(t->state!=ZOMBIE)
  		sleep(t,&proc->lock);
  	if(t->state==UNUSED)
  		panic("kthread_join: won't wake up because thread was going from ready to unused");
  	goto free_and_return;
  }
   
 

    return 0;
}


