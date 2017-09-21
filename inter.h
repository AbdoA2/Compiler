#ifndef INTER_FILE
#define INTER_FILE

#include "nodes.h"
#include "sym.h"

int label;

void generate_inter(struct Node *root, struct scope *current_scope, int inner_loop_start, int inner_loop_end); 

void generate_function(struct Node *root, struct scope *current_scope);

void generate_block(struct Node *root, struct scope *current_scope, int inner_loop_start, int inner_loop_end);

void generate_assign(struct Node *root, struct scope *current_scope);

void generate_if(struct Node *root, struct scope *current_scope, int els, int inner_loop_start, int inner_loop_end);

int generate_inter_exp(struct Node *n, struct scope *current_scope, int p, char type);

int generate_inter_factor(struct Node *n, struct scope *current_scope, int p, char type);

void generate_loop(struct Node *root, struct scope *current_scope, char t, int inner_loop_start, int inner_loop_end);

void eval_function(struct Node *n, struct scope *current_scope, char ret);

void add_to_symbol_table(struct scope *current_scope, struct Node *d);

int map2register(struct symbol *var);

void generate_op(char op, char type, int reg, int r1, int r2);

int generate_loc(struct Node *n, struct scope *current_scope, int p, char wr);

char get_exp_type(struct Node *n, struct scope *current_scope);

int eval_exp(struct Node *exp, struct scope *current_scope);

void generate_switch(struct Node *root, struct Node *current_scope, int inner_loop_start, int inner_loop_end);

#endif