#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <semaphore.h>
#include "msgqueue.h"

struct msgq *a;
struct msgq *b;
void test_forward(int &x1);
void test_reverse(int &x1);
void *prog_0(void *arg);
void *prog_1(void *arg);

void test_forward(int &x1) {
	printf("%d\n",x1);
		
	sem_init(&mutex,0,1);
	pthread_create(&p1,NULL,prog_0,NULL);
	pthread_create(&p2,NULL,prog_1,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	sem_destroy(&mutex);

}

void test_reverse(int &x1) {
	
	sem_init(&mutex,0,1);
	pthread_create(&p1,NULL,prog_1,NULL);
	pthread_create(&p2,NULL,prog_0,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	sem_destroy(&mutex);

	printf("%d\n",x1);
}

int main(int argc, char const *argv[]) {
	int x1 = 0;
	x1 += 4;
	test_forward(x1);
}

void *prog_0(void *arg){
	int x2 = 0;
	x2 += 2;
	ssend(x2,&a,P1);
	printf("%d\n",x2);
	assert(x2 == 2);
}
void *prog_1(void *arg){
	int x3 = 0;
	srcv(x3,&a,P2);
	printf("%d\n",x3);
	assert(x3 == 2);
}
