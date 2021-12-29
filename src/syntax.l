%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring> // strerror
#include <string>
#include "parser_driver.h"
#include "grammar.hpp"
%}
%option noyywrap nounput noinput batch debug

%{
  #define YY_USER_ACTION  loc.columns(yyleng);
%}
%%
%{
  // A handy shortcut to the location held by the driver.
  yy::location &loc = driver.location;
  // Code run each time yylex is called.
  loc.step ();
%}
\s+             loc.step();
\n+             loc.lines(yyleng); loc.step();
":"             return yy::parser::make_COLON(loc);
","             return yy::parser::make_COMMA(loc);
";"             return yy::parser::make_SEMICOLON(loc);
"#"             return yy::parser::make_HASH(loc);
"mov"           return yy::parser::make_OP_MOV(loc);
"load"          return yy::parser::make_OP_LOAD(loc);
"store"         return yy::parser::make_OP_STORE(loc);
"add"           return yy::parser::make_OP_ADD(loc);
"sub"           return yy::parser::make_OP_SUB(loc);
"jnz"           return yy::parser::make_OP_JNZ(loc);
"jz"            return yy::parser::make_OP_JZ(loc);
"jmp"           return yy::parser::make_OP_JMP(loc);
"nop"           return yy::parser::make_OP_NOP(loc);
r[0-9][0-9]?    return yy::parser::make_REGISTER(std::stoi(&yytext[1]), loc);
[a-z][a-z0-9]*  return yy::parser::make_IDENTIFIER(yytext, loc);
[0-9]+          return yy::parser::make_INTEGER(std::stoi(yytext), loc);
<<EOF>>         return yy::parser::make_YYEOF(loc);
%%

void ParserDriver::scan_begin()
{
  yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      std::cerr << "cannot open " << file << ": " << strerror (errno) << '\n';
      exit (EXIT_FAILURE);
    }
}

void ParserDriver::scan_end()
{
  fclose (yyin);
}
