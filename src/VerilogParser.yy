// SPDX-FileCopyrightText: 2023 The Naja verilog authors <https://github.com/keplertech/naja-verilog/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

%skeleton "lalr1.cc"
%require  "3.0"
%define parse.trace
%defines 
%define api.namespace {naja::verilog}
%language "c++"
%define api.parser.class {VerilogParser}

%define parse.error verbose

%code requires {

  #include "VerilogTypes.h"
  namespace naja { namespace verilog {
    class VerilogScanner;
    class VerilogConstructor;
  }}

  namespace {
    using Instances = std::vector<std::string>;
  }

// The following definitions is missing when %locations isn't used
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

}

%parse-param { VerilogScanner& scanner }
%parse-param { VerilogConstructor* constructor }

%code{

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <utility>

#include "VerilogException.h"

/* include for all driver functions */ 
#include "VerilogConstructor.h"
#include "VerilogScanner.h"


#undef yylex
#define yylex scanner.yylex

size_t portIndex = 0;

static naja::verilog::Number generateNumber(
  bool hasSize,
  const std::string& size,
  const std::string& base,
  const std::string& digits,
  int beginLine,
  int beginColumn,
  int endLine,
  int endColumn) {
  if (base.size() == 2) {
    //LCOV_EXCL_START
    if (not (base[0] == 's' || base[0] == 'S')) {
      //Following should not happen as long as lexer is correct
      //should this be replaced by an assertion ?
      std::ostringstream reason;
      reason << "Parser error: ";
      if (hasSize) {
        reason << "\'" << size << base << digits;
      } else {
        reason << "\'" << base << digits; 
      }
      reason << " is not a valid number: wrong signed character.\n"
        << "  begin at line " << beginLine <<  " col " << beginColumn  << '\n' 
        << "  end   at line " << endLine <<  " col " << endColumn << "\n";
      throw naja::verilog::VerilogException(reason.str());
    }
    //LCOV_EXCL_STOP
    if (hasSize) {
      return naja::verilog::Number(size, true, base[1], digits);
    } else {
      return naja::verilog::Number(true, base[1], digits);
    }
  } else if (base.size() == 1) {
    if (hasSize) {
      return naja::verilog::Number(size, false, base[0], digits);
    } else {
      return naja::verilog::Number(false, base[0], digits);
    }
  } else {
    //LCOV_EXCL_START
    //Same as previously: should not be accessible, as this is filtered by lexer.
    std::ostringstream reason;
    reason << "Parser error: ";
    if (hasSize) {
        reason << "\'" << size << base << digits;
    } else {
        reason << "\'" << base << digits; 
    }
    reason << " is not a valid number\n"
      << "  begin at line " << beginLine <<  " col " << beginColumn  << '\n' 
      << "  end   at line " << endLine <<  " col " << endColumn << "\n";
    throw naja::verilog::VerilogException(reason.str());
  }
  //LCOV_EXCL_STOP
}

}

%define api.value.type variant
%define parse.assert

%token MODULE_KW
%token ENDMODULE_KW
%token INOUT_KW
%token INPUT_KW
%token OUTPUT_KW
%token SUPPLY0_KW
%token SUPPLY1_KW
%token WIRE_KW
%token SIGNED_KW
%token ASSIGN_KW
%token DEFPARAM_KW
%token END 0 "end of file"
%token AND_KW
%token NAND_KW
%token OR_KW
%token NOR_KW
%token XOR_KW
%token XNOR_KW
%token BUF_KW
%token NOT_KW
//%token END 0 "end of file"

%token<std::string> IDENTIFIER_TK
%token<std::string> ESCAPED_IDENTIFIER_TK
%token<std::string> STRING_TK
%token<std::string> CONSTVAL_TK BASE_TK BASED_CONSTVAL_TK
%token<std::string> SIGN_TK

//%left '[' ']'
//%nonassoc LOWER_THAN_RANGE

%type<naja::verilog::Identifier> identifier;
//no support for XMRs for the moment
%type<naja::verilog::Identifier> hierarchical_net_identifier;
%type<naja::verilog::Identifier> module_identifier;
%type<naja::verilog::Identifier> name_of_module_instance;
%type<naja::verilog::Identifier> parameter_identifier;

%type<naja::verilog::Port> port_declaration
%type<naja::verilog::Ports> internal_ports_declaration
%type<naja::verilog::Port::Direction> port_type_io
%type<naja::verilog::Net::Type> net_type;
%type<bool> signed.opt;
%type<naja::verilog::Range> range;
%type<naja::verilog::Range> range.opt
%type<naja::verilog::Range> constant_range_expression.opt;
%type<naja::verilog::Identifier> net_identifier;
%type<naja::verilog::Identifiers> list_of_identifiers;
%type<naja::verilog::Identifiers> hierarchical_identifier;
%type<naja::verilog::Identifiers> hierarchical_parameter_identifier;
%type<naja::verilog::RangeIdentifiers> net_lvalue;
%type<naja::verilog::RangeIdentifiers> list_of_net_lvalues;
%type<naja::verilog::Identifier> module_instance;
%type<naja::verilog::Identifier> port_identifier;
%type<naja::verilog::Identifier> name_of_gate_instance.opt;

%type<naja::verilog::Number> number;
%type<naja::verilog::ConstantExpression> constant_primary;
%type<naja::verilog::ConstantExpression> constant_expression;
%type<naja::verilog::ConstantExpression> constant_mintypmax_expression;
%type<std::string> unary_operator;
%type<naja::verilog::Expression> primary;
%type<naja::verilog::Expression> expression;
%type<naja::verilog::Expression> expression.opt;
%type<naja::verilog::Expression> mintypmax_expression;
%type<naja::verilog::Expression> mintypmax_expression.opt;
%type<naja::verilog::Concatenation::Expressions> concatenation;
%type<naja::verilog::Concatenation::Expressions> list_of_expressions; 
%type<naja::verilog::ConstantExpression> attr_spec_value;
%type<naja::verilog::Attribute> attr_spec;

%type<naja::verilog::GateType> n_input_gatetype;
%type<naja::verilog::GateType> n_output_gatetype;

%locations 
%start source_text

%%

source_text: list_of_descriptions

list_of_descriptions: description | list_of_descriptions description;

description: module_declaration;

identifier
  : IDENTIFIER_TK { $$ = naja::verilog::Identifier($1); }
  | ESCAPED_IDENTIFIER_TK {
    std::string escaped = $1.substr(1, $1.size()-2);
    $$ = naja::verilog::Identifier(escaped, true);
  }

primary
: number {
  $$.valid_ = true; $$.value_ = $1; }
//no support for XMRs for the moment: should be hierarchical_identifier below
| identifier constant_range_expression.opt { 
  $$.valid_ = true; $$.value_ = naja::verilog::RangeIdentifier($1, $2); }
| STRING_TK { $$.valid_ = true; $$.value_ = $1.substr(1, $1.size()-2); } 
| concatenation { $$.valid_ = true; $$.value_ = std::make_shared<naja::verilog::Concatenation>($1); }
;

constant_primary
: number {
  $$.valid_ = true; $$.value_ = $1; }
| STRING_TK { $$.valid_ = true; $$.value_ = $1.substr(1, $1.size()-2); } 
;

unary_operator: SIGN_TK; 

constant_expression: constant_primary { 
  $$ = $1;
} 
| unary_operator constant_primary {
  auto expression = $2;
  if (expression.value_.index() == naja::verilog::ConstantExpression::NUMBER) {
    auto number = std::get<naja::verilog::Number>(expression.value_);
    if ($1 == "-") { number.sign_ = false; }
    $$.valid_ = true;
    $$.value_ = number;
  } else {
    throw VerilogException("Only constant number expression are supported"); //LCOV_EXCL_LINE
  }
}

range: '[' constant_expression ':' constant_expression ']' {
  if ($2.value_.index() == naja::verilog::ConstantExpression::NUMBER and
      $4.value_.index() == naja::verilog::ConstantExpression::NUMBER) {
    auto number1 = std::get<naja::verilog::Number>($2.value_);
    auto number2 = std::get<naja::verilog::Number>($4.value_);
    $$ = Range(number1.getInt(), number2.getInt());
  } else {
    throw VerilogException("Only constant number expression are supported"); //LCOV_EXCL_LINE
  }
}

range.opt: %empty { $$.valid_ = false; } | range { $$ = $1; }

port_declaration: list_of_attribute_instance.opt port_type_io range.opt identifier {
  $$ = Port($4, $2, $3);
  constructor->setCurrentSourceRange(@2.begin.line, @2.begin.column, @4.end.line, @4.end.column);
}

internal_ports_declaration: list_of_attribute_instance.opt port_type_io range.opt list_of_identifiers {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@2.begin.line, @2.begin.column, @4.end.line, @4.end.column);
  for (auto portIdentifier: $4) {
    constructor->internalModuleImplementationPort(Port(portIdentifier, $2, $3));
  }
}

port_type_io
  : INOUT_KW  { $$ = naja::verilog::Port::Direction::InOut; } 
  | INPUT_KW  { $$ = naja::verilog::Port::Direction::Input; }
  | OUTPUT_KW { $$ = naja::verilog::Port::Direction::Output; }
  ;

non_port_module_item: module_or_generate_item;

hierarchical_parameter_identifier: hierarchical_identifier;

constant_mintypmax_expression: constant_expression;

defparam_assignment: hierarchical_parameter_identifier '=' constant_mintypmax_expression {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->addDefParameterAssignment($1, $3);
}

list_of_defparam_assignments: defparam_assignment| list_of_defparam_assignments defparam_assignment;

parameter_override: DEFPARAM_KW list_of_defparam_assignments ';'

hierarchical_net_identifier: identifier;

net_lvalue: hierarchical_net_identifier constant_range_expression.opt {
  $$ = { naja::verilog::RangeIdentifier($1, $2) };
}
| '{' list_of_net_lvalues '}' {
  $$ = $2;
}

//Only one level of list is supported
list_of_net_lvalues: hierarchical_net_identifier constant_range_expression.opt {
  $$ = { naja::verilog::RangeIdentifier($1, $2) };
}
| list_of_net_lvalues ',' hierarchical_net_identifier constant_range_expression.opt {
  $1.push_back(naja::verilog::RangeIdentifier($3, $4));
  $$ = std::move($1);
}

net_assignment: net_lvalue '=' expression {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->addAssign($1, $3);
}

list_of_net_assignments: net_assignment | list_of_net_assignments ',' net_assignment;

continuous_assign: ASSIGN_KW list_of_net_assignments ';' 

module_or_generate_item: 
  list_of_attribute_instance.opt module_or_generate_item_declaration
| list_of_attribute_instance.opt module_instantiation
| list_of_attribute_instance.opt parameter_override
| list_of_attribute_instance.opt continuous_assign
| list_of_attribute_instance.opt gate_instantiation
; 

module_or_generate_item_declaration: net_declaration;

net_declaration: net_type signed.opt range.opt list_of_identifiers ';' {
  for (auto netIdentifier: $4) {
    constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
    constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
    constructor->addNet(Net(netIdentifier, $3, $1, $2));
  }
}

signed.opt
: %empty   { $$ = false; }
| SIGNED_KW { $$ = true; }
;

list_of_identifiers
: net_identifier {
  $$ = { $1 };
}
| list_of_identifiers ',' net_identifier {
  $1.push_back($3);
  $$ = std::move($1);
}

net_identifier: identifier { $$ = naja::verilog::Identifier($1); }

net_type
: SUPPLY0_KW { $$ = naja::verilog::Net::Type::Supply0; }
| SUPPLY1_KW { $$ = naja::verilog::Net::Type::Supply1; }
| WIRE_KW    { $$ = naja::verilog::Net::Type::Wire; }
;

list_of_module_instances
: module_instance
| list_of_module_instances ',' module_instance
;

number 
: BASE_TK BASED_CONSTVAL_TK {
  $$ = generateNumber(false, "", $1, $2, @$.begin.line, @$.end.line, @$.begin.column, @$.end.column);
} 
| CONSTVAL_TK BASE_TK BASED_CONSTVAL_TK {
  $$ = generateNumber(true, $1, $2, $3, @$.begin.line, @$.end.line, @$.begin.column, @$.end.column);
}
| CONSTVAL_TK {
  $$ = Number($1);
}

//hierarchical_identifier ::=
//{ identifier [ [ constant_expression ] ] . } identifier
hierarchical_identifier
: identifier {
  $$ = { $1 };
}
| hierarchical_identifier '.' identifier {
  $1.push_back($3);
  $$ = std::move($1);
}

//only numeric values (one bit) [4] or [4:5] are supported
constant_range_expression.opt: %empty { $$.valid_ = false; } 
| '[' constant_expression ']' {
  if ($2.value_.index() == naja::verilog::ConstantExpression::NUMBER) {
    auto number = std::get<naja::verilog::Number>($2.value_);
    $$ = Range(number.getInt());
  } else {
    throw VerilogException("Only constant number expression are supported"); //LCOV_EXCL_LINE
  }
}
| range {
  $$ = $1;
}

list_of_expressions:
expression {
  $$ = { $1 };
}
| list_of_expressions ',' expression {
  $1.push_back($3);
  $$ = std::move($1);
}

concatenation: '{' list_of_expressions '}' { $$ = $2; }

expression: primary { $$ = $1; }

expression.opt: %empty { $$.valid_ = false; } | expression { $$ = $1; }

ordered_port_connection: expression {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->addOrderedInstanceConnection(portIndex++, $1);
}

list_of_ordered_port_connections
: ordered_port_connection
| list_of_ordered_port_connections ',' ordered_port_connection
;

port_identifier: identifier;

named_port_connection: '.' port_identifier '(' expression.opt ')' {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->addInstanceConnection($2, $4);
}

list_of_named_port_connections: named_port_connection | list_of_named_port_connections ',' named_port_connection;

list_of_port_connections
: { portIndex = 0; } list_of_ordered_port_connections 
| list_of_named_port_connections
;

list_of_port_connections.opt: %empty | list_of_port_connections;

name_of_module_instance: identifier 

module_instance: name_of_module_instance {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@1.begin.line, @1.begin.column, @1.end.line, @1.end.column);
  constructor->addInstance(std::move($1));
} '(' list_of_port_connections.opt ')'

parameter_identifier: identifier;

mintypmax_expression: expression;

mintypmax_expression.opt: %empty { $$.valid_ = false; } | mintypmax_expression { $$ = $1; }

named_parameter_assignment: '.' parameter_identifier '(' mintypmax_expression.opt ')' {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->addParameterAssignment($2, $4);
}

list_of_named_parameter_assignments
: named_parameter_assignment
| list_of_named_parameter_assignments ',' named_parameter_assignment
;

list_of_parameter_assignments: /* list_of_ordered_parameter_assignment | */ list_of_named_parameter_assignments;

parameter_value_assignment: %empty | '#' '(' list_of_parameter_assignments ')'

//(From A.4.1) 
module_instantiation: module_identifier {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->startInstantiation(std::move($1));
} parameter_value_assignment list_of_module_instances ';' {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->endInstantiation();
}

module_identifier: identifier 

port: identifier {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->internalModuleInterfaceSimplePort($1);
}

module_item: internal_ports_declaration ';' | non_port_module_item;

list_of_module_items: module_item | list_of_module_items module_item;

list_of_module_items.opt: %empty | list_of_module_items; 

/* A.1.2 */
module_arg
: port_declaration {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->internalModuleInterfaceCompletePort($1);
}
| port;

list_of_module_args: module_arg | list_of_module_args ',' module_arg;

list_of_module_args.opt: %empty | '(' ')' | '(' list_of_module_args ')';

module_declaration: list_of_attribute_instance.opt {
} MODULE_KW module_identifier {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@3.begin.line, @3.begin.column, @4.end.line, @4.end.column);
  constructor->internalStartModule(std::move($4));
} list_of_module_args.opt ';' list_of_module_items.opt ENDMODULE_KW {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->internalEndModule();
}

/* 3.8 */
attr_spec_value: %empty {
  $$.valid_ = false;
}
| '=' constant_expression {
  $$ = $2;
}

attr_spec: identifier attr_spec_value {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->addAttribute($1, $2);
}

list_of_attr_spec: attr_spec | list_of_attr_spec ',' attr_spec; 

//A.9.1
attribute_instance: '(' '*' list_of_attr_spec '*' ')';

list_of_attribute_instance: attribute_instance | list_of_attribute_instance attribute_instance;

list_of_attribute_instance.opt: %empty | list_of_attribute_instance;

name_of_gate_instance.opt: %empty { $$ = naja::verilog::Identifier(); } | identifier { $$ = $1; }

input_terminal: expression {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->addGateInputInstanceConnection(portIndex++, $1);
}

output_terminal: net_lvalue {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->addGateOutputInstanceConnection(portIndex++, $1);
}

//list_of_output_terminals: output_terminal | list_of_output_terminals ',' output_terminal;

list_of_input_terminals: input_terminal | list_of_input_terminals ',' input_terminal;

n_input_gate_instance: name_of_gate_instance.opt {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@1.begin.line, @1.begin.column, @1.end.line, @1.end.column);
  constructor->addGateInstance(std::move($1));
  portIndex = 0;
} '(' output_terminal ',' list_of_input_terminals ')' ;

n_output_gate_instance: name_of_gate_instance.opt {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@1.begin.line, @1.begin.column, @1.end.line, @1.end.column);
  constructor->addGateInstance(std::move($1));
  portIndex = 0;
} '(' output_terminal ',' input_terminal ')' ;
//} '(' list_of_output_terminals ',' input_terminal ')' ;

list_of_n_output_gate_instances:
n_output_gate_instance | list_of_n_output_gate_instances ',' n_output_gate_instance ;

list_of_n_input_gate_instances:
n_input_gate_instance | list_of_n_input_gate_instances ',' n_input_gate_instance ;

n_input_gatetype:
  AND_KW  { $$ = naja::verilog::GateType::And; }
| NAND_KW { $$ = naja::verilog::GateType::Nand; }
| OR_KW   { $$ = naja::verilog::GateType::Or; }
| NOR_KW  { $$ = naja::verilog::GateType::Nor; }
| XOR_KW  { $$ = naja::verilog::GateType::Xor; }
| XNOR_KW { $$ = naja::verilog::GateType::Xnor; }

n_output_gatetype: 
  BUF_KW  { $$ = naja::verilog::GateType::Buf; }
| NOT_KW  { $$ = naja::verilog::GateType::Not; }

//A.3.1
gate_instantiation:
  n_input_gatetype {
    constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
    constructor->setCurrentSourceRange(@1.begin.line, @1.begin.column, @1.end.line, @1.end.column);
    constructor->startGateInstantiation($1);
  } list_of_n_input_gate_instances {
    constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
    constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
    constructor->endGateInstantiation();
  }
  ';'
| n_output_gatetype {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@1.begin.line, @1.begin.column, @1.end.line, @1.end.column);
  constructor->startGateInstantiation($1);
} list_of_n_output_gate_instances {
  constructor->setCurrentLocation(@$.begin.line, @$.begin.column);
  constructor->setCurrentSourceRange(@$.begin.line, @$.begin.column, @$.end.line, @$.end.column);
  constructor->endGateInstantiation();
}
';'

%%

void naja::verilog::VerilogParser::error(
  const location_type& l,
  const std::string& err_message ) {
  std::ostringstream reason;
  reason << "Parser error: " << err_message  << '\n'
            << "  begin at line " << l.begin.line <<  " col " << l.begin.column  << '\n' 
            << "  end   at line " << l.end.line <<  " col " << l.end.column << "\n";
  throw VerilogException(reason.str());
}
