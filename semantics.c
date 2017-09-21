#include <stdio.h>
#include "semantics.h"
#include "nodes.h"

void semantic_error(int e, char *name, int l1, int l2) {
	switch(e) {
		case 1:
			fprintf(stderr, "semantic error:%d: variable %s was declared before at line %d.\n", l1, name, l2);
			break;
		case 2:
			fprintf(stderr, "semantic error:%d: variable %s wasn't declared before.\n", l1, name);
			break;
		case 3:
			fprintf(stderr, "semantic error:%d: can't use break outside a loop.\n", l1);
			break;
		case 4:
			fprintf(stderr, "semantic error:%d: can't use continue outside a loop.\n", l1);
			break;
		case 5:
			fprintf(stderr, "semantic warning:%d: assigning float value to integer variable (may lose data).\n", l1);
			break;
		case 6:
			fprintf(stderr, "semantic error:%d: redeclaration of function parameter %s.\n", l1, name);
			break;
		case 7:
			fprintf(stderr, "semantic error:%d: trying to get a return value from a void function %s.\n", l1, name);
			break;
		case 8:
			fprintf(stderr, "semantic error:%d: trying to assign constant %s.\n", l1, name);
			break;
		case 9:
			fprintf(stderr, "semantic warning:%d: passing float value to integer parameter %s.\n", l1, name);
			break;
		case 10:
			fprintf(stderr, "semantic error:%d: function %s is not declared.\n", l1, name);
			break;
		case 11:
			fprintf(stderr, "semantic error:%d: the number of passed parameters is not the same as function %s parameters.\n", l1, name);
			break;
		case 12:
			fprintf(stderr, "semantic warning:%d: initializing integer variable %s with float value.\n", l1, name);
			break;
		case 13:
			fprintf(stderr, "semantic error:%d: using float value as a case for integer expression.\n", l1);
			break;
		case 14:
			fprintf(stderr, "semantic error: the program doesn't have a main function with argc and argv array.\n");
			break;
		case 15:
			fprintf(stderr, "semantic error:%d: trying to index an array %s using a float value.\n", l1, name);
			break;
		case 16:
			fprintf(stderr, "semantic error:%d: reading value from uninitialized variable %s.\n", l1, name);
			break;
		case 17:
			fprintf(stderr, "semantic error:%d: accessing array %s as a single variable.\n", l1, name);
			break;
		case 18:
			fprintf(stderr, "semantic error:%d: accessing variable %s as an array.\n", l1, name);
			break;
	}
}

int check_main(struct Node *n) {
	int i, ans=0;
	for (i=0; i < n->children.count; i++) {
		if (n->children.arr[i]->type == 'F') {
			struct Node *func = n->children.arr[i]; 
			if (strcmp(func->info.function->name, "main") == 0) {
				if (func->children.arr[0]->children.count != 2)
					continue;
				struct Arg *a1 = func->children.arr[0]->children.arr[0]->info.arg;
				struct Arg *a2 = func->children.arr[0]->children.arr[1]->info.arg;
				if (strcmp(a1->name, "argc") != 0 || a1->type != 'i' || a1->arr == '1')
					continue;
				if (strcmp(a2->name, "argv") != 0 || a2->type != 'i' || a2->arr == '0')
					continue;
				ans = 1;
			}
		}
	}
	return ans;
}