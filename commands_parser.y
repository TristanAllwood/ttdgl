%{
#include <stdio.h>

extern int yylex();
extern int yyparse();

static void yyerror(char * s);

%}

%token STRING NUMBER

%%

toplevel: command                     { YYACCEPT }

command	: ']' NUMBER ';'              { printf("set title\n");      /* case on number, if 0 -> read bytes up to bell and set title. (whatever that means) */ }
	| '[' attr_list 'm'           { printf("set attr list\n");  /* done via the recursive case */ }
        | '[' NUMBER ';' NUMBER 'H'   { printf("goto position\n");  /* go to absolute position */ }
        ;

attr_list: 
         | ne_attr_list               { /* set attribute number */ }
         ;

ne_attr_list: NUMBER
            | NUMBER ';' attr_list { /* process lhs, then set attribute number */ }
            ;

%%

static void yyerror(char * s) {
  printf("Unknown escape command: %s\n", s);
}
