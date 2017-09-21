#ifndef TRANSLATE_FILE
#define TRANSLATE_FILE

#include "helpers.h"
#include "sym.h"
#include "nodes.h"

void bfs_walk(struct Node *root);

int translate(struct Node* head, struct scope* global);

#endif