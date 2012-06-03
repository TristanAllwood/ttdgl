%{
#include <stdio.h>

#include "ttdgl_state.h"

static void yyerror(char * s);

extern int yylex();
extern int yyparse();
extern int total_command_length;
extern void * yy_scan_bytes(const char * bytes, int len);
extern int yyparse(void);
extern void yy_delete_buffer(void * data);

%}

%token STRING NUMBER

%%

toplevel: command                     { printf("size: %i\n", total_command_length); YYACCEPT; }

command	: ']' NUMBER ';'              { printf("set title\n");      /* case on number, if 0 -> read bytes up to bell and set title. (whatever that means) */ }
	| '[' attr_list 'm'           { printf("set attr list\n");  /* done via the recursive case */ }
        | '[' NUMBER ';' NUMBER 'H'   { printf("goto position\n");  /* go to absolute position */ }
        | '[' '?' NUMBER 'h'          { printf("various commands\n"); }
        ;

attr_list: 
         | ne_attr_list               { /* set attribute number */ }
         ;

ne_attr_list: NUMBER
            | NUMBER ';' attr_list { /* process lhs, then set attribute number */ }
            ;

%%

int parse_command(char * buffer, size_t buffer_size) {

  total_command_length = 0;

  void * hdl = yy_scan_bytes(buffer, buffer_size);
  int ret = yyparse();
  yy_delete_buffer(hdl);
    
  /* if (position >= buffer_count) { */
  if (ret != 0) {
    fprintf(stderr, "TODO: handle escape code run %i\n", ret);
    return 0;
  }

  char * tmp = calloc(total_command_length + 1, sizeof(char));
  strncpy(tmp, buffer, total_command_length);
  printf("Parsed command: %s\n", tmp);
  free(tmp);

  return total_command_length;
}

static void yyerror(char * s) {
  printf("Unknown escape command: %s\n", s);
}
