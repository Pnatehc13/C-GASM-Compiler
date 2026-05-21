#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "sema.h"


FILE* out;
int lc = 0;
void gen_code(Node* n);

void gen_return(Node* n) {
  if (n->ret.value) 
  {
    gen_code(n->ret.value);
    fprintf(out, "  STORE 0\n");
  }
  fprintf(out, "  RET\n");
}

void gen_cond(Node* cond, int my_id,char* l)
{
  if(!cond)return;
  gen_code(cond->bin.left); 
  gen_code(cond->bin.right);
  switch (cond->bin.op) {
    case '<':  
        fprintf(out, "  JGE %s%d\n",l ,my_id);break;
    case '>':  
        fprintf(out, "  JLE %s%d\n", l,my_id);break;
    case T_LEQ: 
        fprintf(out, "  JG %s%d\n",l, my_id);break;
    case T_GEQ: 
        fprintf(out, "  JL %s%d\n",l, my_id);break;
    case T_EQ:  
        fprintf(out, "  JNE %s%d\n",l, my_id);break;
    case T_NEQ: 
        fprintf(out, "  JE %s%d\n",l, my_id);break;
    default:
        fprintf(out, "  PUSH 0\n");
        fprintf(out, "  JE %s%d\n",l, my_id);
        break;
    }
}

void gen_while(Node* n)
{
  int id = lc++;
  fprintf(out, "LABEL .L_while_%d\n", id);
  gen_cond(n->flow.cond , id ,".L_end_");
  gen_code(n->flow.then_stmt);
  fprintf(out, "  JMP .L_while_%d\n", id);
  fprintf(out, "LABEL .L_end_%d\n", id);
}


void gen_if(Node* n)
{
  int id = lc++;
  if(n->flow.else_stmt) gen_cond(n->flow.cond , id,".L_else_");
  else gen_cond(n->flow.cond , id,".L_exit_");
  gen_code(n->flow.then_stmt);
  fprintf(out, "  JMP .L_exit_%d\n", id);
  
  if (n->flow.else_stmt) {
    fprintf(out, "LABEL .L_else_%d\n", id);
    gen_code(n->flow.else_stmt);
  }
  fprintf(out, "LABEL .L_exit_%d\n", id);
}


void gen_arg_rev(Node* n)
{
  if(!n)return;
  gen_arg_rev(n->next);
  gen_code(n);
}


void gen_node_call(Node* n)
{
  Node* a = n->func.args;
  int i = 0;
  while(a)
  {
    i++;
    a = a->next;
  }
  gen_arg_rev(n->func.args);
  fprintf(out,"  CALL %s\n",n->func.name);
  for(int j =0;j<i;j++)fprintf(out, "  POP\n");
  fprintf(out, "  LOAD 0\n");
}

void gen_node_assign(Node* n)
{
  gen_code(n->var.value);
  fprintf(out,"  POKEL %d\n",n->var.offset);
}


void gen_global_initializers(Node* global_list) 
{
  fprintf(out, "LABEL __init_globals\n");
  
  Node* curr = global_list;
  while(curr) {
    if(curr->type == NODE_GVAR && curr->gvar.value) {
      gen_code(curr->gvar.value);
      fprintf(out, "  PUSH [global_%s]\n", curr->gvar.name);
      fprintf(out, "  POKE\n");
    }
    curr = curr->next;
  }
  fprintf(out, "  RET\n");
}


void gen_gvar(Node* n)
{
  fprintf(out, "global_%s\n", n->gvar.name);
}


void gen_func(Node* n)
{
  fprintf(out, "FUNC %s\n", n->func.name);
  for(int i = 0; i < n->func.localvarbyte / 4; i++) { fprintf(out, "  PUSH 0\n");}
  if(n->func.body) { gen_code(n->func.body);}
  fprintf(out, "  RET\n");
}

void gen_var_access(Node* n, int is_store)
{
  char* name = gettokenname(&tokens[n->token_id]);
  int id = find_symbol(name);
  int is_global = (id != -1 && symtab[id].is_global);
  if (is_store) {
    if(is_global)
    {
      fprintf(out, "  PUSH [global_%s]\n", name);
      fprintf(out, "  POKE\n");
    }
    else
      fprintf(out, "  POKEL %d\n", n->var.offset);
  }  
  else 
  {
    if(is_global)
    {
      fprintf(out, "  PUSH [global_%s]\n", name);
      fprintf(out, "  PEEK\n");
    }
    else
      fprintf(out, "  PEEKL %d\n", n->var.offset);    
  }
}


void gen_code(Node* n)
{
  if(!n)return;
  switch(n->type)
  {
    case NODE_WHILE:
      gen_while(n);break;
    case NODE_IF:
      gen_if(n);break;
    case NODE_FUNC: 
      gen_func(n);break;
    case NODE_INT:
      fprintf(out, "  PUSH %ld\n", n->int_val);
      break;
    case NODE_BLOCK:
      gen_code(n->func.body);
      break;
    case NODE_VAR:
    {
      if(n->var.value)
      {
         gen_code(n->var.value);
         gen_var_access(n,1);
      }
      else 
        gen_var_access(n,0);
      break;
    }
    case NODE_BIN:
      gen_code(n->bin.left);  
      gen_code(n->bin.right);
      
      if (n->bin.op == '+')      fprintf(out, "  ADD\n");
      else if (n->bin.op == '-') fprintf(out, "  SUB\n");
      else if (n->bin.op == '*') fprintf(out, "  MUL\n");
      else if (n->bin.op == '/') fprintf(out, "  DIV\n");
      break;
        
    case NODE_ASSIGN:
      gen_code(n->var.value);
      gen_var_access(n,1);
      break;
    case NODE_RETURN:
      gen_return(n);break;
    case NODE_CALL:
      gen_node_call(n);
      break;
  }
  gen_code(n->next);
}

void init_code_gen(char* path)
{
  out = fopen(path,"w");
  if(!out) { perror("Failed to open output file"); return; }
  for(int i = 0; i < gt_count; i++) {
    if(global_table[i].p && global_table[i].p->type == NODE_GVAR) {
      gen_gvar(global_table[i].p);
    }
  }

  fprintf(out, "  CALL __init_globals\n");
  fprintf(out, "  CALL main\n");          
  fprintf(out, "  HALT\n");

  gen_global_initializers(global_table[0].p);
  
  for(int i =0;i<gt_count;i++)
  {
    if(global_table[i].p && global_table[i].p->type == NODE_FUNC)
      gen_code(global_table[i].p);
  }
  fclose(out);
}
