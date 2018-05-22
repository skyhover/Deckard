D			[0-9]
L			[a-zA-Z$_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <error.h>
#include <stdio.h>
#include <ptree.h>
#include "pt_solidity.tab.hh"
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

"/*"			{ count(); comment(); }
"//"			{ count(); cpp_comment(); }
^"#"			{count(); macro();}

pragma			{
		count();
		yylvalp->t = new Terminal(name2id["PRAGMA"],yytext,line);
		return(PRAGMA); }
from			{ /* from can be an IDENTIFIER too */
		count();
		yylvalp->t = new Terminal(name2id["FROM"],yytext,line);
		return(FROM); }
import			{
		count();
		yylvalp->t = new Terminal(name2id["IMPORT"],yytext,line);
		return(IMPORT); }
as			{
		count();
		yylvalp->t = new Terminal(name2id["AS"],yytext,line);
		return(AS); }
contract			{
		count();
		yylvalp->t = new Terminal(name2id["CONTRACT"],yytext,line);
		return(CONTRACT); }
interface			{
		count();
		yylvalp->t = new Terminal(name2id["INTERFACE"],yytext,line);
		return(INTERFACE); }
library			{
		count();
		yylvalp->t = new Terminal(name2id["LIBRARY"],yytext,line);
		return(LIBRARY); }
is			{
		count();
		yylvalp->t = new Terminal(name2id["IS"],yytext,line);
		return(IS); }
anonymous			{
		count();
		yylvalp->t = new Terminal(name2id["ANONYMOUS"],yytext,line);
		return(ANONYMOUS); }
external			{
		count();
		yylvalp->t = new Terminal(name2id["EXTERNAL"],yytext,line);
		return(EXTERNAL); }
indexed			{
		count();
		yylvalp->t = new Terminal(name2id["INDEXED"],yytext,line);
		return(INDEXED); }
internal			{
		count();
		yylvalp->t = new Terminal(name2id["INTERNAL"],yytext,line);
		return(INTERNAL); }
"constant"|"__constant__"|"__constant"	{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		return(CONSTANT); }
payable			{
		count();
		yylvalp->t = new Terminal(name2id["PAYABLE"],yytext,line);
		return(PAYABLE); }
private			{
		count();
		yylvalp->t = new Terminal(name2id["PRIVATE"],yytext,line);
		return(PRIVATE); }
public			{
		count();
		yylvalp->t = new Terminal(name2id["PUBLIC"],yytext,line);
		return(PUBLIC); }
pure			{
		count();
		yylvalp->t = new Terminal(name2id["PURE"],yytext,line);
		return(PURE); }
view			{
		count();
		yylvalp->t = new Terminal(name2id["VIEW"],yytext,line);
		return(VIEW); }
using			{
		count();
		yylvalp->t = new Terminal(name2id["USING"],yytext,line);
		return(USING); }
function			{
		count();
		yylvalp->t = new Terminal(name2id["FUNCTION"],yytext,line);
		return(FUNCTION); }
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
"inline"|"__inline__"|"__inline"	{
		count();
		yylvalp->t = new Terminal(name2id["INLINE"],yytext,line);
		return(INLINE); }
"for"			{
		count();
		yylvalp->t = new Terminal(name2id["FOR"],yytext,line);
		return(FOR); }
"if"			{
		count();
		yylvalp->t = new Terminal(name2id["IF"],yytext,line);
		return(IF); }
modifier			{
		count();
		yylvalp->t = new Terminal(name2id["MODIFIER"],yytext,line);
		return(MODIFIER); }
"returns"		{
		count();
		yylvalp->t = new Terminal(name2id["RETURNS"],yytext,line);
		return(RETURNS); }
"return"		{
		count();
		yylvalp->t = new Terminal(name2id["RETURN"],yytext,line);
		return(RETURN); }
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
address			{
		count();
		yylvalp->t = new Terminal(name2id["ADDRESS"],yytext,line);
		return(ADDRESS); }
bool			{
		count();
		yylvalp->t = new Terminal(name2id["BOOL"],yytext,line);
		return(BOOL); }
true			{
		count();
		yylvalp->t = new Terminal(name2id["TRUE"],yytext,line);
		return(TRUE); }
false			{
		count();
		yylvalp->t = new Terminal(name2id["FALSE"],yytext,line);
		return(FALSE); }
string			{
		count();
		yylvalp->t = new Terminal(name2id["STRING"],yytext,line);
		return(STRING); }
var			{
		count();
		yylvalp->t = new Terminal(name2id["VAR"],yytext,line);
		return(VAR); }
"int"|"int8"|"int16"|"int24"|"int32"|"int40"|"int48"|"int56"|"int64"|"int72"|"int80"|"int88"|"int96"|"int104"|"int112"|"int120"|"int128"|"int136"|"int144"|"int152"|"int160"|"int168"|"int176"|"int184"|"int192"|"int200"|"int208"|"int216"|"int224"|"int232"|"int240"|"int248"|"int256"		{
		count();
		yylvalp->t = new Terminal(name2id["INT"],yytext,line);
		return(INT); }
"uint"|"uint8"|"uint16"|"uint24"|"uint32"|"uint40"|"uint48"|"uint56"|"uint64"|"uint72"|"uint80"|"uint88"|"uint96"|"uint104"|"uint112"|"uint120"|"uint128"|"uint136"|"uint144"|"uint152"|"uint160"|"uint168"|"uint176"|"uint184"|"uint192"|"uint200"|"uint208"|"uint216"|"uint224"|"uint232"|"uint240"|"uint248"|"uint256"		{
		count();
		yylvalp->t = new Terminal(name2id["UINT"],yytext,line);
		return(UINT); }
"bytes"|"bytes1"|"bytes2"|"bytes3"|"bytes4"|"bytes5"|"bytes6"|"bytes7"|"bytes8"|"bytes9"|"bytes10"|"bytes11"|"bytes12"|"bytes13"|"bytes14"|"bytes15"|"bytes16"|"bytes17"|"bytes18"|"bytes19"|"bytes20"|"bytes21"|"bytes22"|"bytes23"|"bytes24"|"bytes25"|"bytes26"|"bytes27"|"bytes28"|"bytes29"|"bytes30"|"bytes31"|"bytes32"		{
		count();
		yylvalp->t = new Terminal(name2id["BYTES"],yytext,line);
		return(BYTES); }
"byte"		{
		count();
		yylvalp->t = new Terminal(name2id["BYTE"],yytext,line);
		return(BYTE); }
fixed			{
		count();
		yylvalp->t = new Terminal(name2id["FIXED"],yytext,line);
		return(FIXED); }
ufixed			{
		count();
		yylvalp->t = new Terminal(name2id["UFIXED"],yytext,line);
		return(UFIXED); }
mapping			{
		count();
		yylvalp->t = new Terminal(name2id["MAPPING"],yytext,line);
		return(MAPPING); }
memory			{
		count();
		yylvalp->t = new Terminal(name2id["MEMORY"],yytext,line);
		return(MEMORY); }
storage			{
		count();
		yylvalp->t = new Terminal(name2id["STORAGE"],yytext,line);
		return(STORAGE); }
throw			{
		count();
		yylvalp->t = new Terminal(name2id["THROW"],yytext,line);
		return(THROW); }
emit			{
		count();
		yylvalp->t = new Terminal(name2id["EMIT"],yytext,line);
		return(EMIT); }
event			{
		count();
		yylvalp->t = new Terminal(name2id["EVENT"],yytext,line);
		return(EVENT); }
new			{
		count();
		yylvalp->t = new Terminal(name2id["NEW"],yytext,line);
		return(NEW); }
after			{
		count();
		yylvalp->t = new Terminal(name2id["AFTER"],yytext,line);
		return(AFTER); }
delete			{
		count();
		yylvalp->t = new Terminal(name2id["DELETE"],yytext,line);
		return(DELETE); }
let			{
		count();
		yylvalp->t = new Terminal(name2id["LET"],yytext,line);
		return(LET); }

"while"			{
		count();
		yylvalp->t = new Terminal(name2id["WHILE"],yytext,line);
		return(WHILE); }
"assembly"|"__assembly__"|"__assembly"		{
		count();
		yylvalp->t = new Terminal(name2id["ASSEMBLY"],yytext,line);
		return(ASSEMBLY); }
"typeof"|"__typeof__"|"__typeof"		{
		count();
		yylvalp->t = new Terminal(name2id["TYPEOF"],yytext,line);
		return(TYPEOF); }

wei		{
		count();
		yylvalp->t = new Terminal(name2id["WEI"],yytext,line);
		return(WEI); }
szabo		{
		count();
		yylvalp->t = new Terminal(name2id["SZABO"],yytext,line);
		return(SZABO); }
finney		{
		count();
		yylvalp->t = new Terminal(name2id["FINNEY"],yytext,line);
		return(FINNEY); }
ether		{
		count();
		yylvalp->t = new Terminal(name2id["ETHER"],yytext,line);
		return(ETHER); }
seconds		{
		count();
		yylvalp->t = new Terminal(name2id["SECONDS"],yytext,line);
		return(SECONDS); }
minutes		{
		count();
		yylvalp->t = new Terminal(name2id["MINUTES"],yytext,line);
		return(MINUTES); }
hours		{
		count();
		yylvalp->t = new Terminal(name2id["HOURS"],yytext,line);
		return(HOURS); }
days		{
		count();
		yylvalp->t = new Terminal(name2id["DAYS"],yytext,line);
		return(DAYS); }
weeks		{
		count();
		yylvalp->t = new Terminal(name2id["WEEKS"],yytext,line);
		return(WEEKS); }
years		{
		count();
		yylvalp->t = new Terminal(name2id["YEARS"],yytext,line);
		return(YEARS); }
hex		{
		count();
		yylvalp->t = new Terminal(name2id["HEX"],yytext,line);
		return(HEX); }
x		{ /* for fixed/ufixed points, e.g., ddd 'x' ddd; but x can also be an IDENTIFIER */
		count();
		yylvalp->t = new Terminal(name2id["'x'"],yytext,line);
		return('x'); }

^([ \t])*_([ \t])*;		{ /* _ in its own line ended with ';' is a placeholder statement. To disambiguate '_' with an empty statement. TODO: allow comments in the line too. */
		count();
		yylvalp->t = new Terminal(name2id["PLACEHOLDER"],"_;",line);
		return(PLACEHOLDER); }
\r([ \t])*_([ \t])*;		{ /* In addition to above, handle Macintosh EOL */
		count();
		yylvalp->t = new Terminal(name2id["PLACEHOLDER"],"_;",line);
		return(PLACEHOLDER); }
^([ \t])*"_"([ \t])*$		{ /* _ in its own line is a placeholder statement. This disambiguation is not necessary as '_' is disallowed to be a typeName. TODO: allow comments in the line too. */
		count();
		yylvalp->t = new Terminal(name2id["PLACEHOLDER"],"_",line);
		return(PLACEHOLDER); }
^([ \t])*"_"([ \t])*/\r\n		{ /* In addition to above, handle Windows EOL */
		count();
		yylvalp->t = new Terminal(name2id["PLACEHOLDER"],"_",line);
		return(PLACEHOLDER); }
^([ \t])*"_"([ \t])*/\r		{ /* In addition to above, handle Macintosh EOL */
		count();
		yylvalp->t = new Terminal(name2id["PLACEHOLDER"],"_",line);
		return(PLACEHOLDER); }
\r([ \t])*"_"([ \t])*/\r		{ /* In addition to above, handle Macintosh EOL */
		count();
		yylvalp->t = new Terminal(name2id["PLACEHOLDER"],"_",line);
		return(PLACEHOLDER); }
_		{ /* for _ in modifiers; allow '_' to be an identifier too, but not a typeName */
		count();
		yylvalp->t = new Terminal(name2id["'_'"],yytext,line);
		return('_'); }

{L}({L}|{D})*		{
		count();
		yylvalp->t = new Terminal(name2id["IDENTIFIER"],yytext,line);
		return(IDENTIFIER); }
{D}+"."{D}+"."{D}+		{
		count();
		yylvalp->t = new Terminal(name2id["VERSIONID"],yytext,line);
		return(VERSIONID); }
0[xX]{H}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONST_INT_H"],yytext,line);
		return(CONST_INT_H); }
0[xX]		{
		count();
		yylvalp->t = new Terminal(name2id["HEX_TAG"],yytext,line);
		return(HEX_TAG); }
0{D}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONST_INT_O"],yytext,line);
		return(CONST_INT_O); }
{D}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONST_INT"],yytext,line);
		return(CONST_INT); }
{D}+{E}{FS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONST_INT"],yytext,line);
		return(CONST_INT); }
{D}*"."{D}+({E})?{FS}?	{
		count();
		yylvalp->t = new Terminal(name2id["CONST_NUM"],yytext,line);
		return(CONST_NUM); }
{D}+"."{D}*({E})?{FS}?	{
		count();
		yylvalp->t = new Terminal(name2id["CONST_NUM"],yytext,line);
		return(CONST_NUM); }

{L}?\"(\\.|[^\\"])*\"	{ /* match multiple-line strings too */
		count();
		yylvalp->t = new Terminal(name2id["STRING_LIT"],yytext,line);
		return(STRING_LIT); }
{L}?\'(\\.|[^\\'])*\'	{ /* match multiple-line strings too */
		count();
		yylvalp->t = new Terminal(name2id["STRING_LIT"],yytext,line);
		return(STRING_LIT); }

"..."			{
		count();
		yylvalp->t = new Terminal(name2id["ELLIPSIS"],yytext,line);
		return(ELLIPSIS); }
">>="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_RSHIFT"],yytext,line);
		return(ASSIGN_RSHIFT); }
"<<="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_LSHIFT"],yytext,line);
		return(ASSIGN_LSHIFT); }
"+="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_PLUS"],yytext,line);
		return(ASSIGN_PLUS); }
"-="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_MINUS"],yytext,line);
		return(ASSIGN_MINUS); }
"*="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_MUL"],yytext,line);
		return(ASSIGN_MUL); }
"/="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_DIV"],yytext,line);
		return(ASSIGN_DIV); }
"%="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_REM"],yytext,line);
		return(ASSIGN_REM); }
"&="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_AND"],yytext,line);
		return(ASSIGN_AND); }
"^="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_XOR"],yytext,line);
		return(ASSIGN_XOR); }
"|="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_OR"],yytext,line);
		return(ASSIGN_OR); }
":"[ \t]*"="	{ /* allow white spaces in-between ':' and '=' */
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_ASSEMBLY_L"],yytext,line);
		return(ASSIGN_ASSEMBLY_L); }
"="[ \t]*":"	{ /* allow white spaces in-between '=' and ':' */
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_ASSEMBLY_R"],yytext,line);
		return(ASSIGN_ASSEMBLY_R); }
"=>"	{
		count();
		yylvalp->t = new Terminal(name2id["MAPSTO"],yytext,line);
		return(MAPSTO); }
">>"			{
		count();
		yylvalp->t = new Terminal(name2id["RSHIFT"],yytext,line);
		return(RSHIFT); }
"<<"			{
		count();
		yylvalp->t = new Terminal(name2id["LSHIFT"],yytext,line);
		return(LSHIFT); }
"**"			{
		count();
		yylvalp->t = new Terminal(name2id["MULMUL"],yytext,line);
		return(MULMUL); }
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
"<="			{
		count();
		yylvalp->t = new Terminal(name2id["LEQ"],yytext,line);
		return(LEQ); }
">="			{
		count();
		yylvalp->t = new Terminal(name2id["GEQ"],yytext,line);
		return(GEQ); }
"<"			{
		count();
		yylvalp->t = new Terminal(name2id["LT"],yytext,line);
		return(LT); }
">"			{
		count();
		yylvalp->t = new Terminal(name2id["GT"],yytext,line);
		return(GT); }
"=="		{
		count();
		yylvalp->t = new Terminal(name2id["EQ"],yytext,line);
		return(EQ); }
"!="		{
		count();
		yylvalp->t = new Terminal(name2id["NEQ"],yytext,line);
		return(NEQ); }
"="			{
		count();
		yylvalp->t = new Terminal(name2id["'='"],yytext,line);
		return('='); }
";"			{
		count();
		yylvalp->t = new Terminal(name2id["';'"],yytext,line);
		return(';'); }
"{"		{
		count();
		yylvalp->t = new Terminal(name2id["'{'"],yytext,line);
		return('{'); }
"}"		{
		count();
		yylvalp->t = new Terminal(name2id["'}'"],yytext,line);
		return('}'); }
","			{
		count();
		yylvalp->t = new Terminal(name2id["','"],yytext,line);
		return(','); }
"("			{
		count();
		yylvalp->t = new Terminal(name2id["'('"],yytext,line);
		return('('); }
")"			{
		count();
		yylvalp->t = new Terminal(name2id["')'"],yytext,line);
		return(')'); }
"["		{
		count();
		yylvalp->t = new Terminal(name2id["'['"],yytext,line);
		return('['); }
"]"		{
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
\"			{
		count();
		yylvalp->t = new Terminal(name2id["DQUOTE"],yytext,line);
		return(DQUOTE); }
\'			{
		count();
		yylvalp->t = new Terminal(name2id["SQUOTE"],yytext,line);
		return(SQUOTE); }
\r\n	|
\r		|
\n		{count();}
.		{count();} /* . doesn't match newline, but may match \r depending on systems */

%%

int yywrap()
{
	return(1);
}



void comment()
{
/* code adapted from: https://westes.github.io/flex/manual/Actions.html#index-discarding-C-comments.
 * But note that yyinput doesn't really return EOF (-1); it returns YY_NULL (0) instead.
 */
	int c;

	for (;;) {
		while ( (c = yyinput()) != '*' && c != YY_NULL && c != EOF) {
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

		if ( c == YY_NULL || c == EOF ) {
//			error(1, 1, "EOF in comment at line:column=%d:%d\n", line, column);
			break;	
		}
	}
}

void cpp_comment()
{
	int c;
	while ((c = yyinput()) != '\n' && c != YY_NULL && c!=EOF)
		column++;
	line++;
	column= 0;
}

void macro()
{
	int c,last=0;
	again:
	last= 0;
	while ((c = yyinput()) != '\n' && c != YY_NULL && c!=EOF) {
		if ( c == '\\' )
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
		if ( (yytext[i] == '\r' && yytext[i+1] != '\n') /* for Machintosh EOL */
			|| yytext[i] == '\n') {
			column = 0;
			line++;
		} else if (yytext[i] == '\t')
			column += 4;
		else
			column++;

	//ECHO;
}

