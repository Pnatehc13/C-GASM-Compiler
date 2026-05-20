#include "./sema.h"
#include <stdio.h>
#include <stdlib.h>
#include "./parser.h"
#include "./lexer.h"
#include <string.h>


void sema_error(char* message, int line) {
    printf("Semantic Error (Line %d): %s\n", line, message);
    exit(1);
}
Node* current_function = NULL;

TokenType sema_analyze(Node* n) 
{
    if(!n) return T_VOID;
    switch(n->type)
    {
        case NODE_INT:
            return T_INT;
        case NODE_STR:
            return T_STRING;
        case NODE_FUNC:
            current_function = n;
            if(n->func.body)
                sema_analyze(n->func.body);
            current_function = NULL;
            break;
        case NODE_VAR:
            if(n->var.dt != 0)
            {
                if(n->var.value)
                {
                    TokenType t = sema_analyze(n->var.value);
                    if (t != n->var.dt) 
                    {
                        sema_error("Type mismatch in variable initialization", tokens[n->token_id].line);
                    }
                }
                return n->var.dt;
            }
            else
            {
                char* vn = gettokenname(&tokens[n->token_id]);
                int sym_idx = find_symbol(vn);
                if (sym_idx == -1) {
                    char err_msg[128];
                    sprintf(err_msg, "Undeclared variable '%s'", vn);
                    sema_error(err_msg, tokens[n->token_id].line);
                }
                n->var.dt = symtab[sym_idx].type;
                n->var.offset = symtab[sym_idx].offset;
                return n->var.dt;
            }
            break;
        
        
        case NODE_BIN:
        {
            TokenType left_t = sema_analyze(n->bin.left);
            TokenType right_t = sema_analyze(n->bin.right);
            if(left_t == T_STRING)
            {
                sema_error("Cannot perform such action on Strings", tokens[n->token_id].line); 
            }
            if (left_t != right_t) {
                sema_error("Type mismatch in binary operation", tokens[n->token_id].line);
            }
            return left_t;
        }
        case NODE_IF:
        {
            TokenType cond = sema_analyze(n->flow.cond);
            if (cond == T_STRING) {
                sema_error("Condition expression cannot be a String", tokens[n->token_id].line);
            }
            if (n->flow.then_stmt) {
                sema_analyze(n->flow.then_stmt);
            }
            if (n->flow.else_stmt) {
                sema_analyze(n->flow.else_stmt);
            }
            return T_VOID;
        }
        case NODE_WHILE:
        {
            TokenType cond_t = sema_analyze(n->flow.cond);
            if (cond_t == T_STRING) {
                sema_error("Loop condition expression cannot be a String", tokens[n->token_id].line);
            }
            if (n->flow.then_stmt) {
                sema_analyze(n->flow.then_stmt);
            }
            return T_VOID;
        }
        case NODE_CALL:
        {
            int target_idx = -1;
            for (int i = 0; i < gt_count; i++) {
                if (strcmp(global_table[i].name, n->func.name) == 0) {
                    target_idx = i;
                    break;
                }
            }
            if (strcmp(n->func.name, "printf") == 0) 
            {
                if (n->func.args) {
                    TokenType first_arg_t = sema_analyze(n->func.args);
                    if (first_arg_t != T_STRING) {
                        sema_error("First argument to printf must be a String literal", tokens[n->token_id].line);
                    }
                }
                return T_VOID;
            }

            if (target_idx == -1) {
                char err_msg[128];
                sprintf(err_msg, "Call to undeclared function '%s'", n->func.name);
                sema_error(err_msg, tokens[n->token_id].line);
            }

            Node* def_arg = global_table[target_idx].p->func.args; 
            Node* call_arg = n->func.args;

            while (def_arg && call_arg)
            {
                TokenType expected_t = def_arg->var.dt; // Definition nodes hold types
                TokenType passed_t = sema_analyze(call_arg);
                if (expected_t != passed_t) 
                {
                    sema_error("Type mismatch in function argument", tokens[n->token_id].line);
                }
                def_arg = def_arg->next;
                call_arg = call_arg->next;
            }

            if (def_arg != NULL || call_arg != NULL) {
                sema_error("Argument count mismatch in function call", tokens[n->token_id].line);
            }
            return global_table[target_idx].p->func.returntype;
        }
        case NODE_RETURN:
            if (!current_function) 
            {
                sema_error("Return statement outside of any function", tokens[n->token_id].line);
            }
            TokenType expected_rt = current_function->func.returntype;
            if (n->ret.value) {
                TokenType actual_rt = sema_analyze(n->ret.value);
                if (actual_rt != expected_rt) 
                {
                    char err_msg[256];
                    sprintf(err_msg, "Function '%s' expects return type %d but got %d", current_function->func.name, expected_rt, actual_rt);
                    sema_error(err_msg, tokens[n->token_id].line);
                }
            }
            else 
            {
                if (expected_rt != T_VOID) 
                {
                    char err_msg[256];
                    sprintf(err_msg, "Function '%s' must return a value", current_function->func.name);
                    sema_error(err_msg, tokens[n->token_id].line);
                }
            }
            return T_VOID;
        
        default:
            if (n->next) sema_analyze(n->next);
            break;
    }
    if (n->next && n->type != NODE_FUNC && n->type != NODE_VAR) {
        sema_analyze(n->next);
    }
    return T_VOID;
    
}


void sema_run_global_analysis()
{
    printf("\n--- STARTING SEMANTIC ANALYSIS PASS ---\n");
    for (int i = 0; i < gt_count; i++) 
    {
        if (global_table[i].p != NULL) 
        {
            sema_analyze(global_table[i].p);
        }
    }
    printf("Sema Pass: Success! Abstract Syntax Tree is verified and decorated.\n");
}
