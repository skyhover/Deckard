// Copyright 2016-2017 Federico Bond <federicobond@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/* Convert the LL grammar to LR for bison
 * Modified by JLX PI 2018/5/8
*/
// grammar Solidity;

sourceUnit: /* empty */
	   | directives_or_definitions
	   ;

directives_or_definitions:
			 directive_or_definition
			| directives_or_definitions directive_or_definition
			;	

directive_or_definition:
		       pragmaDirective
		| importDirective
		| contractDefinition
		;

pragmaDirective:
	       PRAGMA pragmaName pragmaValue ';'
		;

pragmaName:
	  identifier
	;

pragmaValue: /* according to solidity code, it should take in anything up to the trailing ';', even a newline: [^;]+ */ 
	   version
	| expression
	;

version:
       versionConstraint
	| versionConstraint versionConstraint /* hack: a format to allow spaces in VERSIONID due to limits in the non-adaptive tokenizer */
	;

versionConstraint:
		 version_operator_or_empty VERSIONID
		| version_operator_or_empty DecimalNumber
		;

version_operator_or_empty: /* empty */
			 | versionOperator
			;

versionOperator:
	       '^'
	| '~'
	| LT
	| GT
	| LEQ
	| GEQ
	| '='
	;

importDirective:
	       IMPORT StringLiteral as_alias ';'
	| IMPORT star_identifier as_alias FROM StringLiteral ';'
	| IMPORT '{' import_declarations '}' FROM StringLiteral ';'
	;

as_alias: /* empty */
	| AS identifier
	;

star_identifier:
	       '*'
	| identifier
	;

import_declarations:
		   importDeclaration
		| import_declarations ',' importDeclaration
		;

importDeclaration:
		 identifier as_alias
		;

contractDefinition:
		  contract_type identifier contract_is_inheritance '{' contract_parts_or_empty '}'
		;

contract_type:
	     CONTRACT
	| INTERFACE
	| LIBRARY
	;

contract_is_inheritance: /* empty */
		       | IS inheritance_specifiers
			;

inheritance_specifiers:
		      inheritanceSpecifier
		| inheritance_specifiers ',' inheritanceSpecifier
		;

contract_parts_or_empty: /* empty */
		       | contract_parts
			;

contract_parts:
	      contractPart
	| contract_parts contractPart
	;

inheritanceSpecifier:
		    userDefinedTypeName
		| userDefinedTypeName '(' expression_list_or_empty ')'
		;

contractPart:
	    stateVariableDeclaration %dprec 3
	| usingForDeclaration
  	| structDefinition
  	| modifierDefinition
	| constructor
  	| functionDefinition %dprec 4 /* E.g., is "function() internal callback;" a stateVariableDeclaration or a functionDefinition if the function body is allowed to be empty? So far, give dynamic precedence to stateVariableDeclaration. */
  	| eventDefinition
  	| enumDefinition
	;

stateVariableDeclaration:
			typeName state_var_keywords_or_empty identifier initializer_or_empty ';'
			;

state_var_keywords_or_empty: /* empty */
		       | state_var_keywords
			;

state_var_keywords:
	      state_var_keyword
	| state_var_keywords state_var_keyword
	;

state_var_keyword:
	     PublicKeyword
	| InternalKeyword
	| PrivateKeyword
	| ConstantKeyword
	;

initializer_or_empty: /* empty */
		    | '=' expression
		;

usingForDeclaration:
		   USING identifier FOR star_typename ';'
		;

star_typename:
	     '*'
	| typeName
	;

structDefinition:
		STRUCT identifier '{' variable_declarations_or_empty '}'
	;

variable_declarations_or_empty: /* empty */
			      | variable_declarations
			;

variable_declarations:
		     variableDeclaration ';'
		| variable_declarations variableDeclaration ';'
		;

variableDeclaration:
		   typeName storage_location_or_empty identifier
		;

storage_location_or_empty: /* empty */
			 | storageLocation
			;

storageLocation:
	       MEMORY
		| STORAGE
		;

modifierDefinition:
		  MODIFIER identifier parameter_list_or_empty block
		;

parameter_list_or_empty: /* empty */
		       | parameterList
			;

parameterList:
	     '(' parameters_or_empty ')'
	;

parameters_or_empty: /* empty */
		   | parameters
		;

parameters:
	  parameter
	| parameters ',' parameter
	;

parameter:
	 typeName storage_location_or_empty identifier_or_empty
	;

identifier_or_empty: /* empty */
		   | identifier
		;

constructor:
	   CONSTRUCTOR parameterList modifier_list_or_empty return_parameters_or_empty block_or_empty_statement
	;

functionDefinition:
		  FUNCTION identifier_or_empty parameterList modifier_list_or_empty return_parameters_or_empty block_or_empty_statement
		;

return_parameters_or_empty: /* empty */
			  | returnParameters
			;

modifier_list_or_empty: /* empty */
		      | modifierList
			;

modifierList:
	    modifier
	| modifierList modifier
	;

modifier:
	modifierInvocation
	| stateMutability
	| ExternalKeyword
    	| PublicKeyword
	| InternalKeyword
	| PrivateKeyword
	;

modifierInvocation:
		  identifier
		| identifier '(' expression_list_or_empty ')'
		;

returnParameters:
		RETURNS parameterList
		;

block_or_empty_statement:
			block
			| ';'
			;

eventDefinition:
	       EVENT identifier eventParameterList anonymous_keyword_or_empty ';'
		;

eventParameterList:
		  '(' event_parameter_list_or_empty ')'
		;

event_parameter_list_or_empty: /* empty */
			     | event_parameter_list
			;

event_parameter_list:
		    eventParameter
		| event_parameter_list ',' eventParameter
		;

eventParameter:
	      typeName indexed_keyword_or_empty identifier_or_empty
		;

indexed_keyword_or_empty: /* empty */
			| IndexedKeyword
			;

anonymous_keyword_or_empty: /* empty */
			  | AnonymousKeyword
			;

expression_list_or_empty: /* empty */
			| expression_list
			;

enumDefinition:
	      ENUM identifier '{' enum_value_or_empty enum_value_with_comma_list_or_empty '}'
		;

enum_value_with_comma_list_or_empty: /* empty */
				   | enum_value_with_comma_list
				;

enum_value_with_comma_list:
			  enum_value_with_comma
			| enum_value_with_comma_list enum_value_with_comma
			;

enum_value_with_comma:
		     ',' enumValue
			;

enum_value_or_empty: /* empty */
		   | enumValue
		;

enumValue:
	 identifier
	;

typeName:
	elementaryTypeName
	| userDefinedTypeName
	| mapping
	| typeName '[' expression_or_empty ']' /* This rule requires LR(*) to differentiate from many other rules, e.g., expression: expression '[' expression ']'; use glr-parser */
	| functionTypeName
	;

userDefinedTypeName: /* exclude certain special identifier from typeName: FROM, EMIT, '_' */
		   IDENTIFIER
		| 'x'
		| userDefinedTypeName '.' IDENTIFIER /* This rule requires potentially LR(*) to differentiate from many other rules, e.g., expression: expression '.' identifier; use glr-parser */
		| userDefinedTypeName '.' 'x'
		;

mapping:
       MAPPING '(' elementaryTypeName MAPSTO typeName ')'
	;

functionTypeName:
		FUNCTION functionTypeParameterList state_func_keywords_or_empty returns_or_empty
		;

functionTypeParameterList:
			 '(' function_type_parameters_or_empty ')'
			;

function_type_parameters_or_empty: /* empty */
				 | function_type_parameters
				;

function_type_parameters:
			functionTypeParameter
		| function_type_parameters ',' functionTypeParameter
		;

functionTypeParameter:
		     typeName storage_location_or_empty
		;

state_func_keywords_or_empty: /* empty */
			    | state_func_keywords
			;

state_func_keywords:
		   state_func_keyword
		| state_func_keywords state_func_keyword
		;

state_func_keyword:
		  InternalKeyword
		| ExternalKeyword
		| stateMutability
		;

returns_or_empty: /* empty */
		| RETURNS functionTypeParameterList
		;

stateMutability:
	       PureKeyword
	| ConstantKeyword
	| ViewKeyword
	| PayableKeyword
	;

block:
     '{' statements_or_empty '}'
	;

statements_or_empty: /* empty */
		   | statements
		;

statements:
	  statement
	| statements statement
	;

statement:
	 ifStatement
	| whileStatement
  	| forStatement
  	| block
  	| inlineAssemblyStatement
  	| doWhileStatement
  	| continueStatement
  	| breakStatement
  	| returnStatement
  	| throwStatement
  	| emitStatement
  	| simple_or_empty_statement
	;

ifStatement:
	   IF '(' expression ')' statement %prec THEN
	| IF '(' expression ')' statement ELSE statement
	;

whileStatement:
	      WHILE '(' expression ')' statement
	;

forStatement:
	    FOR '(' simple_or_empty_statement expression_or_empty ';' expression_or_empty ')' statement
	;

simple_or_empty_statement:
			 simpleStatement
			| ';'
			;

expression_or_empty: /* empty */
		   | expression
		;

inlineAssemblyStatement:
		       ASSEMBLY string_lit_or_empty assemblyBlock
			;

string_lit_or_empty: /* empty */
		   | StringLiteral
		;

doWhileStatement:
		DO statement WHILE '(' expression ')' ';'
		;

continueStatement:
		 CONTINUE ';'
		;

breakStatement:
	      BREAK ';'
		;

returnStatement:
	       RETURN expression_or_empty ';'
		;

throwStatement:
	      THROW ';' ;

emitStatement:
	     EMIT functionCall ';'
	;

functionCall:
	    expression '(' functionCallArguments ')'
	;

functionCallArguments:
		     expression_list_or_empty
		| '{' name_value_list_or_empty '}'
		;

name_value_list_or_empty: /* empty */
			| name_value_list
			;

name_value_list:
	       name_value_with_comma
	| name_value_list ',' name_value_with_comma
	;

name_value_with_comma:
		     nameValue
		| nameValue ','
		;

nameValue:
	 identifier ':' expression
	;

expression_list:
	       expression
	| expression_list ',' expression
	;

simpleStatement:
	       variableDeclarationStatement %dprec 6
	| expressionStatement %dprec 7
	| placeholder_statement %dprec 5
	;

variableDeclarationStatement:
			    variable_declaration_left initializer_or_empty ';'
			;

variable_declaration_left:
			 VAR identifierList
			| variableDeclaration
			;

identifierList: /* : '(' ( identifier? ',' )* identifier? ')' ; */
	      '(' identifier_list_with_comma_or_empty identifier_or_empty ')'
	;

identifier_list_with_comma_or_empty: /* empty */
				   | identifier_list_with_comma
				;

identifier_list_with_comma:
			  identifier_with_comma
			| identifier_list_with_comma identifier_with_comma
			;

identifier_with_comma:
		     identifier_or_empty ','
		;

expressionStatement:
		   expression ';'
		;

placeholder_statement: /* TODO: what can be in a placeholder is unclear; only function body or any expression? */
		     '_' /* '_' can be an identifier too; leave it to GLR to disambiguate. */
//		| '_' ';' /* minimize changes to grammar and to avoid ambiguity here, use the lexer to skip the ';' */
		;

elementaryTypeName:
		  ADDRESS
		| BOOL
		| STRING
		| VAR
		| INT
		| UINT
		| BYTE
		| BYTES
		| fixed_point
		| ufixed_point
		;

fixed_point:
	   FIXED
	| FIXED CONST_INT 'x' CONST_INT
	;

ufixed_point:
	    UFIXED
	| UFIXED CONST_INT 'x' CONST_INT
	;

expression: /* TODO: differentiate expressions of different kinds of operands */
	  expression PLUSPLUS
	| expression MINUSMINUS
	| NEW typeName
  	| expression '[' expression ']'
  	| expression '(' functionCallArguments ')'
  	| expression '.' identifier
  	| '(' expression ')' /* Ambiguous with tupleExpression/primaryExpression when only one expression is inside. Give this rule a higher precedence */ %dprec 1
	| PLUSPLUS expression %prec PREFIX_DOUBLEPLUSMINUS
	| MINUSMINUS expression %prec PREFIX_DOUBLEPLUSMINUS
  	| '+' expression %prec UNARY_PLUSMINUS
	| '-' expression %prec UNARY_PLUSMINUS
	| AFTER expression
	| DELETE expression
  	| '!' expression
  	| '~' expression
  	| expression MULMUL expression
  	| expression '*' expression
  	| expression '/' expression
  	| expression '%' expression
  	| expression '+' expression
  	| expression '-' expression
  	| expression LSHIFT expression
  	| expression RSHIFT expression
  	| expression '&' expression
  	| expression '^' expression
  	| expression '|' expression
  	| expression LT expression
  	| expression GT expression
  	| expression LEQ expression
  	| expression GEQ expression
  	| expression EQ expression
  	| expression NEQ expression
  	| expression ANDAND expression
  	| expression OROR expression
	| conditional_expression %prec '?'
  	| expression assignment_operator expression %prec '='
  	| primaryExpression %dprec 2
	;

conditional_expression:
		      expression '?' expression ':' expression
			;

assignment_operator:
		   '='
		| ASSIGN_AND
		| ASSIGN_OR
		| ASSIGN_XOR
		| ASSIGN_LSHIFT
		| ASSIGN_RSHIFT
		| ASSIGN_PLUS
		| ASSIGN_MINUS
		| ASSIGN_MUL
		| ASSIGN_DIV
		| ASSIGN_REM
		;

primaryExpression:
		 BooleanLiteral
  		| numberLiteral
  		| HexLiteral
  		| StringLiteral
  		| identifier
  		| tupleExpression
  		| elementaryTypeNameExpression
		;

BooleanLiteral:
	      TRUE
	| FALSE
	;

numberLiteral:
	     number_const number_unit_or_empty
	;

number_const:
	    DecimalNumber
	| HexNumber
	;

number_unit_or_empty: /* empty */
		    | NumberUnit
		;


assemblyBlock:
	     '{' assembly_item_list_or_empty '}'
	;

assembly_item_list_or_empty: /* empty */
			   | assembly_item_list
			;

assembly_item_list:
		  assemblyItem
		| assembly_item_list assemblyItem
		;

assemblyItem:
	    identifier
	| assemblyBlock
  	| assemblyExpression
  	| assemblyLocalDefinition
  	| assemblyAssignment
  	| assemblyStackAssignment
  	| labelDefinition
  	| assemblySwitch
  	| assemblyFunctionDefinition
  	| assemblyFor
  	| assemblyIf
  	| BreakKeyword
  	| ContinueKeyword
  	| subAssembly
  	| numberLiteral
  	| StringLiteral
  	| HexLiteral
	;

assemblyExpression:
		  assemblyCall
		| assemblyLiteral
		;

assemblyCall:
	    assembly_call_head assembly_call_expressions_or_empty
	;

assembly_call_head:
		  RETURN
		| ADDRESS
		| BYTE
		| identifier 
		;

assembly_call_expressions_or_empty: /* empty */
				  | '(' assembly_call_expressions ')'
				;

assembly_call_expressions:
			 assembly_expression_or_empty assembly_expression_with_comma_list_or_empty
			;

assembly_expression_or_empty: /* empty */
			    | assemblyExpression
			;

assembly_expression_with_comma_list_or_empty: /* empty */
					    | assembly_expression_with_comma_list
					;

assembly_expression_with_comma_list:
				   assembly_expression_with_comma
				| assembly_expression_with_comma_list assembly_expression_with_comma
				;

assembly_expression_with_comma:
			      ',' assemblyExpression
				;

assemblyLiteral:
	       StringLiteral
		| DecimalNumber
		| HexNumber
		| HexLiteral
		;

assemblyLocalDefinition:
		       LET assemblyIdentifierOrList assembly_initializer_or_empty
		;

assemblyIdentifierOrList:
			identifier
			| '(' assemblyIdentifierList ')'
			;

assemblyIdentifierList:
		      identifier
		| assemblyIdentifierList ',' identifier
		;

assembly_initializer_or_empty: /* empty */
			     | ASSIGN_ASSEMBLY_L assemblyExpression
			;

assemblyAssignment:
		  assemblyIdentifierOrList ASSIGN_ASSEMBLY_L assemblyExpression
		;

assemblyStackAssignment:
		       ASSIGN_ASSEMBLY_R identifier
			;

labelDefinition:
	       identifier ':'
		;

assemblySwitch:
	      SWITCH assemblyExpression assembly_case_or_empty
		;

assembly_case_or_empty: /* empty */
		      | assembly_cases
			;

assembly_cases:
	      assemblyCase
		| assembly_cases assemblyCase
		;

assemblyCase:
	    CASE assemblyLiteral assemblyBlock
	| DEFAULT assemblyBlock
	;

assemblyFunctionDefinition:
			  FUNCTION identifier '(' assembly_identifier_list_or_empty ')' assembly_returns_or_empty assemblyBlock
			;

assembly_identifier_list_or_empty: /* empty */
				 | assemblyIdentifierList
				;

assembly_returns_or_empty: /* empty */
			 | assemblyFunctionReturns
			;

assemblyFunctionReturns:
		       POINTSAT assemblyIdentifierList
			;

assemblyFor:
	   FOR assembly_block_or_expression assemblyExpression assembly_block_or_expression assemblyBlock
	;

assembly_block_or_expression:
			    assemblyBlock
			| assemblyExpression
			;

assemblyIf:
	  IF assemblyExpression assemblyBlock
	;

subAssembly:
	   ASSEMBLY identifier assemblyBlock
	;

tupleExpression:
	       '(' expression_or_empty expression_with_comma_list_or_empty ')'
	| '[' expression_list_or_empty ']'
	;

expression_with_comma_list_or_empty: /* empty */
				   | expression_with_comma_list
				;

expression_with_comma_list:
			  expression_with_comma
			| expression_with_comma_list expression_with_comma
			;

expression_with_comma:
		     ',' expression_or_empty
		;

elementaryTypeNameExpression:
			    elementaryTypeName
			;

identifier:
	  IDENTIFIER
	| FROM /* Different from reserved keywords (e.g., 'for'), 'from' can be used as an identifier too. */
	| 'x' /* a special identifier used for fixed/unfixed points */
	| EMIT /* emitStatement; 'emit' can be an identifier too */
	| '_'
	;

DecimalNumber:
	     CONST_INT
	| CONST_NUM
	;

HexNumber: /* TODO: allow space after '0x': '0x' HexCharacter+ ; */
	 CONST_INT_H
	| CONST_INT_O
	;

NumberUnit:
	  WEI
	| SZABO
	| FINNEY
	| ETHER
	| SECONDS
	| MINUTES
	| HOURS
	| DAYS
	| WEEKS
	| YEARS
	;

HexLiteral: /* TODO: separating tokenization from parsing cannot recognize it as a number; only as a string : 'hex' ('"' HexPair* '"' | '\'' HexPair* '\'') ; */
	  HEX hexpair_list
	;

hexpair_list:
	    STRING_LIT
	;
/* cannot differetiate HexPair from a normal number; simply treat them as strings.
hexpair_list:
	    DQUOTE hexpairs_or_empty DQUOTE 
	| SQUOTE hexpairs_or_empty SQUOTE
	;

hexpairs_or_empty:
		 | hexpairs
		;
hexpairs:
	HexPair
	| hexpairs HexPair
	;
fragment
HexPair:
       HexCharacter HexCharacter
	;
*/

/* not used yet 
ReservedKeyword:
	       ABSTRACT
	| AFTER
	| CASE
	| CATCH
	| DEFAULT
	| FINAL
	| IN
	| INLINE
	| LET
	| MATCH
	| NULL
	| OF
	| RELOCATABLE
	| STATIC
	| SWITCH
	| TRY
	| TYPE
	| TYPEOF
	;
  : 'abstract'
  | 'after'
  | 'case'
  | 'catch'
  | 'default'
  | 'final'
  | 'in'
  | 'inline'
  | 'let'
  | 'match'
  | 'null'
  | 'of'
  | 'relocatable'
  | 'static'
  | 'switch'
  | 'try'
  | 'type'
  | 'typeof' ;
*/

AnonymousKeyword:
		ANONYMOUS
	;
BreakKeyword:
	    BREAK
	;
ConstantKeyword:
	       CONSTANT
	;
ContinueKeyword:
	       CONTINUE
	;
ExternalKeyword:
	       EXTERNAL
	;
IndexedKeyword:
	      INDEXED
	;
InternalKeyword:
	       INTERNAL
	;
PayableKeyword:
	      PAYABLE
	;
PrivateKeyword:
	      PRIVATE
	;
PublicKeyword:
	     PUBLIC
	;
PureKeyword:
	   PURE
	;
ViewKeyword:
	   VIEW
	;

StringLiteral:
	     STRING_LIT
	;

