#include <signal.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include "msgqueue.h"

using namespace std;

sem_t mutex;
int npk[2] = {0,0};
pthread_t p1,p2;
struct msgq *head;

void suspend(void) {
	int sig;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigwait(&set, &sig);
}

void wakeup(pthread_t thread) {
	pthread_kill(thread, SIGUSR1);
}

__attribute__((constructor)) void _suspend_init() {
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
}

//msgq *msgadd(char *buff,struct msgq *msg,char *sender){
msgq *msgadd(int buff,struct msgq *msg,char *sender){
	if(msg != NULL){
    struct msgq *newmsg = new msgq;
    //one element case
    if(msg->next == NULL){
      msg->next = newmsg;
    }
    //more element case
    else{
      newmsg = msg;
			//go to the tail
      while(newmsg->next != NULL){
        newmsg = newmsg->next;
      }
    }
    //asprintf(&(newmsg->buffer),buff);
		newmsg->buffer = buff;
		asprintf(&(newmsg->sender),sender);
    newmsg->next = NULL;
		delete(newmsg);
  }
  //queue empty
  else{
    msg = new msgq;
    //asprintf(&(msg->buffer),buff);
		msg->buffer = buff;
		asprintf(&(msg->sender),sender);
  }
  if(strcmp(sender,P1)==0){
      npk[1]++;
  }
  else npk[0]++;

  return msg;

}

msgq *msgrmv(int &buff,struct msgq *msg,char *sender){
    if (msg != NULL){
      //first elemento
      if(strcmp(msg->sender,sender)!=0){
        //asprintf(buff,msg->buffer);
				buff=msg->buffer;
				msg = msg->next;
      }
      //search element
      else{
        struct msgq *scan = new msgq;
        struct msgq *sent = new msgq;
        scan = msg->next;
        sent = msg;
        while(strcmp(scan->sender,sender)==0){
          sent = scan;
          scan = scan->next;
        }
        //asprintf(buff,scan->buffer);
				buff = scan->buffer;
				sent->next = scan->next;
        delete(scan);
      }
			if(strcmp(sender,P1)==0){
	      npk[0]--;
	    }
	    else npk[1]--;
    }
    return msg;
}

//put x port p asynchronous send
void asend(int x,msgq **p,char *sender){
	//char *sx=x;
  sem_wait(&mutex);
  *p=msgadd(x,*p,sender);
  if(strcmp(sender,P1)==0){
      wakeup(p2);
  }
  else {
    wakeup(p1);
  }
  sem_post(&mutex);
}

//read y port p asynchronous receive
//void arcv(char **y,msgq **p,char *sender){
void arcv(int &y,msgq **p,char *sender){
  int turn;
  if(strcmp(sender,P1)==0){
    turn = npk[0];
  }
  else {
    turn = npk[1];
  }
  if(turn == 0){
    suspend();
  }
  sem_wait(&mutex);
  *p=msgrmv(y,*p,sender);
  sem_post(&mutex);
}
//put x port p synchronous send
void ssend(int x,msgq **p, char *sender){
	//send msg
	sem_wait(&mutex);
  *p=msgadd(x,*p,sender);
  if(strcmp(sender,P1)==0){
      wakeup(p2);
  }
  else {
    wakeup(p1);
  }
  sem_post(&mutex);
	//wait ack
	int turn;
  if(strcmp(sender,P1)==0){
    turn = npk[0];
  }
  else {
    turn = npk[1];
  }
  if(turn == 0){
    suspend();
  }
	int y;
  sem_wait(&mutex);
  *p=msgrmv(y,*p,sender);
  sem_post(&mutex);

}
//read y port p synchronous receive
void srcv(int &y,msgq **p,char *sender){
	int turn;
  if(strcmp(sender,P1)==0){
    turn = npk[0];
  }
  else {
    turn = npk[1];
  }
  if(turn == 0){
    suspend();
  }
  sem_wait(&mutex);
  *p=msgrmv(y,*p,sender);
  *p=msgadd(0,*p,sender);
  if(strcmp(sender,P1)==0){
      wakeup(p2);
  }
  else {
    wakeup(p1);
  }
  sem_post(&mutex);
}
