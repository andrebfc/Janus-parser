#ifndef _STRUCTUREH_
#define _STRUCTUREH_

#include <stdio.h>

struct call_ist{
  char *type = NULL;
  char *arg = NULL;
  char *name = NULL;
};

struct msgch{
  char *type = NULL;
  char *value = 0;
  char *port = NULL;
  msgch *next;
};

struct instruction {
  char *type = NULL;
  char *l_value = NULL;
  char *r_value = NULL;
  char *op = NULL;
  int flag_sub = 0;
  int loc = 0; //0=null, 1=local,2=delocal
  call_ist *call;
  msgch *msg = NULL;
  instruction *next;
  instruction *prev;
};

struct argument {
  char *type;
  char *name;
  argument *next;
};

struct rda {
  char *name;
  argument *arg;
  bool wr = 0; // bool to set when i write this rda
  instruction *isp;
  instruction *f_ist;
  rda *next;
  rda *prev;
  rda *sub_rda;
  rda *sup_rda;
};

struct condition{
  char *cond;
  condition *next;
  condition *prev;
};

struct decport{
  char *name;
  decport *next;
};

rda *create_rda(struct rda *iter, struct argument *iter_arg, struct instruction *iter_isp);
argument *create_arg(struct argument *arg);
instruction *create_isp(struct instruction *isp);
instruction *create_call(struct instruction *isp);
condition *add_cond(struct condition *cond,char *condi);
condition *del_cond(struct condition *cond);

#endif
