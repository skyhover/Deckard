D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include <ptree.h>
#include "pt_c.tab.hh"
#include <map>
#include <string>
using namespace std;

extern map<string,int> name2id;
void count();
int comment();
int cpp_comment();
%}

%%
"/*"			{ count(); comment(); }
"//"			{ count(); cpp_comment(); }

"auto"			{
		count();
		yylval.t= new Terminal(name2id["AUTO"],yytext);
		return(AUTO); }
"break"			{
		count();
		yylval.t= new Terminal(name2id["BREAK"],yytext);
		return(BREAK); }
"case"			{
		count();
		yylval.t= new Terminal(name2id["CASE"],yytext);
		return(CASE); }
"char"			{
		count();
		yylval.t= new Terminal(name2id["CHAR"],yytext);
		return(CHAR); }
"const"			{
		count();
		yylval.t= new Terminal(name2id["CONST"],yytext);
		return(CONST); }
"continue"		{
		count();
		yylval.t= new Terminal(name2id["CONTINUE"],yytext);
		return(CONTINUE); }
"default"		{
		count();
		yylval.t= new Terminal(name2id["DEFAULT"],yytext);
		return(DEFAULT); }
"do"			{
		count();
		yylval.t= new Terminal(name2id["DO"],yytext);
		return(DO); }
"double"		{
		count();
		yylval.t= new Terminal(name2id["DOUBLE"],yytext);
		return(DOUBLE); }
"else"			{
		count();
		yylval.t= new Terminal(name2id["ELSE"],yytext);
		return(ELSE); }
"enum"			{
		count();
		yylval.t= new Terminal(name2id["ENUM"],yytext);
		return(ENUM); }
"extern"		{
		count();
		yylval.t= new Terminal(name2id["EXTERN"],yytext);
		return(EXTERN); }
"float"			{
		count();
		yylval.t= new Terminal(name2id["FLOAT"],yytext);
		return(FLOAT); }
"for"			{
		count();
		yylval.t= new Terminal(name2id["FOR"],yytext);
		return(FOR); }
"goto"			{
		count();
		yylval.t= new Terminal(name2id["GOTO"],yytext);
		return(GOTO); }
"if"			{
		count();
		yylval.t= new Terminal(name2id["IF"],yytext);
		return(IF); }
"inline"		{
		count();
		yylval.t= new Terminal(name2id["INLINE"],yytext);
		return(INLINE); }
"int"			{
		count();
		yylval.t= new Terminal(name2id["INT"],yytext);
		return(INT); }
"long"			{
		count();
		yylval.t= new Terminal(name2id["LONG"],yytext);
		return(LONG); }
"register"		{
		count();
		yylval.t= new Terminal(name2id["REGISTER"],yytext);
		return(REGISTER); }
"return"		{
		count();
		yylval.t= new Terminal(name2id["RETURN"],yytext);
		return(RETURN); }
"short"			{
		count();
		yylval.t= new Terminal(name2id["SHORT"],yytext);
		return(SHORT); }
"signed"		{
		count();
		yylval.t= new Terminal(name2id["SIGNED"],yytext);
		return(SIGNED); }
"sizeof"		{
		count();
		yylval.t= new Terminal(name2id["SIZEOF"],yytext);
		return(SIZEOF); }
"static"		{
		count();
		yylval.t= new Terminal(name2id["STATIC"],yytext);
		return(STATIC); }
"struct"		{
		count();
		yylval.t= new Terminal(name2id["STRUCT"],yytext);
		return(STRUCT); }
"switch"		{
		count();
		yylval.t= new Terminal(name2id["SWITCH"],yytext);
		return(SWITCH); }
"typedef"		{
		count();
		yylval.t= new Terminal(name2id["TYPEDEF"],yytext);
		return(TYPEDEF); }
"union"			{
		count();
		yylval.t= new Terminal(name2id["UNION"],yytext);
		return(UNION); }
"unsigned"		{
		count();
		yylval.t= new Terminal(name2id["UNSIGNED"],yytext);
		return(UNSIGNED); }
"void"			{
		count();
		yylval.t= new Terminal(name2id["VOID"],yytext);
		return(VOID); }
"volatile"		{
		count();
		yylval.t= new Terminal(name2id["VOLATILE"],yytext);
		return(VOLATILE); }
"while"			{
		count();
		yylval.t= new Terminal(name2id["WHILE"],yytext);
		return(WHILE); }

{L}({L}|{D})*		{
		count();
		yylval.t= new Terminal(name2id["IDENTIFIER"],yytext);
		return(IDENTIFIER); }

0[xX]{H}+{IS}?		{
		count();
		yylval.t= new Terminal(name2id["CONSTANT"],yytext);
		return(CONSTANT); }
0{D}+{IS}?		{
		count();
		yylval.t= new Terminal(name2id["CONSTANT"],yytext);
		return(CONSTANT); }
{D}+{IS}?		{
		count();
		yylval.t= new Terminal(name2id["CONSTANT"],yytext);
		return(CONSTANT); }
L?'(\\.|[^\\'])+'	{
		count();
		yylval.t= new Terminal(name2id["CONSTANT"],yytext);
		return(CONSTANT); }

{D}+{E}{FS}?		{
		count();
		yylval.t= new Terminal(name2id["CONSTANT"],yytext);
		return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{
		count();
		yylval.t= new Terminal(name2id["CONSTANT"],yytext);
		return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{
		count();
		yylval.t= new Terminal(name2id["CONSTANT"],yytext);
		return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{
		count();
		yylval.t= new Terminal(name2id["STRING_LITERAL"],yytext);
		return(STRING_LITERAL); }
"..."			{
		count();
		yylval.t= new Terminal(name2id["ELLIPSIS"],yytext);
		return(ELLIPSIS); }
">>="			{
		count();
		yylval.t= new Terminal(name2id["RIGHT_ASSIGN"],yytext);
		return(RIGHT_ASSIGN); }
"<<="			{
		count();
		yylval.t= new Terminal(name2id["LEFT_ASSIGN"],yytext);
		return(LEFT_ASSIGN); }
"+="			{
		count();
		yylval.t= new Terminal(name2id["ADD_ASSIGN"],yytext);
		return(ADD_ASSIGN); }
"-="			{
		count();
		yylval.t= new Terminal(name2id["SUB_ASSIGN"],yytext);
		return(SUB_ASSIGN); }
"*="			{
		count();
		yylval.t= new Terminal(name2id["MUL_ASSIGN"],yytext);
		return(MUL_ASSIGN); }
"/="			{
		count();
		yylval.t= new Terminal(name2id["DIV_ASSIGN"],yytext);
		return(DIV_ASSIGN); }
"%="			{
		count();
		yylval.t= new Terminal(name2id["MOD_ASSIGN"],yytext);
		return(MOD_ASSIGN); }
"&="			{
		count();
		yylval.t= new Terminal(name2id["AND_ASSIGN"],yytext);
		return(AND_ASSIGN); }
"^="			{
		count();
		yylval.t= new Terminal(name2id["XOR_ASSIGN"],yytext);
		return(XOR_ASSIGN); }
"|="			{
		count();
		yylval.t= new Terminal(name2id["OR_ASSIGN"],yytext);
		return(OR_ASSIGN); }
">>"			{
		count();
		yylval.t= new Terminal(name2id["RIGHT_OP"],yytext);
		return(RIGHT_OP); }
"<<"			{
		count();
		yylval.t= new Terminal(name2id["LEFT_OP"],yytext);
		return(LEFT_OP); }
"++"			{
		count();
		yylval.t= new Terminal(name2id["INC_OP"],yytext);
		return(INC_OP); }
"--"			{
		count();
		yylval.t= new Terminal(name2id["DEC_OP"],yytext);
		return(DEC_OP); }
"->"			{
		count();
		yylval.t= new Terminal(name2id["PTR_OP"],yytext);
		return(PTR_OP); }
"&&"			{
		count();
		yylval.t= new Terminal(name2id["AND_OP"],yytext);
		return(AND_OP); }
"||"			{
		count();
		yylval.t= new Terminal(name2id["OR_OP"],yytext);
		return(OR_OP); }
"<="			{
		count();
		yylval.t= new Terminal(name2id["LE_OP"],yytext);
		return(LE_OP); }
">="			{
		count();
		yylval.t= new Terminal(name2id["GE_OP"],yytext);
		return(GE_OP); }
"=="			{
		count();
		yylval.t= new Terminal(name2id["EQ_OP"],yytext);
		return(EQ_OP); }
"!="			{
		count();
		yylval.t= new Terminal(name2id["NE_OP"],yytext);
		return(NE_OP); }
";"			{
		count();
		yylval.t= new Terminal(name2id["';'"],yytext);
		return(';'); }
("{"|"<%")		{
		count();
		yylval.t= new Terminal(name2id["'{'"],yytext);
		return('{'); }
("}"|"%>")		{
		count();
		yylval.t= new Terminal(name2id["'}'"],yytext);
		return('}'); }
","			{
		count();
		yylval.t= new Terminal(name2id["','"],yytext);
		return(','); }
":"			{
		count();
		yylval.t= new Terminal(name2id["':'"],yytext);
		return(':'); }
"="			{
		count();
		yylval.t= new Terminal(name2id["'='"],yytext);
		return('='); }
"("			{
		count();
		yylval.t= new Terminal(name2id["'('"],yytext);
		return('('); }
")"			{
		count();
		yylval.t= new Terminal(name2id["')'"],yytext);
		return(')'); }
("["|"<:")		{
		count();
		yylval.t= new Terminal(name2id["'['"],yytext);
		return('['); }
("]"|":>")		{
		count();
		yylval.t= new Terminal(name2id["']'"],yytext);
		return(']'); }
"."			{
		count();
		yylval.t= new Terminal(name2id["'.'"],yytext);
		return('.'); }
"&"			{
		count();
		yylval.t= new Terminal(name2id["'&'"],yytext);
		return('&'); }
"!"			{
		count();
		yylval.t= new Terminal(name2id["'!'"],yytext);
		return('!'); }
"~"			{
		count();
		yylval.t= new Terminal(name2id["'~'"],yytext);
		return('~'); }
"-"			{
		count();
		yylval.t= new Terminal(name2id["'-'"],yytext);
		return('-'); }
"+"			{
		count();
		yylval.t= new Terminal(name2id["'+'"],yytext);
		return('+'); }
"*"			{
		count();
		yylval.t= new Terminal(name2id["'*'"],yytext);
		return('*'); }
"/"			{
		count();
		yylval.t= new Terminal(name2id["'/'"],yytext);
		return('/'); }
"%"			{
		count();
		yylval.t= new Terminal(name2id["'%'"],yytext);
		return('%'); }
"<"			{
		count();
		yylval.t= new Terminal(name2id["'<'"],yytext);
		return('<'); }
">"			{
		count();
		yylval.t= new Terminal(name2id["'>'"],yytext);
		return('>'); }
"^"			{
		count();
		yylval.t= new Terminal(name2id["'^'"],yytext);
		return('^'); }
"|"			{
		count();
		yylval.t= new Terminal(name2id["'|'"],yytext);
		return('|'); }
"?"			{
		count();
		yylval.t= new Terminal(name2id["'?'"],yytext);
		return('?'); }

[ \t\v\n\f]		{
		count();
		 }
.			{ /* ignore bad characters */ }

%%

int yywrap()
{
	return(1);
}

int column = 0;
int line = 0;



int comment()
{
	int c, c1;

loop:
	while ((c = getchar()) != '*' && c != 0 && c!= EOF) {
		column++;
		if ( c == '\n' ) {
			column= 0;
			line++;
		}
	}
	column++;

	if ((c1 = getchar()) != '/' && c1 != 0 && c1!=EOF)
	{
		unput(c1);
		goto loop;
	}
	column++;

//	if (c != 0)
//		putchar(c1);
}

int cpp_comment()
{
int c;
	while ((c = getchar()) != '\n' && c != 0 && c!=EOF)
		column++;
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
			column += 8 - (column % 8);
		else
			column++;

	//ECHO;
}

