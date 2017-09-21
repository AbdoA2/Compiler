#ifndef NODES_FILE
#define NODES_FILE

typedef int bool;
#define true 1
#define false 0

struct DeclarationHead {			
	char type;								// 'V' for variable, 'C' for constant
	char data_type;							// 'i' for integer, 'f' for float
};

struct Declaration {
	int value; 								// the initial integer value of integer variable
	float fvalue; 							// the initial float value of float variable
	char *name; 							// the variable name
	bool init; 								// boolean to check if the variable is initialized in declaration
	char arr; 								// '1' if the variable is array else '0'
};

struct Function {				
	char* name;								// function name
	int return_type; 						// function return type 'i' for integer, 'f' for float or 'v' for void
};

struct Arg {
	char *name; 							// arg name
	char type;								// arg data type 'i' for integer, 'f' for float 
	char arr;								// '1' if the arg is array else '0'
};

// this union holds a pointer to an info object
union Info {
	struct DeclarationHead *decl_head;
	struct Declaration *decl;
	struct Arg *arg;
	struct Function *function;
	struct Loc *loc;
	struct Factor *factor;
};

struct Loc {
	char *name; 							// variable or array name
};

// union for data integer, float or name
union Data {
	int i;
	float f;
	char *name;
};

// factor info object holds a data union
struct Factor {
	union Data data;
};

// vector of nodes pointers
struct Vector {
	int count;								// vector elements count
	int max;								// max size
	struct Node **arr; 						// array pointer
};

// node structure
/*
type:
	'P' --> Program (the tree root) (Declarations and Functions)
	'F' --> Function (Args, Block)
	'D' --> Declaration (declarations ('d' nodes))
			sub type:
					 'C' constant
					 'V' variable
	'S' --> Statement (variable number of children)
	        sub type:
	        		 'A' --> Assign (location factor, expression)
	        		 'I' --> IF (expression, statement)
	        		 'E' --> IF-ELSE (expression, statement, statement)
	        		 'F' --> For Loop(expression, statement, assign, assign)
	        		 'W' --> While Loop (expression, statement)
	        		 'U' --> Repeat Unitil Loop (expression, statement)
	        		 'D' --> Do While Loop (expression, statement)
	        		 'C' --> Continue
	        		 'B' --> Block
	        		 'R' --> Return
	        		 'c' --> Function Call
	'B' --> Block (varaiable number of statements and declaration nodes)
	'A' --> Args (variable number of 'a' nodes in case of function definition, 'E' nodes in function call)
	'a' --> Arg node
			sub type:
					 'd' --> arg definition
	'e' --> Empty node
	'E' --> Expression node
			sub type:
					 '+', '-', '/', '*', '&', '|', '!', 'E' (==), 'N' (!=)
					 '0' (<), '1' (<=), '2' (>=), '3' (>)
	'f' --> Factor node
			sub type:
					 'i' --> integer value
					 'f' --> float value
					 'c' --> function call (has 'A' node whose children are 'E' nodes)
					 'l' --> location (has child node expr if it is an array position)

line: the statement line number at the source file
info: pointer to info object
children: node children vector
*/ 
struct Node {
	char type; 
	char sub_type;
	int line;
	union Info info;
	struct Vector children;
};


void append(struct Node *p, struct Node *n);

#endif