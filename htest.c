/*
 * htest.c
 *
 *  Created on: Apr 26, 2015
 *      Author: hodai
 */
#include "kthread.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

#define STACK_SIZE 1000

void* execThread(){
  char* cat[] ={"cat", 0};
  printf(1, "exec thread, tid:%d\n", kthread_id());
  exec("cat", cat);
  printf(2, "error - return from exec\n");
  exit();
}

void* exitThread(){
  printf(1, "exit thread, tid:%d\n", kthread_id());
  exit();
}
void* normalThread(){
  printf(1, "normal thread, tid:%d\n", kthread_id());
  kthread_exit();
  return 0;
}

void* loopThread(){
  printf(0," \n\n");
  printf(1, "loop thread, tid:%d\n", kthread_id());
  for(;;){};
  kthread_exit();
  return 0;
}

void* sleepThread(){
  printf(1, "loop thread, tid:%d\n", kthread_id());
  sleep(1000);
  printf(1, "tid:%d done\n", kthread_id());
  kthread_exit();
  return 0;
}

void* theThread(){
  int i;
  for(i = 0 ;i < 10; i++){
    printf(1, "thread:%d,i=%d\n", kthread_id(), i);
  }
  kthread_exit();
  return 0;
}

void stressTest1(int count){
  int tid[count];
  int i,ans;
  char* stack;
  int fail = 0;

  for (i = 0 ; i < count; i++){
    stack = malloc(STACK_SIZE);
    tid[i] = kthread_create(&theThread, stack+STACK_SIZE, STACK_SIZE);
    if(tid[i] <= 0){
      printf(2, "error: kthread_create return with: %d, for index:%d", tid[i], i);
      fail = 1;
    }
  }

  for (i = 0 ; i < count; i++){
    ans = kthread_join(tid[i]);
    if(ans != 0){
      printf(2, "error: kthread_join(%d) return with: %d", tid[i], ans);
      fail = 1;
    }
  }

  if(fail){
    printf(1, "stressTest1 fail\n");
  } else {
    printf(1, "stressTest1 success\n");
  }

}

int main(){
  char buf[100];
  char* stack;
  int t;

  printf(1, "chose operation\n");

  while(read(0, buf, 100) >= 0){
    stack = malloc(STACK_SIZE);
    if(strcmp("sleep\n", buf) == 0){
      t = kthread_create(&sleepThread, stack+STACK_SIZE, STACK_SIZE);
      printf(1, "creating sleep thread tid:%d\n", t);

    } else if(strcmp(buf, "loop\n") == 0){
      t = kthread_create(&loopThread, stack+STACK_SIZE, STACK_SIZE);
      printf(1, "creating loop thread tid:%d\n", t);

    } else if(strcmp(buf, "norm\n") == 0){
      t = kthread_create(&normalThread, stack+STACK_SIZE, STACK_SIZE);
      printf(1, "creating normal thread tid:%d\n", t);

    } else if(strcmp(buf, "join\n") == 0){
      printf(1,"chose tid: ");
      if(read(0, buf, 100) > 0){
        t = atoi(buf);
        printf(1, "joining thread tid:%d\n", t);
        t = kthread_join(t);
        printf(1, "join result:%d \n", t);
      } else{
        printf(2,"fail\n");
      }

    } else if(strcmp(buf, "et\n") == 0){
      t = kthread_create(&exitThread, stack+STACK_SIZE, STACK_SIZE);
      printf(1, "creating exit thread tid:%d\n", t);

    } else if(strcmp(buf, "exec\n") == 0){
      t = kthread_create(&execThread, stack+STACK_SIZE, STACK_SIZE);
      printf(1, "creating exec thread tid:%d\n", t);

    } else if(strcmp(buf, "exit\n") == 0){
      printf(1, "exiting program\n");
      exit();

    } else if(strcmp(buf, "stress\n") == 0){
      printf(1,"chose num of threads: ");
      if(read(0, buf, 100) > 0){
        t = atoi(buf);
        stressTest1(t);
      } else{
        printf(2,"fail\n");
      }
    } else {
      printf(2, "error: %s\n", buf);
    }
    printf(0,"hi\n");
    memset(buf, 0, 100);
    printf(0,"bye\n");
  }

  exit();
}

