D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include <ptree.h>
#include "pt_j.tab.hh"
#include <map>
#include <string>
using namespace std;

extern map<string,int> name2id;
void count();
void comment();
void cpp_comment();
void macro();

// TODO: potential troublesome global variables
int column = 0;
int line = 1;

#define YY_DECL int yylex(YYSTYPE *yylvalp)

%}

%x COMMENT

%%

"/*"			{ count(); comment();}
"//"			{ count(); cpp_comment(); }
^"#"			{count(); macro();}
"public" {count();yylvalp->t=new Terminal(name2id["PUBLIC_TK"],yytext,line);return (PUBLIC_TK);}
"private" {count();yylvalp->t=new Terminal(name2id["PRIVATE_TK"],yytext,line);return (PRIVATE_TK);}
"protected" {count();yylvalp->t=new Terminal(name2id["PROTECTED_TK"],yytext,line);return (PROTECTED_TK);}
"static" {count();yylvalp->t=new Terminal(name2id["STATIC_TK"],yytext,line);return (STATIC_TK);}
"final" {count();yylvalp->t=new Terminal(name2id["FINAL_TK"],yytext,line);return (FINAL_TK);}
"synchronized" {count();yylvalp->t=new Terminal(name2id["SYNCHRONIZED_TK"],yytext,line);return (SYNCHRONIZED_TK);}
"volatile" {count();yylvalp->t=new Terminal(name2id["VOLATILE_TK"],yytext,line);return (VOLATILE_TK);}
"transient" {count();yylvalp->t=new Terminal(name2id["TRANSIENT_TK"],yytext,line);return (TRANSIENT_TK);}
"native" {count();yylvalp->t=new Terminal(name2id["NATIVE_TK"],yytext,line);return (NATIVE_TK);}
"abstract" {count();yylvalp->t=new Terminal(name2id["ABSTRACT_TK"],yytext,line);return (ABSTRACT_TK);}
"strictfp" {count();yylvalp->t=new Terminal(name2id["STRICT_TK"],yytext,line);return (STRICT_TK);}
"default" {count();yylvalp->t=new Terminal(name2id["DEFAULT_TK"],yytext,line);return (DEFAULT_TK);}
"if" {count();yylvalp->t=new Terminal(name2id["IF_TK"],yytext,line);return (IF_TK);}
"throw" {count();yylvalp->t=new Terminal(name2id["THROW_TK"],yytext,line);return (THROW_TK);}
"boolean" {count();yylvalp->t=new Terminal(name2id["BOOLEAN_TK"],yytext,line);return (BOOLEAN_TK);}
"do" {count();yylvalp->t=new Terminal(name2id["DO_TK"],yytext,line);return (DO_TK);}
"implements" {count();yylvalp->t=new Terminal(name2id["IMPLEMENTS_TK"],yytext,line);return (IMPLEMENTS_TK);}
"throws" {count();yylvalp->t=new Terminal(name2id["THROWS_TK"],yytext,line);return (THROWS_TK);}
"break" {count();yylvalp->t=new Terminal(name2id["BREAK_TK"],yytext,line);return (BREAK_TK);}
"import" {count();yylvalp->t=new Terminal(name2id["IMPORT_TK"],yytext,line);return (IMPORT_TK);}
"else" {count();yylvalp->t=new Terminal(name2id["ELSE_TK"],yytext,line);return (ELSE_TK);}
"instanceof" {count();yylvalp->t=new Terminal(name2id["INSTANCEOF_TK"],yytext,line);return (INSTANCEOF_TK);}
"return" {count();yylvalp->t=new Terminal(name2id["RETURN_TK"],yytext,line);return (RETURN_TK);}
"void" {count();yylvalp->t=new Terminal(name2id["VOID_TK"],yytext,line);return (VOID_TK);}
"catch" {count();yylvalp->t=new Terminal(name2id["CATCH_TK"],yytext,line);return (CATCH_TK);}
"interface" {count();yylvalp->t=new Terminal(name2id["INTERFACE_TK"],yytext,line);return (INTERFACE_TK);}
"case" {count();yylvalp->t=new Terminal(name2id["CASE_TK"],yytext,line);return (CASE_TK);}
"extends" {count();yylvalp->t=new Terminal(name2id["EXTENDS_TK"],yytext,line);return (EXTENDS_TK);}
"finally" {count();yylvalp->t=new Terminal(name2id["FINALLY_TK"],yytext,line);return (FINALLY_TK);}
"super" {count();yylvalp->t=new Terminal(name2id["SUPER_TK"],yytext,line);return (SUPER_TK);}
"while" {count();yylvalp->t=new Terminal(name2id["WHILE_TK"],yytext,line);return (WHILE_TK);}
"class" {count();yylvalp->t=new Terminal(name2id["CLASS_TK"],yytext,line);return (CLASS_TK);}
"switch" {count();yylvalp->t=new Terminal(name2id["SWITCH_TK"],yytext,line);return (SWITCH_TK);}
"const" {count();yylvalp->t=new Terminal(name2id["CONST_TK"],yytext,line);return (CONST_TK);}
"try" {count();yylvalp->t=new Terminal(name2id["TRY_TK"],yytext,line);return (TRY_TK);}
"for" {count();yylvalp->t=new Terminal(name2id["FOR_TK"],yytext,line);return (FOR_TK);}
"new" {count();yylvalp->t=new Terminal(name2id["NEW_TK"],yytext,line);return (NEW_TK);}
"continue" {count();yylvalp->t=new Terminal(name2id["CONTINUE_TK"],yytext,line);return (CONTINUE_TK);}
"goto" {count();yylvalp->t=new Terminal(name2id["GOTO_TK"],yytext,line);return (GOTO_TK);}
"package" {count();yylvalp->t=new Terminal(name2id["PACKAGE_TK"],yytext,line);return (PACKAGE_TK);}
"this" {count();yylvalp->t=new Terminal(name2id["THIS_TK"],yytext,line);return (THIS_TK);}
"assert" {count();yylvalp->t=new Terminal(name2id["ASSERT_TK"],yytext,line);return (ASSERT_TK);}
"byte" {count();yylvalp->t=new Terminal(name2id["BYTE_TK"],yytext,line);return (BYTE_TK);}
"short" {count();yylvalp->t=new Terminal(name2id["SHORT_TK"],yytext,line);return (SHORT_TK);}
"int" {count();yylvalp->t=new Terminal(name2id["INT_TK"],yytext,line);return (INT_TK);}
"long" {count();yylvalp->t=new Terminal(name2id["LONG_TK"],yytext,line);return (LONG_TK);}
"char" {count();yylvalp->t=new Terminal(name2id["CHAR_TK"],yytext,line);return (CHAR_TK);}
"integral" {count();yylvalp->t=new Terminal(name2id["INTEGRAL_TK"],yytext,line);return (INTEGRAL_TK);}
"float" {count();yylvalp->t=new Terminal(name2id["FLOAT_TK"],yytext,line);return (FLOAT_TK);}
"double" {count();yylvalp->t=new Terminal(name2id["DOUBLE_TK"],yytext,line);return (DOUBLE_TK);}
"true" {count();yylvalp->t=new Terminal(name2id["BOOL_LIT_TK"],yytext,line);return (BOOL_LIT_TK);}
"false" {count();yylvalp->t=new Terminal(name2id["BOOL_LIT_TK"],yytext,line);return (BOOL_LIT_TK);}
"null" {count();yylvalp->t=new Terminal(name2id["NULL_TK"],yytext,line);return (NULL_TK);}
{L}({L}|{D})* {count();yylvalp->t=new Terminal(name2id["ID_TK"],yytext,line);return(ID_TK); }
(0[xX]){H}+([Ll]?) {count();yylvalp->t=new Terminal(name2id["INT_LIT_TK"],yytext,line);return(INT_LIT_TK); }
{D}+([Ll]?) {count();yylvalp->t=new Terminal(name2id["INT_LIT_TK"],yytext,line);return(INT_LIT_TK); }
{D}+[FfDd] {count();yylvalp->t=new Terminal(name2id["FP_LIT_TK"],yytext,line);return(FP_LIT_TK); }
{D}+"."{D}*({E})?[fFdD]? {count();yylvalp->t=new Terminal(name2id["FP_LIT_TK"],yytext,line);return(FP_LIT_TK); }
{D}*"."{D}+({E})?[fFdD]? {count();yylvalp->t=new Terminal(name2id["FP_LIT_TK"],yytext,line);return(FP_LIT_TK); }
"\"".*"\"" {count();yylvalp->t=new Terminal(name2id["STRING_LIT_TK"],yytext,line);return(STRING_LIT_TK); }
"'"[^ \r\n\t]*"'" {count();yylvalp->t=new Terminal(name2id["CHAR_LIT_TK"],yytext,line);return(CHAR_LIT_TK); }
{L}?\"(\\.|[^\\"])*\" {count();yylvalp->t=new Terminal(name2id["STRING_LIT_TK"],yytext,line);return(STRING_LIT_TK); }
">>=" {count();yylvalp->t=new Terminal(name2id["SRS_ASSIGN_TK"],yytext,line);return(SRS_ASSIGN_TK); }
"<<=" {count();yylvalp->t=new Terminal(name2id["LS_ASSIGN_TK"],yytext,line);return(LS_ASSIGN_TK); }
">>>=" {count();yylvalp->t=new Terminal(name2id["ZRS_ASSIGN_TK"],yytext,line);return(ZRS_ASSIGN_TK); }
"+=" {count();yylvalp->t=new Terminal(name2id["PLUS_ASSIGN_TK"],yytext,line);return(PLUS_ASSIGN_TK); }
"-=" {count();yylvalp->t=new Terminal(name2id["MINUS_ASSIGN_TK"],yytext,line);return(MINUS_ASSIGN_TK); }
"*=" {count();yylvalp->t=new Terminal(name2id["MULT_ASSIGN_TK"],yytext,line);return(MULT_ASSIGN_TK); }
"/=" {count();yylvalp->t=new Terminal(name2id["DIV_ASSIGN_TK"],yytext,line);return(DIV_ASSIGN_TK); }
"%=" {count();yylvalp->t=new Terminal(name2id["REM_ASSIGN_TK"],yytext,line);return(REM_ASSIGN_TK); }
"&=" {count();yylvalp->t=new Terminal(name2id["AND_ASSIGN_TK"],yytext,line);return(AND_ASSIGN_TK); }
"^=" {count();yylvalp->t=new Terminal(name2id["XOR_ASSIGN_TK"],yytext,line);return(XOR_ASSIGN_TK); }
"|=" {count();yylvalp->t=new Terminal(name2id["OR_ASSIGN_TK"],yytext,line);return(OR_ASSIGN_TK); }
">>" {count();yylvalp->t=new Terminal(name2id["SRS_TK"],yytext,line);return(SRS_TK); }
"<<" {count();yylvalp->t=new Terminal(name2id["LS_TK"],yytext,line);return(LS_TK); }
">>>" {count();yylvalp->t=new Terminal(name2id["ZRS_TK"],yytext,line);return(ZRS_TK); }
"++" {count();yylvalp->t=new Terminal(name2id["INCR_TK"],yytext,line);return(INCR_TK); }
"--" {count();yylvalp->t=new Terminal(name2id["DECR_TK"],yytext,line);return(DECR_TK); }
"&&" {count();yylvalp->t=new Terminal(name2id["BOOL_AND_TK"],yytext,line);return(BOOL_AND_TK); }
"||" {count();yylvalp->t=new Terminal(name2id["BOOL_OR_TK"],yytext,line);return(BOOL_OR_TK); }
"<=" {count();yylvalp->t=new Terminal(name2id["LTE_TK"],yytext,line);return(LTE_TK); }
">=" {count();yylvalp->t=new Terminal(name2id["GTE_TK"],yytext,line);return(GTE_TK); }
"==" {count();yylvalp->t=new Terminal(name2id["EQ_TK"],yytext,line);return(EQ_TK); }
"!=" {count();yylvalp->t=new Terminal(name2id["NEQ_TK"],yytext,line);return(NEQ_TK); }
";" {count();yylvalp->t=new Terminal(name2id["SC_TK"],yytext,line);return(SC_TK); }
"{" {count();yylvalp->t=new Terminal(name2id["OCB_TK"],yytext,line);return(OCB_TK); }
"}" {count();yylvalp->t=new Terminal(name2id["CCB_TK"],yytext,line);return(CCB_TK); }
"," {count();yylvalp->t=new Terminal(name2id["C_TK"],yytext,line);return(C_TK); }
"=" {count();yylvalp->t=new Terminal(name2id["ASSIGN_TK"],yytext,line);return(ASSIGN_TK); }
"(" {count();yylvalp->t=new Terminal(name2id["OP_TK"],yytext,line);return(OP_TK); }
")" {count();yylvalp->t=new Terminal(name2id["CP_TK"],yytext,line);return(CP_TK); }
"[" {count();yylvalp->t=new Terminal(name2id["OSB_TK"],yytext,line);return(OSB_TK); }
"]" {count();yylvalp->t=new Terminal(name2id["CSB_TK"],yytext,line);return(CSB_TK); }
"\." {count();yylvalp->t=new Terminal(name2id["DOT_TK"],yytext,line);return(DOT_TK); }
"&" {count();yylvalp->t=new Terminal(name2id["AND_TK"],yytext,line);return(AND_TK); }
"!" {count();yylvalp->t=new Terminal(name2id["NOT_TK"],yytext,line);return(NOT_TK); }
"~" {count();yylvalp->t=new Terminal(name2id["NEG_TK"],yytext,line);return(NEG_TK); }
"-" {count();yylvalp->t=new Terminal(name2id["MINUS_TK"],yytext,line);return(MINUS_TK); }
"+" {count();yylvalp->t=new Terminal(name2id["PLUS_TK"],yytext,line);return(PLUS_TK); }
"*" {count();yylvalp->t=new Terminal(name2id["MULT_TK"],yytext,line);return(MULT_TK); }
"/" {count();yylvalp->t=new Terminal(name2id["DIV_TK"],yytext,line);return(DIV_TK); }
"%" {count();yylvalp->t=new Terminal(name2id["REM_TK"],yytext,line);return(REM_TK); }
"<" {count();yylvalp->t=new Terminal(name2id["LT_TK"],yytext,line);return(LT_TK); }
">" {count();yylvalp->t=new Terminal(name2id["GT_TK"],yytext,line);return(GT_TK); }
"^" {count();yylvalp->t=new Terminal(name2id["XOR_TK"],yytext,line);return(XOR_TK); }
"|" {count();yylvalp->t=new Terminal(name2id["OR_TK"],yytext,line);return(OR_TK); }
"?" {count();yylvalp->t=new Terminal(name2id["REL_QM_TK"],yytext,line);return(REL_QM_TK); }
":" {count();yylvalp->t=new Terminal(name2id["REL_CL_TK"],yytext,line);return(REL_CL_TK); }
([ \t\v\n\r])+ {count();}
. {count();} 

%%

int yywrap()
{
	return(1);
}



void comment()
{
	int c;

	for (;;) {
		while ( (c = yyinput()) != '*' && c != YY_NULL && c != EOF ) {
			if (c=='\n') {
				line++;column=0;
			} else {
				column++;
			}
		}

                if ( c == '*' ) {
			while ( (c = yyinput()) == '*' )
				column++;
			column++;
			if (c =='\n') {line++;column=0;}
			if ( c == '/' )
				break;
		}

                if ( c == EOF || c == YY_NULL ) {
			break;	
		}
            }
}

void cpp_comment()
{
	int c;
	while ((c = yyinput()) != '\n' && c != 0 && c!=EOF)
		column++;
	line++;
	column= 0;
}

void macro()
{
	int c,last=0;
	again:
	while ((c = yyinput()) != '\n' && c != 0 && c!=EOF) {
		if (c == '\n' && last == '\\')  {
			line++;
			goto again;
		}
		last= c;
	}

	line++;
	column= 0;
}

void count()
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n') {
			column = 0;
			line++;
		} else if (yytext[i] == '\t')
			column += 4;
		else
			column++;

	//ECHO;
}

