#include "nodes.h"

// appends node n as a child of node p (double p children array size if it is full)
void append(struct Node *p, struct Node *n) {
	// if the node is a leaf allocate array for 4 children
	if (p->children.count == 0) {
		p->children.max = 4;
		p->children.arr = malloc(4 * sizeof(struct Node*));
		p->children.arr[0] = n;
	}
	else if (p->children.count < p->children.max) {
		p->children.arr[p->children.count] = n;
	}
	else {
		p->children.max = p->children.max * 2;
		p->children.arr = realloc(p->children.arr, p->children.max * sizeof(struct Node*));
		p->children.arr[p->children.count] = n;
	}
	p->children.count += 1;
}
