// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "types.h"
#include "stat.h"
#include "user.h"
#include "syscall.h"

#define STK_SIZE 4096

void test1(void);
void* test1_threadfunc();

int tester;

int main(void){
  int pid;
  pid = fork();
  if(pid == 0){
  	test1();
  	printf(1, "SHOULDNT BE HERE\n");
  	exit();
  }
  else
  	wait();
  exit();
}

void test1(void){
	tester = -1;
	printf(1, "test 1 - start:\n");
	void *stack = malloc(STK_SIZE);
	int tid = kthread_id();
	int new_tid = kthread_create(test1_threadfunc, stack, STK_SIZE);
	
	if(new_tid<0){
		printf(1, "TEST 1.a. FAILED - thread was not created\n");
		exit();
	}

	//thread was created
	printf(1, "join = %d\n", kthread_join(new_tid));
	if(tester == -1)
		printf(1, "TEST 1.b. FAILED - kthread_join not waiting\n");
	else
		printf(1, "TEST 1.b. PASSED - thread #%d joined with #%d\n", tid, new_tid);
	if(tester == 1)
		printf(1, "TEST 1.c PASSED - thread exit with kthread_exit()\n");

	printf(1, "test 1 - end\n");
	kthread_exit();
}

void* test1_threadfunc(){
	int tid = kthread_id();
	printf(1, "TEST 1.a. PASSED - thread #%d created succesfully\n", tid);
	tester = 1;
	kthread_exit();
	tester = 0;
	printf(1, "TEST 1.c FAILED - thread #%d was not exit with kthread_exit()\n", tid);
	exit();
}