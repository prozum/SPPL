%skeleton "lalr1.cc"
%name-prefix "parser"
%define "parser_class_name" {Parser}
%locations
%initial-action { @$.begin.filename = @$.end.filename = &driver.streamname; }
%debug
%parse-param { class Driver& driver }
%error-verbose

%{
#include <string>
#include <vector>

#include "Node.h"

%}

%union {
    common::Program *                  prog_val;
    common::Function *                 func_val;
    common::Type *                     type_val;
    common::Case *                     case_val;
    common::Pattern *                  pattern_val;
    common::Expr *                     expr_val;

    std::vector<common::Function *> *  func_vec;
    std::vector<common::Type *> *      type_vec;
    std::vector<common::Case *> *      case_vec;
    std::vector<common::Pattern *> *   pattern_vec;
    std::vector<common::Expr *> *      expr_vec;

    std::string*	                str_val;
    long	                        long_val;
    long double	                    longdouble_val;
    bool	                        bool_val;
}

%token END 0 "end of stream"
%token EOL "end of line"
%token DEF INTTYPE BOOLTYPE FLOATTYPE STRINGTYPE CHARTYPE ARROR EQUAL NOTEQUAL AND OR LESSEREQUAL GREATEREQUAL LESSER GREATER MUL DIV MOD ADD SUB ASSIGN SQSTART SQEND PARSTART PAREND EXMARK COMMA PIPE ARROW COLON
%token <long_val> INTLITERAL
%token <str_val> ID STRINGLITERAL CHARLITERAL
%token <bool_val> BOOLLITERAL
%token <longdouble_val> FLOATLITERAL

%left OR
%left AND
%left EQUAL NOTEQUAL
%left LESSER GREATER GREATEREQUAL LESSEREQUAL
%left ADD SUB
%left MUL DIV MOD
%right COLON
%precedence EXMARK

%type <prog_val> program
%type <func_vec> funcs_ne
%type <func_val> func decl
%type <type_val> type signature
%type <type_vec> types_comma_ne
%type <case_vec> cases_ne
%type <case_val> case
%type <pattern_vec> patterns patterns_comma_ne
%type <pattern_val> pattern literal
%type <expr_val> expr struct_inst
%type <expr_vec> exprs_comma exprs_comma_ne

%{

#include "Driver.h"
#include "Scanner.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex

using namespace common;
using namespace std;

%}

%start program

%%

program:	funcs_ne                                        { driver.main = new Program(); driver.main->funcs = * $1; delete $1; };
funcs_ne:	funcs_ne func                                   { $$ = $1; $$->push_back($2); }
	| func                                                  { $$ = new std::vector<Function *>(); $$->push_back($1); } ;
func:		decl cases_ne                                   { $$ = $1; $$->cases = * $2; delete $2; }
decl:		DEF ID COLON signature                    { $$ = new Function(* $2); $$->types = $4->types; delete $2; delete $4; }
signature:	signature ARROR type                          { $$ = $1; $$->types.push_back($3); }
	| type                                                  { $$ = new Type(SIGNATURE); $$->types.push_back($1); } ;
type:		BOOLTYPE                                      { $$ = new Type(BOOL); }
	|	INTTYPE                                           { $$ = new Type(INT); }
	|	FLOATTYPE                                         { $$ = new Type(FLOAT); }
	|	CHARTYPE                                          { $$ = new Type(CHAR); }
	|	STRINGTYPE                                        { $$ = new Type(STRING); }
	|	SQSTART type SQEND                              { $$ = new Type(LIST); $$->types.push_back($2); }
	|	PARSTART signature PAREND                       { $$ = $2; }
	|	PARSTART types_comma_ne COMMA type PAREND     { $$ = new Type(TUPLE, $2); delete $2; $$->types.push_back($4); }
types_comma_ne: types_comma_ne COMMA type                 { $$ = $1; $$->push_back($3); }
	|	type                                                { $$ = new vector<Type *>(); $$->push_back($1); };
cases_ne:	cases_ne case                                   { $$ = $1; $$->push_back($2); }
	|   case                                                  { $$ = new vector<Case *>(); $$->push_back($1); };
case: 		PIPE patterns ASSIGN expr                       { $$ = new Case($4); $$->patterns = * $2; delete $2; }
patterns:	patterns pattern                                { $$ = $1; $$->push_back($2); }
	|                                                       { $$ = new vector<Pattern *>(); } ;
pattern:    literal                                         { $$ = $1; }
	| 	ID                                                  { $$ = new Id(* $1); delete $1;  }
	| 	PARSTART pattern COLON pattern PAREND               { $$ = new ListSplit($2, $4);  }
	| 	PARSTART patterns_comma_ne COMMA pattern PAREND     { TuplePattern *t = new TuplePattern(); t->patterns = * $2; delete $2; t->patterns.push_back($4); $$ = t; }
patterns_comma_ne:  patterns_comma_ne COMMA pattern         { $$ = $1; $$->push_back($3); }
	|   pattern                                             { $$ = new vector<Pattern *>(); $$->push_back($1); } ;
literal:	INTLITERAL                                      { $$ = new Int($1); }
	|	SUB INTLITERAL                                      { $$ = new Int(- $2); }
	|	FLOATLITERAL                                        { $$ = new Float($1); }
	|	SUB FLOATLITERAL                                    { $$ = new Float(- $2); }
	|	CHARLITERAL                                         { $$ = new Char(* $1); delete $1; }
	|	STRINGLITERAL                                       { $$ = new String(* $1); delete $1; }
	|	BOOLLITERAL                                         { $$ = new Bool($1); } ;
expr:	expr OR expr                                        { $$ = new Or($1, $3); }
	|	expr AND expr                                       { $$ = new And($1, $3); }
	|	expr EQUAL expr                                     { $$ = new Equal($1, $3); }
	|	expr NOTEQUAL expr                                  { $$ = new NotEqual($1, $3); }
	|	expr LESSER expr                                    { $$ = new Lesser($1, $3); }
	|	expr GREATER expr                                   { $$ = new Greater($1, $3); }
	|	expr LESSEREQUAL expr                               { $$ = new LesserEq($1, $3); }
	|	expr GREATEREQUAL expr                              { $$ = new GreaterEq($1, $3); }
	|	expr ADD expr                                       { $$ = new Add($1, $3); }
	|	expr SUB expr                                       { $$ = new Sub($1, $3); }
	|	expr MUL expr                                       { $$ = new Mul($1, $3); }
	|	expr DIV expr                                       { $$ = new Div($1, $3); }
	|	expr MOD expr                                       { $$ = new Mod($1, $3); }
	|	expr COLON expr                                     { $$ = new ListAdd($1, $3); }
	|	ID                                                  { $$ = new Id(* $1); delete $1; }
	|	literal                                             { $$ = $1; }
	|	struct_inst                                         { $$ = $1; }
	|	PARSTART expr PAREND                                { $$ = new Par($2); }
	|	expr PARSTART exprs_comma PAREND                    { Call *t = new Call($1); t->exprs = * $3; delete $3; $$ = t; }
	|	EXMARK expr                                         { $$ = new Not($2); } ;
struct_inst:	SQSTART exprs_comma SQEND                   { List *t = new List(); t->exprs = * $2; delete $2; $$ = t; }
	|	PARSTART exprs_comma_ne COMMA expr PAREND           { Tuple *t = new Tuple(); t->exprs = * $2; delete $2; t->exprs.push_back($4); $$ = t; }  ;
exprs_comma:    exprs_comma_ne                              { $$ = $1; }
	|	                                                    { $$ = new vector<Expr *>(); } ;
exprs_comma_ne: exprs_comma_ne COMMA expr                   { $$ = $1; $$->push_back($3);  }
	|	expr                                                { $$ = new vector<Expr *>(); $$->push_back($1); } ;

%%

void parser::Parser::error(const Parser::location_type& l,
                            const std::string& m)
{
    driver.error(l, m);
}