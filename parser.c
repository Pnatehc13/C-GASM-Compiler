#include "./parser.h"
#include <stdio.h>
#include "./lexer.h"
#include <stdlib.h>
#include <string.h>

int tp1 = 0;
int gbr = 0x100000;

Token* peek()
{
  return &tokens[tp1];
}

int match(int t)
{
  if(t == tokens[tp1].type)return 1;
  return 0;
}

Token* advance()
{
  if(tokens[tp1].type != T_EOF)
  {
    tp1++;
    printf("Advancing from %d\n",tokens[tp1-1].type);
    return &tokens[tp1-1];
  }
  //need to handel the else part 
}
Token* consume(int t)
{
  if(match(t))
  {
    tp1++;
    printf("consuming %d\n",t);
    return &tokens[tp1-1];  
  }
  printf("Parser Error: Expected token type %d but got %d at index %d\n", t, tokens[tp1].type, tp1);
  exit(1);
  //need to handle the error part properly instead of just exiting 
}

void add_to_symtab(Token* t, int bp, int isg, int gt)
{
  strcpy(symtab[sp].name, gettokenname(t));
  symtab[sp].type = t->type;
  symtab[sp].offset = bp;
  symtab[sp].gt_index = gt;
  symtab[sp].is_global = isg;
  sp++;
}

Node* new_node(NodeType t) {
    Node* node = malloc(sizeof(Node)); 
    node->type = t;
    node->next = NULL;
    node->func.args = NULL;
    return node;
}

Node* new_bin_node(int op, Node* left, Node* right) {
    Node* node = new_node(NODE_BIN);
    node->bin.op = op;
    node->bin.left = left;
    node->bin.right = right;
    return node;
}

int is_type(int t)
{
  if(t == K_INT || t == K_CHAR || t == K_VOID )return 1;
  return 0;
}

void append_node(Node* n)
{
  if (n->type == NODE_FUNC || n->type == NODE_GVAR) 
  {
    strcpy(global_table[gt_count].name, n->func.name);
    global_table[gt_count].p = n;
    global_table[gt_count].id = gt_count;
    gt_count++;
  }
  else 
  {
    printf("Error : Type not Listed \n");
  }
}


int find_symbol(char* name) 
{
  for (int i = sp - 1; i >= 0; i--) 
  {
    if (strcmp(symtab[i].name, name) == 0) return i; 
  }
  return -1; 
}

int get_precedence(int t) {
  switch(t) {
    case ')':
    case ';': 
    case '}':
    case 0:
      return 0;
    case '(':
    case '[':
    case T_ARROW:  
      return 80;
    case '*':
    case '/':
    case '%':
      return 60;
    case '+':
    case '-':
      return 40;
    case '<':
    case '>':
    case T_LEQ:   
    case T_GEQ:   
      return 30;
    case T_EQ:     
    case T_NEQ:    
      return 20;
    case '=':
      return 10;
    
    default:
      return 0;
  }
}


Node* nud()
{
  printf("Got into nud!\n");
  Token* t = peek();
  if(t->type == T_INT)
  {
    
    Node* n = new_node(NODE_INT);
    n->token_id = t->ind;
    n->int_val = atoi(gettokenname(t));
    advance();
    
    return n;
  }
  if(t->type == T_IDENTIFIER)
  {  
    printf("in nud -> is T_IDENTIFIER\n");
    Node* n = new_node(NODE_VAR);
    n->token_id = tp1;
    int id = find_symbol(gettokenname(t));
    n->var.offset = symtab[id].offset;
    advance();
    return n;
  }
  if(t->type == '"' || t->type == T_STRING)
  {
    printf("in nud -> is T_STRING\n");
    Node* n = new_node(NODE_STR);
    n->token_id = t->ind;     
    advance(); 
    return n;
  }
}

void print_tree(Node* n, int depth) {
    if (!n) return;

    // Indentation for scannable visualization
    for (int i = 0; i < depth; i++) printf("  ");
    printf("DEBUG: Node Type is %d | ", n->type);

    switch (n->type) {
        case NODE_FUNC:
            printf("[FUNCTION DEF] Name: %s (Ret Type: %d)\n", n->func.name, n->func.returntype);
            if (n->func.args) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("Params:\n");
                print_tree(n->func.args, depth + 2);
            }
            print_tree(n->func.body, depth + 1);
            break;

        case NODE_INT:
            printf("[INT] Value: %ld\n", n->int_val);
            break;

        case NODE_GVAR:
            printf("[GVAR] Name: %s Type: %d Offset: 0x%X (TokenID: %d)\n", n->gvar.name, n->gvar.dt, n->gvar.offset, n->token_id);
            if (n->gvar.value) print_tree(n->gvar.value, depth + 1);
            break;

        case NODE_VAR:
            printf("[VAR] DataType: %d | Offset: %d (TokenID: %d)\n", n->var.dt, n->var.offset, n->token_id);
            if (n->var.value) print_tree(n->var.value, depth + 1);
            break;

        case NODE_BIN:
            printf("[BIN_OP] Operator: '%c' (Type: %d)\n", n->bin.op, n->bin.op);
            print_tree(n->bin.left, depth + 1);
            print_tree(n->bin.right, depth + 1);
            break;

        case NODE_CALL:
            printf("[CALL] Function: %s\n", n->func.name);
            if (n->func.args) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("Args:\n");
                print_tree(n->func.args, depth + 2);
            }
            break;

        case NODE_BLOCK:
            // Blocks just hold the head of a statement list. 
            // We just print a marker and let the sequential chain print the children.
            printf("[BLOCK]\n");
            break;

        case NODE_IF:
            printf("[IF STATEMENT]\n");
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Condition:\n");
            print_tree(n->flow.cond, depth + 2);
            
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Then:\n");
            print_tree(n->flow.then_stmt, depth + 2);
            
            if (n->flow.else_stmt) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("Else:\n");
                print_tree(n->flow.else_stmt, depth + 2);
            }
            break;

        case NODE_WHILE:
            printf("[WHILE LOOP]\n");
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Condition:\n");
            print_tree(n->flow.cond, depth + 2);
            
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Body:\n");
            print_tree(n->flow.then_stmt, depth + 2);
            break;

        case NODE_RETURN:
            printf("[RETURN]\n");
            if (n->ret.value) {
                print_tree(n->ret.value, depth + 1);
            }
            break;

        case NODE_STR:
            printf("[STRING] TokenID: %d\n", n->token_id);
            break;

        default:
            printf("[NODE] Unknown Type: %d\n", n->type);
            break;
    }

    // Traverses sibling statements sequentially.
    // If the child was a block, its statements are linked via next, so they flow down nicely.
    if (n->next) {
        print_tree(n->next, depth);
    }
}

Node* led(Node* left)
{
  printf("Got into lud!\n");
  Token* op = advance();

  if(op->type == '(')
  {
    Node* node = new_node(NODE_CALL);
    Token* name_token = &tokens[left->token_id]; 
    node->func.name = _strdup(gettokenname(name_token));
    if (!match(')')) 
      node->func.args = parse_expression(0);
    consume(')');
    return node;
  }
  else 
  {
    Node* node = new_node(NODE_BIN);
    node->bin.op = op->type;
    node->bin.left = left;
    printf(" currently in led ,get_precedence for the token %d is %d\n",op->type, get_precedence(op->type));
    node->bin.right = parse_expression(get_precedence(op->type));
    return node;
  }
}

Node* parse_expression(int p)
{
  printf("Entering parse_expression\n");
  Node* left = nud();
  printf("Current in parse_expression,  precedence for %d is %d\n ", peek()->type,get_precedence(peek()->type));
  while(get_precedence(peek()->type) > p)left = led(left);
  return left;
}



Node* parse_block()
{
  printf("enterign parse block\n");
  consume('{');
  Node* head = NULL;
  Node* curr = NULL;

  while(!match('}'))
  {
    Node* s = parse_statement();
    if (head == NULL) 
    {
      head = s;
      curr = head;
    } 
    else 
    {
      curr->next = s; 
      curr = s;
    }
  }
  consume('}');
  printf("Ending block\n");
  
  return head;
  
}

Node* parse_return()
{
  consume(K_RETURN);
  Node* n = new_node(NODE_RETURN);
  n->ret.value = NULL;
  if(!match(';'))
  {
    n->ret.value = parse_expression(0);
  }
  consume(';');
  return n;
}


Node* parse_function() 
{
    printf("Entering parse func\n");
    if (!is_type(peek()->type)) exit(1); 
    Token* rt = advance(); 
    Token* f_tok = consume(T_IDENTIFIER); 
    Node* n = new_node(NODE_FUNC);
    n->func.name = gettokenname(f_tok);
    n->func.returntype = rt->type;
    add_to_symtab(f_tok, gt_count, 1,gt_count);
    bp = 0; 
    consume('(');
    Node** arg_ptr = &(n->func.args); 
    int curr_gt_ind = gt_count;
    while (!match(')')) {
        if (is_type(peek()->type)) {
            advance(); 
            Token* a_tok = consume(T_IDENTIFIER);
            
            add_to_symtab(a_tok, bp,0,curr_gt_ind);
            bp += 4;

            *arg_ptr = new_node(NODE_VAR);
            (*arg_ptr)->token_id = a_tok->ind;
            arg_ptr = &((*arg_ptr)->next);
        }
        if (match(',')) advance(); 
    }
    consume(')');
    if(match(';'))
    {  consume(';');n->func.body = NULL;}
    else n->func.body = parse_block(); 
    
    return n;
}

Node* parse_declaration()
{
  printf("Entering parse dec\n");
  if(!is_type(peek()->type)) exit(1); 
  Token* rt = advance();
  Token* d_tok = consume(T_IDENTIFIER); 
  Node* n = new_node(NODE_VAR);
  n->var.dt = rt->type;
  n->var.offset = bp;
  n->token_id = d_tok->ind;
  add_to_symtab(d_tok,bp,0,gt_count);
  bp+=4;
  if(peek()->type == '=')
  {
    advance();
    n->var.value = parse_expression(0);
  }
  consume(';');
  return n;
}

Node* parse_global_declaration()
{
  printf("Entering parse global dec\n");
  if(!is_type(peek()->type)) exit(1); 
  Token* rt = advance();
  Token* d_tok = consume(T_IDENTIFIER); 
  Node* n = new_node(NODE_GVAR);
  n->gvar.name = gettokenname(d_tok);
  n->gvar.dt = rt->type;
  n->gvar.offset = gbr;
  n->token_id = d_tok->ind;
  gbr += 4;
  add_to_symtab(d_tok,gt_count,1,gt_count);
  if(peek()->type == '=')
  {
    advance();
    n->gvar.value = parse_expression(0);
  }
  consume(';');
  return n;
}

Node* parse_if_stmt()
{
    Token* ift = consume(K_IF);
    Node* n = new_node(NODE_IF);
    n->token_id = ift->ind;
    consume('(');
    n->flow.cond = parse_expression(0);
    consume(')');
    n->flow.then_stmt = parse_statement();
    if(match(K_ELSE))
    {
      consume(K_ELSE);
      n->flow.else_stmt = parse_statement();
    }
    else n->flow.else_stmt = NULL;
    return n;
}


Node* parse_while()
{
  Token* wt = consume(K_WHILE);
  Node* n = new_node(NODE_WHILE);
  n->token_id = wt->ind;
  consume('(');
  n->flow.cond = parse_expression(0);
  consume(')');
  n->flow.then_stmt = parse_statement();
  n->flow.else_stmt = NULL;
  return n;
}

Node* parse_for()
{
  Token* ft  =consume(K_FOR);
  consume('(');
  Node* init = parse_statement();
  Node* cond = parse_expression(0);
  consume(';');
  Node* post = parse_expression(0);
  consume(')');

  Node* body = parse_statement();

  Node* wnode = new_node(NODE_WHILE);
  wnode->flow.cond = cond;
  if(body->type != NODE_BLOCK)
  {
    Node* nblock = new_node(NODE_BLOCK);
    nblock->func.body = body;
    body = nblock;
  }

  
  Node* t = body->func.body;
  if(!t){  body->func.body = post;}
  else 
  {
    while(t->next)t = t->next;
    t->next = post;
  }
  wnode->flow.then_stmt = body;
  init->next = wnode;
  Node* outer_block = calloc(1, sizeof(Node));
  outer_block->type = NODE_BLOCK;
  outer_block->func.body = init;
  return outer_block;
}



void parse_top_level() 
{
  init_parser();
  while (tokens[tp1].type != T_EOF) {
    if (is_type(tokens[tp1].type)) {
        printf("Token is a type , enting next phase \n");
        if (tokens[tp1 + 2].type == '(') 
          append_node(parse_function()); 
        else if(tokens[tp1+2].type == '=')
        {
          append_node(parse_global_declaration());
        }
        else 
        {
          tp1++;
        }
    }
    else 
        tp1++;
  }
}



Node* parse_statement()
{
  printf("Got into parse statement \n");
  switch(tokens[tp1].type)
  {
    case K_INT:
    case K_CHAR:
    case K_VOID: 
      return parse_declaration();
    case '{':
      return parse_block();
    case K_IF:
      return parse_if_stmt();
    case K_WHILE:
      return parse_while();
    case K_FOR:
      return parse_for();
    case K_RETURN:
      return parse_return();
      break;
    default:
      printf("going into parse_expression\n");
      Node* n = parse_expression(0); 
      if (peek()->type == 59) {
          printf("Successfully consumed semicolon at index %d\n", tp1);
          advance(); // Move past the ';' so the next loop sees the '}'
      } else {
        printf("Parser Error: Missing semicolon\n");
        exit(1);
      }
      return n;
      
  }
}
void init_parser() {
    tp1 = 0;  
    sp = 0;  
    gt_count = 0; 
    bp = 0;  
}


//


