%{
#include <stdio.h>
#include "syntax.tab.h"

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;

void yyerror(const char *str)
{
        fprintf(stderr,"%s at line %d\n", str, yylineno);
}
%}


%option yylineno

%%
int|float  				yylval=strdup(yytext); return BASIC;
if						return IF;
else					return ELSE;
for						return FOR;
do						return DO;
while					return WHILE;
switch					return SWITCH;
break					return BREAK;
continue				return CONTINUE;
true					return TRUE;
false					return FALSE;
void					yylval=strdup(yytext); return VOID;
return					return RETURN;
case					return CASE;
const    				return CONST;
default 				return DEFAULT;
function 				return FUNCTION;
repeat					return REPEAT;
until					return UNTIL;
[a-zA-Z_][a-zA-Z0-9_]*  yylval=strdup(yytext); return ID;
[0-9]+					yylval=strdup(yytext); return NUM;
[0-9]+\.[0-9]+			yylval=strdup(yytext); return REAL;
\<\=					return LE;
\>\=					return GE;
\=\=					return EQ;
\!\=					return NE;
\=						return '=';
\:						return ':';
\<						return '<';
\>						return '>';
\!						return '!';
\&\&					return AND;
\|\|					return OR;
\{                      return '{';
\}                      return '}';
\[						return '[';
\]						return ']';
\(						return '(';
\)						return ')';
\+						return '+';
\-						return '-';
\*						return '*';
\/						return '/';
\,						return ',';
;                       return ';';
\n                      /* ignore EOL */;
[ \t]+                  /* ignore whitespace */;
%%