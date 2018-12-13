#include "structure.h"

rda *create_rda(struct rda *it, struct argument *argu, struct instruction *istp){
  if(it != NULL){
    it->next = new rda;
    it->next->prev = it;
    it = it->next;
    it->next = NULL;
    it->arg = NULL;
    it->isp = NULL;
    it->sub_rda = NULL;
    it->sup_rda = NULL;
    it->f_ist = NULL;
    istp = it->isp;
    argu = it->arg;
  }
  else{
    it = new rda;
    it->next = NULL;
    it->prev = NULL;
    it->arg = NULL;
    it->isp = NULL;
    it->sub_rda = NULL;
    it->sup_rda = NULL;
    istp = it->isp;
    argu = it->arg;
  }
  return it;
}

argument *create_arg(struct argument *arg){
  if(arg != NULL){
    arg->next = new argument;
    arg = arg->next;
    arg->next = NULL;
    return arg;
  }
  else {
    arg = new argument;
    arg->next = NULL;
    //iter->arg = iter_arg;
    return arg;
  }
}

instruction *create_isp(struct instruction *isp){
  if(isp != NULL){
    isp->next = new instruction;
    isp->next->prev = isp;
    isp = isp->next;
    isp->next = NULL;
  }
  else {
    isp = new instruction;
    isp->next = NULL;
    isp->prev = NULL;
  }
  return isp;
}

instruction *create_call(struct instruction *isp){
  if(isp != NULL){
    isp->call = new call_ist;
    return isp;
  }
  else{
    isp = new instruction;
    isp->next = NULL;
    isp->call = new call_ist;
    return isp;
  }
}

condition *add_cond(struct condition *cond,char *condi){
  if(cond !=NULL){
    cond->next = new condition;
    cond->next->prev = cond;
    cond = cond->next;
    asprintf(&(cond->cond),condi);
    cond->next = NULL;
    return cond;
  }
  else {
    cond = new condition;
    asprintf(&(cond->cond),condi);
    cond->next = NULL;
    cond->prev = NULL;
    return cond;
  }
}

condition *del_cond(struct condition *cond){
  if(cond != NULL){
    if(cond->prev != NULL){
      cond = cond->prev;
      cond->next = NULL;
    }
    else cond = NULL;
    return cond;
  }
  else return NULL;
}
