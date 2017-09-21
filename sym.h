#ifndef SYM_FILE
#define SYM_FILE

#include <stdlib.h>
#include "nodes.h"

struct symbol {
	char *name;				// symbol name
	char sym_type;			// symbol type 'V' for variable, 'C' for constant, 'a' for array, 'p' for function parameter, 'f' for function
	char data_type; 		// data type 'i' for integer, 'f' for float, 'v' for void
	int value; 				// initial integer value (array size for array symbol)
	float fvalue; 			// initial float value
	bool init;				// boolean to check if initialized
	int line; 				// symbol first appearnce in source code
	int offset; 			// varaiable offset in memory
	int reg; 				// variable register
	struct Arg **args; 		// args array for function symbol
	int args_count;			// args count
	struct symbol *left;	// left symbol in the tree
	struct symbol *right;	// right symbol in the tree
};

struct scope {
	struct symbol *table; 			// symbol table of the scope
	struct scope **inner_scopes; 	// array of inner scopes
	int count;						// count of inner scopes
	int max;						// max size of inner scopes array
	struct scope *parent;			// pointer to parent scope
};

int used_reg; 						// the number of current used integer registers $i

int used_freg; 						// the number of current used float registers $f

char *ops[256]; 					// strings array indexed by operation (eg '+', '-') to ('add', 'sub')

int stack_offset;

int global_offset;

struct scope *global; 				// global scope pointer

struct symbol *make_symbol(char *name, char sym_type, char data_type, int value, float fvalue, struct Arg **args, int args_count, int line, bool init);

struct symbol * insert_symbol(struct symbol **root, struct symbol *s);

struct symbol *search_symbol(struct symbol *root, char* name);

void insert_scope(struct scope *parent, struct scope *s);

struct symbol *search_scope(char *name, struct scope *current_scope);


#endif