#!/usr/bin/python

# Given a normal yacc .y file, simply split it into three files for our yacc.g to work: *.y (which contains and only contains all the rules), *.y.head, *.y.foot

# Pi: To make the parse reentrant. http://www.gnu.org/software/bison/manual/html_mono/bison.html#Pure-Decl

import sys
sys.path.append('..')

import YaccParser,YaccLexer

if len(sys.argv) != 2:
    print >> sys.stderr, "usage %s grammar"%sys.argv[0]
    sys.exit(1)


grammar= open(sys.argv[1],'r')
y= YaccParser.Parser(YaccLexer.Lexer(grammar))
y.grammar()
grammar.close()


outg= open('pt_'+sys.argv[1],'w')

print >> outg, """
%pure-parser

%{
#include<ptree.h>

using namespace std;
%}

%union{
Tree *t;
}

%{
void yyerror(char*s);
int yylex(YYSTYPE *yylvalp);

Tree *root;
%}

"""

for nt in y.NonTerminals:
    print >> outg, '%type <t>', nt


header= open(sys.argv[1]+'.head','r')
outg.write( header.read() )
header.close()

print >> outg, """

%%

"""

name2id= {}
current= 0
for nt in y.NonTerminals:
    name2id[nt]=current
    current+=1
for t in y.Terminals:
    name2id[t]=current
    current+=1

#outh= open('tokid.h','w')
#for nt in y.NonTerminals:
#    print >> outh, '#define ID_'+nt, name2id[nt]
#outh.close()


for rule in y.Rules:
    print >> outg, rule[0], ':', 
    for production in rule[1]:
        print >> outg, production[1],
    """
    leave= False
    for production in rule[1]:
        if production[0]=="error":
            leave= True
            break
    if leave:
        print >> outg, ';\n'
        continue
    """
    print >> outg, """
    {
        $$= new NonTerminal(""", name2id[rule[0]], ");"
    nodelist= [ production[0] for production \
                in zip(range(1,len(rule[1])+1),rule[1]) \
                if production[1][0]=="node"]

    for i in range(len(nodelist)):
        print >> outg, "\n        $$->addChild($%d);"%nodelist[i]
        print >> outg, "\n        $%d->parent= $$;"%nodelist[i]
        if i + 1 < len(nodelist):
            print >> outg,\
                       "\n        $%d->nextSibbling= $%d;"%\
                       (nodelist[i],nodelist[i+1])
    if rule[0]=="program":
        print >> outg, "root= $$;"
    print >> outg, """
    }
    ;
"""

print >> outg, """

%%

"""

footer= open(sys.argv[1]+'.foot','r')
print >> outg, footer.read()
footer.close()


outg.close()


head = open('head.cc','w')

print >> head, """
#include <map>
#include <string>

using namespace std;

extern map<string,int> name2id;
extern map<int,string> id2name;

void id_init()
{
"""
for nt in y.NonTerminals:
    id= name2id[nt]
    print >> head, 'name2id["%s"]= %d;'%(nt,id)
    print >> head, 'id2name[%d]= "%s";'%(id,nt)
for t in y.Terminals:
    id= name2id[t]
    print >> head, 'name2id["%s"]= %d;'%(t,id)
    print >> head, 'id2name[%d]= "%s";'%(id,t)
print >> head, '}'
print >> head

head.close()

