#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <pthread.h>

#define P1 "p1"
#define P2 "p2"

extern sem_t mutex;
extern int npk[2];
extern pthread_t p1,p2;
extern struct msgq *head;

void suspend(void);
void wakeup(pthread_t thread);
__attribute__((constructor)) void _suspend_init();

struct msgq {
	size_t size;
	char *sender;
	//char *buffer;
	int buffer;
  struct msgq *next;
};

msgq *msgadd(char *buff,struct msgq *msg,char *sender);
msgq *msgrmv(char **buff,struct msgq *msg,char *sender);
void asend(int x,msgq **p,char *sender);
//void arcv(char **y,msgq **p,char *sender);
void arcv(int &y,msgq **p,char *sender);
//void ssend(char *x,msgq **p, char *sender);
//void srcv(char **y,msgq **p,char *sender);
void ssend(int x,msgq **p, char *sender);
void srcv(int &y,msgq **p,char *sender);
#endif //MSGQUEUE_H
