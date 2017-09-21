#ifndef HELPERS_FILE
#define HELPERS_FILE

#include "nodes.h"

struct Node *make_node(char type, char sub_type, union Info info);

struct Node *join_nodes(char t, struct Node *n1, struct Node *n2);

struct Node *make_decl_head(char type, char *data_type, struct Node *n, int line);

struct Node *make_decl(char* name, int value, float value1, bool init, char arr);

struct Node *make_empty_node();

struct Node *make_arg(char *type, char *name, char arr);

struct Node *make_function(char *name, struct Node *args, struct Node *block);

struct Node *make_loc(char *name, struct Node* expr, int line);

struct Node *make_expr(char type, struct Node *n1, struct Node *n2);

struct Node *make_factor(char sub_type, union Data data, struct Node *n, int line);

struct Node *make_stmt(char sub_type, int line, int n, ...);

struct Node *eval_exp_sem(struct Node *exp);

#endif