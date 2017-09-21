#include "inter.h"
#include "sym.h"
#include "semantics.h"

// receives a a node at the parse tree and based on its type calls a specific function to generate the corresponding code
void generate_inter(struct Node *root, struct scope *current_scope, int inner_loop_start, int inner_loop_end) {
	int i, e;
	switch(root->type) {
		case 'P':
			for (i = 0; i < root->children.count; i++)
				generate_inter(root->children.arr[i], current_scope, inner_loop_start, inner_loop_end);
			break;
		case 'D':
			add_to_symbol_table(current_scope, root);
			break;
		case 'F':
			generate_function(root, current_scope);
			break;
		case 'S':
			switch (root->sub_type) {
				case 'A':
					generate_assign(root, current_scope);
					break;
				case 'I':
					generate_if(root, current_scope, 0, inner_loop_start, inner_loop_end);
					break;
				case 'E':
					generate_if(root, current_scope, 1, inner_loop_start, inner_loop_end);
					break;
				case 'W':
					generate_loop(root, current_scope, 'W', inner_loop_start, inner_loop_end);
					break;
				case 'D':
					generate_loop(root, current_scope, 'D', inner_loop_start, inner_loop_end);
					break;
				case 'F':
					generate_loop(root, current_scope, 'F', inner_loop_start, inner_loop_end);
					break;
				case 'U':
					generate_loop(root, current_scope, 'U', inner_loop_start, inner_loop_end);
					break;
				case 'B':
					if (inner_loop_end == -1)
						semantic_error(3, NULL, root->line, 0);
					printf("\tj lbl%d\n", inner_loop_end);
					break;
				case 'c':
					eval_function(root, current_scope, '0');
					break;
				case 'C':
					if (inner_loop_start == -1)
						semantic_error(4, NULL, root->line, 0);
					printf("\tj lbl%d\n", inner_loop_start);
					break;
				case 'R':
					e = eval_exp(root->children.arr[0], current_scope);
					printf("\tmov $re.%c, $%c%d\n", e>>16, e >> 16, e & 0xff);
					printf("\taddi, $sp, $sp, %d\n\tret\n\n", -stack_offset);
					break;
				case 'S':
					generate_switch(root, current_scope, inner_loop_start, inner_loop_end);
					break;
			}
			break;
		case 'B':
			generate_block(root, current_scope, inner_loop_start, inner_loop_end);
			break;
	} 

}

/* generates the code for a function node
1 - creates a new scope for the function
2 - reset the stack offset (used to allocate 4 bytes for each variable in the function scope)
3 - generate code that loads the function args from memory + the current stack pointer
4 - add the function args to the symbol table
5 - loop on the children of the function block and generate their corresponding code
*/
void generate_function(struct Node *root, struct scope *current_scope) {
	// create a new scope for the functon
	struct scope *new_scope = malloc(sizeof(struct scope));
	new_scope->count = 0;
	new_scope->table = NULL;
	new_scope->parent = current_scope;

	// insert the new scope into the parent scope
	insert_scope(current_scope, new_scope);

	// set stack_offset to zero (we are in a function scope)
	stack_offset = 0; int i;
	struct symbol *func = search_scope(root->info.function->name, current_scope);
	printf("%s:\n\t;loading %s args\n", root->info.function->name, root->info.function->name);

	// add the function args to the symbol table
	for (i = 0; i < func->args_count; i++) {
		// create ymbol table entry for each arg
		struct symbol *var = make_symbol(func->args[i]->name, 'v', func->args[i]->type, 0, 0, NULL, 0, func->line, true);
		var->offset = 4 * (i + 1);
		var->sym_type = 'p';

		// check if array
		if (func->args[i]->arr == '1') var->sym_type = 'a';
		struct symbol *s = insert_symbol(&(new_scope->table), var);
		if (s != NULL) semantic_error(6, func->args[i]->name, s->line, 0);

		// load function args
		int reg = map2register(var);
		if (var->sym_type == 'a' || var->data_type == 'i')
			printf("\tld $i%d, $sp[%d] ; %s\n", reg, var->offset, var->name);
		else
			printf("\tld.f $f%d, $sp[%d] ; %s\n", reg, var->offset, var->name);
	}
	printf("\n");

	// generate code of the function body
	for (i = 0; i < root->children.arr[1]->children.count; i++) {
		generate_inter(root->children.arr[1]->children.arr[i], new_scope, -1, -1);
	}

	// return from function
	if (stack_offset < 0)
		printf("\taddi, $sp, $sp, %d\n", -stack_offset);
	printf("\tret\n\n");
	
}

// generates code for a block
void generate_block(struct Node *root, struct scope *current_scope, int inner_loop_start, int inner_loop_end) {
	// creates a new scope for the block
	struct scope *new_scope = malloc(sizeof(struct scope));
	new_scope->count = 0;
	new_scope->table = NULL;
	new_scope->parent = current_scope;
	insert_scope(current_scope, new_scope);

	// generate code for the block constructs
	int i, old_stack_offset = stack_offset;
	for (i = 0; i < root->children.count; i++) {
		generate_inter(root->children.arr[i], new_scope, inner_loop_start, inner_loop_end);
	}

	// free the used memory on the stack
	if (stack_offset < old_stack_offset) {
		printf("\taddi $sp, $sp, %d\n\n", old_stack_offset - stack_offset);
		stack_offset = old_stack_offset;
	}
}

// generates code for assign statement
void generate_assign(struct Node *root, struct scope *current_scope) {
	// search for the left side loction and check if it is defined
	struct symbol *var = search_scope(root->children.arr[0]->info.loc->name, current_scope);
	if (var == NULL || var->sym_type == 'f') {
		semantic_error(2, root->children.arr[0]->info.loc->name, root->line, 0);
		return ;
	}

	// if constant then return semantic error (can't be assigned)
	if (var->sym_type == 'C')
		semantic_error(8, root->children.arr[0]->info.loc->name, root->line, 0);
	else if (var->sym_type == 'a') {
		// generate the right side expression
		int e = eval_exp(root->children.arr[1], current_scope);
		char type = (e >> 16) == 'a'? 'i': 'f';

		// check if the user is accessing this array as a single variable
		if (root->children.arr[0]->children.count == 0)
			semantic_error(17, root->children.arr[0]->info.loc->name, root->line, 0);

		// generate the array position
		int loc = generate_loc(root->children.arr[0], current_scope, 1, 1);

		// store the value in the memory
		if (type == 'i' && var->data_type == 'i')
			printf("\tst $a%d, $a%d, $i%d\n\n", e & 0xffff, loc & 0xffff, (loc >> 16));
		else if (type == 'f' && var->data_type == 'f')
			printf("\tst.f $f%d, $a%d, $i%d\n\n", e & 0xffff, loc & 0xffff, (loc >> 16));
		else if (type == 'f' && var->data_type == 'i') {
			semantic_error(12, root->children.arr[0]->info.loc->name, root->line, 0);
			printf("\tcvt.f.i $a1, $d1\n");
			printf("\tst $a1, $a%d, $i%d\n\n", loc & 0xffff, (loc >> 16));
		}
		else {
			printf("\tcvt.i.f $d1, $i1\n");
			printf("\tst.f $d1, $a%d, $i%d\n\n", loc & 0xffff, (loc >> 16));
		}
	}
	else {
		// generate the right side expression
		int e = eval_exp(root->children.arr[1], current_scope);
		char type = (e >> 16) == 'a'? 'i': 'f';

		// check to see if accessing a single varoable as array
		if (root->children.arr[0]->children.count == 1)
			semantic_error(18, root->children.arr[0]->info.loc->name, root->line, 0);

		// generate the variable 
		int loc = generate_loc(root->children.arr[0], current_scope, 1, 1);
		if (type == 'i' && var->data_type == 'i')
			printf("\tmov $i%d, $a%d\n\n", loc & 0xffff, e & 0xffff);
		else if (type == 'f' && var->data_type == 'f')
			printf("\tmov.f $f%d, $d%d\n\n", loc & 0xffff, e & 0xffff);
		else if (type == 'f' && var->data_type == 'i') {
			semantic_error(5, root->children.arr[0]->info.loc->name, root->line, 0);
			printf("\tcvt.f.i $a1, $d1\n");
			printf("\tmov $i%d, $a1\n\n", loc & 0xffff);
		}
		else {
			printf("\tcvt.i.f $d1, $i1\n");
			printf("\tmov $f%d, $d1\n\n", loc & 0xffff);	
		}
	}
}

// generates if, if-else statements
void generate_if(struct Node *root, struct scope *current_scope, int els, int inner_loop_start, int inner_loop_end) {
	// generate code for the predicate
	int e = eval_exp(root->children.arr[0], current_scope);
	char type = (e >> 16) == 'a'? 'i': 'd';
	int end_lbl = label;
	label += 1;

	// branch to the end if the predicate is false
	printf("\tbeq $%c%d, $0, lbl%d\n", e >> 16, e & 0xffff, end_lbl);

	// generate the if body
	generate_inter(root->children.arr[1], current_scope, inner_loop_start, inner_loop_end);

	// if no else print the end label and return, else generate the else body
	if (els == 0)
		printf("\nlbl%d:\n", end_lbl);
	else {
		// this will jump if the predicate was true so the else part is not executed
		printf("\tj lbl%d\n", end_lbl + 1);

		// the else label 
		printf("\nlbl%d:\n", end_lbl);
		label += 1;

		// generate the else body
		generate_inter(root->children.arr[2], current_scope, inner_loop_start, inner_loop_end);
		printf("\nlbl%d:\n", end_lbl + 1);
	}
}

// generates code for loops
void generate_loop(struct Node *root, struct scope *current_scope, char t, int inner_loop_start, int inner_loop_end) {
	int e, start_lbl = label, end_lbl = label + 1; char type;
	label += 2;

	// generate the initial for loop assign stmt
	if (t == 'F')
		generate_inter(root->children.arr[2], current_scope, inner_loop_start, inner_loop_end);

	// print the start label of the loop
	printf("lbl%d:\n", start_lbl);

	// if 'while' or 'for' 
	if (t == 'W' || t == 'F') {
		// generate the predicate and branch if its false to the end label
		e = eval_exp(root->children.arr[0], current_scope);
		type = (e >> 16) == 'a'? 'i': 'd';
		printf("\tbeq $%c%d, $0, lbl%d\n", e >> 16, e & 0xffff, end_lbl);

		// generate the loop body
		generate_inter(root->children.arr[1], current_scope, start_lbl, end_lbl);

		// if 'for' generate the last assign stmt
		if (t == 'F')
			generate_inter(root->children.arr[3], current_scope, inner_loop_start, inner_loop_end);

		// jump to start label
		printf("\tj lbl%d\n", start_lbl);
	}
	else {
		// if 'do while' or 'repeat until'
		// generate the body first
		generate_inter(root->children.arr[1], current_scope, start_lbl, end_lbl);

		// generate code for the predicate
		e = eval_exp(root->children.arr[0], current_scope);
		type = (e >> 16) == 'a'? 'i': 'd';

		// if 'do while' branch to start if the condition is true else branch to start if it is false
		if (t == 'D')
			printf("\tbne $%c%d, $0, lbl%d", e >> 16, e & 0xffff, start_lbl);
		else
			printf("\tbeq $%c%d, $0, lbl%d", e >> 16, e & 0xffff, start_lbl);
	}

	// print the end label
	printf("\nlbl%d:\n", end_lbl);
}



int eval_exp(struct Node *exp, struct scope *current_scope) {
	// get the expression type ('i' for int, 'f' for float)
	char type = get_exp_type(exp, current_scope);

	// set the type of accumelator registers ('a' for int, 'd' for float)
	char t = (type == 'i'? 'a': 'd');

	// generate code for the expression and return the expression output register and its type ('a', 'i', 'f' or 'd')
	return generate_inter_exp(exp, current_scope, (t << 16) + 1, type);
}

// generates code for the expression and return the expression output register and its type ('a', 'i', 'f' or 'd')
int generate_inter_exp(struct Node *n, struct scope *current_scope, int p, char type) {
	// if the root is a factor generate it
	if (n->type == 'f') 
		return generate_inter_factor(n, current_scope, p, type);

	// evaluate left expression - return (reg_type, reg)
	int r1 = generate_inter_exp(n->children.arr[0], current_scope, p+1, type);

	// evaluate right expression - return (reg_type, reg)
	int r2 = generate_inter_exp(n->children.arr[1], current_scope, p+2, type);

	// generate the current node operation
	generate_op(n->sub_type, type, p, r1, r2);

	// return the output register number and its type
	return p;
}

// generate function call
void eval_function(struct Node *n, struct scope *current_scope, char ret) {
	// get the function symbol entry
	char *name = n->info.function->name; int i;
	struct symbol *s = search_scope(name, global);

	// semantic error if doesn't exist
	if (s == NULL || s->sym_type != 'f') {
		semantic_error(10, name, n->line, 0);
		return ;
	}

	// if this function is expected to return value but it's of type void then semantic error
	if (ret == '1' && s->data_type == 'v')
		semantic_error(7, name, n->line, 0);

	// if the number of given args are not the same as the function signature then semantic error
	if (n->children.arr[0]->children.count != s->args_count)
		semantic_error(11, name, n->line, 0);
	
	// evaluate each arg and push it on the stack in reverse order
	for (i = 0; i < n->children.arr[0]->children.count; i++) {
		int e = eval_exp(n->children.arr[0]->children.arr[i], current_scope);
		char reg_type = e >> 16;
		char t1 = (reg_type == 'a' || reg_type == 'i' ? 'i': 'f');

		// convert the given value to the parameter type if needed
		if (t1 != s->args[i]->type) {
			// if parameter is int but a float is passed then semantic warning
			if (t1 == 'f') {
				semantic_error(9, name, n->line, 0);
				printf("\tcvt.f.i $a1, $%c%d\n", reg_type, e & 0xffff);
				reg_type = 'a'; e = 1;
			}
			else {
				printf("\tcvt.i.f $d1, $%c%d\n", reg_type, e & 0xffff);
				reg_type = 'd'; e = 1;
			}
		}

		// push onto stack
		printf("\tst $%c%d, $sp[%d]\n", reg_type, e & 0xffff, stack_offset-(n->children.arr[0]->children.count - i)*4);
	}

	// modify stack pointer and call then return the stack pointer to its original value
	printf("\taddi $sp, $sp, %d\n", stack_offset-n->children.arr[0]->children.count*4);
	printf("\tcall %s\n", name);
	printf("\taddi $sp, $sp, %d\n", 4 * s->args_count);
}

/* factors are the terminal values of the expressions
we have 4 types:
1 - integer value factor - sub_type 'i'
2 - float value factor   - sub_type 'f'
3 - location factor (variable or array position) - sub_type 'l'
4 - function call factor - sub_type 'c'
this function generates the code for these factors and returns the type and the number of the register that holds
the factor value. (register type (bits from 16 to 23) || register number (bits from 0 to 15))
type: is the type of the expression this factor is used in
p: the register we prefer if possible to return the output into it
we may need to convert int values to float in float expressions
*/
int generate_inter_factor(struct Node *n, struct scope *current_scope, int p, char type) {
	if (n->sub_type == 'i') {
		// immidiate integer value
		// the expression is of type int
		if (type == 'i')
			printf("\tli $a%d, %d\n", p & 0xffff, n->info.factor->data.i);
		else
			printf("\tli.f $d%d, %d\n", p & 0xffff, n->info.factor->data.i);
		return p;
	}
	else if (n->sub_type == 'f') {
		// immidiate float value
		printf("\tli.s $d%d, %f\n", p & 0xffff, n->info.factor->data.f);
		return 	p;
	}
	else if (n->sub_type == 'l') {
		// location factor (generate_loc will return the corresponding register number and its type)
		return generate_loc(n->children.arr[0], current_scope, p, 0);
	}
	else {
		// function call factor (eval_function will generate the code)
		eval_function(n->children.arr[0], current_scope, '1');

		// move the return value from $re.('i' for integer, 'f' for float) to the prefered output register
		printf("\tmov $%c%d, $re.%c\n", p >> 16, p & 0xffff, type);
		return p;
	}
}

/* generates cod for a location
wr: '0' for read location, '1' for write in location
p: the register we prefer to use if possible
returns the register (type bits 16-23, number 0-15)
in case of writting to array it returns the array register number (bits 16-31, arrays addresses are always in 'i' registers)
the number of register that holds the offset in the array (allways of type 'a')
*/
int generate_loc(struct Node *n, struct scope *current_scope, int p, char wr) {
	// search the symbol table and check if it is not  function
	struct symbol *var = search_scope(n->info.loc->name, current_scope);
	if (var == NULL || var->sym_type == 'f') {
		semantic_error(2, n->info.loc->name, n->line, 0);
		return 1 + 'i' >> 16;
	}

	// if variable or constant or parameter return the register type and number
	if (var->sym_type == 'V' || var->sym_type == 'C' || var->sym_type == 'p' || n->children.count == 0) {
		// mark the variable as initialized in writting case
		if (wr == 1) var->init = true;

		// check if the variable is initialized (in reading case)
		if (wr == 0 && var->init == false) semantic_error(16, n->info.loc->name, n->line, 0);
		
		// return the register number and type
		int reg = map2register(var);
		return reg + (var->data_type << 16);
	}
	else {
		// the symbol is array: get the expression type of the index expression
		char t1 = get_exp_type(n->children.arr[0], current_scope);
		if (t1 == 'f') {
			// error: trying to index an array using float value
			semantic_error(15, n->info.loc->name, n->line, 0);
			return 1 + 'i' >> 16;
		}

		// generate the index expression code and shift the value by 2 (to multiply by 4 as each word is 4 bytes)
		int e = generate_inter_exp(n->children.arr[0], current_scope, ('a' << 16) + (p & 0xffff), t1);
		int reg = map2register(var);

		// if we are indexing by another variable
		if ((e >> 16) == 'i') {
			printf("\tmov $a%d, $i%d\n",p & 0xffff, e & 0xffff);
			e = p & 0xffff;
		}
		printf("\tsll $a%d, 2\n", e & 0xffff);
		if (wr == 0) {
			// we are reading from the array
			printf("\tld $%c%d, $i%d, $a%d\n", p >> 16, p & 0xffff, reg, e & 0xffff);
			return p;
		}
		else 
			return (reg << 16) | (e & 0xffff);	
	}
}

/* generate a single assembly operation
op: the operation (e.g. '+', '-', .etc)
type: 'i' for integer, 'f' for float
reg: the output reg (bits 16-23 are the type, 0-15 the number)
r1: the left input reg
r2: the right input reg
*/
void generate_op(char op, char type, int reg, int r1, int r2) {
	char suffix[] = {'.', 'f', '\0'};
	if (type == 'i') suffix[0] = '\0';
	char t1 = r1 >> 16, t2 = r2 >> 16, t0 = reg >> 16;

	if (op == '!')
		printf("\tnot%s $%c%d, $%c%d\n", suffix, t0, reg & 0xffff, t1, r1 & 0xffff);
	else if (op == '4')
		printf("\tneg%s $%c%d, $%c%d\n", suffix, t0, reg & 0xffff, t1,  r1 & 0xffff);
	else
		printf("\t%s%s $%c%d, $%c%d, $%c%d\n", ops[op], suffix, t0, reg & 0xffff, t1, r1 & 0xffff, t2, r2 & 0xffff);

} 

// generates code for switch statement
void generate_switch(struct Node *root, struct Node *current_scope, int inner_loop_start, int inner_loop_end) {
	// generate the switching expression
	int e = eval_exp(root->children.arr[0], current_scope);

	// calculate the labels
	int switch_end = label, next_lbl, s_default, i;
	label += 1;
	char exp_type = ((e >> 16) == 'i' || (e >> 16) == 'a')? 'i': 'f';
	char t = ((e >> 16) == 'i' || (e >> 16) == 'a')? 'a': 'd';

	// check for default
	if (root->children.arr[2]->type != 'e')
		s_default = 1;
	else
		s_default = 0;

	// generate code for each case
	for (i = 0; i < root->children.arr[1]->children.count; i++) {
		struct Node *n = root->children.arr[1]->children.arr[i];

		// semantic error if we are switching based on float value of integer expression
		if (exp_type != n->sub_type)
			semantic_error(13, NULL, n->line, 0);

		// load the constant value on wich we switch
		if (n->sub_type == 'f')
			printf("\tli.f $d2, %f\n", n->info.factor->data.f);
		else 
			printf("\tli $a2, %d\n", n->info.factor->data.i);	
		
		if (s_default == 1 || i < root->children.arr[1]->children.count - 1) {
			// if not the last case (or one before last if there is no default)
			next_lbl = label;
			label += 1;

			// check predicate
			printf("\tbne $%c2, $%c%d, lbl%d\n", t, e >> 16, e & 0xffff, next_lbl);

			// generate code
			generate_inter(n->children.arr[0], current_scope, inner_loop_start, inner_loop_end);

			// jump to end
			printf("\tj lbl%d\nlbl%d:\n", switch_end, next_lbl);	
		}
		else if (i == root->children.arr[1]->children.count - 1) {
			// check predicate
			printf("\tbne $%c2, $%c%d, lbl%d\n", t, e >> 16, e & 0xffff, switch_end);

			// generate code
			generate_inter(n->children.arr[0], current_scope, inner_loop_start, inner_loop_end);
		}
	}
	
	// generate the default part
	if (s_default == 1)
		generate_inter(root->children.arr[2]->children.arr[0], current_scope, inner_loop_start, inner_loop_end);
	
	// print the end label
	printf("lbl%d:\n", switch_end);
}
