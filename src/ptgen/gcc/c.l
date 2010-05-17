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
void comment();
void cpp_comment();
void macro();

// The global variables are not good for reentrance.
// But for now, the wrong line number doesn't hurt our bug finding because
// we rely on terminal IDs instead of line numbers.
int column = 0;
int line = 1;

#define YY_DECL int yylex(YYSTYPE *yylvalp)

%}

%x COMMENT

%%

"/*"			{ count(); comment();}
"//"			{ count(); cpp_comment(); }
^"#"			{count(); macro();}

break			{
		count();
		yylvalp->t = new Terminal(name2id["BREAK"],yytext,line);
		return(BREAK); }
case		{
		count();
		yylvalp->t = new Terminal(name2id["CASE"],yytext,line);
		return(CASE); }
continue		{
		count();
		yylvalp->t = new Terminal(name2id["CONTINUE"],yytext,line);
		return(CONTINUE); }
default		{
		count();
		yylvalp->t = new Terminal(name2id["DEFAULT"],yytext,line);
		return(DEFAULT); }
"do"			{
		count();
		yylvalp->t = new Terminal(name2id["DO"],yytext,line);
		return(DO); }
"else"			{
		count();
		yylvalp->t = new Terminal(name2id["ELSE"],yytext,line);
		return(ELSE); }
"enum"			{
		count();
		yylvalp->t = new Terminal(name2id["ENUM"],yytext,line);
		return(ENUM); }
"extern"|"register"|"auto"|"typedef"|"__thread"|"inline"|"__inline__"|"__inline"	{
		count();
		yylvalp->t = new Terminal(name2id["SCSPEC"],yytext,line);
		return(SCSPEC); }
"for"			{
		count();
		yylvalp->t = new Terminal(name2id["FOR"],yytext,line);
		return(FOR); }
"goto"			{
		count();
		yylvalp->t = new Terminal(name2id["GOTO"],yytext,line);
		return(GOTO); }
"if"			{
		count();
		yylvalp->t = new Terminal(name2id["IF"],yytext,line);
		return(IF); }
"return"		{
		count();
		yylvalp->t = new Terminal(name2id["RETURN"],yytext,line);
		return(RETURN); }
"sizeof"		{
		count();
		yylvalp->t = new Terminal(name2id["SIZEOF"],yytext,line);
		return(SIZEOF); }
"static"		{
		count();
		yylvalp->t = new Terminal(name2id["STATIC"],yytext,line);
		return(STATIC); }
"struct"		{
		count();
		yylvalp->t = new Terminal(name2id["STRUCT"],yytext,line);
		return(STRUCT); }
"switch"		{
		count();
		yylvalp->t = new Terminal(name2id["SWITCH"],yytext,line);
		return(SWITCH); }
"union"			{
		count();
		yylvalp->t = new Terminal(name2id["UNION"],yytext,line);
		return(UNION); }
"unsigned"|"long"|"short"|"signed"|"int"|"char"|"float"|"double"|"void"|"bool" {
		count();
		yylvalp->t = new Terminal(name2id["TYPESPEC"],yytext,line);
		return(TYPESPEC);
		}
"const"|"__const__"|"__const"|"volatile"|"__volatile__"|"__volatile"|"restrict"|"__restrict__"|"__restrict" {
		count();
		yylvalp->t = new Terminal(name2id["TYPE_QUAL"],yytext,line);
		return(TYPE_QUAL); }
"while"			{
		count();
		yylvalp->t = new Terminal(name2id["WHILE"],yytext,line);
		return(WHILE); }
"asm"|"__asm__"|"__asm"		{
		count();
		yylvalp->t = new Terminal(name2id["ASM_KEYWORD"],yytext,line);
		return(ASM_KEYWORD); }
"typeof"|"__typeof__"|"__typeof"		{
		count();
		yylvalp->t = new Terminal(name2id["TYPEOF"],yytext,line);
		return(TYPEOF); }

"__alignof__"|"__alignof"		{
		count();
		yylvalp->t = new Terminal(name2id["ALIGNOF"],yytext,line);
		return(ALIGNOF); }

"__attribute__"|"__attribute"		{
		count();
		yylvalp->t = new Terminal(name2id["ATTRIBUTE"],yytext,line);
		return(ATTRIBUTE); }

"__extension__"		{
		count();
		yylvalp->t = new Terminal(name2id["EXTENSION"],yytext,line);
		return(EXTENSION); }
{L}({L}|{D})*"_t"		{
		count();
		yylvalp->t = new Terminal(name2id["TYPENAME"],yytext,line);
		return(TYPENAME); }
"__init"|"__iomem"|"__user"|"__exit"|"__devexit"|"__devinit"|"__cpuinit"|"__cpuexit"|"__INLINE__"|"__kprobes"|"__inline__"|"__lockfunc"|"__force"|"__sched"|"__deprecated"|"__memzero"|"__pminit"|"__weak"|"__xipram"|"__attribute_used__"|"__apicdebuginit"|"__unused"|"__initdata" {count();}

{L}({L}|{D})*		{
		count();
		yylvalp->t = new Terminal(name2id["IDENTIFIER"],yytext,line);
		return(IDENTIFIER); }
0[xX]{H}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		return(CONSTANT); }
0{D}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		return(CONSTANT); }
{D}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		return(CONSTANT); }
L?'(\\.|[^\\'])+'	{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		return(CONSTANT); }

{D}+{E}{FS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{
		count();
		yylvalp->t = new Terminal(name2id["STRING"],yytext,line);
		return(STRING); }
"..."			{
		count();
		yylvalp->t = new Terminal(name2id["ELLIPSIS"],yytext,line);
		return(ELLIPSIS); }
">>="|"<<="|"+="|"-="|"*="|"/="|"%="|"&="|"^="|"|="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN"],yytext,line);
		return(ASSIGN); }
">>"			{
		count();
		yylvalp->t = new Terminal(name2id["RSHIFT"],yytext,line);
		return(RSHIFT); }
"<<"			{
		count();
		yylvalp->t = new Terminal(name2id["LSHIFT"],yytext,line);
		return(LSHIFT); }
"++"			{
		count();
		yylvalp->t = new Terminal(name2id["PLUSPLUS"],yytext,line);
		return(PLUSPLUS); }
"--"			{
		count();
		yylvalp->t = new Terminal(name2id["MINUSMINUS"],yytext,line);
		return(MINUSMINUS); }
"->"			{
		count();
		yylvalp->t = new Terminal(name2id["POINTSAT"],yytext,line);
		return(POINTSAT); }
"&&"			{
		count();
		yylvalp->t = new Terminal(name2id["ANDAND"],yytext,line);
		return(ANDAND); }
"||"			{
		count();
		yylvalp->t = new Terminal(name2id["OROR"],yytext,line);
		return(OROR); }
"<="|">="			{
		count();
		yylvalp->t = new Terminal(name2id["ARITHCOMPARE"],yytext,line);
		return(ARITHCOMPARE); }
"=="|"!="		{
		count();
		yylvalp->t = new Terminal(name2id["EOCOMPARE"],yytext,line);
		return(EQCOMPARE); }
";"			{
		count();
		yylvalp->t = new Terminal(name2id["';'"],yytext,line);
		return(';'); }
("{"|"<%")		{
		count();
		yylvalp->t = new Terminal(name2id["'{'"],yytext,line);
		return('{'); }
("}"|"%>")		{
		count();
		yylvalp->t = new Terminal(name2id["'}'"],yytext,line);
		return('}'); }
","			{
		count();
		yylvalp->t = new Terminal(name2id["','"],yytext,line);
		return(','); }
"="			{
		count();
		yylvalp->t = new Terminal(name2id["'='"],yytext,line);
		return('='); }
"("			{
		count();
		yylvalp->t = new Terminal(name2id["'('"],yytext,line);
		return('('); }
")"			{
		count();
		yylvalp->t = new Terminal(name2id["')'"],yytext,line);
		return(')'); }
("["|"<:")		{
		count();
		yylvalp->t = new Terminal(name2id["'['"],yytext,line);
		return('['); }
("]"|":>")		{
		count();
		yylvalp->t = new Terminal(name2id["']'"],yytext,line);
		return(']'); }
":"			{
		count();
		yylvalp->t = new Terminal(name2id["':'"],yytext,line);
		return(':'); }
"."			{
		count();
		yylvalp->t = new Terminal(name2id["'.'"],yytext,line);
		return('.'); }
"&"			{
		count();
		yylvalp->t = new Terminal(name2id["'&'"],yytext,line);
		return('&'); }
"!"			{
		count();
		yylvalp->t = new Terminal(name2id["'!'"],yytext,line);
		return('!'); }
"~"			{
		count();
		yylvalp->t = new Terminal(name2id["'~'"],yytext,line);
		return('~'); }
"-"			{
		count();
		yylvalp->t = new Terminal(name2id["'-'"],yytext,line);
		return('-'); }
"+"			{
		count();
		yylvalp->t = new Terminal(name2id["'+'"],yytext,line);
		return('+'); }
"*"			{
		count();
		yylvalp->t = new Terminal(name2id["'*'"],yytext,line);
		return('*'); }
"/"			{
		count();
		yylvalp->t = new Terminal(name2id["'/'"],yytext,line);
		return('/'); }
"%"			{
		count();
		yylvalp->t = new Terminal(name2id["'%'"],yytext,line);
		return('%'); }
"<"|">"			{
		count();
		yylvalp->t = new Terminal(name2id["ARITHCOMPARE"],yytext,line);
		return(ARITHCOMPARE); }
"^"			{
		count();
		yylvalp->t = new Terminal(name2id["'^'"],yytext,line);
		return('^'); }
"|"			{
		count();
		yylvalp->t = new Terminal(name2id["'|'"],yytext,line);
		return('|'); }
"?"			{
		count();
		yylvalp->t = new Terminal(name2id["'?'"],yytext,line);
		return('?'); }

[ \t\v\n\r]		{
		count();
		 }
.			{count();}

%%

int yywrap()
{
	return(1);
}



void comment()
{
	int c;

	for (;;) {
		while ( (c = yyinput()) != '*' && c != EOF ) {
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

                if ( c == EOF ) {
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
	last= 0;
	while ((c = yyinput()) != '\n' && c != 0 && c!=EOF) {
		last= c;
	}
	if (c == '\n' && last == '\\')  {
		line++;
		goto again;
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

