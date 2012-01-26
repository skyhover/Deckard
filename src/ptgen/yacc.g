//
// 
// Copyright (c) 2007-2012,
//   Lingxiao Jiang         <lxjiang@ucdavis.edu>
//   Ghassan Misherghi      <ghassanm@ucdavis.edu>
//   Zhendong Su            <su@ucdavis.edu>
//   Stephane Glondu        <steph@glondu.net>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the University of California nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
//
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
    if id.getText() == id.getText().lower():
        self.NonTerminals.add(id.getText())
    else:
        self.Terminals.add(id.getText())
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

COMMENT: "/*" (~('*')|('*')~('/'))* "*/" {$setType(antlr.SKIP)};
//doesn't recognize '//' though

WS    : ( ' '
        | '\r' '\n' { self.newline() }
        | '\n' { self.newline() }
        | '\t'
        )+
        { $setType(antlr.SKIP) }
      ;

