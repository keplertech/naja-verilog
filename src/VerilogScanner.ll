/*
 * SPDX-FileCopyrightText: 2023 The Naja verilog authors <https://github.com/keplertech/naja-verilog/blob/main/AUTHORS>
 * 
 * SPDX-License-Identifier: Apache-2.0
*/

%{
/* C++ string header, for string ops below */
#include <string>
#include <sstream>

#include "VerilogException.h"

/* Implementation of yyFlexScanner */ 
#include "VerilogScanner.h"
#undef  YY_DECL
#define YY_DECL \
  int naja::verilog::VerilogScanner::yylex( \
    naja::verilog::VerilogParser::semantic_type* const lval, \
    naja::verilog::VerilogParser::location_type*loc)

/* typedef to make the returns for the tokens shorter */
using token = naja::verilog::VerilogParser::token;

/* define yyterminate as this instead of NULL */
#define yyterminate() return( token::END )

/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

/* update location on matching */
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

%}

%option debug
/* %option nodefault */
%option yyclass="naja::verilog::VerilogScanner"
%option noyywrap
%option nounput
%option never-interactive
%option c++

%x in_comment
%x based_const

/* Predefined rules */
NEWLINE       "\n"|"\r\n"
SPACE         " "|"\t"|"\f"
COMMENT_BEGIN "/*"
COMMENT_END   "*/" 
COMMENT_LINE  "//".*\n

IDENTIFIER [_a-zA-Z][$_a-zA-Z0-9]*
UNSIGNED_NUMBER [0-9][0-9_]*
STRING \"([^\"\\]|\\.)*\"


/* 
  Check the escape rule inside character class 
  https://www.regular-expressions.info/charclass.html
*/
ESCAPED_IDENTIFIER \\[\\^!"#$%&',()*+\-.a-zA-Z0-9/{|}~[\]_:;<=>?@]+[\n\t\f ]

%%
%{          /** Code executed at the beginning of yylex **/
  yylval = lval;
%}

<*>{SPACE}  { /* ignore any space */ }
{NEWLINE}   {
              // Update line number
              loc->lines();
            }

{COMMENT_LINE}  { loc->lines(); }

{COMMENT_BEGIN} { BEGIN(in_comment); }
<in_comment><<EOF>> {
  BEGIN(INITIAL);
  std::ostringstream reason;
  reason << "Unclosed comment at line " 
    << loc->end.line << " col " << loc->end.column;
  throw VerilogException(reason.str());
}

<in_comment>{NEWLINE} { loc->lines(); }
<in_comment>. { /* ignore characters in comment */ }
<in_comment>{COMMENT_END} { BEGIN(INITIAL); }

"."|","|";"|"("|")"|"*"|"#"|"["|"]"|":"|"{"|"}"|"=" {
  return yytext[0];
}

"+"|"-"     { yylval->build<std::string>( yytext ); return token::SIGN_TK; }

module      { return token::MODULE_KW; }
endmodule   { return token::ENDMODULE_KW; }
input       { return token::INPUT_KW; } 
output      { return token::OUTPUT_KW; }
inout       { return token::INOUT_KW; }
wire        { return token::WIRE_KW; }
signed      { return token::SIGNED_KW; }
supply0     { return token::SUPPLY0_KW; }
supply1     { return token::SUPPLY1_KW; }
assign      { return token::ASSIGN_KW; }
defparam    { return token::DEFPARAM_KW; }
and         { return token::AND_KW; }
nand        { return token::NAND_KW; }
or          { return token::OR_KW; }
nor         { return token::NOR_KW; }
xor         { return token::XOR_KW; }
xnor        { return token::XNOR_KW; }
buf         { return token::BUF_KW; }
not         { return token::NOT_KW; }

{IDENTIFIER}          { yylval->build<std::string>( yytext ); return token::IDENTIFIER_TK; }
{ESCAPED_IDENTIFIER}  {
  //if the character ending the escape identifier is '\n'
  //we need to update the line number
  if (yytext[strlen(yytext)-1] == '\n') {
    loc->lines();
  }
  yylval->build<std::string>( yytext );
  return token::ESCAPED_IDENTIFIER_TK; 
}
{STRING}              { yylval->build<std::string>( yytext ); return token::STRING_TK; }

{UNSIGNED_NUMBER} {
	yylval->build<std::string>(yytext);
	return token::CONSTVAL_TK;
}

\'[sS]?[bodhBODH] {
	BEGIN(based_const);
  //we don't need the "'" first character and we are sure that yytext is 's?b|o|...
	yylval->build<std::string>(yytext+1);
	return token::BASE_TK;
}

<based_const>[0-9a-fA-FzxZX?][0-9a-fA-FzxZX?_]* {
	BEGIN(INITIAL);
	yylval->build<std::string>(yytext);
	return token::BASED_CONSTVAL_TK;
}

 /* Last rule catches everything */
 /* LCOV_EXCL_START */
. {
  std::ostringstream reason;
  reason << "Failed to match: " << yytext << " at line "
    << loc->end.line << " col " << loc->end.column;
  throw VerilogException(reason.str());
}
 /* LCOV_EXCL_STOP */

%%
