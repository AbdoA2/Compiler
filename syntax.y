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
		externs;

externs:
		externs extern 
		|			   
		;

extern:
		decl
		|
		function
		;

function:
		FUNCTION ID '(' args ')' ':' return_type block
		|
		error '{'
		;

block:
		'{' constructs '}'
		;

return_type:
		VOID					
		|
		BASIC				
		;


args:
		args ',' arg
		|
		arg
		;

arg:
		BASIC ID
        |
        BASIC ID '[' ']'
		;

constructs:
		constructs construct
		|
		;

construct:
		decl
		|
		stmt
		;

decl:
		BASIC vars ';'
		|
		CONST BASIC ID '=' const_val ';'
		;

vars:
		ID
        |
        ID '[' NUM ']'
		|
		ID '=' const_val
		|
		vars ',' ID
        |
        vars ',' ID '[' NUM ']'
		|
		vars ',' ID '=' const_val
		;

const_val:
    NUM
    |
    REAL                                                                                     
    ;

stmt:
        assign ';'
        |
        IF '(' bool ')' stmt                            %prec LOWER_THAN_ELSE;    
        |
        IF '(' bool ')' stmt ELSE stmt
        |
        WHILE '(' bool ')' stmt
        |
        DO stmt WHILE '(' bool ')' ';'
        |
        FOR '(' assign ';' bool ';' assign ')' stmt        
        |
        REPEAT stmt UNTIL '(' bool ')' ';'
        |
        BREAK ';'                        
        |
        func_call ';'                    
        |
        CONTINUE ';'                  
        |
        block                         
        |
        RETURN bool ';'               
        |
        SWITCH '(' bool ')' '{' cases default '}' 
        |
        error ';'
        |
        error '{'
        ;

assign:
        loc '=' bool                          
        ;

cases:
        case                                  
        |
        cases case                            
        ;

default:
        DEFAULT ':' stmt 					
        |                                   
        ;

case:
    CASE REAL ':' stmt
   	|
   	CASE NUM ':' stmt                   
    ;


loc:
        ID '[' bool ']'
        |
        ID         
        ;

bool:   
        bool OR join   
        |
        join           
        ;

join:
        join AND equality  
        |
        equality           
        ;

equality:
        equality EQ rel    
        |
        equality NE rel    
        |
        rel                
        ;

rel:
        expr '<' expr      
        |
        expr LE expr  		       
        |
        expr GE expr        
        |
        expr '>' expr      	
        |
        expr                
        ;

expr:
        expr '+' term      	
        |
        expr '-' term       
        |
        term                
        ;

term:
        term '*' unary      
        |
        term '/' unary     
        |
        unary              
        ;

unary:
        '!' unary          
        |
        '-' unary          
        |
        factor             
        ;

factor:
        '(' bool ')'       
        |
        func_call			
        |
        loc                 
        |
        NUM  				               
        |
        REAL				                
        |
        TRUE                
        |
        FALSE               

fargs:
        bool                                
        |
        fargs ',' bool      
        ;


func_call:
		ID '(' fargs ')'
%%