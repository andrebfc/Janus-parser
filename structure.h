#ifndef _STRUCTUREH_
#define _STRUCTUREH_

#include <stdio.h>

struct call_ist{
  char *type = NULL;
  char *arg = NULL;
  char *name = NULL;
};

struct instruction {
  char *type = NULL;
  char *l_value = NULL;
  char *r_value = NULL;
  char *op = NULL;
  int flag_sub = 0;
  call_ist *call;
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

rda *create_rda(struct rda *iter, struct argument *iter_arg, struct instruction *iter_isp);
argument *create_arg(struct argument *arg);
instruction *create_isp(struct instruction *isp);
instruction *create_call(struct instruction *isp);
condition *add_cond(struct condition *cond,char *condi);
condition *del_cond(struct condition *cond);

#endif
