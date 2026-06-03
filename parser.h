#ifndef PARSER_H_
#define PARSER_H_
#include "./lexer.h"

typedef enum {
    NODE_FUNC, NODE_BLOCK, NODE_VAR, NODE_INT, 
    NODE_BIN, NODE_IF, NODE_WHILE, NODE_RETURN,
    NODE_CALL, NODE_ASSIGN,NODE_STR,NODE_GVAR,NODE_POINTER,
    NODE_ADDR,NODE_BREAK,NODE_CONTINUE,NODE_ARR_ACCESS
} NodeType;

extern int bp;



typedef struct Node {
    NodeType type;
    int token_id; 
    
    union {
        long int_val;      
        
        struct{
            struct Node* expr;
        } unary;
        
        struct {
            TokenType dt;
            int offset;    
            struct Node* value; 
        } var;

        struct {
            char* string;
        } str;
        struct{
            TokenType dt;
            int offset;
            struct Node* value;
            int addr;
        } pointer;

        struct {           
            int op;        
            struct Node* left;
            struct Node* right;
        } bin;

        struct {           
            struct Node* cond;
            struct Node* then_stmt;
            struct Node* else_stmt;
            int label_a;   
            int label_b;   
            int isfor;
        } flow;

        struct {           
            char* name;
            struct Node* args; 
            struct Node* body;
            TokenType returntype; 
            int localvarbyte;
            int argcount;
        } func;

        struct {
            char* name;
            TokenType dt;
            int offset;
            struct Node* value;
        } gvar;

        struct {
            struct Node* value; 
        }ret;
    };
    struct Node* indnxt;
    struct Node* next;
} Node;

typedef struct r
{
    char name[64];   
    int type;        
    int offset;      
    int gt_index;    
    int is_global;   
    int size;
    int isarray;
    int ptrlvl;
    int dim_size[256];
    int dim_cnt;
}Record;
extern int sp;

typedef struct g
{
    int id;
    char name[60];
    Node* p;
}Globalentry;

extern Globalentry global_table[256];
extern int gt_count;
void print_tree(Node* n, int depth);
extern Record symtab[1024];
void parse_top_level();
void init_parser();
Node* parse_statement();
Node* parse_expression(int p);
int find_symbol(char* name);

#endif
