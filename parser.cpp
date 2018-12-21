#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "structure.h"

using namespace std;

const int SIZE = 100;

void get_token(char *token);
int get_instruction(char *token, char *tmp);
int get_sub_instruction(char post[]);
void str_cpy(char *dest, char *src, int to_start, int len);
int count_space(char *tmp);

typedef rda* ptr_rda;
typedef instruction* ptr_isp;
typedef argument* ptr_arg;
typedef condition* ptr_cond;
typedef decport *ptr_port;
ptr_rda iter = NULL;
ptr_rda head = NULL;
ptr_isp isp = NULL;
ptr_arg arg = NULL;
ptr_arg iter_arg = NULL;
ptr_isp iter_isp = NULL;
ptr_isp isp_assert = NULL;

ptr_cond iter_cond = NULL;
ptr_port port_h = NULL;
ptr_port port_t = NULL;

/*function for writing c file */
int declare_fun(struct rda *rda,struct argument *arg,int reverse); //flag = 0 forward, flag = 1 backward
int write_file(rda*,argument*,instruction*,int flag,int main_r);
int write_arg(struct argument *arg);
int write_instruction(struct instruction *isp,int flag);
int write_call(struct instruction *isp,int flag, int indent);
int write_forward(struct rda *iter,struct argument *arg, struct instruction *isp,int flag);
int write_reverse(rda*,argument*,instruction*,int flag,int main_flag);
int write_reverse_instruction(struct instruction *isp,int flag_indent,int main_flag);
void retrive_instruction(struct instruction *isp,char ist[],int reverse);
void write_program_thread(struct rda *search,const char *type);

/*function for common use*/
void swap_string(struct instruction *isp,char ist[]);
void reset_arr(char a[]);
void iter_argument_declare(struct argument *arg);

bool procedure_open = 0;
bool w_name_procedure = 0;
bool w_argument = 0;
bool w_call_arg = 0;
bool w_call_name = 0;
bool w_if_condition = 0;
bool w_assert = 0;
bool w_if_assert = 0;
bool w_if = 0;
bool w_else = 0;
bool main_r = 0;
bool w_comment = 0;
bool w_t_comment = 0;
bool w_fork_p1 = 0;
bool w_fork_p2 = 0;
bool write_fork = 0;
bool declare_thread = 0;
bool w_msg = 0;
bool w_port = 0;
bool w_dec_port = 0;
int count_procedure = 0;
int count_line = 0;
int countsl = 0;
int countax = 0;
int count_fork = 0;
int count_print_fork = 0;

const char delim[5] = " (),";
const char *procedure = "procedure";
const char *r_main = "main";
const char *r_int = "int";
const char *r_if = "if";
const char *then = "then";
const char *r_else = "else";
const char *fi = "fi";
const char *condition = "condition";
const char *r_assert = "assert";
const char *r_fork = "fork";
const char *r_andfork = "and";
const char *r_joinfork = "join";
const char *inita = "\t\n\tsem_init(&mutex,0,1);\n\tpthread_create(&p1,NULL,prog_";
const char *initb = ",NULL);\n\tpthread_create(&p2,NULL,prog_";
const char *initc = ",NULL);\n\tpthread_join(p1,NULL);\n\tpthread_join(p2,NULL);\n\tsem_destroy(&mutex);\n\n";
const char *r_ssend = "ssend";
const char *r_srcv = "srcv";
const char *r_asend = "asend";
const char *r_arcv = "arcv";
char *tmp_port;


FILE *out;
char tmp_if_condition[SIZE];
char if_condition[SIZE];

int main(int argc, char const *argv[]) {

  char *token;
  char tmp[SIZE];
  char riga_tmp[SIZE];
  int i = 0;
  int c;
  FILE *file;
  FILE *comment;
  char prog[30];
  if(argc == 3){
    //reverse main
    if(strcmp(argv[1],"-r")==0){
      main_r = 1;
      strcpy(prog,argv[2]);
    }
    else {
      fprintf(stderr,"Invalid option\n");
      exit(1);
    }
  }
  else if(argc == 2){
    strcpy(prog,argv[1]);
  }
  else if(argc < 1 || argc > 3){
    fprintf(stderr,"Error argument\n");
    exit(1);
  }
  file = fopen(prog,"r");
  comment = fopen("./comment.txt","w");

  if (file){
    while ((c=getc(file)) != EOF){
      if(c == '\n'){  //all line on tmp
        count_line++;
        tmp[i]='\0';
        if(w_dec_port){
          w_dec_port = 0;
        }
        // find a comment and save on comment file
        if(w_comment){
          w_comment = 0;
          fprintf(comment,"%s%d%s\n","comment at line ",count_line,tmp);
        }
        else if(w_t_comment){
          fprintf(comment,"%s%d%s\n","comment at line ",count_line,tmp);
        }
        //argument function close and procedure open
        else if(w_argument == 0 && procedure_open){
          strcpy(riga_tmp,tmp); // save a copy of tmp, strtok modify string
          token = strtok(tmp,delim);
          iter_isp = create_isp(iter_isp);
          if(iter->isp == NULL){
            iter->isp = iter_isp;
          }
          get_instruction(token,riga_tmp);
        }
        else{
          token = strtok(tmp, delim);
          get_token(token);
        }
        reset_arr(tmp);
        reset_arr(riga_tmp);
        i = 0;
      }
      //string "procedure" and name on tmp
      else if(c == 40 ){ // 40 = (
        if(w_comment!=1){
          tmp[i] = '\0';
          token = strtok(tmp,delim);
          get_token(token);
          reset_arr(tmp);
          i = 0;
        }
      }
      //argoment on tmp
      else if(c==41){ // 41 = )
        if(w_comment!=1){
          tmp[i] = '\0';
          if(w_call_arg){ //case call or uncall
            asprintf(&(iter_isp->call->arg),tmp);
            w_call_arg = 0;
          }
          else {
            w_argument = 1;
            token = strtok(tmp,delim);
            get_token(token);
            w_argument = 0;     // close wait argument
            procedure_open = 1; // open procedure
          }
          reset_arr(tmp);
          i = 0;
        }
      }
      //find a comment tag
      else if(c==47){ // 47 = /
        countsl++;
        if(countsl == 2){
          w_comment = 1;
          countsl = 0;
        }
        else if(countax == 2){
          w_t_comment = 0;
          countax = 0;
          countsl = 0;
          fprintf(comment,"%s%d%s\n","comment at line ",count_line,tmp);
          reset_arr(tmp);
        }
      }
      else if(c==42){ // 42 = *
        if(countsl == 1){
          w_t_comment = 1;
          countsl = 0;
        }
        countax++;
      }
      //copy all on tmp
      else{
        tmp[i]=c;
        i++;
      }
    }
    fclose(file);
  }
  else{
    fprintf(stderr,"%s\n","error open input file");
    exit(1);
  }
  iter = head;
  out = fopen("./outfile/out.cpp","w");
  if(out){
    if(declare_thread){
      fprintf(out,"%s\n%s\n%s\n%s\n%s\"%s\"\n\n","#include <stdio.h>","#include <assert.h>","#include <math.h>","#include <semaphore.h>","#include ","msgqueue.h");
      while(port_h != NULL){
        if(port_h->name != NULL && strcmp(port_h->name,"port")!=0){
          fprintf(out,"%s%s;\n","struct msgq *",port_h->name);
        }
        port_h = port_h->next;
      }
    }
    else{
      fprintf(out,"%s\n%s\n%s\n\n","#include <stdio.h>","#include <assert.h>","#include <math.h>");
    }
    //declaration function
    while(iter != NULL){
      if(strcmp(iter->name,r_main)!=0){
        declare_fun(iter,iter->arg,0);
        if(iter->arg != NULL){
          iter_argument_declare(iter->arg);
        }
        else{
          fprintf(out,"%s\n",");");
        }
        declare_fun(iter,iter->arg,1);
        if(iter->arg != NULL){
          iter_argument_declare(iter->arg);
        }
        else{
          fprintf(out,"%s\n",");");
        }
      }
	    iter = iter->next;
    }
    if(declare_thread){
      for(int i=0;i<count_fork;i++){
        fprintf(out,"%s%d%s\n","void *prog_",i,"(void *arg);");
      }
      fprintf(out,"\n");
    }
    iter = head;
    write_file(iter,iter->arg,iter->isp,0,main_r);//flag 0 for indent
    if(declare_thread){
      iter = head;
      for(int i=0;i<count_fork;i+=2){
        fprintf(out,"%s%d%s\n","void *prog_",i,"(void *arg){");
        write_program_thread(iter,r_fork);
        fprintf(out,"%s\n","}");
        fprintf(out,"%s%d%s\n","void *prog_",i+1,"(void *arg){");
        write_program_thread(iter,r_andfork);
        fprintf(out,"%s\n","}");
      }
    }
  }
  else{
    fprintf(stderr,"error to create out file");
  }
  fclose(out);
  return 0;
}

void get_token(char *token){
  while( token != NULL ) {
     // token is procedure
   if(strcmp(token,procedure)==0){
       count_procedure++;
       w_name_procedure = 1;
       procedure_open = 0;
     }
     // token is int
     else if(strcmp(token,r_int)==0){
       if(w_argument){
         iter_arg = create_arg(iter_arg);
         if(iter->arg == NULL){
           iter->arg = iter_arg;
         }
         asprintf(&(iter_arg->type),token);
       }
     }
     else if(strcmp(token,"local")==0){
     }
     else if(strcmp(token,"delocal")==0){
     }
     // token is call or uncall
     else if(strcmp(token,"call")==0 || strcmp(token,"uncall")==0){
       w_call_arg = 1;
       w_call_name = 1;
       iter_isp = create_call(iter_isp);
       if(iter->isp == NULL){
         iter->isp = iter_isp;
       }
       asprintf(&(iter_isp->call->type),token);
     }
     // token is if
     else if(strcmp(token,r_if)==0){
       iter_isp->flag_sub = 1;
       //create a sub rda
       if(iter->sub_rda == NULL){
         iter->sub_rda = create_rda(iter->sub_rda,iter_arg,iter_isp);
         iter->sub_rda->sup_rda = iter;
         iter->sub_rda->f_ist = iter_isp;
         iter = iter->sub_rda;
         iter_arg = iter->arg;
         iter_isp = iter->isp;
       }
       else {
         iter = iter->sub_rda;
         while(iter->next != NULL){
           iter = iter->next;
         }
         iter = create_rda(iter,iter_arg,iter_isp);
         iter->f_ist = iter_isp;
         iter_arg = iter->arg;
         iter_isp = iter->isp;
       }
       asprintf(&(iter->name),token);
       w_if_condition = 1;
     }
     // token is then
     else if(strcmp(token,then)==0){
       iter_isp = create_isp(iter_isp);
       if(iter->isp == NULL){
         iter->isp = iter_isp;
       }
       get_sub_instruction(tmp_if_condition);
       asprintf(&(iter_isp->type),condition);
       reset_arr(tmp_if_condition);
       w_if_condition = 0;
     }
     // token is else
     else if(strcmp(token,r_else)==0){
       iter = create_rda(iter,iter_arg,iter_isp);
       iter_arg = iter->arg;
       iter_isp = iter->isp;
       asprintf(&(iter->name),r_else);
     }
     // token is fi
     else if(strcmp(token,fi)==0){
       isp_assert = iter->prev->isp;
       while(isp_assert->next != NULL){
         isp_assert = isp_assert->next;
       }
       w_if_assert = 1;
       asprintf(&(isp_assert->type),r_assert);
       asprintf(&(iter_isp->type),r_assert);
       w_assert = 1;
     }
     //token is fork
     else if(strcmp(token,r_fork)==0){
       declare_thread = 1;
       count_fork++;
       iter_isp->flag_sub = 1;
       //create a sub rda
       if(iter->sub_rda == NULL){
         iter->sub_rda = create_rda(iter->sub_rda,iter_arg,iter_isp);
         iter->sub_rda->sup_rda = iter;
         iter->sub_rda->f_ist = iter_isp;
         iter = iter->sub_rda;
         iter_arg = iter->arg;
         iter_isp = iter->isp;
       }
       else {
         iter = iter->sub_rda;
         while(iter->next != NULL){
           iter = iter->next;
         }
         iter = create_rda(iter,iter_arg,iter_isp);
         iter->f_ist = iter_isp;
         iter_arg = iter->arg;
         iter_isp = iter->isp;
       }
       asprintf(&(iter->name),token);
       //reset_arr(tmp_if_condition);
       w_fork_p1 = 1;
     }
     //token is and
    else if(strcmp(token,r_andfork)==0){
      //w_fork_p1 = 0;
      //reset_arr(tmp_if_condition);
      count_fork++;
      iter = create_rda(iter,iter_arg,iter_isp);
      iter_arg = iter->arg;
      iter_isp = iter->isp;
      asprintf(&(iter->name),r_andfork);
      //w_fork_p2 = 1;
    }
    else if(strcmp(token,r_joinfork)==0){
    }
    // token is ssend srcv asend arcv
    else if(strcmp(token,r_ssend)==0 || strcmp(token,r_srcv)==0 || strcmp(token,r_asend)==0 || strcmp(token,r_arcv)==0){
      if(iter->isp == NULL){
        iter->isp = create_isp(iter->isp);
        iter_isp = iter->isp;
      }
      //asprintf(&(iter_isp->type),token);
      iter_isp->msg = new msgch;
      asprintf(&(iter_isp->msg->type),token);
      w_msg = 1;
    }
     // token not know
     else {
       // wait procedure name
       if(w_name_procedure){
        iter = create_rda(iter,iter_arg,iter_isp);
        iter_arg = iter->arg;
        iter_isp = iter->isp;
        if(count_procedure == 1){
          head = iter;
        }
        asprintf(&(iter->name),token);
        w_name_procedure = 0;
       }
       //wait port name
       else if(w_dec_port){
         if(port_t == NULL){
           port_t = new decport;
           port_t->next = NULL;
           port_h = port_t;
         }
         else{
           port_t->next = new decport;
           port_t = port_t->next;
           port_t->next = NULL;
         }
         asprintf(&(port_t->name),token);
       }
       // wait argument
       else if(w_argument){
         if(w_msg){
            //iter_isp->msg = new msgch;
            asprintf(&(iter_isp->msg->value),token);
            w_msg = 0;
            w_port = 1;
         }
         else if(w_port){
           asprintf(&(iter_isp->msg->port),token);
           w_port = 0;
         }
         else {
         asprintf(&(iter_arg->name),token);
        }
       }
       // wait call or uncall name function
       else if(w_call_name){
         asprintf(&(iter_isp->call->name),token);
         w_call_name = 0;
       }
       // wait if condition
       else if(w_if_condition){
         strcat(tmp_if_condition,token);
       }
       // wait assert
       else if(w_assert){
         strcat(tmp_if_condition,token);
       }
       // wait program
       else if (w_fork_p1){
         strcat(tmp_if_condition,token);
       }
       // token not know and not wait anything
       else {
         fprintf(stderr,"%s%d\n","Error invalid token at line: ",count_line);
         exit(1);
       }
     }
     token = strtok(NULL, delim);
  }
}

int get_instruction(char *token, char *tmp){
  int count = 0;
  char post[SIZE];
  while(token != NULL){
    // find a new procedure, close previus procedure
    if(strcmp(token,procedure)==0){
      procedure_open = 0;
      get_token(token);
      return 1;
    }
    else if(strcmp(token,"print")==0){
      asprintf(&(iter_isp->type),token);
      count = count_space(tmp) + 6; //6 = print + space
      str_cpy(post,tmp,count,strlen(tmp));
      get_sub_instruction(post);
      reset_arr(post);
      count = 0;
      return 1;
    }
    //token is port
    else if(strcmp(token,"port")==0){
       w_dec_port = 1;
       port_h = new decport;
       port_t = port_h;
       get_token(token);
       return 1;
    }
    // find int instruction
    else if(strcmp(token,r_int)==0){
      asprintf(&(iter_isp->type),token);
      count = count_space(tmp) + 4; //4 = int + space
      str_cpy(post,tmp,count,strlen(tmp)); // read post int
      get_sub_instruction(post);
      reset_arr(post);
      count = 0;
      return 2;
    }
    else if(strcmp(token,"local")==0){
      asprintf(&(iter_isp->type),r_int);
      iter_isp->loc = 1;
      count = count_space(tmp) + 10;//4 int + 6 local
      str_cpy(post,tmp,count,strlen(tmp)); // read post int
      get_sub_instruction(post);
      reset_arr(post);
      count = 0;
      return 1;
    }
    else if(strcmp(token,"delocal")==0){
      asprintf(&(iter_isp->type),r_int);
      iter_isp->loc = 2;
      count = count_space(tmp) + 12;//4 int + 8 delocal
      str_cpy(post,tmp,count,strlen(tmp)); // read post int
      get_sub_instruction(post);
      reset_arr(post);
      count = 0;
      return 2;
    }
    // find if instruction
    else if(strcmp(token,r_if)==0){
      get_token(token);
      return 3;
    }
    // find else instruction
    else if(strcmp(token,r_else)==0){
      get_token(token);
      return 4;
    }
    // find fi instruction
    else if(strcmp(token,fi)==0){
      get_token(token);
      get_sub_instruction(tmp_if_condition);
      reset_arr(tmp_if_condition);
      w_assert = 0;
      w_if_assert = 0;
      isp_assert = NULL;
      // return at first rda element
      while(iter->prev != NULL){
        iter = iter->prev;
      }
      iter = iter->sup_rda;
      iter_isp = iter->isp;
      while(iter_isp->next != NULL){
        iter_isp = iter_isp->next;
      }
      //iter_arg = iter->arg;
      return 5;
    }
    // find fork
    else if(strcmp(token,r_fork)==0){
      get_token(token);
      return 6;
    }
    else if(strcmp(token,r_andfork)==0){
      get_token(token);
      return 7;
    }
    else if(strcmp(token,r_joinfork)==0){
      // return at first rda element
      while(iter->prev != NULL){
        iter = iter->prev;
      }

      iter = iter->sup_rda;
      iter_isp = iter->isp;
      while(iter_isp->next != NULL){
        iter_isp = iter_isp->next;
      }
      w_fork_p1 = 0;
      return 8;
    }
    else if(strcmp(token,r_ssend)==0 || strcmp(token,r_srcv)==0 || strcmp(token,r_asend)==0 || strcmp(token,r_arcv)==0){
      w_msg = 1;
    }
    // get sub instruction
    else {
      get_sub_instruction(tmp);
      return 9;
    }
    token = strtok(NULL, delim);
  }
  return 0;
}

int get_sub_instruction(char post[]){
  int l_or_r_value = 0;
  int no_r_value = 1;
  char l_value[10];
  char r_value[10];
  char operazione[4];
  int j = 0;
  int h = 0;
  int k = 0;
  for(int i = 0; i < strlen(post);i++){ // 43 -> +, 45 -> -, 60 -> <, 61 -> =, 62 -> >
    if(post[i] == 43 || post[i] == 45 || post[i] == 60 || post[i] == 61 || post[i] == 62){
      //previus operator copy on l_value
      operazione[k] = post[i];
      k++;
      l_or_r_value = 1;
    }
    else if(post[i] == 32){ // 32 = space
      //nothing
    }
    else{
      if(l_or_r_value == 0){      //if not find operator copy all on l_value
        l_value[j] = post[i];
        j++;
      }
      else {                      //if find operator copy all on r_value;
        r_value[h] = post[i];
        h++;
      }
    }
  }
  l_value[j] = '\0';
  r_value[h] = '\0';
  operazione[k] = '\0';
  //insert on list l_value
  asprintf(&(iter_isp->l_value),l_value);
  if(w_if_assert){
    asprintf(&(isp_assert->l_value),l_value);
    asprintf(&(isp_assert->r_value),r_value);
  }
  //insert on list operation
  if(w_if_condition || w_assert){
    if(strcmp(operazione,"=")==0){
        asprintf(&(iter_isp->op),"==");
        if(w_if_assert){
          asprintf(&(isp_assert->op),"==");
        }
    }
  }
  else {
    asprintf(&(iter_isp->op),operazione);
  }
  //insert on list r_value
  asprintf(&(iter_isp->r_value),r_value);
}

// copy with to start
void str_cpy(char *dest,char *src, int to_start,int len){
  int j = 0;
  for(int i=to_start;i<len;i++){
    dest[j] = src[i];
    j++;
  }
  dest[j]='\0';
}
//return number of space
int count_space(char *tmp){
  int i = 0;
  while(tmp[i] == ' '){
    i++;
  }
  return i;
}
// for opeartor swap <=>
void swap_string(struct instruction *isp,char ist[]){
  strcat(ist," ^= ");
  strcat(ist,isp->r_value);
  strcat(ist,";\n");
  strcat(ist,"\t\t");
  strcat(ist,isp->r_value);
  strcat(ist," ^= ");
  strcat(ist,isp->l_value);
  strcat(ist,";\n\t\t");
  strcat(ist,isp->l_value);
  strcat(ist," ^= ");
}

/*function for writing c file*/

int write_file(struct rda *iter,struct argument *arg, struct instruction *isp,int flag,int main_r){
  if(iter == NULL){
    return 0;
  }
  while (iter != NULL){
    if(strcmp(iter->name,r_main)!= 0){
      isp = iter->isp;
      arg = iter->arg;
      write_forward(iter,arg,isp,flag);
      isp = iter->isp;
      arg = iter->arg;
      write_reverse(iter,arg,isp,flag,0);
    }
    // write main
    else {
      if(main_r == 0){
        isp = iter->isp;
        arg = iter->arg;
        write_forward(iter,arg,isp,flag);
      }
      //write main reverse
      else{
        isp = iter->isp;
        arg = iter->arg;
        write_reverse(iter,arg,isp,flag,1);
      }
    }
    iter = iter->next;
  }
  return 0;
}

// write a declaration function
int declare_fun(struct rda *rda,struct argument *arg,int reverse){
  char declare[SIZE];
  if(rda == NULL){
    return 0;
  }
  //fork case
  else if(strcmp(rda->name,r_fork)==0 || strcmp(rda->name,r_andfork)==0){
    return 0;
  }
  //write main
  else if(strcmp(rda->name,r_main)==0){
      strcpy(declare,"int main(int argc, char const *argv[]) {\n");
      fprintf(out,"%s",declare);
    return 0;
  }
  // write if then else constructor
  else if(strcmp(rda->name,r_if)==0){
    strcpy(declare,"\tif(");
    fprintf(out,"%s",declare);
    return 0;
  }
  else if(strcmp(rda->name,r_else)==0){
    strcpy(declare,"\telse {\n");
    fprintf(out,"%s",declare);
    return 0;
  }
  // write a declaration function procedure
  else{
    if(strcmp(rda->name,r_main)!=0){
      strcpy(declare,"void ");
      strcat(declare,rda->name);
      if(reverse == 0){
        strcat(declare,"_forward");
      }
      else{
        strcat(declare,"_reverse");
      }
      strcat(declare,"(");
      fprintf(out,"%s",declare);
    }
    return 0;
  }
}

// write argument for function
int write_arg(struct argument *arg){
  if(arg != NULL){
    char argument[SIZE];
    strcpy(argument,arg->type);
    strcat(argument," &");
    strcat(argument,arg->name);
    fprintf(out,"%s",argument);
  }
}

//write instruction for function forward
int write_instruction(struct instruction *isp,int flag){
  int close_ = 0;
  char istruzione[SIZE];
  strcpy(istruzione,"\t");

  if(isp->type != NULL){
    if (strcmp(isp->type,condition)==0){
      retrive_instruction(isp,istruzione,0);
      //strcpy(if_condition,istruzione);//insert if condition for assert on else reverse
      iter_cond = add_cond(iter_cond,istruzione);
      strcat(istruzione,"){\n");
      fprintf(out,"%s",istruzione);
      return 0;
    }

    else if(strcmp(isp->type,r_assert)==0){
        if(w_if){
          strcat(istruzione,"\tassert(");
          close_ = 1;
          w_if = 0;
        }
        else {
          strcat(istruzione,"\tassert(!(");
          close_ = 2;
        }
    }
    else if(strcmp(isp->type,"print")==0){
      strcat(istruzione,isp->type);
      strcat(istruzione,"f(\"%d\\n\",");
    }
    else if (strcmp(isp->type,r_int)==0){
      if(isp->loc == 2){
          strcat(istruzione,"assert(");
      }
      else {
        strcat(istruzione,isp->type);
        strcat(istruzione," ");
      }
    }
  }
  if(flag == 1){
    strcat(istruzione,"\t");
  }
  retrive_instruction(isp,istruzione,0);
  if(close_ == 1){
    strcat(istruzione,")");
    close_ = 0;
    w_if = 0;
  }
  else if(close_ == 2){
    strcat(istruzione,"))");
    //reset_arr(if_condition);
    close_ = 0;
  }
  if(isp->loc == 2){
    strcat(istruzione,")");
  }
  if(strcmp(istruzione,"\t")==0 || strcmp(istruzione,"\t\t")==0){
    return 1;
  }
  else {
    fprintf(out,"%s;\n",istruzione);
    return 0;
  }
}


// write function forward
int write_forward(struct rda *iter,struct argument *arg, struct instruction *isp,int flag){
  declare_fun(iter,arg,0);
  if(arg == NULL && strcmp(iter->name,r_main)!=0 && strcmp(iter->name,r_if)!=0 && strcmp(iter->name,r_else)!=0){
    fprintf(out,"%s\n","){");
  }
  while(arg != NULL){
    write_arg(arg);
    if(arg->next == NULL){
      fprintf(out,"%s %s\n",")","{");
    }
    else{
      fprintf(out,"%s",", ");
    }
    arg = arg->next;
  }
  while(isp != NULL){
    //find sub rda
    if(isp->flag_sub == 1){
      ptr_rda rdatmp = iter->sub_rda;
      ptr_isp isptmp;
      // search rda
      while(rdatmp->f_ist != isp){
        rdatmp = rdatmp->next;
      }
      if(strcmp(rdatmp->name,r_fork)==0){
        fprintf(out,"\t%s%d%s",inita,count_print_fork,initb);
        count_print_fork++;
        fprintf(out,"%d%s",count_print_fork,initc);
        count_print_fork++;
      }
      else {
        for(int i = 0; i < 2;i++){
          isptmp = rdatmp->isp;
          // if case
          if(strcmp(rdatmp->name,r_if)==0){
            w_if = 1;
            flag = 1;
          }
        write_forward(rdatmp,NULL,isptmp,flag);//flag 1
        if(rdatmp->next != NULL){
          rdatmp = rdatmp->next;
        }
        else rdatmp = NULL;
        }
      }
    }
    write_instruction(isp,flag);
    if(isp->call != NULL){
      if(flag == 1){
        write_call(isp,0,1);//0 = forward, 1 = 1+indent
      }
      else {
        write_call(isp,0,0);//0 = forward, 0 = indent
      }
    }
    isp = isp->next;
  }
  if(flag == 1){
    fprintf(out,"\t%s\n\n","}");
  }
  else if(flag == 2){
    return 0;
  }
  else{
    fprintf(out,"%s\n\n","}");
  }
  return 0;
}

// write function reverse
int write_reverse(struct rda *iter,struct argument *arg,struct instruction *isp,int flag,int main_flag){
  declare_fun(iter,arg,1);
  if(arg == NULL && strcmp(iter->name,r_main)!=0 && strcmp(iter->name,r_if)!=0 && strcmp(iter->name,r_else)!=0){
    fprintf(out,"%s\n","){");
  }
  while(arg != NULL){
    write_arg(arg);
    if(arg->next == NULL){
      fprintf(out,"%s %s\n",")","{");
    }
    else{
      fprintf(out,"%s",", ");
    }
    arg = arg->next;
  }
  if(main_flag == 1){
    ptr_isp int_isp = isp;
    while(int_isp->next != NULL){
      if (int_isp->type != NULL){
        if (strcmp(int_isp->type,r_int)==0){
          write_instruction(int_isp,flag);
        }
      }
      int_isp = int_isp->next;
    }
  }

  ptr_isp head_isp = isp;
  while (isp->next != NULL){
    isp = isp->next;
  }//isp on the tail
  while(isp != head_isp){
    // find sub rda
    if(isp->flag_sub == 1){
      ptr_rda rdatmp = iter->sub_rda;
      ptr_isp isptmp;
      //search rda
      while(rdatmp->f_ist != isp){
        rdatmp = rdatmp->next;
      }
      //fork case
      if(strcmp(rdatmp->name,r_fork)==0){
        if(count_print_fork == count_fork){
          count_print_fork--;
        }
        fprintf(out,"%s%d%s",inita,count_print_fork,initb);
        count_print_fork--;
        fprintf(out,"%d%s",count_print_fork,initc);
        count_print_fork--;
      }
      else if(strcmp(rdatmp->name,r_andfork)==0){
      }
      //if/else case
      else {
        for(int i = 0;i < 2;i++){
          if(strcmp(rdatmp->name,r_else)==0){
            w_else = 1;
          }
          isptmp = rdatmp->isp;
          write_reverse(rdatmp,NULL,isptmp,1,main_flag);//flag 1 = reverse
          rdatmp = rdatmp->next;
        }
      }
    }
    // find call on sub rda
    if(isp->call != NULL){
      write_call(isp,1,0);//flag 1 = reverse, 1 = 1 + indent
    }
    write_reverse_instruction(isp,flag,main_flag);
    isp = isp->prev;
  }
  if(isp->call != NULL){
    if(flag == 1){
      write_call(isp,1,1);
    }
    else {
      write_call(isp,1,0);//flag 1 = reverse, 0 = indent
    }
  }
  if (isp == head_isp){
      write_reverse_instruction(isp,flag,main_flag);// first instruction
      if(w_else){//else assert
        //fprintf(out,"\t\tassert(!(%s));\n",if_condition);
        fprintf(out,"\t\tassert(!(%s));\n",iter_cond->cond);
        //reset_arr(if_condition);
        if(iter_cond->prev != NULL){
          iter_cond = iter_cond->prev;
          iter_cond->next = NULL;
        }
        w_else = 0;
      }
  }
  if(flag == 1){
      fprintf(out,"\t%s\n\n","}");
  }
  else{
      fprintf(out,"%s\n\n","}");
  }
}

int write_reverse_instruction(struct instruction *isp,int flag,int main_flag){ //0=1 tab, 1=2 tab, .....
  char istruzione[SIZE];
  strcpy(istruzione,"\t");

  if(isp->type != NULL){
    // find if condition -> assert
    if (strcmp(isp->type,condition)==0){
      strcat(istruzione,"\tassert(");
      retrive_instruction(isp,istruzione,1);
      strcat(istruzione,");\n");
      fprintf(out,"%s",istruzione);
      return 0;
    }
    else if(strcmp(isp->type,r_assert)==0){
      if(w_else){
        return 0;
      }
      else {
        retrive_instruction(isp,istruzione,1);
      }
      strcat(istruzione,"){\n");
      fprintf(out,"%s",istruzione);
      return 0;
    }
    else if(strcmp(isp->type,"print")==0){
      strcat(istruzione,isp->type);
      strcat(istruzione,"f(\"%d\\n\",");
    }
    else if (strcmp(isp->type,r_int)==0){
      if(main_flag == 1){ // not write declare var on reverse main
        return 0;
      }
      if(isp->loc == 1){
          strcat(istruzione,"assert(");
      }
      else {
        strcat(istruzione,isp->type);
        strcat(istruzione," ");
      }
    }
  }
  if(flag == 1){
    strcat(istruzione,"\t");
  }
  retrive_instruction(isp,istruzione,1);
  if(isp->loc == 1){
    strcat(istruzione,")");
  }
  if(strcmp(istruzione,"\t")==0 || strcmp(istruzione,"\t\t")==0){//if instruction is empty
    return 1;
  }
  else {
    fprintf(out,"%s;\n",istruzione);
    return 0;
  }
}

void retrive_instruction(struct instruction *isp,char ist[],int reverse){
  if(isp->l_value != NULL && strcmp(isp->l_value," ")!=0 && strcmp(isp->l_value,"")!=0){
    strcat(ist,isp->l_value);
  }
  if (reverse == 1 && isp->op != NULL && strcmp(isp->op," ")!=0 && strcmp(isp->op,"")!=0){
    if(strcmp(isp->op,"+=")==0){
      strcat(ist," -= ");
    }
    else if(strcmp(isp->op,"-=")==0){
      strcat(ist," += ");
    }
    else if(strcmp(isp->op,"<=>")==0){
      swap_string(isp,ist);
    }
    else{
      strcat(ist," ");
      if(isp->loc == 1){ // local on reverse is assert
        strcat(ist,"=");
      }
      strcat(ist,isp->op);
      strcat(ist," ");
    }
  }
  else if(isp->op != NULL && strcmp(isp->op," ")!=0 && strcmp(isp->op,"")!=0){
    if(strcmp(isp->op,"<=>")==0){
      swap_string(isp,ist);
    }
    else{
    strcat(ist," ");
    strcat(ist,isp->op);
      if(isp->loc == 2){ //&& strcmp(isp->op,"=")==0){
        strcat(ist,"=");
      }
    strcat(ist," ");
    }
  }
  else if(isp->l_value != NULL && (isp->op == NULL || strcmp(isp->op," ")==0 || strcmp(isp->op,"")==0)){//case int x
    if(isp->loc == 2 && reverse != 1){
      strcat(ist," == 0");
    }
    else if(strcmp(isp->type,"print")==0){
      strcat(ist,")");
    }
    else {
      strcat(ist," = 0");
    }
  }

  if(isp->r_value != NULL && strcmp(isp->r_value," ")!=0 && strcmp(isp->r_value,"")!=0){
    strcat(ist,isp->r_value);
  }
}

// write call forward and reverse
int write_call(struct instruction *isp,int reverse, int indent){
  char call[SIZE];
  if(indent == 1){
    strcpy(call,"\t\t");
  }
  else {
    strcpy(call,"\t");
  }
  if(strcmp(isp->call->type,"call")==0){
    strcat(call,isp->call->name);
    if(reverse == 0){
      strcat(call,"_forward(");
    }
    else {
      strcat(call,"_reverse(");
    }
  }
  else if(strcmp(isp->call->type,"uncall")==0){
    strcat(call,isp->call->name);
    if(reverse == 0){
      strcat(call,"_reverse(");
    }
    else{
      strcat(call,"_forward(");
    }
  }
  strcat(call,isp->call->arg);
  strcat(call,");\n");
  fprintf(out,"%s",call);
}

void reset_arr(char a[]){
  for(int i = 0; i < SIZE; i++){
    a[i]=0;
  }
}

void iter_argument_declare(struct argument *arg){
  ptr_arg decarg = arg;
  while(decarg != NULL){
    write_arg(decarg);
    if(decarg->next != NULL){
      fprintf(out,"%s",", ");
    }
    else {
      fprintf(out,"%s\n",");");
    }
    decarg = decarg->next;
  }
}
// write function thread
void write_program_thread(struct rda *search,const char *type){
  while(search != NULL){
    if(strcmp(search->name,type)==0 && search->wr == 0){
      search->wr = 1;
      ptr_isp isp = search->isp;
      while(isp != NULL){
        write_instruction(isp,0);
        if(isp->call != NULL){
          write_call(isp,0,0);//0 = forward, 1 = 1+indent
        }
        if(isp->msg != NULL){

          if(strcmp(type,r_fork)==0){
            asprintf(&tmp_port,isp->msg->port);
            //fprintf(out,"\t%s%s%s,%s,%s%s\n",isp->msg->type,"(",isp->msg->value,"&head","P1",");");
            fprintf(out,"\t%s(%s,&%s,%s);\n",isp->msg->type,isp->msg->value,isp->msg->port,"P1");
          }
          else{
            //check port
            if(strcmp(tmp_port,isp->msg->port)!=0){
              fprintf(stderr,"error port\n");
              exit(1);
            }
            //fprintf(out,"\t%s%s%s,%s,%s%s\n",isp->msg->type,"(",isp->msg->value,"&head","P2",");");
            fprintf(out,"\t%s(%s,&%s,%s);\n",isp->msg->type,isp->msg->value,isp->msg->port,"P2");
          }
        }
        isp = isp->next;
      }
      return;
    }
    if(search->sub_rda != NULL){
      write_program_thread(search->sub_rda,type);
    }
    search = search->next;
  }
}
