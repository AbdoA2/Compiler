#include "helpers.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


// creates a node with the given type, sub_type and info
struct Node *make_node(char type, char sub_type, union Info info) {
	struct Node *n = malloc(sizeof(struct Node));
	n->type = type;
	n->sub_type = sub_type;
	n->info = info;
	n->children.count = 0;
	
	return n;
}

// joins two varaible declaration nodes together 
// if the left node is of type t it append the right one to it else creates new node of type t and make n1, n2 its children
struct Node *join_nodes(char t, struct Node *n1, struct Node *n2) {
	if (n1->type == t) {
		append(n1, n2);
		return 	n1;
	}
	else {
		struct Node *p = malloc(sizeof(struct Node));
		p->children.count = 0;
		p->type = t;
		if (n1->type != 'e')
			append(p, n1);
		append(p, n2);
		return p;
	}
}

// creates a declaration head (the type: constant C or varoable V) (data_type: i, f)
struct Node *make_decl_head(char type, char *data_type, struct Node *n, int line) {
	// create info
	struct DeclarationHead *d = malloc(sizeof(struct DeclarationHead));
	d->type = type;
	d->data_type = data_type[0];
	union Info info;
	info.decl_head = d;

	// if the given node is Declaration set its info an return it
	if (n->type == 'D') {
		n->info = info;
		n->line = line;
		return n;
	} 
	else {
		// else: create a new D node and append n to it
		struct Node *h = malloc(sizeof(struct Node));
		h->type = 'D';
		h->line = line;
		h->children.count = 0;
		append(h, n);
		h->info = info;
		return h;
	}
}

// creates a declaration node with given name, value (if initialized) and arr if it's an array declaration
struct Node *make_decl(char* name, int value, float value1, bool init, char arr) {
	// create the declaration info
	struct Declaration *d = malloc(sizeof(struct Declaration));
	d->name = (char*)malloc(strlen(name) * sizeof(char));
	strcpy(d->name, name);
	d->init = init;
	d->value = value;
	d->fvalue = value1;
	d->arr = arr;

	// create and return the declaration node
	union Info info;
	info.decl = d;
	return make_node('d', '0', info);
}

// creates a function arg definition node 
struct Node *make_arg(char *type, char *name, char arr) {
	// create the arg info
	struct Arg *arg = malloc(sizeof(struct Arg));
	arg->name = (char*)malloc(strlen(name) * sizeof(char));
	strcpy(arg->name, name);
	arg->type = type[0];
	arg->arr = arr;

	// create and return the arg node
	union Info info;
	info.arg = arg;
	return make_node('a', 'd', info);
}

// creates a function node
struct Node *make_function(char *name, struct Node *args, struct Node *block) {
	// create the function info
	struct Function *function = malloc(sizeof(struct Function));
	function->name = (char*)malloc(strlen(name) * sizeof(char));
	strcpy(function->name, name);
	
	// create the function node and append its children to it.
	union Info info;
	info.function = function;
	struct Node *n = make_node('F', '0', info);
	append(n, args);
	append(n, block);
	return n;
}

// this function creates a location node 
struct Node *make_loc(char *name, struct Node *expr, int line) {
	// create location node
	struct Node *loc = malloc(sizeof(struct Node));
	loc->children.count = 0;
	loc->type = 'L';
	loc->line = line;

	// create location info (variable name)
	union Info info;
	info.loc =  malloc(sizeof(struct Loc));
	info.loc->name = (char*)malloc(strlen(name) * sizeof(char));
	strcpy(info.loc->name, name);
	loc->info = info;
	
	// if there is an expr then (this is array indexing)
	// add the index expr as a child to the node.
	if (expr != NULL) {
		append(loc, expr);
		loc->sub_type = 'a';
	}
	else
		loc->sub_type = 'v';

	return loc;
}

// this function makes expression node
struct Node *make_expr(char sub_type, struct Node *n1, struct Node *n2) {
	// create expr node
	struct Node *exp = malloc(sizeof(struct Node));
	exp->type = 'E';
	exp->sub_type = sub_type;
	exp->children.count = 0;

	// append the left child (and the right one if any)
	append(exp, n1);
	if (n2 != NULL)
		append(exp, n2);

	// if this node can be evaluted during compilation time then evaluate it
	struct Node *e = eval_exp_sem(exp);
	if (e->type == 'f')
		return e;

	return exp;
}


// this function creates a factor node
struct Node *make_factor(char sub_type, union Data data, struct Node *n, int line) {
	// create factor structure and add data to it.
	struct Factor *factor = malloc(sizeof(struct Factor));
	factor->data =  data;

	// create the factor node
	struct Node *f = malloc(sizeof(struct Node));
	f->type = 'f';
	f->sub_type = sub_type;
	f->info.factor = factor;
	f->line = line;
	f->children.count = 0;

	// if te factor has a child append it (location, func_call factors has children)
	if (n != NULL)
		append(f, n);

	return f;
}

// this function creates statments node
struct Node *make_stmt(char sub_type, int line, int n, ...) {
	// create statment node
	struct Node *stmt = malloc(sizeof(struct Node));
	stmt->children.count = 0;
	stmt->line = line;
	stmt->type = 'S';
	stmt->sub_type = sub_type;

	// add its children (variable length list)
	va_list nodes;
	va_start(nodes, n);
	int i = 0;
	for (i = 0; i < n; i++)
		append(stmt, va_arg(nodes, struct Node*));
	va_end(nodes);

	return stmt;
}

// this function creates an empty node
struct Node *make_empty_node() {
	struct Node *n = malloc(sizeof(struct Node));
	n->children.count = 0;
	n->type = 'e';
	return n;
}

// this function takes an expression and checks its left and right child if both of them are numbers it evaluates it and returns
// a new node instead of it with the value
struct Node *eval_exp_sem(struct Node *exp) {
	double f1, f2;
	char t1 = exp->children.arr[0]->sub_type, t2 = 'i', t;
	if (exp->children.arr[0]->type == 'f' && (t1 == 'f' || t1 == 'i'))
		if (t1 == 'f')
			f1 = exp->children.arr[0]->info.factor->data.f;
		else
			f1 = exp->children.arr[0]->info.factor->data.i;
	else
		return exp;

	if (exp->children.count == 2 && exp->children.arr[1]->type == 'f' && (exp->children.arr[1]->sub_type == 'i' || exp->children.arr[1]->sub_type == 'f')) {
		if (exp->children.arr[1]->sub_type == 'f')
			f2 = exp->children.arr[1]->info.factor->data.f;
		else
			f2 = exp->children.arr[1]->info.factor->data.i;

		t2 = exp->children.arr[1]->sub_type;
	}
	else if (exp->children.count == 2)
		return exp;

	union Data d;
	switch (exp->sub_type) { 
		case '+':
			d.f = f1 + f2;
			break;
		case '-':
			d.f = f1 - f2;
			break;
		case '/':
			d.f = f1 / f2;
			break;
		case '*':
			d.f = f1 * f2;
			break;
		case '&':
			d.f = (int)f1 & (int)f2;
			break;
		default:
			return exp;
	}
	if (t1 == 'i' && t2 == 'i') {
	 	d.i = (int) d.f;
	 	t = 'i';
	}
	else
		t = 'f';
	return make_factor(t, d, NULL, exp->line);	
}