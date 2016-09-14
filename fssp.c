#include "types.h"
#include "stat.h"
#include "user.h"
//#include "uthread.h"

#define Q 0
#define P 1
#define R 2
#define Z 3
#define M 4
#define X 5
#define F 6




int table[5][6][6];
int* arrayCurrent;
int* arrayNext;
int numberOfSoldiers;
int endOfLine = 0; //to check if we done to calculate the whole line.
//struct binary_semaphore print;
//struct binary_semaphore calc;
int print;
int calc;

struct idToSoldier{
	int soldier;
	int id;
};

typedef struct {
	int pre_lock;
	int post_lock;
	int counter;
	int max;
} barrier;


barrier b;
static int mutex;
struct idToSoldier *map;


void initialTable(){
	table[Q][Q][Q] = Q;
	table[Q][P][Q] = P;
	table[Q][R][Q] = Q;
	table[Q][Z][Q] = Q;
	table[Q][M][Q] = -1;
	table[Q][X][Q] = Q;

	table[Q][Q][P] = P;
	table[Q][P][P] = P;
	table[Q][R][P] = -1;
	table[Q][Z][P] = -1;
	table[Q][M][P] = -1;
	table[Q][X][P] = P;

	table[Q][Q][R] = Q;
	table[Q][P][R] = -1;
	table[Q][R][R] = Q;
	table[Q][Z][R] = -1;
	table[Q][M][R] = -1;
	table[Q][X][R] = Q;

	table[Q][Q][Z] = Q;
	table[Q][P][Z] = -1;
	table[Q][R][Z] = -1;
	table[Q][Z][Z] = Q;
	table[Q][M][Z] = -1;
	table[Q][X][Z] = Q;

	table[Q][Q][M] = -1;
	table[Q][P][M] = -1;
	table[Q][R][M] = -1;
	table[Q][Z][M] = -1;
	table[Q][M][M] = -1;
	table[Q][X][M] = -1;

	table[Q][Q][X] = Q;
	table[Q][P][X] = P;
	table[Q][R][X] = -1;
	table[Q][Z][X] = Q;
	table[Q][M][X] = -1;
	table[Q][X][X] = -1;

	//-------------------------------

	table[Z][Q][Q] = -1;
	table[Z][P][Q] = -1;
	table[Z][R][Q] = Q;
	table[Z][Z][Q] = P;
	table[Z][M][Q] = Q;
	table[Z][X][Q] = -1;

	table[Z][Q][P] = -1;
	table[Z][P][P] = Z;
	table[Z][R][P] = -1;
	table[Z][Z][P] = Z;
	table[Z][M][P] = -1;
	table[Z][X][P] = Z;

	table[Z][Q][R] = Q;
	table[Z][P][R] = -1;
	table[Z][R][R] = Q;
	table[Z][Z][R] = Q;
	table[Z][M][R] = -1;
	table[Z][X][R] = Q;

	table[Z][Q][Z] = P;
	table[Z][P][Z] = Z;
	table[Z][R][Z] = Q;
	table[Z][Z][Z] = F;
	table[Z][M][Z] = Q;
	table[Z][X][Z] = F;

	table[Z][Q][M] = Q;
	table[Z][P][M] = -1;
	table[Z][R][M] = -1;
	table[Z][Z][M] = Q;
	table[Z][M][M] = Q;
	table[Z][X][M] = Q;

	table[Z][Q][X] = -1;
	table[Z][P][X] = -1;
	table[Z][R][X] = Q;
	table[Z][Z][X] = F;
	table[Z][M][X] = Q;
	table[Z][X][X] = -1;

	//-------------------------------

	table[P][Q][Q] = Z;
	table[P][P][Q] = Z;
	table[P][R][Q] = R;
	table[P][Z][Q] = R;
	table[P][M][Q] = -1;
	table[P][X][Q] = Z;

	table[P][Q][P] = Z;
	table[P][P][P] = -1;
	table[P][R][P] = Z;
	table[P][Z][P] = Z;
	table[P][M][P] = -1;
	table[P][X][P] = -1;

	table[P][Q][R] = R;
	table[P][P][R] = Z;
	table[P][R][R] = Z;
	table[P][Z][R] = -1;
	table[P][M][R] = -1;
	table[P][X][R] = Z;

	table[P][Q][Z] = R;
	table[P][P][Z] = Z;
	table[P][R][Z] = -1;
	table[P][Z][Z] = Z;
	table[P][M][Z] = -1;
	table[P][X][Z] = Z;

	table[P][Q][M] = -1;
	table[P][P][M] = -1;
	table[P][R][M] = -1;
	table[P][Z][M] = -1;
	table[P][M][M] = -1;
	table[P][X][M] = -1;

	table[P][Q][X] = -1;
	table[P][P][X] = -1;
	table[P][R][X] = Z;
	table[P][Z][X] = Z;
	table[P][M][X] = -1;
	table[P][X][X] = F;

	//-------------------------------

	table[M][Q][Q] = -1;
	table[M][P][Q] = -1;
	table[M][R][Q] = -1;
	table[M][Z][Q] = -1;
	table[M][M][Q] = -1;
	table[M][X][Q] = -1;

	table[M][Q][P] = -1;
	table[M][P][P] = -1;
	table[M][R][P] = -1;
	table[M][Z][P] = -1;
	table[M][M][P] = -1;
	table[M][X][P] = -1;

	table[M][Q][R] = -1;
	table[M][P][R] = -1;
	table[M][R][R] = R;
	table[M][Z][R] = Z;
	table[M][M][R] = -1;
	table[M][X][R] = -1;

	table[M][Q][Z] = -1;
	table[M][P][Z] = -1;
	table[M][R][Z] = Z;
	table[M][Z][Z] = -1;
	table[M][M][Z] = -1;
	table[M][X][Z] = -1;

	table[M][Q][M] = -1;
	table[M][P][M] = -1;
	table[M][R][M] = -1;
	table[M][Z][M] = -1;
	table[M][M][M] = -1;
	table[M][X][M] = -1;

	table[M][Q][X] = -1;
	table[M][P][X] = -1;
	table[M][R][X] = -1;
	table[M][Z][X] = -1;
	table[M][M][X] = -1;
	table[M][X][X] = -1;

	//-------------------------------

	table[R][Q][Q] = -1;
	table[R][P][Q] = -1;
	table[R][R][Q] = R;
	table[R][Z][Q] = P;
	table[R][M][Q] = Z;
	table[R][X][Q] = -1;

	table[R][Q][P] = -1;
	table[R][P][P] = -1;
	table[R][R][P] = M;
	table[R][Z][P] = R;
	table[R][M][P] = M;
	table[R][X][P] = -1;

	table[R][Q][R] = R;
	table[R][P][R] = M;
	table[R][R][R] = -1;
	table[R][Z][R] = -1;
	table[R][M][R] = M;
	table[R][X][R] = -1;

	table[R][Q][Z] = P;
	table[R][P][Z] = R;
	table[R][R][Z] = -1;
	table[R][Z][Z] = -1;
	table[R][M][Z] = R;
	table[R][X][Z] = -1;

	table[R][Q][M] = Z;
	table[R][P][M] = M;
	table[R][R][M] = M;
	table[R][Z][M] = R;
	table[R][M][M] = M;
	table[R][X][M] = -1;

	table[R][Q][X] = -1;
	table[R][P][X] = -1;
	table[R][R][X] = -1;
	table[R][Z][X] = -1;
	table[R][M][X] = -1;
	table[R][X][X] = -1;
}
void printSoldier(int i){
	if(i==0) printf(1,"Q");
	else if(i==1) printf(1,"P");
	else if(i==2) printf(1,"R");
	else if(i==3) printf(1,"Z");
	else if(i==4) printf(1,"M");
	else if(i==5) printf(1,"X");
	else if(i==6) printf(1,"F");


}
void print_all(){
	int i;
	for(i = 0; i < numberOfSoldiers; i++){
		//printf(1,"loop\n");
		printSoldier(arrayCurrent[i]);

	}
	printf(1,"\n");
}
int getSoldierFromId(int id){
	int i;
	for(i=0;i<numberOfSoldiers;i++){
		if(map[i].id==id)
			return map[i].soldier;
	}
	return -1;
}

void free_barrier(barrier *b){
	kthread_mutex_dealloc(b->pre_lock);
	kthread_mutex_dealloc(b->post_lock);
	b->max = 0;
	b->counter = 0;
}

void init_barrier(barrier *b, int max){
	b->pre_lock = kthread_mutex_alloc();
	b->post_lock = kthread_mutex_alloc();
	b->max = max;
	b->counter = 0;
	kthread_mutex_lock(b->post_lock);	
}

void barrier_increase(barrier* b){
	kthread_mutex_lock(b->pre_lock);
	b->counter++;
	if(b->max > b->counter)
		kthread_mutex_unlock(b->pre_lock);	
	else // max == counter
		kthread_mutex_unlock(b->post_lock);			
	
	kthread_mutex_lock(b->post_lock);
	b->counter--;
	if(b->counter > 0)
		kthread_mutex_unlock(b->post_lock);
	else // counter == 0
		kthread_mutex_unlock(b->pre_lock);
}

void updateState(){
	barrier_increase(&b);
	int i=getSoldierFromId(kthread_id());
	int next=0;
	while(arrayCurrent[i] != F){
		
		if(i == 0){
			next = table[arrayCurrent[i]][X][arrayCurrent[i+1]];
		}
		else if (i > 0 && i < numberOfSoldiers-1){
			next = table[arrayCurrent[i]][arrayCurrent[i-1]][arrayCurrent[i+1]];
		}
		else if (i == numberOfSoldiers-1){
			next = table[arrayCurrent[i]][arrayCurrent[i-1]][X];
		}
		barrier_increase(&b);	// wait for all thread to calculate their next_state
		arrayNext[i]=next;
		arrayCurrent=arrayNext;
		barrier_increase(&b);
		if(i==0){
			kthread_mutex_lock(mutex);
			
			print_all();
			kthread_mutex_unlock(mutex);
		}
		
	}

	kthread_exit();
}


int main(int argc, char **argv)
{
	if(argc != 2){
		printf(1,"Missing arguments. cannot init without number of sodliers\n");
		exit();
	}
	numberOfSoldiers = atoi(argv[1]);
	if(numberOfSoldiers > 15){ //15=16-1 = MAXTHREADS-MAIN
		printf(1,"There is too many soldiers. cannot support more threads .\n ");
		exit();
	}

	if(numberOfSoldiers <= 1){
		printf(1,"The number of soldier must be positive.\n");
		exit();
	}
	int i;
	struct idToSoldier map1[numberOfSoldiers];
	map= map1;
	mutex = kthread_mutex_alloc();
	init_barrier(&b,numberOfSoldiers);
	arrayCurrent = (int*)malloc(numberOfSoldiers * sizeof(int)); //current state
	arrayNext = (int*)malloc(numberOfSoldiers * sizeof(int)); //next state
	initialTable();
	//uthread_init();
	arrayCurrent[0] = P;


	for(i = 1; i < numberOfSoldiers; i++){
		arrayCurrent[i] = Q;
		
	}
	
	for(i =0; i < numberOfSoldiers; i++){
		void* stack = malloc (1024);
		int id=kthread_create((void*) updateState, stack,1024);
		map[i].id=id;
		map[i].soldier=i;

	}
	print_all();

	for(i=0;i<numberOfSoldiers;i++){
		kthread_join(map[i].id);
	}

	kthread_mutex_dealloc(mutex);
	free_barrier(&b);



	exit();

	return 0;
}



