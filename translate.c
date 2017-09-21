#include <stdio.h>
#include "translate.h"
#include "sym.h"
#include "inter.h"
#include "semantics.h"

void init_tables() {
	ops['+'] = "add";
	ops['-'] = "sub";
	ops['*'] = "mul";
	ops['/'] = "div";
	ops['0'] = "slt";
	ops['1'] = "slte";
	ops['2'] = "sgte";
	ops['3'] = "sgt";
	ops['E'] = "seq";
	ops['N'] = "sne";
	ops['A'] = "and";
	ops['O'] = "or";

	used_reg = 1;
	used_freg = 1;

	global_offset = 0;
	stack_offset = 0;
	label = 1;
}

void bfs(struct Node* root) {
	printf("%c(%c-%d) ", root->type, root->sub_type, root->children.count);
	int i = 0;
	for (i = 0; i < root->children.count; i += 1)
		bfs(root->children.arr[i]);
}


/*
void dfs(struct symbol *root, struct scope *sc) {
	if (root == NULL)
		return ; 
	dfs(root->left, sc);
	dfs(root->right, sc);
	
}
*/
int translate(struct Node* head, struct scope* g) {
	init_tables();
	global = g;
	int m = check_main(head);
	if (m != 1)
		semantic_error(14, NULL, 0, 0);
	else
		generate_inter(head, g, -1, -1);
	return 0;
}
