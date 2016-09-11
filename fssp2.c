 
#include "types.h"
#include "stat.h"
#include "user.h"

#define Q  0
#define P  1
#define R  2
#define Z  3
#define M  4
#define X  5
#define F  6


#define MAX_THREAD 16
struct idToSoldier{
	int id;
	int soldier;
};

struct idToSoldier *map;


int states[5][6][6] = { //State Q
                        {{Q, P, Q, Q, X, Q},
			 {P, P, X, X, X, P},
			 {Q, X, Q, X, X, X},
			 {Q, X, X, Q, X, Q},
			 {X, X, X, X, X, X},
			 {Q, P, Q, Q, X, X}},
			  
			//State P
                        {{Z, Z, R, R, X, X},
			 {Z, X, Z, Z, X, X},
			 {R, Z, Z, X, X, Z},
			 {R, Z, X, Z, X, Z},
			 {X, X, X, X, X, X},
			 {Z, X, Z, Z, X, X}},
			  
			//State R
                        {{X, X, R, P, Z, X},
			 {X, X, M, R, M, X},
			 {R, M, X, X, M, X},
			 {P, R, X, X, R, X},
			 {Z, M, M, R, M, X},
			 {X, X, X, X, X, X}},
			  
			//State Z
                        {{X, X, Q, P, Q, X},
			 {X, Z, X, Z, X, X},
			 {Q, X, Q, Q, X, Q},
			 {P, Z, Q, F, Q, F},
			 {Q, X, X, Q, Q, Q},
			 {X, Z, Q, F, Q, X}},

			//State M
                        {{X, X, X, X, X, X},
			 {X, X, X, X, X, X},
			 {X, X, R, Z, X, X},
			 {X, X, Z, X, X, X},
			 {X, X, X, X, X, X},
			 {X, X, X, X, X, X}}
		      };

char strStates[7] = {'Q','P','R','Z', 'M', ' ', 'F'};

int isInitialize = 1;
int curr_stage[MAX_THREAD] = {Q};
int next_stage[MAX_THREAD] = {Q};
int num_soldiers;
int fire_count = 0;
int soldier_count = 0;
//struct binary_semaphore enter;
//struct binary_semaphore leave;
int enter;
int leave;
int start_fssp(int soldiers);

int getSoldierFromId(int id){
	int i;
	for(i=0;i<num_soldiers;i++){
		if(map[i].id==id)
			return map[i].soldier;
	}
	return -1;
}

int
main(int argc, char *argv[])
{
  if ( argc < 1 ) { 
    printf(1, "FSSP: bad input!\n");
    exit();
  }
  return start_fssp(atoi(argv[1]));
}

void
barrier()
{
  //binary_semaphore_down(&enter);
  kthread_mutex_lock(enter);
  soldier_count++;
  
  if (soldier_count < num_soldiers+1)
  {
  	//binary_semaphore_up(&enter);
  	kthread_mutex_unlock(enter);
  }
  else 
  {
  	//binary_semaphore_up(&leave);	
  	kthread_mutex_unlock(leave);
  }
    
  //binary_semaphore_down(&leave);
  kthread_mutex_lock(leave);
  soldier_count--;
  
  if(soldier_count > 0)
  {
  	//binary_semaphore_up(&leave);
  	kthread_mutex_unlock(leave);

  }
  else
  {
  	//binary_semaphore_up(&enter);
  	kthread_mutex_unlock(leave);
  }
}

void
new_thread_state()
{
	  int cell_num=getSoldierFromId(kthread_id());

  int new_state;
  while (curr_stage[cell_num] != F) {
    if (cell_num == 0) //first cell
	  new_state = states[curr_stage[cell_num]][X][curr_stage[cell_num+1]];
    else if (cell_num == num_soldiers-1) //last cell
	  new_state = states[curr_stage[cell_num]][curr_stage[cell_num-1]][X];
    else
	  new_state = states[curr_stage[cell_num]][curr_stage[cell_num-1]][curr_stage[cell_num+1]];
   
    next_stage[cell_num] = new_state;

    if (new_state == F)
      fire_count++;

    barrier();
    next_stage[cell_num] = new_state;
    barrier();
  }
}

void
printSoldiersState()
{
  int i;
  for(i = 0 ; i < num_soldiers ; i++) {
    if (!isInitialize)
      curr_stage[i]=next_stage[i];
    printf(1,"%c ", strStates[curr_stage[i]]);
    }
  printf(0,"\n"); 
}

int
start_fssp(int soldiers)
{
  num_soldiers = soldiers;
  curr_stage[0] = P;

  //binary_semaphore_init(&enter, 1);
  //binary_semaphore_init(&leave, 0);
  enter= kthread_mutex_alloc();
  leave = kthread_mutex_alloc(); 
  kthread_mutex_lock(leave);// so will start from 0
  //uthread_init();
  int i;
  	struct idToSoldier map1[num_soldiers];
	map= map1;

  for(i = 0 ; i < num_soldiers ; i++)
  {
  	void* stack = malloc (1024);
  	int id=kthread_create((void*) new_thread_state, stack,1024);
  	 // printf(1,"123123123\n" );

  	map[i].id=id;
	map[i].soldier=i;
  	//uthread_create((void*)&new_thread_state,(void*)i);

  }

  printSoldiersState();
  isInitialize = 0;
  while (fire_count < num_soldiers) {
    barrier();
    barrier();
    printSoldiersState();
  }

  exit();
  return 0;
}

