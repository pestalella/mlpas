%language "c++"
%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.5.1"
//%header
%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires {
# include <string>
class ParserDriver;

class ParsedLine {
public:
    std::string label;
    std::string opcode;
    uint16_t arg0_reg;
    std::string arg0_label;
    uint16_t arg1_reg;
    uint16_t arg1_imm;
    std::string arg1_label;
    uint16_t arg2_reg;
    std::string arg2_label;
};
}
// The parsing context.
%param { ParserDriver &driver }
%locations
%define parse.trace
%define parse.error verbose
%define parse.lac full
%code {
# include "parser_driver.h"
}

%define api.token.prefix {TOK_}
%token
  HASH
  COLON
  SEMICOLON
  COMMA
  OP_MOV
  OP_LOAD
  OP_STORE
  OP_ADD  
  OP_SUB  
  OP_JNZ  
  OP_JZ   
  OP_JMP  
  OP_NOP
;
%token YYEOF 0
%token <std::string> IDENTIFIER
%token <int> INTEGER
%token <int> REGISTER
%nterm <std::string> line_label
%nterm <ParsedLine> line
%nterm <ParsedLine> instruction
%nterm <int> immediate

%printer { yyo << 
            "label=" << $$.label << " " <<
            "opcode=" << $$.opcode << " " <<
            "arg0=" << $$.arg0_reg << " " <<
            "arg1=" << $$.arg1_reg << " " <<
            "arg2=" << $$.arg2_reg; } <ParsedLine>;

%%
%start unit;
unit: 
    lines  { }
    ;

lines:
    %empty
    | lines line {
        driver.lines.push_back($2);
    };

line:
    line_label instruction { 
        $$ = $2;
        $$.label = $1; }
    ;

line_label:
    %empty {$$ = "";}
    | IDENTIFIER COLON {
        $$ = $1; }
    ;

instruction:
      OP_MOV   REGISTER COMMA immediate {
        $$.opcode = "mov";
        $$.arg0_reg = $2;
        $$.arg1_imm = $4; }
    | OP_LOAD  REGISTER COMMA REGISTER COLON REGISTER {
        $$.opcode = "load";
        $$.arg0_reg = $2;
        $$.arg1_reg = $4;
        $$.arg2_reg = $6; }
    | OP_STORE REGISTER COLON REGISTER COMMA REGISTER { 
        $$.opcode = "store";
        $$.arg0_reg = $6;
        $$.arg1_reg = $2;
        $$.arg2_reg = $4; }
    | OP_ADD   REGISTER COMMA REGISTER COMMA REGISTER { 
        $$.opcode = "addr";
        $$.arg0_reg = $2;
        $$.arg1_reg = $4;
        $$.arg2_reg = $6; }
    | OP_ADD   REGISTER COMMA immediate { 
        $$.opcode = "addi";
        $$.arg0_reg = $2;
        $$.arg1_imm = $4; }
    | OP_SUB   REGISTER COMMA REGISTER COMMA REGISTER { 
        $$.opcode = "subr";
        $$.arg0_reg = $2;
        $$.arg1_reg = $4; }
    | OP_SUB   REGISTER COMMA immediate { 
        $$.opcode = "subi";
        $$.arg0_reg = $2;
        $$.arg1_imm = $4; }
    | OP_JNZ   IDENTIFIER { 
        $$.opcode = "jnz";
        $$.arg0_label = $2; }
    | OP_JZ    IDENTIFIER { 
        $$.opcode = "jz";
        $$.arg0_label = $2; }
    | OP_JMP   REGISTER COLON REGISTER { 
        $$.opcode = "jmp";
        $$.arg0_reg = $2;
        $$.arg1_reg = $4;  }
    | OP_NOP { 
        $$.opcode = "nop"; }
    ;

immediate:
    HASH INTEGER { $$ = $2; }
%%