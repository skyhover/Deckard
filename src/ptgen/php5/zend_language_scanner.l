%{

/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2006 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id: zend_language_scanner.l,v 1.131.2.11.2.7 2007/01/18 23:28:08 iliaa Exp $ */

#include <stdio.h>
#include <ptree.h>
#include "pt_zend_language_parser.tab.hh"
#include <map>
#include <string>
using namespace std;

extern map<string,int> name2id;
void count();

// TODO: potential troublesome global variables; better to have encapsulated states...
/* NOTE: yylineno is expensive, so disable it */
int column = 0;
int line = 1;
// handle heredoc:
int heredoc_len = 0;
char * heredoc = NULL;

#define YY_DECL int yylex(YYSTYPE *yylvalp)
#define YY_FATAL_ERROR zend_fatal_scanner_error

void zend_fatal_scanner_error(char *message)
{
	fprintf(stderr, "%s", message);
}

%}

%x ST_IN_SCRIPTING
%x ST_DOUBLE_QUOTES
%x ST_SINGLE_QUOTE
%x ST_BACKQUOTE
%x ST_HEREDOC
%x ST_LOOKING_FOR_PROPERTY
%x ST_LOOKING_FOR_VARNAME
%x ST_COMMENT
%x ST_DOC_COMMENT
%x ST_ONE_LINE_COMMENT
%option stack

LNUM	[0-9]+
DNUM	([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
EXPONENT_DNUM	(({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM	"0x"[0-9a-fA-F]+
LABEL	[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]*
TOKENS [;:,.\[\]()|^&+-/*=%!~$<>?@]
ENCAPSED_TOKENS [\[\]{}$]
ESCAPED_AND_WHITESPACE [\n\t\r #'.:;,()|^&+-/*=%!~<>?@]+
ANY_CHAR (.|[\n])
NEWLINE ("\r"|"\n"|"\r\n")

%option noyylineno
%option yywrap

%%

<ST_IN_SCRIPTING>"exit" {
	count(); yylvalp->t=new Terminal(name2id["T_EXIT"],yytext,line); return T_EXIT;
}

<ST_IN_SCRIPTING>"die" {
	count(); yylvalp->t=new Terminal(name2id["T_EXIT"],yytext,line); return T_EXIT;
}

<ST_IN_SCRIPTING>"function" {
	count(); yylvalp->t=new Terminal(name2id["T_FUNCTION"],yytext,line); return T_FUNCTION;
}

<ST_IN_SCRIPTING>"const" {
	count(); yylvalp->t=new Terminal(name2id["T_CONST"],yytext,line); return T_CONST;
}

<ST_IN_SCRIPTING>"return" {
	count(); yylvalp->t=new Terminal(name2id["T_RETURN"],yytext,line); return T_RETURN;
}

<ST_IN_SCRIPTING>"try" {
	count(); yylvalp->t=new Terminal(name2id["T_TRY"],yytext,line); return T_TRY;
}

<ST_IN_SCRIPTING>"catch" {
	count(); yylvalp->t=new Terminal(name2id["T_CATCH"],yytext,line); return T_CATCH;
}

<ST_IN_SCRIPTING>"throw" {
	count(); yylvalp->t=new Terminal(name2id["T_THROW"],yytext,line); return T_THROW;
}

<ST_IN_SCRIPTING>"if" {
	count(); yylvalp->t=new Terminal(name2id["T_IF"],yytext,line); return T_IF;
}

<ST_IN_SCRIPTING>"elseif" {
	count(); yylvalp->t=new Terminal(name2id["T_ELSEIF"],yytext,line); return T_ELSEIF;
}

<ST_IN_SCRIPTING>"endif" {
	count(); yylvalp->t=new Terminal(name2id["T_ENDIF"],yytext,line); return T_ENDIF;
}

<ST_IN_SCRIPTING>"else" {
	count(); yylvalp->t=new Terminal(name2id["T_ELSE"],yytext,line); return T_ELSE;
}

<ST_IN_SCRIPTING>"while" {
	count(); yylvalp->t=new Terminal(name2id["T_WHILE"],yytext,line); return T_WHILE;
}

<ST_IN_SCRIPTING>"endwhile" {
	count(); yylvalp->t=new Terminal(name2id["T_ENDWHILE"],yytext,line); return T_ENDWHILE;
}

<ST_IN_SCRIPTING>"do" {
	count(); yylvalp->t=new Terminal(name2id["T_DO"],yytext,line); return T_DO;
}

<ST_IN_SCRIPTING>"for" {
	count(); yylvalp->t=new Terminal(name2id["T_FOR"],yytext,line); return T_FOR;
}

<ST_IN_SCRIPTING>"endfor" {
	count(); yylvalp->t=new Terminal(name2id["T_ENDFOR"],yytext,line); return T_ENDFOR;
}

<ST_IN_SCRIPTING>"foreach" {
	count(); yylvalp->t=new Terminal(name2id["T_FOREACH"],yytext,line); return T_FOREACH;
}

<ST_IN_SCRIPTING>"endforeach" {
	count(); yylvalp->t=new Terminal(name2id["T_ENDFOREACH"],yytext,line); return T_ENDFOREACH;
}

<ST_IN_SCRIPTING>"declare" {
	count(); yylvalp->t=new Terminal(name2id["T_DECLARE"],yytext,line); return T_DECLARE;
}

<ST_IN_SCRIPTING>"enddeclare" {
	count(); yylvalp->t=new Terminal(name2id["T_ENDDECLARE"],yytext,line); return T_ENDDECLARE;
}

<ST_IN_SCRIPTING>"instanceof" {
	count(); yylvalp->t=new Terminal(name2id["T_INSTANCEOF"],yytext,line); return T_INSTANCEOF;
}

<ST_IN_SCRIPTING>"as" {
	count(); yylvalp->t=new Terminal(name2id["T_AS"],yytext,line); return T_AS;
}

<ST_IN_SCRIPTING>"switch" {
	count(); yylvalp->t=new Terminal(name2id["T_SWITCH"],yytext,line); return T_SWITCH;
}

<ST_IN_SCRIPTING>"endswitch" {
	count(); yylvalp->t=new Terminal(name2id["T_ENDSWITCH"],yytext,line); return T_ENDSWITCH;
}

<ST_IN_SCRIPTING>"case" {
	count(); yylvalp->t=new Terminal(name2id["T_CASE"],yytext,line); return T_CASE;
}

<ST_IN_SCRIPTING>"default" {
	count(); yylvalp->t=new Terminal(name2id["T_DEFAULT"],yytext,line); return T_DEFAULT;
}

<ST_IN_SCRIPTING>"break" {
	count(); yylvalp->t=new Terminal(name2id["T_BREAK"],yytext,line); return T_BREAK;
}

<ST_IN_SCRIPTING>"continue" {
	count(); yylvalp->t=new Terminal(name2id["T_CONTINUE"],yytext,line); return T_CONTINUE;
}

<ST_IN_SCRIPTING>"echo" {
	count(); yylvalp->t=new Terminal(name2id["T_ECHO"],yytext,line); return T_ECHO;
}

<ST_IN_SCRIPTING>"print" {
	count(); yylvalp->t=new Terminal(name2id["T_PRINT"],yytext,line); return T_PRINT;
}

<ST_IN_SCRIPTING>"class" {
	count(); yylvalp->t=new Terminal(name2id["T_CLASS"],yytext,line); return T_CLASS;
}

<ST_IN_SCRIPTING>"interface" {
	count(); yylvalp->t=new Terminal(name2id["T_INTERFACE"],yytext,line); return T_INTERFACE;
}

<ST_IN_SCRIPTING>"extends" {
	count(); yylvalp->t=new Terminal(name2id["T_EXTENDS"],yytext,line); return T_EXTENDS;
}

<ST_IN_SCRIPTING>"implements" {
	count(); yylvalp->t=new Terminal(name2id["T_IMPLEMENTS"],yytext,line); return T_IMPLEMENTS;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"->" {
	yy_push_state(ST_LOOKING_FOR_PROPERTY);
	count(); yylvalp->t=new Terminal(name2id["T_OBJECT_OPERATOR"],yytext,line); return T_OBJECT_OPERATOR;
}

<ST_LOOKING_FOR_PROPERTY>{LABEL} {
	yy_pop_state();
	count(); yylvalp->t=new Terminal(name2id["T_STRING"],yytext,line); return T_STRING;
}

<ST_LOOKING_FOR_PROPERTY>{ANY_CHAR} {
	yyless(0);
	yy_pop_state();
        count();
}

<ST_IN_SCRIPTING>"::" {
	count(); yylvalp->t=new Terminal(name2id["T_PAAMAYIM_NEKUDOTAYIM"],yytext,line); return T_PAAMAYIM_NEKUDOTAYIM;
}

<ST_IN_SCRIPTING>"new" {
	count(); yylvalp->t=new Terminal(name2id["T_NEW"],yytext,line); return T_NEW;
}

<ST_IN_SCRIPTING>"clone" {
	count(); yylvalp->t=new Terminal(name2id["T_CLONE"],yytext,line); return T_CLONE;
}

<ST_IN_SCRIPTING>"var" {
	count(); yylvalp->t=new Terminal(name2id["T_VAR"],yytext,line); return T_VAR;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("int"|"integer"){TABS_AND_SPACES}")" {
	count(); yylvalp->t=new Terminal(name2id["T_INT_CAST"],yytext,line); return T_INT_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("real"|"double"|"float"){TABS_AND_SPACES}")" {
	count(); yylvalp->t=new Terminal(name2id["T_DOUBLE_CAST"],yytext,line); return T_DOUBLE_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"string"{TABS_AND_SPACES}")" {
	count(); yylvalp->t=new Terminal(name2id["T_STRING_CAST"],yytext,line); return T_STRING_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"binary"{TABS_AND_SPACES}")" {
	count(); yylvalp->t=new Terminal(name2id["T_STRING_CAST"],yytext,line); return T_STRING_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"array"{TABS_AND_SPACES}")" {
	count(); yylvalp->t=new Terminal(name2id["T_ARRAY_CAST"],yytext,line); return T_ARRAY_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"object"{TABS_AND_SPACES}")" {
	count(); yylvalp->t=new Terminal(name2id["T_OBJECT_CAST"],yytext,line); return T_OBJECT_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("bool"|"boolean"){TABS_AND_SPACES}")" {
	count(); yylvalp->t=new Terminal(name2id["T_BOOL_CAST"],yytext,line); return T_BOOL_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("unset"){TABS_AND_SPACES}")" {
	count(); yylvalp->t=new Terminal(name2id["T_UNSET_CAST"],yytext,line); return T_UNSET_CAST;
}

<ST_IN_SCRIPTING>"eval" {
	count(); yylvalp->t=new Terminal(name2id["T_EVAL"],yytext,line); return T_EVAL;
}

<ST_IN_SCRIPTING>"include" {
	count(); yylvalp->t=new Terminal(name2id["T_INCLUDE"],yytext,line); return T_INCLUDE;
}

<ST_IN_SCRIPTING>"include_once" {
	count(); yylvalp->t=new Terminal(name2id["T_INCLUDE_ONCE"],yytext,line); return T_INCLUDE_ONCE;
}

<ST_IN_SCRIPTING>"require" {
	count(); yylvalp->t=new Terminal(name2id["T_REQUIRE"],yytext,line); return T_REQUIRE;
}

<ST_IN_SCRIPTING>"require_once" {
	count(); yylvalp->t=new Terminal(name2id["T_REQUIRE_ONCE"],yytext,line); return T_REQUIRE_ONCE;
}

<ST_IN_SCRIPTING>"use" {
	count(); yylvalp->t=new Terminal(name2id["T_USE"],yytext,line); return T_USE;
}

<ST_IN_SCRIPTING>"global" {
	count(); yylvalp->t=new Terminal(name2id["T_GLOBAL"],yytext,line); return T_GLOBAL;
}

<ST_IN_SCRIPTING>"isset" {
	count(); yylvalp->t=new Terminal(name2id["T_ISSET"],yytext,line); return T_ISSET;
}

<ST_IN_SCRIPTING>"empty" {
	count(); yylvalp->t=new Terminal(name2id["T_EMPTY"],yytext,line); return T_EMPTY;
}

<ST_IN_SCRIPTING>"__halt_compiler" {
	count(); yylvalp->t=new Terminal(name2id["T_HALT_COMPILER"],yytext,line); return T_HALT_COMPILER;
}

<ST_IN_SCRIPTING>"static" {
	count(); yylvalp->t=new Terminal(name2id["T_STATIC"],yytext,line); return T_STATIC;
}

<ST_IN_SCRIPTING>"abstract" {
	count(); yylvalp->t=new Terminal(name2id["T_ABSTRACT"],yytext,line); return T_ABSTRACT;
}

<ST_IN_SCRIPTING>"final" {
	count(); yylvalp->t=new Terminal(name2id["T_FINAL"],yytext,line); return T_FINAL;
}

<ST_IN_SCRIPTING>"private" {
	count(); yylvalp->t=new Terminal(name2id["T_PRIVATE"],yytext,line); return T_PRIVATE;
}

<ST_IN_SCRIPTING>"protected" {
	count(); yylvalp->t=new Terminal(name2id["T_PROTECTED"],yytext,line); return T_PROTECTED;
}

<ST_IN_SCRIPTING>"public" {
	count(); yylvalp->t=new Terminal(name2id["T_PUBLIC"],yytext,line); return T_PUBLIC;
}

<ST_IN_SCRIPTING>"unset" {
	count(); yylvalp->t=new Terminal(name2id["T_UNSET"],yytext,line); return T_UNSET;
}

<ST_IN_SCRIPTING>"=>" {
	count(); yylvalp->t=new Terminal(name2id["T_DOUBLE_ARROW"],yytext,line); return T_DOUBLE_ARROW;
}

<ST_IN_SCRIPTING>"list" {
	count(); yylvalp->t=new Terminal(name2id["T_LIST"],yytext,line); return T_LIST;
}

<ST_IN_SCRIPTING>"array" {
	count(); yylvalp->t=new Terminal(name2id["T_ARRAY"],yytext,line); return T_ARRAY;
}

<ST_IN_SCRIPTING>"++" {
	count(); yylvalp->t=new Terminal(name2id["T_INC"],yytext,line); return T_INC;
}

<ST_IN_SCRIPTING>"--" {
	count(); yylvalp->t=new Terminal(name2id["T_DEC"],yytext,line); return T_DEC;
}

<ST_IN_SCRIPTING>"===" {
	count(); yylvalp->t=new Terminal(name2id["T_IS_IDENTICAL"],yytext,line); return T_IS_IDENTICAL;
}

<ST_IN_SCRIPTING>"!==" {
	count(); yylvalp->t=new Terminal(name2id["T_IS_NOT_IDENTICAL"],yytext,line); return T_IS_NOT_IDENTICAL;
}

<ST_IN_SCRIPTING>"==" {
	count(); yylvalp->t=new Terminal(name2id["T_IS_EQUAL"],yytext,line); return T_IS_EQUAL;
}

<ST_IN_SCRIPTING>"!="|"<>" {
	count(); yylvalp->t=new Terminal(name2id["T_IS_NOT_EQUAL"],yytext,line); return T_IS_NOT_EQUAL;
}

<ST_IN_SCRIPTING>"<=" {
	count(); yylvalp->t=new Terminal(name2id["T_IS_SMALLER_OR_EQUAL"],yytext,line); return T_IS_SMALLER_OR_EQUAL;
}

<ST_IN_SCRIPTING>">=" {
	count(); yylvalp->t=new Terminal(name2id["T_IS_GREATER_OR_EQUAL"],yytext,line); return T_IS_GREATER_OR_EQUAL;
}

<ST_IN_SCRIPTING>"+=" {
	count(); yylvalp->t=new Terminal(name2id["T_PLUS_EQUAL"],yytext,line); return T_PLUS_EQUAL;
}

<ST_IN_SCRIPTING>"-=" {
	count(); yylvalp->t=new Terminal(name2id["T_MINUS_EQUAL"],yytext,line); return T_MINUS_EQUAL;
}

<ST_IN_SCRIPTING>"*=" {
	count(); yylvalp->t=new Terminal(name2id["T_MUL_EQUAL"],yytext,line); return T_MUL_EQUAL;
}

<ST_IN_SCRIPTING>"/=" {
	count(); yylvalp->t=new Terminal(name2id["T_DIV_EQUAL"],yytext,line); return T_DIV_EQUAL;
}

<ST_IN_SCRIPTING>".=" {
	count(); yylvalp->t=new Terminal(name2id["T_CONCAT_EQUAL"],yytext,line); return T_CONCAT_EQUAL;
}

<ST_IN_SCRIPTING>"%=" {
	count(); yylvalp->t=new Terminal(name2id["T_MOD_EQUAL"],yytext,line); return T_MOD_EQUAL;
}

<ST_IN_SCRIPTING>"<<=" {
	count(); yylvalp->t=new Terminal(name2id["T_SL_EQUAL"],yytext,line); return T_SL_EQUAL;
}

<ST_IN_SCRIPTING>">>=" {
	count(); yylvalp->t=new Terminal(name2id["T_SR_EQUAL"],yytext,line); return T_SR_EQUAL;
}

<ST_IN_SCRIPTING>"&=" {
	count(); yylvalp->t=new Terminal(name2id["T_AND_EQUAL"],yytext,line); return T_AND_EQUAL;
}

<ST_IN_SCRIPTING>"|=" {
	count(); yylvalp->t=new Terminal(name2id["T_OR_EQUAL"],yytext,line); return T_OR_EQUAL;
}

<ST_IN_SCRIPTING>"^=" {
	count(); yylvalp->t=new Terminal(name2id["T_XOR_EQUAL"],yytext,line); return T_XOR_EQUAL;
}

<ST_IN_SCRIPTING>"||" {
	count(); yylvalp->t=new Terminal(name2id["T_BOOLEAN_OR"],yytext,line); return T_BOOLEAN_OR;
}

<ST_IN_SCRIPTING>"&&" {
	count(); yylvalp->t=new Terminal(name2id["T_BOOLEAN_AND"],yytext,line); return T_BOOLEAN_AND;
}

<ST_IN_SCRIPTING>"OR" {
	count(); yylvalp->t=new Terminal(name2id["T_LOGICAL_OR"],yytext,line); return T_LOGICAL_OR;
}

<ST_IN_SCRIPTING>"AND" {
	count(); yylvalp->t=new Terminal(name2id["T_LOGICAL_AND"],yytext,line); return T_LOGICAL_AND;
}

<ST_IN_SCRIPTING>"XOR" {
	count(); yylvalp->t=new Terminal(name2id["T_LOGICAL_XOR"],yytext,line); return T_LOGICAL_XOR;
}

<ST_IN_SCRIPTING>"<<" {
	count(); yylvalp->t=new Terminal(name2id["T_SL"],yytext,line); return T_SL;
}

<ST_IN_SCRIPTING>">>" {
	count(); yylvalp->t=new Terminal(name2id["T_SR"],yytext,line); return T_SR;
}

<ST_IN_SCRIPTING>{TOKENS} {
        char temp[4] = {'\'', yytext[0], '\'', 0};
	count(); yylvalp->t=new Terminal(name2id[temp],yytext,line); return yytext[0];
}


<ST_IN_SCRIPTING>"{" {
	yy_push_state(ST_IN_SCRIPTING);
	count(); yylvalp->t=new Terminal(name2id["'{'"],yytext,line); return '{';
}


<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"${" {
	yy_push_state(ST_LOOKING_FOR_VARNAME);
	count(); yylvalp->t=new Terminal(name2id["T_DOLLAR_OPEN_CURLY_BRACES"],yytext,line); return T_DOLLAR_OPEN_CURLY_BRACES;
}


<ST_IN_SCRIPTING>"}" {
	//RESET_DOC_COMMENT();
	/* This is a temporary fix which is dependant on flex and it's implementation */
	if (yy_start_stack_ptr) { // prevent syntax errors in php source files to crash the parser...
		yy_pop_state();
	}
	count(); yylvalp->t=new Terminal(name2id["'}'"],yytext,line); return '}';
}


<ST_LOOKING_FOR_VARNAME>{LABEL} {
	yy_pop_state();
	yy_push_state(ST_IN_SCRIPTING); // the push is used to reduce certain rules (e.g., <ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"}"), but why need "ST_LOOKING_FOR_VARNAME" at first place?
	count(); yylvalp->t=new Terminal(name2id["T_STRING_VARNAME"],yytext,line); return T_STRING_VARNAME;
}


<ST_LOOKING_FOR_VARNAME>{ANY_CHAR} {
	yyless(0);
	yy_pop_state();
	yy_push_state(ST_IN_SCRIPTING);
        count();
}


<ST_IN_SCRIPTING>{LNUM} {
	count(); yylvalp->t=new Terminal(name2id["T_LNUMBER"],yytext,line); return T_LNUMBER;
}

<ST_IN_SCRIPTING>{HNUM} {
		count(); yylvalp->t=new Terminal(name2id["T_LNUMBER"],yytext,line); return T_LNUMBER;
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>{LNUM}|{HNUM} { /* treat numbers (almost) as strings inside encapsulated strings */
	count(); yylvalp->t=new Terminal(name2id["T_NUM_STRING"],yytext,line); return T_NUM_STRING;
}

<ST_IN_SCRIPTING>{DNUM}|{EXPONENT_DNUM} {
	count(); yylvalp->t=new Terminal(name2id["T_DNUMBER"],yytext,line); return T_DNUMBER;
}

<ST_IN_SCRIPTING>"__CLASS__" {
	count(); yylvalp->t=new Terminal(name2id["T_CLASS_C"],yytext,line); return T_CLASS_C;
}

<ST_IN_SCRIPTING>"__FUNCTION__" {
	count(); yylvalp->t=new Terminal(name2id["T_FUNC_C"],yytext,line); return T_FUNC_C;
}

<ST_IN_SCRIPTING>"__METHOD__" {
	count(); yylvalp->t=new Terminal(name2id["T_METHOD_C"],yytext,line); return T_METHOD_C;
}

<ST_IN_SCRIPTING>"__LINE__" {
	count(); yylvalp->t=new Terminal(name2id["T_LINE"],yytext,line); return T_LINE;
}

<ST_IN_SCRIPTING>"__FILE__" {
	count(); yylvalp->t=new Terminal(name2id["T_FILE"],yytext,line); return T_FILE;
}

<INITIAL>(([^<]|"<"[^?%s<]){1,400})|"<s"|"<" {
        //?? why "400" limit?
	count(); yylvalp->t=new Terminal(name2id["T_INLINE_HTML"],yytext,line); return T_INLINE_HTML;
}

<INITIAL>"<?"|"<script"{WHITESPACE}+"language"{WHITESPACE}*"="{WHITESPACE}*("php"|"\"php\""|"\'php\'"){WHITESPACE}*">" {
    /* "<?" is the shorthand for "<?php" and requires some flags to be enabled on the server; but we enable it anyway. */
	if (1) { /* yyleng>2 means it's not <? but <script> */
		BEGIN(ST_IN_SCRIPTING);
                count(); //return T_OPEN_TAG;
	} else {
		count(); yylvalp->t=new Terminal(name2id["T_INLINE_HTML"],yytext,line); return T_INLINE_HTML;
	}
}


<INITIAL>"<%="|"<?=" {
	if (yytext[1]=='%' || yytext[1]=='?') {
		BEGIN(ST_IN_SCRIPTING);
                count(); //return T_OPEN_TAG; //return T_OPEN_TAG_WITH_ECHO;
	} else {
		count(); yylvalp->t=new Terminal(name2id["T_INLINE_HTML"],yytext,line); return T_INLINE_HTML;
	}
}


<INITIAL>"<%" {
	if (1) {
		BEGIN(ST_IN_SCRIPTING);
                count(); //return T_OPEN_TAG;
	} else {
		count(); yylvalp->t=new Terminal(name2id["T_INLINE_HTML"],yytext,line); return T_INLINE_HTML;
	}
}


<INITIAL>"<?php"([ \t]|{NEWLINE})? {
	BEGIN(ST_IN_SCRIPTING);
        count(); //return T_OPEN_TAG;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE>"$"{LABEL} {
	count(); yylvalp->t=new Terminal(name2id["T_VARIABLE"],yytext,line); return T_VARIABLE;
}

<ST_IN_SCRIPTING>{LABEL} {
	count(); yylvalp->t=new Terminal(name2id["T_STRING"],yytext,line); return T_STRING;
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>{LABEL} {
	count(); yylvalp->t=new Terminal(name2id["T_STRING"],yytext,line); return T_STRING;
}


<ST_IN_SCRIPTING>{WHITESPACE} {
	count(); //return T_WHITESPACE;
}


<ST_IN_SCRIPTING>"#"|"//" {
	BEGIN(ST_ONE_LINE_COMMENT);
	//yymore();
        count();
}

<ST_ONE_LINE_COMMENT>"?"|"%"|">" {
	yymore();
        //count(); caused wrong "line" and "column" numbers
}

<ST_ONE_LINE_COMMENT>[^\n\r?%>]*{ANY_CHAR} {
	switch (yytext[yyleng-1]) {
		case '?': case '%': case '>':
			yyless(yyleng-1);
			yymore();
                        //count();
			break; // why need yymore() and why not BEGIN(ST_IN_SCRIPTING) here? it depends on whether we need to use the info about the comments.
		default:
			BEGIN(ST_IN_SCRIPTING);
			count(); //return T_COMMENT;
	}
}

<ST_ONE_LINE_COMMENT>{NEWLINE} {
	BEGIN(ST_IN_SCRIPTING);
	count(); //return T_COMMENT;
}

<ST_ONE_LINE_COMMENT>"?>"|"%>" { /* one-line comments can also be ended by "?>" or "%>"? */
        if (yytext[yyleng-2] != '%') { /* asp comment? */
		yyless(yyleng-2);
		BEGIN(ST_IN_SCRIPTING);
		count(); //return T_COMMENT;
	} else {
		yymore();
                //count();
	}
}

<ST_IN_SCRIPTING>"/**"{WHITESPACE} {
	//RESET_DOC_COMMENT();
	BEGIN(ST_DOC_COMMENT);
	//yymore();
        count();
}

<ST_IN_SCRIPTING>"/*" {
	BEGIN(ST_COMMENT);
	//yymore();
        count();
}


<ST_COMMENT,ST_DOC_COMMENT>[^*]+ { /* TODO: input buffer of flex overflows when the comments are tooooooooo long */
	//yymore();
        count();
}

<ST_DOC_COMMENT>"*/" {
	BEGIN(ST_IN_SCRIPTING);
	count(); //return T_DOC_COMMENT;
}

<ST_COMMENT>"*/" {
	BEGIN(ST_IN_SCRIPTING);
	count(); //return T_COMMENT;
}

<ST_COMMENT,ST_DOC_COMMENT>"*" {
	//yymore();
        count();
}

<ST_IN_SCRIPTING>(";"{WHITESPACE}*"?>"|"</script"{WHITESPACE}*">"){NEWLINE}? {
	BEGIN(INITIAL);
	count(); yylvalp->t=new Terminal(name2id["T_CLOSE_TAG"],yytext,line); return T_CLOSE_TAG;  /* implicit ';' at php-end tag */
}

<ST_IN_SCRIPTING>("?>"|"</script"{WHITESPACE}*">"){NEWLINE}? {
	BEGIN(INITIAL);
	count(); yylvalp->t=new Terminal(name2id["';'"],yytext,line); return ';';//return T_CLOSE_TAG;  /* implicit ';' at php-end tag */
}


<ST_IN_SCRIPTING>"%>"{NEWLINE}? {
	if (1) {
		BEGIN(INITIAL);
		count(); //return T_CLOSE_TAG;  /* implicit ';' at php-end tag */
	} else {
		yyless(1); //??
		count(); //return yytext[0];
	}
}

<ST_IN_SCRIPTING>";"{WHITESPACE}*"%>"{NEWLINE}? {
	if (1) {
		BEGIN(INITIAL);
		count(); yylvalp->t=new Terminal(name2id["T_CLOSE_TAG"],yytext,line); return T_CLOSE_TAG;  /* implicit ';' at php-end tag */
	} else {
		yyless(1); //??
		count(); //return yytext[0];
	}
}


<ST_IN_SCRIPTING>("b"?["]([^$"\\]|("\\".))*["]) { //?? Does this handle multi-line strings?
	count(); yylvalp->t=new Terminal(name2id["T_CONSTANT_ENCAPSED_STRING"],yytext,line); return T_CONSTANT_ENCAPSED_STRING;
}


<ST_IN_SCRIPTING>("b"?[']([^'\\]|("\\".))*[']) { //?? Does this handle multi-line strings?
	count(); yylvalp->t=new Terminal(name2id["T_CONSTANT_ENCAPSED_STRING"],yytext,line); return T_CONSTANT_ENCAPSED_STRING;
}


<ST_IN_SCRIPTING>b?["] {
	BEGIN(ST_DOUBLE_QUOTES);
	count(); yylvalp->t=new Terminal(name2id["'\"'"],yytext,line); return '\"';
}


<ST_IN_SCRIPTING>"b"?"<<<"{TABS_AND_SPACES}{LABEL}{NEWLINE} {
        char *s;
        int bprefix = (*yytext == 'b') ? 1 : 0;

        heredoc_len = yyleng-bprefix-3-1-(yytext[yyleng-2]=='\r'?1:0);
        s = yytext+bprefix+3;
        while ((*s == ' ') || (*s == '\t')) {
                s++;
                heredoc_len--;
        }
        heredoc = (char*)malloc(heredoc_len+1);
        if ( heredoc ) {
            memcpy(heredoc, s, heredoc_len);
            heredoc[heredoc_len] = 0;
        } else
            heredoc_len = 0;
	BEGIN(ST_HEREDOC);
	count(); yylvalp->t=new Terminal(name2id["T_START_HEREDOC"],yytext,line); return T_START_HEREDOC;
}


<ST_IN_SCRIPTING>[`] {
	BEGIN(ST_BACKQUOTE);
	count(); yylvalp->t=new Terminal(name2id["'`'"],yytext,line); return '`';
}


<ST_IN_SCRIPTING>b?['] {
	BEGIN(ST_SINGLE_QUOTE);
	count(); yylvalp->t=new Terminal(name2id["'\''"],yytext,line); return '\'';
}


<ST_HEREDOC>^{LABEL}(";")?{NEWLINE} {
	int label_len;

	if (yytext[yyleng-2]=='\r') {
		label_len = yyleng-2;
	} else {
		label_len = yyleng-1;
	}

	if (yytext[label_len-1]==';') {
		label_len--;
	}

	if (label_len==heredoc_len && !memcmp(yytext, heredoc, label_len)) {
		yyless(yyleng - (yyleng - label_len));
		if ( heredoc ) {
                    free(heredoc);
		    heredoc=NULL;
                }
		heredoc_len=0;
		BEGIN(ST_IN_SCRIPTING);
		count(); yylvalp->t=new Terminal(name2id["T_END_HEREDOC"],yytext,line); return T_END_HEREDOC;
	} else {
		count(); yylvalp->t=new Terminal(name2id["T_STRING"],yytext,line); return T_STRING;
	}
}


<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>{ESCAPED_AND_WHITESPACE} {
	count(); yylvalp->t=new Terminal(name2id["T_ENCAPSED_AND_WHITESPACE"],yytext,line); return T_ENCAPSED_AND_WHITESPACE;
}

<ST_SINGLE_QUOTE>([^'\\]|\\[^'\\])+ {
	count(); yylvalp->t=new Terminal(name2id["T_ENCAPSED_AND_WHITESPACE"],yytext,line); return T_ENCAPSED_AND_WHITESPACE;
}


<ST_DOUBLE_QUOTES>[`]+ {
	count(); yylvalp->t=new Terminal(name2id["T_ENCAPSED_AND_WHITESPACE"],yytext,line); return T_ENCAPSED_AND_WHITESPACE;
}


<ST_BACKQUOTE>["]+ {
	count(); yylvalp->t=new Terminal(name2id["T_ENCAPSED_AND_WHITESPACE"],yytext,line); return T_ENCAPSED_AND_WHITESPACE;
}


<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"$"[^a-zA-Z_\x7f-\xff{] {
	if (yyleng == 2) {
		yyless(1);
	}
	count(); yylvalp->t=new Terminal(name2id["T_CHARACTER"],yytext,line); return T_CHARACTER;
}


<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>{ENCAPSED_TOKENS} {
        char temp[4] = {'\'', yytext[0], '\'', 0};
	count(); yylvalp->t=new Terminal(name2id[temp],yytext,line); return yytext[0];
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"\\{" {
	count(); yylvalp->t=new Terminal(name2id["T_STRING"],yytext,line); return T_STRING;
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"{$" {
	yy_push_state(ST_IN_SCRIPTING);
	yyless(1);
	count(); yylvalp->t=new Terminal(name2id["T_CURLY_OPEN"],yytext,line); return T_CURLY_OPEN;
}


<ST_SINGLE_QUOTE>"\\'" {
	count(); yylvalp->t=new Terminal(name2id["T_CHARACTER"],yytext,line); return T_CHARACTER;
}

<ST_SINGLE_QUOTE>"\\\\" {
	count(); yylvalp->t=new Terminal(name2id["T_CHARACTER"],yytext,line); return T_CHARACTER;
}

<ST_DOUBLE_QUOTES>"\\\"" {
	count(); yylvalp->t=new Terminal(name2id["T_CHARACTER"],yytext,line); return T_CHARACTER;
}

<ST_BACKQUOTE>"\\`" {
	count(); yylvalp->t=new Terminal(name2id["T_CHARACTER"],yytext,line); return T_CHARACTER;
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"\\"[0-7]{1,3} {
	count(); yylvalp->t=new Terminal(name2id["T_CHARACTER"],yytext,line); return T_CHARACTER;
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"\\x"[0-9A-Fa-f]{1,2} {
	count(); yylvalp->t=new Terminal(name2id["T_CHARACTER"],yytext,line); return T_CHARACTER;
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"\\"{ANY_CHAR} {
	switch (yytext[1]) { //?? Is "\\\n" handled?
		case 'n':
			break;
		case 't':
			break;
		case 'r':
			break;
		case '\\':
			break;
		case '$':
			break;
		default:
			count(); yylvalp->t=new Terminal(name2id["T_BAD_CHARACTER"],yytext,line); return T_BAD_CHARACTER;
	}
	count(); yylvalp->t=new Terminal(name2id["T_CHARACTER"],yytext,line); return T_CHARACTER;
}


<ST_HEREDOC>["'`]+ {
	count(); yylvalp->t=new Terminal(name2id["T_ENCAPSED_AND_WHITESPACE"],yytext,line); return T_ENCAPSED_AND_WHITESPACE;
}


<ST_DOUBLE_QUOTES>["] {
	BEGIN(ST_IN_SCRIPTING);
	count(); yylvalp->t=new Terminal(name2id["'\"'"],yytext,line); return '\"';
}


<ST_BACKQUOTE>[`] {
	BEGIN(ST_IN_SCRIPTING);
	count(); yylvalp->t=new Terminal(name2id["'`'"],yytext,line); return '`';
}


<ST_SINGLE_QUOTE>['] {
	BEGIN(ST_IN_SCRIPTING);
	count(); yylvalp->t=new Terminal(name2id["'\''"],yytext,line); return '\'';
}


<ST_DOUBLE_QUOTES,ST_BACKQUOTE,INITIAL,ST_IN_SCRIPTING,ST_LOOKING_FOR_PROPERTY><<EOF>> {
	BEGIN(INITIAL);
	yyterminate();
}

<ST_COMMENT,ST_DOC_COMMENT><<EOF>> {
	fprintf(stderr,"Unterminated comment: %s\nEOF at line %d\n", line);
	BEGIN(INITIAL);
	yyterminate();
}



<ST_IN_SCRIPTING,INITIAL,ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_SINGLE_QUOTE,ST_HEREDOC>{ANY_CHAR} {
	fprintf(stderr,"Unexpected character in input:  '%c' (ASCII=%d) state=%d\n", yytext[0], yytext[0], YYSTATE);
        count();
}

%%

int yywrap()
{
	BEGIN(INITIAL);
	return(1);
}

void count()
{
/* because of yyless, unput, input, and etc, the following counting may not be appropriate: */
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n') {
			column = 0;
			line++;
		} else if (yytext[i] == '\t')
			column += 4;
		else
			column++;

}

