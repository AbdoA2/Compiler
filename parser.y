%{
#include <stdio.h>
#include <string.h>
#include "translate.h"
#include "sym.h"

struct scope *global;
struct scope *current_scope;
 
int yywrap()
{
        return 1;
} 
  
main()
{
	global = malloc(sizeof(struct scope));
	current_scope = global;
	global->count = 0;
	global->max = 4;
	global->parent = NULL;
	global->table = NULL;

    yyparse();
} 

%}

%glr-parser
%error_verbose

%locations

%token ID NUM REAL DO IF ELSE WHILE REPEAT UNTIL CONTINUE BREAK CONST FUNCTION BASIC
%token INT FLOAT TRUE FALSE VOID NE EQ GE LE AND OR SWITCH CASE DEFAULT RETURN FOR

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
program:
		externs { translate($$, global); }
		;

externs:
		externs extern { $$ = join_nodes('P', $1, $2); }
		|			   { $$ = make_empty_node(); }
		;

extern:
		decl 			{ $$ = $1; }
		|
		function 		{ $$ = $1; }
		;

function:
		FUNCTION ID '(' args ')' ':' return_type block 
		{ 
			char* r = $7; struct Node *a = $4;
			int i = 0, args_count;
			struct Arg **args;
            if (a->type == 'a') {
                args = malloc(1 * sizeof(struct Arg*));
                args[0] = a->info.arg;
                struct Node *ar = malloc(sizeof(struct Node));
                ar->children.count = 0;
                ar->type = 'A';
                append(ar, a);
                a = ar;
                args_count = 1;
            }
            else {
                args = malloc(a->children.count * sizeof(struct Arg*));
                args_count = a->children.count;
                for (i = 0; i < a->children.count; i++) {
                if (a->children.arr[i]->type != 'e')
                    args[i] = a->children.arr[i]->info.arg;
                }
            }
			struct symbol *s = make_symbol($2, 'f', r[0], 0, 0, args, args_count, @1.first_line, false);
			insert_symbol(&(global->table), s);

            $$ = make_function($2, a, $8);
		}
		|
		error '{'
		;

block:
		'{' constructs '}'		{ struct Node *c = $2; c->type = 'B'; $$ = c; }
		;

return_type:
		VOID					
		|
		BASIC				
		;


args:
		args ',' arg 			{ $$ = join_nodes('A', $1, $3); }
		|
		arg						{ $$ = $1; }
		;

arg:
		BASIC ID 				{ $$ = make_arg($1, $2, '0'); }
        |
        BASIC ID '[' ']'        { $$ = make_arg($1, $2, '1'); }
		;

constructs:
		constructs construct 	{ $$ = join_nodes('C', $1, $2); }
		|						{ $$ = make_empty_node(); }
		;

construct:
		decl					{ $$ = $1; }
		|
		stmt 					{ $$ = $1; }
		;

decl:
		BASIC vars ';'						{ $$ = make_decl_head('V', $1, $2, @1.first_line); }
		|
		CONST BASIC ID '=' const_val ';' 	{ $$ = make_decl_head('C', $2, make_decl($3, atoi($5), atof($5), true, '0'), @1.first_line); }
		;

vars:
		ID 									{ $$ = make_decl($1, 0, 0, false, '0'); }
        |
        ID '[' NUM ']'                      { $$ = make_decl($1, atoi($3), 0, false, '1'); }
		|
		ID '=' const_val					{ $$ = make_decl($1, atoi($3), atof($3), true, '0'); }
		|
		vars ',' ID 						{ $$ = join_nodes('D', $1, make_decl($3, 0, 0, false, '0')); }
        |
        vars ',' ID '[' NUM ']'             { $$ = join_nodes('D', $1, make_decl($3, atoi($5), 0, false, '1')); }
		|
		vars ',' ID '=' const_val			{ $$ = join_nodes('D', $1, make_decl($3, atoi($5), atof($5), true, '0')); }
		;

const_val:
    NUM
    |
    REAL                                                                                     
    ;

stmt:
        assign ';'                                          { $$ = $1; }
        |
        IF '(' bool ')' stmt                                { $$ = make_stmt('I', @1.first_line, 2, $3, $5); } %prec LOWER_THAN_ELSE;    
        |
        IF '(' bool ')' stmt ELSE stmt                      { $$ = make_stmt('E', @1.first_line, 3, $3, $5, $7); }
        |
        WHILE '(' bool ')' stmt                             { $$ = make_stmt('W', @1.first_line, 2, $3, $5); }
        |
        DO stmt WHILE '(' bool ')' ';'                      { $$ = make_stmt('D', @1.first_line, 2, $5, $2); }
        |
        FOR '(' assign ';' bool ';' assign ')' stmt   		{ $$ = make_stmt('F', @1.first_line, 4, $5, $9, $3, $7); }        
        |
        REPEAT stmt UNTIL '(' bool ')' ';'                 	{ $$ = make_stmt('U', @1.first_line, 2, $5, $2); }
        |
        BREAK ';'                                          	{ $$ = make_stmt('B', @1.first_line, 0); }
        |
        func_call ';'                                		{ $$ = $1; }
        |
        CONTINUE ';'                                        { $$ = make_stmt('C', @1.first_line, 0); }
        |
        block                                               { $$ = $1; }
        |
        RETURN bool ';'                                     { $$ = make_stmt('R', @1.first_line, 1, $2); }
        |
        SWITCH '(' bool ')' '{' cases default '}'            { $$ = make_stmt('S', @1.first_line, 3, $3, $6, $7); }
        |
        error ';'
        |
        error '{'
        ;

assign:
        loc '=' bool                                        { $$ = make_stmt('A', @1.first_line, 2, $1, $3); }
        ;

cases:
        case                                                { $$ = $1; }
        |
        cases case                                          { $$ = join_nodes('W', $1, $2); }
        ;

default:
        DEFAULT ':' stmt 									{ union Info info; $$ = make_node('C', '1', info); append($$, $3); }
        |                                                   { $$ = make_empty_node(); }
        ;

case:
    CASE REAL ':' stmt 										{ 
    															union Info info; info.factor = malloc(sizeof(struct Factor));
    															info.factor->data.f = atof($2);
    															struct Node *p = make_node('C', 'f', info);
                                                                p->line = @1.first_line;
    															append(p, $4);
    															$$ = p;
    														}

   	|
   	CASE NUM ':' stmt 										{ 
    															union Info info; info.factor = malloc(sizeof(struct Factor));
    															info.factor->data.i = atoi($2);
    															struct Node *p = make_node('C', 'i', info);
                                                                p->line = @1.first_line;
    															append(p, $4);
    															$$ = p;
    														}                          
    ;



loc:
        ID '[' bool ']'     								{ $$ = make_loc($1, $3, @1.first_line); }
        |
        ID                  								{ $$ = make_loc($1, NULL, @1.first_line); }
        ;

bool:   
        bool OR join        								{ $$ = make_expr('O', $1, $3); }
        |
        join                								{ $$ = $1; }
        ;

join:
        join AND equality   								{ $$ = make_expr('A', $1, $3); }
        |
        equality            								{ $$ = $1; }
        ;

equality:
        equality EQ rel    									{ $$ = make_expr('E', $1, $3); }
        |
        equality NE rel     								{ $$ = make_expr('N', $1, $3); }
        |
        rel                 								{ $$ = $1; }
        ;

rel:
        expr '<' expr       								{ $$ = make_expr('0', $1, $3); }
        |
        expr LE expr  										{ $$ = make_expr('1', $1, $3); }       
        |
        expr GE expr        								{ $$ = make_expr('2', $1, $3); }
        |
        expr '>' expr      									{ $$ = make_expr('3', $1, $3); }
        |
        expr                								{ $$ = $1; }
        ;

expr:
        expr '+' term      									{ $$ = make_expr('+', $1, $3); }
        |
        expr '-' term       								{ $$ = make_expr('-', $1, $3); }
        |
        term                								{ $$ = $1; }
        ;

term:
        term '*' unary      								{ $$ = make_expr('*', $1, $3); }
        |
        term '/' unary      								{ $$ = make_expr('/', $1, $3); }
        |
        unary              									{ $$ = $1; }
        ;

unary:
        '!' unary           								{ $$ = make_expr('!', $2, NULL); }
        |
        '-' unary           								{ $$ = make_expr('4', $2, NULL); }
        |
        factor              								{ $$ = $1; }
        ;

factor:
        '(' bool ')'        								{ $$ = $2; }
        |
        func_call											{ struct Node *n = $1; union Data d; d.i=0; $$ = $1; $$ = make_factor('c', d, n, @1.first_line); }
        |
        loc                 								{ union Data d; $$ = make_factor('l', d, $1, @1.first_line); }
        |
        NUM  												{ union Data d; d.i = atoi($1); $$ = make_factor('i', d, NULL, @1.first_line); }               
        |
        REAL												{ union Data d; d.f = atof($1); $$ = make_factor('f', d, NULL, @1.first_line); }                
        |
        TRUE                								{ union Data d; d.i = 1; $$ = make_factor('i', d, NULL, @1.first_line); }
        |
        FALSE               								{ union Data d; d.i = 0; $$ = make_factor('i', d, NULL, @1.first_line); }

fargs:
        bool                								{ $$ = $1; }                
        |
        fargs ',' bool      								{ $$ = join_nodes('A', $1, $3); }
        ;


func_call:
		ID '(' fargs ')'									
        { 
            struct Node *n = $3;
            if (n->type != 'A'){
                struct Node *c = malloc(sizeof(struct Node));
                c->type = 'A';
                c->children.count = 0;
                append(c, n);
                n = c; 
            }
        	struct Function *func = malloc(sizeof(struct Function)); func->name = $1; 
        	union Info info;
        	info.function = func;
        	struct Node *s=make_stmt('c', @1.first_line, 1, n);
        	s->info = info;
        	$$ = s;
	    }

%%