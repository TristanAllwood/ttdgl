%{

#include "commands_parser.h"

%}

%option nounput
%option noinput

%%
"]"             { return ']'; }
";"             { return ';'; }
"["             { return '['; }
[a-zA-Z]        { return yytext[0]; }

[0-9]+          { return NUMBER; }

%%

int yywrap() {
  return 0;
}
