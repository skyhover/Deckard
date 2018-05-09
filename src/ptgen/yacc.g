header "YaccParser.__init__" {
self.NonTerminals= set([])
self.Terminals= set([])
self.Rules=[]
}

header "YaccLexer.__init__" {
}

options {
language = "Python";
}


class YaccParser extends Parser;
options { k=1; }

grammar: (rule)+ EOF;

rule:
(id:ID) {
self.NonTerminals.add(id.getText())
} COLON
rhs[id.getText()] SEMICOLON
;

rhs [lhs] {right=[]}:
( (id:ID) {
    right.append(("node",id.getText()))
    /* Heuristic: all UPPER case indicates a Terminal node */
    if id.getText() == id.getText().upper():
        self.Terminals.add(id.getText())
    else:
        self.NonTerminals.add(id.getText())
    }
  | (c:CHAR) {
    right.append(("node",c.getText()))
    self.Terminals.add(c.getText())
    }
  | (str:STRING) {
    right.append(("node",str.getText()))
    self.Terminals.add(str.getText())
    }
  | ERROR { right.append(("error","error")) }
  | PREC ((pi:ID) {right.append(("%prec","%prec "+pi.getText()))}|
          (pc:CHAR) {right.append(("%prec","%prec "+pc.getText()))}
          )
  | ACTION //ignore actions in grammar
)*
{
self.Rules.append( (lhs,right) )
}
(OR rhs[lhs])? ;

rulespec: HYPHEN ;

treespec: CARROT | BANG ;

class YaccLexer extends Lexer;
options { k=7; testLiterals=true; }

COLON: ':';
SEMICOLON: ';';
HYPHEN: '-';
CARROT: '^';
BANG: '!';
OR: '|';
PREC: "%prec";
ERROR: "error";
ID: LETTER (LETTER | DIGIT | '_' | '.')*;
protected
LETTER: ('a'..'z'|'A'..'Z');
protected
DIGIT: ('0'..'9');
//CHAR: '\'' (~'\'')+ '\''; // failed to recognize '\''
CHAR: '\'' (('\\' .)|(~('\''|'\\'))) '\'';
STRING: '"' (~'"')* '"';
ACTION: '{' // naively handle quoted braces:
        {
            lcount= 1
            incomment= False
            indquote= False
            insquote= False
            while lcount != 0:
                if self.LA(1) == '\\':
                    self.consume()
                elif self.LA(1) == '/' and self.LA(2) == '*':
                    if not indquote and not insquote:
                        incomment= True
                    self.consume()
                elif self.LA(1) == '*' and self.LA(2) == '/':
                    if not indquote and not insquote:
                        incomment= False
                    self.consume()
                elif self.LA(1) == '\'':
                    if not indquote and not incomment:
                        insquote= not insquote
                elif self.LA(1) == '"':
                    if not insquote and not incomment:
                        indquote= not indquote
                elif self.LA(1) == antlr.EOF:
                    $setType(antlr.EOF)
                elif self.LA(1) == '\n':
                    self.newline()
                elif not indquote and not insquote and not incomment:
                    if self.LA(1)== '}':
                        lcount -= 1
                    elif self.LA(1)== '{':
                        lcount += 1
                self.consume()
            $setType(antlr.SKIP);
        }
        ;

/* sample grammar from antlr2 java.g source code package */
// Single-line comments
SL_COMMENT
        :       "//"
                (~('\n'|'\r'))* ('\n'|'\r'('\n')?)?
                {$setType(antlr.SKIP); self.newline();}
        ;

// multiple-line comments
ML_COMMENT
        :       "/*"
                (       /*      '\r' '\n' can be matched in one alternative or by matching
                                '\r' in one iteration and '\n' in another.  I am trying to
                                handle any flavor of newline that comes in, but the language
                                that allows both "\r\n" and "\r" and "\n" to all be valid
                                newline is ambiguous.  Consequently, the resulting grammar
                                must be ambiguous.  I'm shutting this warning off.
                         */
                        options {
                                generateAmbigWarnings=false;
                        }
                :
                        { self.LA(2)!='/' }? '*'
                |       '\r' '\n'               {self.newline();}
                |       '\r'                    {self.newline();}
                |       '\n'                    {self.newline();}
                |       ~('*'|'\n'|'\r')
                )*
                "*/"
                {$setType(antlr.SKIP);}
        ;

//COMMENT: "/*" (~('*')|('*')~('/'))* "*/" {$setType(antlr.SKIP)}; // line counting may be wrong here
//COMMENT_LINE: "//" ~( '\r' | '\n' )* {$setType(antlr.SKIP)}; // end-of-line parsing may be wrong

WS    : ( ' '
        | '\r' '\n' { self.newline() }
        | '\n' { self.newline() }
        | '\t'
        )+
        { $setType(antlr.SKIP) }
      ;

