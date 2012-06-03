%{

#include "commands_parser.h"

int total_command_length;

%}

%option nounput
%option noinput

%%
"]"             { total_command_length += yyleng ; return ']'; }
"["             { total_command_length += yyleng ; return '['; }
[;?a-zA-Z]      { total_command_length += yyleng ; return yytext[0]; }

[0-9]+          { total_command_length += yyleng ; return NUMBER; }

%%

int yywrap() {
  return 0;
}
