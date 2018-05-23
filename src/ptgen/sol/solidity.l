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
int lasttoken = 0; // for look backward to determine certain tokens, e.g., '_' for a placeholder or an identifier.

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
		lasttoken = PRAGMA;
		return(PRAGMA); }
from			{ /* from can be an IDENTIFIER too */
		count();
		yylvalp->t = new Terminal(name2id["FROM"],yytext,line);
		lasttoken = FROM;
		return(FROM); }
import			{
		count();
		yylvalp->t = new Terminal(name2id["IMPORT"],yytext,line);
		lasttoken = IMPORT;
		return(IMPORT); }
as			{
		count();
		yylvalp->t = new Terminal(name2id["AS"],yytext,line);
		lasttoken = AS;
		return(AS); }
contract			{
		count();
		yylvalp->t = new Terminal(name2id["CONTRACT"],yytext,line);
		lasttoken = CONTRACT;
		return(CONTRACT); }
interface			{
		count();
		yylvalp->t = new Terminal(name2id["INTERFACE"],yytext,line);
		lasttoken = INTERFACE;
		return(INTERFACE); }
library			{
		count();
		yylvalp->t = new Terminal(name2id["LIBRARY"],yytext,line);
		lasttoken = LIBRARY;
		return(LIBRARY); }
is			{
		count();
		yylvalp->t = new Terminal(name2id["IS"],yytext,line);
		lasttoken = IS;
		return(IS); }
anonymous			{
		count();
		yylvalp->t = new Terminal(name2id["ANONYMOUS"],yytext,line);
		lasttoken = ANONYMOUS;
		return(ANONYMOUS); }
external			{
		count();
		yylvalp->t = new Terminal(name2id["EXTERNAL"],yytext,line);
		lasttoken = EXTERNAL;
		return(EXTERNAL); }
indexed			{
		count();
		yylvalp->t = new Terminal(name2id["INDEXED"],yytext,line);
		lasttoken = INDEXED;
		return(INDEXED); }
internal			{
		count();
		yylvalp->t = new Terminal(name2id["INTERNAL"],yytext,line);
		lasttoken = INTERNAL;
		return(INTERNAL); }
"constant"|"__constant__"|"__constant"	{
		count();
		yylvalp->t = new Terminal(name2id["CONSTANT"],yytext,line);
		lasttoken = CONSTANT;
		return(CONSTANT); }
payable			{
		count();
		yylvalp->t = new Terminal(name2id["PAYABLE"],yytext,line);
		lasttoken = PAYABLE;
		return(PAYABLE); }
private			{
		count();
		yylvalp->t = new Terminal(name2id["PRIVATE"],yytext,line);
		lasttoken = PRIVATE;
		return(PRIVATE); }
public			{
		count();
		yylvalp->t = new Terminal(name2id["PUBLIC"],yytext,line);
		lasttoken = PUBLIC;
		return(PUBLIC); }
pure			{
		count();
		yylvalp->t = new Terminal(name2id["PURE"],yytext,line);
		lasttoken = PURE;
		return(PURE); }
view			{
		count();
		yylvalp->t = new Terminal(name2id["VIEW"],yytext,line);
		lasttoken = VIEW;
		return(VIEW); }
using			{
		count();
		yylvalp->t = new Terminal(name2id["USING"],yytext,line);
		lasttoken = USING;
		return(USING); }
function			{
		count();
		yylvalp->t = new Terminal(name2id["FUNCTION"],yytext,line);
		lasttoken = FUNCTION;
		return(FUNCTION); }
break			{
		count();
		yylvalp->t = new Terminal(name2id["BREAK"],yytext,line);
		lasttoken = BREAK;
		return(BREAK); }
case		{
		count();
		yylvalp->t = new Terminal(name2id["CASE"],yytext,line);
		lasttoken = CASE;
		return(CASE); }
continue		{
		count();
		yylvalp->t = new Terminal(name2id["CONTINUE"],yytext,line);
		lasttoken = CONTINUE;
		return(CONTINUE); }
default		{
		count();
		yylvalp->t = new Terminal(name2id["DEFAULT"],yytext,line);
		lasttoken = DEFAULT;
		return(DEFAULT); }
"do"			{
		count();
		yylvalp->t = new Terminal(name2id["DO"],yytext,line);
		lasttoken = DO;
		return(DO); }
"else"			{
		count();
		yylvalp->t = new Terminal(name2id["ELSE"],yytext,line);
		lasttoken = ELSE;
		return(ELSE); }
"enum"			{
		count();
		yylvalp->t = new Terminal(name2id["ENUM"],yytext,line);
		lasttoken = ENUM;
		return(ENUM); }
"inline"|"__inline__"|"__inline"	{
		count();
		yylvalp->t = new Terminal(name2id["INLINE"],yytext,line);
		lasttoken = INLINE;
		return(INLINE); }
"for"			{
		count();
		yylvalp->t = new Terminal(name2id["FOR"],yytext,line);
		lasttoken = FOR;
		return(FOR); }
"if"			{
		count();
		yylvalp->t = new Terminal(name2id["IF"],yytext,line);
		lasttoken = IF;
		return(IF); }
modifier			{
		count();
		yylvalp->t = new Terminal(name2id["MODIFIER"],yytext,line);
		lasttoken = MODIFIER;
		return(MODIFIER); }
"returns"		{
		count();
		yylvalp->t = new Terminal(name2id["RETURNS"],yytext,line);
		lasttoken = RETURNS;
		return(RETURNS); }
"return"		{
		count();
		yylvalp->t = new Terminal(name2id["RETURN"],yytext,line);
		lasttoken = RETURN;
		return(RETURN); }
"static"		{
		count();
		yylvalp->t = new Terminal(name2id["STATIC"],yytext,line);
		lasttoken = STATIC;
		return(STATIC); }
"struct"		{
		count();
		yylvalp->t = new Terminal(name2id["STRUCT"],yytext,line);
		lasttoken = STRUCT;
		return(STRUCT); }
"switch"		{
		count();
		yylvalp->t = new Terminal(name2id["SWITCH"],yytext,line);
		lasttoken = SWITCH;
		return(SWITCH); }
address			{
		count();
		yylvalp->t = new Terminal(name2id["ADDRESS"],yytext,line);
		lasttoken = ADDRESS;
		return(ADDRESS); }
bool			{
		count();
		yylvalp->t = new Terminal(name2id["BOOL"],yytext,line);
		lasttoken = BOOL;
		return(BOOL); }
true			{
		count();
		yylvalp->t = new Terminal(name2id["TRUE"],yytext,line);
		lasttoken = TRUE;
		return(TRUE); }
false			{
		count();
		yylvalp->t = new Terminal(name2id["FALSE"],yytext,line);
		lasttoken = FALSE;
		return(FALSE); }
string			{
		count();
		yylvalp->t = new Terminal(name2id["STRING"],yytext,line);
		lasttoken = STRING;
		return(STRING); }
var			{
		count();
		yylvalp->t = new Terminal(name2id["VAR"],yytext,line);
		lasttoken = VAR;
		return(VAR); }
"int"|"int8"|"int16"|"int24"|"int32"|"int40"|"int48"|"int56"|"int64"|"int72"|"int80"|"int88"|"int96"|"int104"|"int112"|"int120"|"int128"|"int136"|"int144"|"int152"|"int160"|"int168"|"int176"|"int184"|"int192"|"int200"|"int208"|"int216"|"int224"|"int232"|"int240"|"int248"|"int256"		{
		count();
		yylvalp->t = new Terminal(name2id["INT"],yytext,line);
		lasttoken = INT;
		return(INT); }
"uint"|"uint8"|"uint16"|"uint24"|"uint32"|"uint40"|"uint48"|"uint56"|"uint64"|"uint72"|"uint80"|"uint88"|"uint96"|"uint104"|"uint112"|"uint120"|"uint128"|"uint136"|"uint144"|"uint152"|"uint160"|"uint168"|"uint176"|"uint184"|"uint192"|"uint200"|"uint208"|"uint216"|"uint224"|"uint232"|"uint240"|"uint248"|"uint256"		{
		count();
		yylvalp->t = new Terminal(name2id["UINT"],yytext,line);
		lasttoken = UINT;
		return(UINT); }
"bytes"|"bytes1"|"bytes2"|"bytes3"|"bytes4"|"bytes5"|"bytes6"|"bytes7"|"bytes8"|"bytes9"|"bytes10"|"bytes11"|"bytes12"|"bytes13"|"bytes14"|"bytes15"|"bytes16"|"bytes17"|"bytes18"|"bytes19"|"bytes20"|"bytes21"|"bytes22"|"bytes23"|"bytes24"|"bytes25"|"bytes26"|"bytes27"|"bytes28"|"bytes29"|"bytes30"|"bytes31"|"bytes32"		{
		count();
		yylvalp->t = new Terminal(name2id["BYTES"],yytext,line);
		lasttoken = BYTES;
		return(BYTES); }
"byte"		{
		count();
		yylvalp->t = new Terminal(name2id["BYTE"],yytext,line);
		lasttoken = BYTE;
		return(BYTE); }
fixed			{
		count();
		yylvalp->t = new Terminal(name2id["FIXED"],yytext,line);
		lasttoken = FIXED;
		return(FIXED); }
ufixed			{
		count();
		yylvalp->t = new Terminal(name2id["UFIXED"],yytext,line);
		lasttoken = UFIXED;
		return(UFIXED); }
mapping			{
		count();
		yylvalp->t = new Terminal(name2id["MAPPING"],yytext,line);
		lasttoken = MAPPING;
		return(MAPPING); }
memory			{
		count();
		yylvalp->t = new Terminal(name2id["MEMORY"],yytext,line);
		lasttoken = MEMORY;
		return(MEMORY); }
storage			{
		count();
		yylvalp->t = new Terminal(name2id["STORAGE"],yytext,line);
		lasttoken = STORAGE;
		return(STORAGE); }
throw			{
		count();
		yylvalp->t = new Terminal(name2id["THROW"],yytext,line);
		lasttoken = THROW;
		return(THROW); }
emit			{
		count();
		yylvalp->t = new Terminal(name2id["EMIT"],yytext,line);
		lasttoken = EMIT;
		return(EMIT); }
event			{
		count();
		yylvalp->t = new Terminal(name2id["EVENT"],yytext,line);
		lasttoken = EVENT;
		return(EVENT); }
new			{
		count();
		yylvalp->t = new Terminal(name2id["NEW"],yytext,line);
		lasttoken = NEW;
		return(NEW); }
after			{
		count();
		yylvalp->t = new Terminal(name2id["AFTER"],yytext,line);
		lasttoken = AFTER;
		return(AFTER); }
delete			{
		count();
		yylvalp->t = new Terminal(name2id["DELETE"],yytext,line);
		lasttoken = DELETE;
		return(DELETE); }
let			{
		count();
		yylvalp->t = new Terminal(name2id["LET"],yytext,line);
		lasttoken = LET;
		return(LET); }

"while"			{
		count();
		yylvalp->t = new Terminal(name2id["WHILE"],yytext,line);
		lasttoken = WHILE;
		return(WHILE); }
"assembly"|"__assembly__"|"__assembly"		{
		count();
		yylvalp->t = new Terminal(name2id["ASSEMBLY"],yytext,line);
		lasttoken = ASSEMBLY;
		return(ASSEMBLY); }
"typeof"|"__typeof__"|"__typeof"		{
		count();
		yylvalp->t = new Terminal(name2id["TYPEOF"],yytext,line);
		lasttoken = TYPEOF;
		return(TYPEOF); }

wei		{
		count();
		yylvalp->t = new Terminal(name2id["WEI"],yytext,line);
		lasttoken = WEI;
		return(WEI); }
szabo		{
		count();
		yylvalp->t = new Terminal(name2id["SZABO"],yytext,line);
		lasttoken = SZABO;
		return(SZABO); }
finney		{
		count();
		yylvalp->t = new Terminal(name2id["FINNEY"],yytext,line);
		lasttoken = FINNEY;
		return(FINNEY); }
ether		{
		count();
		yylvalp->t = new Terminal(name2id["ETHER"],yytext,line);
		lasttoken = ETHER;
		return(ETHER); }
seconds		{
		count();
		yylvalp->t = new Terminal(name2id["SECONDS"],yytext,line);
		lasttoken = SECONDS;
		return(SECONDS); }
minutes		{
		count();
		yylvalp->t = new Terminal(name2id["MINUTES"],yytext,line);
		lasttoken = MINUTES;
		return(MINUTES); }
hours		{
		count();
		yylvalp->t = new Terminal(name2id["HOURS"],yytext,line);
		lasttoken = HOURS;
		return(HOURS); }
days		{
		count();
		yylvalp->t = new Terminal(name2id["DAYS"],yytext,line);
		lasttoken = DAYS;
		return(DAYS); }
weeks		{
		count();
		yylvalp->t = new Terminal(name2id["WEEKS"],yytext,line);
		lasttoken = WEEKS;
		return(WEEKS); }
years		{
		count();
		yylvalp->t = new Terminal(name2id["YEARS"],yytext,line);
		lasttoken = YEARS;
		return(YEARS); }
hex		{
		count();
		yylvalp->t = new Terminal(name2id["HEX"],yytext,line);
		lasttoken = HEX;
		return(HEX); }
x		{ /* for fixed/ufixed points, e.g., ddd 'x' ddd; but x can also be an IDENTIFIER */
		count();
		yylvalp->t = new Terminal(name2id["'x'"],yytext,line);
		lasttoken = 'x';
		return('x'); }
_([ \t\v\r\n])*/";"	{ /* TODO: allow more flexibility after '_', e.g., allow comments */
		count();
		/* Suppose a placeholder is always a separate statement, i.e., it's within two statement delimiters. */
		if ( lasttoken==0 || lasttoken==';' || lasttoken=='{' || lasttoken=='}'
			|| lasttoken=='_' || lasttoken=='(' || lasttoken==')' ) {
			/* the above tokens indicate the beginning of a statement */
			// eat the ';'
			int c = yyinput();
			column++;
		}
		yylvalp->t = new Terminal(name2id["'_'"],"_;",line);
		lasttoken = '_';
		return('_');
	}
_		{ /* for _ in modifiers; allow '_' to be an identifier too, but not a typeName */
		count();
		yylvalp->t = new Terminal(name2id["'_'"],yytext,line);
		lasttoken = '_';
		return('_'); }

{L}({L}|{D})*		{
		count();
		yylvalp->t = new Terminal(name2id["IDENTIFIER"],yytext,line);
		lasttoken = IDENTIFIER;
		return(IDENTIFIER); }
{D}+"."{D}+"."{D}+		{
		count();
		yylvalp->t = new Terminal(name2id["VERSIONID"],yytext,line);
		lasttoken = VERSIONID;
		return(VERSIONID); }
0[xX]{H}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONST_INT_H"],yytext,line);
		lasttoken = CONST_INT_H;
		return(CONST_INT_H); }
0[xX]		{
		count();
		yylvalp->t = new Terminal(name2id["HEX_TAG"],yytext,line);
		lasttoken = HEX_TAG;
		return(HEX_TAG); }
0{D}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONST_INT_O"],yytext,line);
		lasttoken = CONST_INT_O;
		return(CONST_INT_O); }
{D}+{IS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONST_INT"],yytext,line);
		lasttoken = CONST_INT;
		return(CONST_INT); }
{D}+{E}{FS}?		{
		count();
		yylvalp->t = new Terminal(name2id["CONST_INT"],yytext,line);
		lasttoken = CONST_INT;
		return(CONST_INT); }
{D}*"."{D}+({E})?{FS}?	{
		count();
		yylvalp->t = new Terminal(name2id["CONST_NUM"],yytext,line);
		lasttoken = CONST_NUM;
		return(CONST_NUM); }
{D}+"."{D}*({E})?{FS}?	{
		count();
		yylvalp->t = new Terminal(name2id["CONST_NUM"],yytext,line);
		lasttoken = CONST_NUM;
		return(CONST_NUM); }

{L}?\"(\\.|[^\\"])*\"	{ /* match multiple-line strings too */
		count();
		yylvalp->t = new Terminal(name2id["STRING_LIT"],yytext,line);
		lasttoken = STRING_LIT;
		return(STRING_LIT); }
{L}?\'(\\.|[^\\'])*\'	{ /* match multiple-line strings too */
		count();
		yylvalp->t = new Terminal(name2id["STRING_LIT"],yytext,line);
		lasttoken = STRING_LIT;
		return(STRING_LIT); }

"..."			{
		count();
		yylvalp->t = new Terminal(name2id["ELLIPSIS"],yytext,line);
		lasttoken = ELLIPSIS;
		return(ELLIPSIS); }
">>="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_RSHIFT"],yytext,line);
		lasttoken = ASSIGN_RSHIFT;
		return(ASSIGN_RSHIFT); }
"<<="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_LSHIFT"],yytext,line);
		lasttoken = ASSIGN_LSHIFT;
		return(ASSIGN_LSHIFT); }
"+="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_PLUS"],yytext,line);
		lasttoken = ASSIGN_PLUS;
		return(ASSIGN_PLUS); }
"-="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_MINUS"],yytext,line);
		lasttoken = ASSIGN_MINUS;
		return(ASSIGN_MINUS); }
"*="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_MUL"],yytext,line);
		lasttoken = ASSIGN_MUL;
		return(ASSIGN_MUL); }
"/="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_DIV"],yytext,line);
		lasttoken = ASSIGN_DIV;
		return(ASSIGN_DIV); }
"%="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_REM"],yytext,line);
		lasttoken = ASSIGN_REM;
		return(ASSIGN_REM); }
"&="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_AND"],yytext,line);
		lasttoken = ASSIGN_AND;
		return(ASSIGN_AND); }
"^="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_XOR"],yytext,line);
		lasttoken = ASSIGN_XOR;
		return(ASSIGN_XOR); }
"|="	{
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_OR"],yytext,line);
		lasttoken = ASSIGN_OR;
		return(ASSIGN_OR); }
":"[ \t]*"="	{ /* allow white spaces in-between ':' and '=' */
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_ASSEMBLY_L"],yytext,line);
		lasttoken = ASSIGN_ASSEMBLY_L;
		return(ASSIGN_ASSEMBLY_L); }
"="[ \t]*":"	{ /* allow white spaces in-between '=' and ':' */
		count();
		yylvalp->t = new Terminal(name2id["ASSIGN_ASSEMBLY_R"],yytext,line);
		lasttoken = ASSIGN_ASSEMBLY_R;
		return(ASSIGN_ASSEMBLY_R); }
"=>"	{
		count();
		yylvalp->t = new Terminal(name2id["MAPSTO"],yytext,line);
		lasttoken = MAPSTO;
		return(MAPSTO); }
">>"			{
		count();
		yylvalp->t = new Terminal(name2id["RSHIFT"],yytext,line);
		lasttoken = RSHIFT;
		return(RSHIFT); }
"<<"			{
		count();
		yylvalp->t = new Terminal(name2id["LSHIFT"],yytext,line);
		lasttoken = LSHIFT;
		return(LSHIFT); }
"**"			{
		count();
		yylvalp->t = new Terminal(name2id["MULMUL"],yytext,line);
		lasttoken = MULMUL;
		return(MULMUL); }
"++"			{
		count();
		yylvalp->t = new Terminal(name2id["PLUSPLUS"],yytext,line);
		lasttoken = PLUSPLUS;
		return(PLUSPLUS); }
"--"			{
		count();
		yylvalp->t = new Terminal(name2id["MINUSMINUS"],yytext,line);
		lasttoken = MINUSMINUS;
		return(MINUSMINUS); }
"->"			{
		count();
		yylvalp->t = new Terminal(name2id["POINTSAT"],yytext,line);
		lasttoken = POINTSAT;
		return(POINTSAT); }
"&&"			{
		count();
		yylvalp->t = new Terminal(name2id["ANDAND"],yytext,line);
		lasttoken = ANDAND;
		return(ANDAND); }
"||"			{
		count();
		yylvalp->t = new Terminal(name2id["OROR"],yytext,line);
		lasttoken = OROR;
		return(OROR); }
"<="			{
		count();
		yylvalp->t = new Terminal(name2id["LEQ"],yytext,line);
		lasttoken = LEQ;
		return(LEQ); }
">="			{
		count();
		yylvalp->t = new Terminal(name2id["GEQ"],yytext,line);
		lasttoken = GEQ;
		return(GEQ); }
"<"			{
		count();
		yylvalp->t = new Terminal(name2id["LT"],yytext,line);
		lasttoken = LT;
		return(LT); }
">"			{
		count();
		yylvalp->t = new Terminal(name2id["GT"],yytext,line);
		lasttoken = GT;
		return(GT); }
"=="		{
		count();
		yylvalp->t = new Terminal(name2id["EQ"],yytext,line);
		lasttoken = EQ;
		return(EQ); }
"!="		{
		count();
		yylvalp->t = new Terminal(name2id["NEQ"],yytext,line);
		lasttoken = NEQ;
		return(NEQ); }
"="			{
		count();
		yylvalp->t = new Terminal(name2id["'='"],yytext,line);
		lasttoken = '=';
		return('='); }
";"			{
		count();
		yylvalp->t = new Terminal(name2id["';'"],yytext,line);
		lasttoken = ';';
		return(';'); }
"{"		{
		count();
		yylvalp->t = new Terminal(name2id["'{'"],yytext,line);
		lasttoken = '{';
		return('{'); }
"}"		{
		count();
		yylvalp->t = new Terminal(name2id["'}'"],yytext,line);
		lasttoken = '}';
		return('}'); }
","			{
		count();
		yylvalp->t = new Terminal(name2id["','"],yytext,line);
		lasttoken = ',';
		return(','); }
"("			{
		count();
		yylvalp->t = new Terminal(name2id["'('"],yytext,line);
		lasttoken = '(';
		return('('); }
")"			{
		count();
		yylvalp->t = new Terminal(name2id["')'"],yytext,line);
		lasttoken = ')';
		return(')'); }
"["		{
		count();
		yylvalp->t = new Terminal(name2id["'['"],yytext,line);
		lasttoken = '[';
		return('['); }
"]"		{
		count();
		yylvalp->t = new Terminal(name2id["']'"],yytext,line);
		lasttoken = ']';
		return(']'); }
":"			{
		count();
		yylvalp->t = new Terminal(name2id["':'"],yytext,line);
		lasttoken = ':';
		return(':'); }
"."			{
		count();
		yylvalp->t = new Terminal(name2id["'.'"],yytext,line);
		lasttoken = '.';
		return('.'); }
"&"			{
		count();
		yylvalp->t = new Terminal(name2id["'&'"],yytext,line);
		lasttoken = '&';
		return('&'); }
"!"			{
		count();
		yylvalp->t = new Terminal(name2id["'!'"],yytext,line);
		lasttoken = '!';
		return('!'); }
"~"			{
		count();
		yylvalp->t = new Terminal(name2id["'~'"],yytext,line);
		lasttoken = '~';
		return('~'); }
"-"			{
		count();
		yylvalp->t = new Terminal(name2id["'-'"],yytext,line);
		lasttoken = '-';
		return('-'); }
"+"			{
		count();
		yylvalp->t = new Terminal(name2id["'+'"],yytext,line);
		lasttoken = '+';
		return('+'); }
"*"			{
		count();
		yylvalp->t = new Terminal(name2id["'*'"],yytext,line);
		lasttoken = '*';
		return('*'); }
"/"			{
		count();
		yylvalp->t = new Terminal(name2id["'/'"],yytext,line);
		lasttoken = '/';
		return('/'); }
"%"			{
		count();
		yylvalp->t = new Terminal(name2id["'%'"],yytext,line);
		lasttoken = '%';
		return('%'); }
"^"			{
		count();
		yylvalp->t = new Terminal(name2id["'^'"],yytext,line);
		lasttoken = '^';
		return('^'); }
"|"			{
		count();
		yylvalp->t = new Terminal(name2id["'|'"],yytext,line);
		lasttoken = '|';
		return('|'); }
"?"			{
		count();
		yylvalp->t = new Terminal(name2id["'?'"],yytext,line);
		lasttoken = '?';
		return('?'); }
\"			{
		count();
		yylvalp->t = new Terminal(name2id["DQUOTE"],yytext,line);
		lasttoken = DQUOTE;
		return(DQUOTE); }
\'			{
		count();
		yylvalp->t = new Terminal(name2id["SQUOTE"],yytext,line);
		lasttoken = SQUOTE;
		return(SQUOTE); }
\r\n	|
\r		|
\n		{count();
		// useless tokens, just for counting 'line', 'column';
		}
.		{count(); /* . doesn't match newline, but may match \r depending on systems */
		// useless tokens, just for counting 'column'.
		}

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

