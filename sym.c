#include "sym.h"
#include "nodes.h"
#include <string.h>

// creates and returns a a symbol
struct symbol *make_symbol(char *name, char sym_type, char data_type, int value, float fvalue, struct Arg **args, int args_count, int line, bool init) {
	struct symbol *s = malloc(sizeof(struct symbol));
	s->name = malloc(strlen(name) * sizeof(char));
	strcpy(s->name, name);
	s->sym_type = sym_type;
	s->data_type = data_type;
	s->args = args;
	s->fvalue = fvalue;
	s->value = value;
	s->line = line;
	s->args_count = args_count;
	s->init = init;
	s->reg = 0;
	s->offset = 0;
	s->left = s->right = NULL;
	return s;
}

//insert a symbol node in a given tree
struct symbol *insert_symbol(struct symbol **root, struct symbol *s) {
	if (*root == NULL) {
		*root = s;
		return NULL;
	}
	else if (strcmp((*root)->name, s->name) < 0)
		return insert_symbol(&(*root)->right, s);
	else if (strcmp((*root)->name, s->name) > 0)
		return insert_symbol(&(*root)->left, s);
	else
		return *root;
}

// search the given tree for a symbol by name
struct symbol *search_symbol(struct symbol *root, char* name) {
	if (root == NULL)
		return NULL;
	else if (strcmp(root->name, name) < 0)
		return search_symbol(root->right, name);
	else if (strcmp(root->name, name) > 0)
		return search_symbol(root->left, name);
	else
		return root;
}

// insert  the given scope s into the parent inner scopes
// resize the array if needed
void insert_scope(struct scope *parent, struct scope *s) {
	if (parent->count == 0) {
		parent->max = 4;
		parent->inner_scopes = malloc(4 * sizeof(struct scope*));
	}
	else if (parent->count == parent->max) {
		parent->max *= 2;
		parent->inner_scopes = realloc(parent->inner_scopes, parent->max * sizeof(struct scope));
	}
	s->parent = parent;
	parent->inner_scopes[parent->count] = s;
	parent->count += 1;
}

// givn a symbol name search the scope chain for it
struct symbol *search_scope(char *name, struct scope *current_scope) {
	if (current_scope == NULL)
		return NULL;

	struct symbol *s = search_symbol(current_scope->table, name);
	if (s != NULL)
		return s;
	else
		return search_scope(name, current_scope->parent);

}

// maps a symbol to register
int map2register(struct symbol *var) {
	if (var->reg > 0)
		return var->reg;
	int reg = 0;
	if (var->data_type == 'f'){
		reg = used_freg;
		used_freg += 1;
	}
	else {
		reg = used_reg;
		used_reg += 1;
	}
	var->reg = reg;
	return reg;
}
 
// given aa dclaration node, loop on its children and create symbol entry for each one
// arays are stored as single variable that holds the address of first location in the array then the array itself
void add_to_symbol_table(struct scope *current_scope, struct Node *d) {
	int i=0; char st;
	for (i = 0; i < d->children.count; i++) {
		struct Node *decl = d->children.arr[i];
		struct symbol *s = make_symbol(decl->info.decl->name, d->info.decl_head->type, d->info.decl_head->data_type, 
			 decl->info.decl->value, decl->info.decl->fvalue, NULL, 0, d->line, decl->info.decl->init);

		// semantic warning if assiging float to int varaible
		if (s->data_type == 'i' && decl->info.decl->value < abs(ceil(decl->info.decl->fvalue)))
			semantic_error(12, decl->info.decl->name, d->line, 0);

		// check if array
		if (decl->info.decl->arr == '1')
			s->sym_type = 'a';

		struct symbol *s1 = insert_symbol(&(current_scope->table), s);
		
		// variable redeclaration
		if (s1 != NULL) {
			semantic_error(1, s1->name, decl->line, s1->line);
			continue;
		}

		// set the offset global or stack
		if (current_scope->parent == NULL) {
			s->offset = global_offset + 4;
			st = 'g';
			global_offset += (s->sym_type == 'a')? 4 * (1+s->value): 4;
		}
		else {
			stack_offset -= (s->sym_type == 'a')? 4 * (1+s->value): 4;
			st = 's';
			s->offset = stack_offset;
		}

		s->reg = -1;

		// if array on stack add the value od $sp to it
		if (s->sym_type == 'a') {
			int reg = map2register(s);
			printf("\tli $i%d, %d\n", reg, s->offset + 4);
			if (st == 's')
				printf("\tadd $i%d, $i%d, $sp\n", reg, reg);
		}
		
		// if the variable is initialized load the value in its register
		if (s->init == true) {
			int reg = map2register(s);
			if (s->data_type == 'i')
				printf("\tli $i%d, %d\n", reg, s->value);
			else
				printf("\tli.f $f%d, %.5f\n", reg, s->fvalue);
		}
	}
}

// return the type of expression ('i' for int, 'f' for float)
char get_exp_type(struct Node *n, struct scope *current_scope) {
	if (n->type == 'f') {
		if (n->sub_type == 'i')
			return 'i';
		else if (n->sub_type == 'f')
			return 'f';
		else if (n->sub_type == 'l') {
			struct symbol *s = search_scope(n->children.arr[0]->info.loc->name, current_scope);
			if (s != NULL)
				return s->data_type;
			else
				return 'i';
		}
		else if(n->sub_type == 'c') {
			struct symbol *s = search_scope(n->children.arr[0]->info.function->name, current_scope);
			if (s != NULL && s->data_type != 'v')
				return s->data_type;
			else
				return 'i';
		}
	}
	char t1 = get_exp_type(n->children.arr[0], current_scope);
	char t2 = get_exp_type(n->children.arr[1], current_scope);

	if (t1 == 'i' && t2 == 'i')
		return 'i';
	else
		return 'f';
}