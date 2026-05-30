#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "sema.h"

#define MAX_LOOP_NESTING 64
static int loop_stack[MAX_LOOP_NESTING];
static int lst = 0;

void push_loop(int id) {
    if (lst >= MAX_LOOP_NESTING - 1) {
        printf("Compiler Error: Loops nested too deeply!\n");
        exit(1);
    }
    loop_stack[++lst] = id;
}

int pop_loop() {
    if (lst == 0) return 0; // No active loop
    return loop_stack[lst--];
}
int peek_loop() {
    if (lst == 0) return 0;
    return loop_stack[lst];
}


FILE* out;
int lc = 1;
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
  if (n->flow.isfor) 
  {
    push_loop(-id);
  } 
  else 
  {
    push_loop(id);
  }
  fprintf(out, "LABEL .L_while_%d\n", id);
  gen_cond(n->flow.cond , id ,".L_end_");
  Node* stmt = n->flow.then_stmt->func.body;
  while(stmt)
  {
    if ( n->flow.isfor == 1 && stmt->next == NULL) 
    {
      fprintf(out, "LABEL .L_update_%d\n", id);
    }
    gen_code(stmt);
    stmt = stmt->next;
  }
  fprintf(out, "  JMP .L_while_%d\n", id);
  fprintf(out, "LABEL .L_end_%d\n", id);
  pop_loop();
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
  gen_arg_rev(n->func.args);
  if(strcmp(n->func.name , "printf") == 0)
  {
    fprintf(out, "  PRINT_STR\n");
  }
  else 
  {
    fprintf(out,"  CALL %s\n",n->func.name);
    for(int j =0;j<n->func.argcount;j++)fprintf(out, "  POP\n");
  }
  fprintf(out, "  LOAD 0\n");
}


void gen_global_initializers(Node* global_list) 
{
  fprintf(out, "LABEL __init_globals\n");
  
  Node* curr = global_list;
  while(curr) 
  {
    if(curr->type == NODE_GVAR && curr->gvar.value) 
    {
      fprintf(out, "  PUSH [global_%s]\n", curr->gvar.name);
      /*if (curr->gvar.value->type == NODE_ADDR) 
      {
        Node* target = curr->gvar.value->unary.expr;
        fprintf(out, "  PUSH [global_%s]\n", target->gvar.name); 
      } else 
      {
        gen_code(curr->gvar.value);
      }*/
      if(curr->gvar.value)gen_code(curr->gvar.value);
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


void gen_lvalue(Node* n)
{
  if(!n)return;
  switch(n->type)
  {
    case NODE_VAR:
    {
      char* name = gettokenname(&tokens[n->token_id]);
      int id = find_symbol(name);
      if (id != -1 && symtab[id].is_global)fprintf(out, "  PUSH [global_%s]\n", name);
      else 
      {
        fprintf(out,"  GETBP\n");
        fprintf(out,"  PUSH %d\n",n->var.offset);
        fprintf(out,"  SUB\n");
      }
      break;
    }
    case NODE_GVAR:
      fprintf(out,"  PUSH [global_%s]\n",n->gvar.name);
      break;
    case NODE_POINTER:
      gen_code(n->unary.expr);
      break;
    default:
      printf("Backend Error: Invalid lvalue write target (Type: %d)\n", n->type);
      exit(1);
  }
  
}


void gen_code(Node* n)
{
  if(!n)return;
  switch(n->type)
  {
    case NODE_WHILE:
      gen_while(n);break;
    case NODE_BREAK:
    {
      int active_id = peek_loop();
      if (active_id == 0) {
          printf("Backend Error: 'break' statement outside of any loop structure\n");
          exit(1);
      }
      if(active_id < 0 )fprintf(out, "  JMP .L_end_%d\n", -active_id);
      else fprintf(out, "  JMP .L_end_%d\n", active_id);
      break;
    }
    case NODE_CONTINUE:
    {
      int active_id = peek_loop();
      if (active_id == 0) {
          printf("Backend Error: 'continue' statement outside of any loop structure\n");
          exit(1);
      }
      else if(active_id < 0)fprintf(out, "  JMP .L_update_%d\n", -active_id);
      else fprintf(out, "  JMP .L_while_%d\n", active_id);
      break;
    }
    case NODE_IF:
      gen_if(n);break;
    case NODE_FUNC: 
      gen_func(n);break;
    case NODE_INT:
      fprintf(out, "  PUSH %ld\n", n->int_val);
      break;

    case NODE_BLOCK:
    {
      Node* stmt = n->func.body;
      while(stmt)
      {
        if (stmt->type == NODE_VAR && stmt->var.value == NULL) 
        {
          stmt = stmt->next;
          continue;
        }
        gen_code(stmt);
        stmt = stmt->next;
      }
      break;
    }
    case NODE_GVAR:
    {
      gen_var_access(n, 0); 
      break;
    }
    case NODE_VAR:
    {
      if (n->var.value) 
      {
         gen_code(n->var.value);
         gen_var_access(n, 1);
      }
      else
      {
        gen_var_access(n,0);
      }
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
    {
      gen_lvalue(n->bin.left);
      gen_code(n->bin.right);
      fprintf(out,"  POKE\n");
      break;
    }
    case NODE_STR:
    {
      fprintf(out, "  PUSH %s\n", n->str.string);
      break;
    }
    case NODE_RETURN:
      gen_return(n);break;
    case NODE_CALL:
      gen_node_call(n);
      break;
    case NODE_POINTER:
    {
      gen_lvalue(n);
      fprintf(out,"  PEEK\n");
      break;
    }
    case NODE_ADDR:
    {
      char* name = gettokenname(&tokens[n->unary.expr->token_id]);
      int id = find_symbol(name);
      if(symtab[id].is_global)
      {
        fprintf(out,"  PUSH [global_%s]\n",name);
      }
      else
      {
        fprintf(out,"  GETBP\n");
        fprintf(out,"  PUSH %d\n",symtab[id].offset);
        fprintf(out,"  SUB\n");
      }
      break;
    }
  }
}

void init_code_gen(char* path)
{
  out = fopen(path,"w");
  if(!out) { perror("Failed to open output file"); return; }
  Node* curr = NULL;
  int fgvar = 0;
  for(int i = 0; i < gt_count; i++) {
    if(global_table[i].p && global_table[i].p->type == NODE_GVAR) {
      if(curr == NULL)
      {
        fgvar = i;
        curr = global_table[i].p;
      }
      else
      {
        curr->next = global_table[i].p;
        curr = curr->next;
      }
      gen_gvar(global_table[i].p);
    }
  }

  fprintf(out, "  CALL __init_globals\n");
  fprintf(out, "  CALL main\n");          
  fprintf(out, "  HALT\n");

  gen_global_initializers(global_table[fgvar].p);
  
  for(int i =0;i<gt_count;i++)
  {
    if(global_table[i].p && global_table[i].p->type == NODE_FUNC)
      gen_code(global_table[i].p);
  }
  fclose(out);
}
