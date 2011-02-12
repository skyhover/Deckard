/*
 * 
 * Copyright (c) 2007-2010,
 *   Lingxiao Jiang         <lxjiang@ucdavis.edu>
 *   Ghassan Misherghi      <ghassanm@ucdavis.edu>
 *   Zhendong Su            <su@ucdavis.edu>
 *   Stephane Glondu        <steph@glondu.net>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <limits.h>

/* better NOT use this in header files for the sake of large programs with lots of components. */
using namespace std;

/** A Tree Node or a tree */
class Tree;

/** A whole parse tree */
class ParseTree {
  public:
    ParseTree(Tree *root, int nTypes, map<int,string> *typeNames, map<string,int> *typeIds);
    ~ParseTree();

    Tree *getRoot();

    int typeCount();

    /** Valid type values range from 0 to typeCount-1 */
    const string & getTypeName(int);

    int getTypeID( const string& );  /* "IDENTIFER" for identifiers. */

    string filename;

    /** relevantNodes, are those that shouold be counted within the vector */
    vector<int> relevantNodes;

    /** leafNodes are the smallest nodes which are used to advance the
     * sliding window */
    vector<int> leafNodes;

    /** validParents are the nodes from which we will generate vectors if they
     * have the required counts */
    vector<int> validParents;

    /** this's something similar to relevantNodes??? just because of a different vector merging strategy. */
    vector<int> mergeableNodes;

    /** dump the whole tree in a graph-like format; output filename is the 'filename'+'.grp' */ 
    bool dumpParseTree(bool toOveride);

    /** return the smallest tree containing all elements from the line number */
    Tree* line2Tree(int ln);
    /** return the smallest tree containing all elements from the line range */
    Tree* line2Tree(int startln, int endln);

    /** return the smallest common ancestor in the parse tree that contains all the tokens in the range: */
    Tree* tokenRange2Tree(long startTokenId, long endTokenId); 
    /** return the "contextual" node above the given node: */
    Tree* getContextualNode(Tree* node);
    Tree* getContextualNode(long startTokenId, long endTokenId);

    /** return the path from the root to the token: */
    std::list<Tree*>* root2Token(long tid); 
    bool root2TokenAux(long tid, Tree* node, std::list<Tree*>& path);

    /** get the order number of a tree node in the parse tree.
     * The order number is based on depth-first traversal and starts with 1.
     * It's currently for use dumpParseTree. */
    long tree2sn(Tree* n);

  private:
    ParseTree();
    ParseTree(const ParseTree &);
    const ParseTree &operator=( const ParseTree &rhs);

    int nTypes;  /* dimension of a vector */
    Tree *root;  /* the root tree node */

    /** map node type ids to type names */
    map<int,string> *typeNames;

    /** map node type names to type ids */
    map<string,int> *typeIDs;
};

/* Valid type ids range from 0 to typeCount-1 */
int typeCount(std::map<int, std::string>& id2name);
int typeCount(std::map<std::string, int>& name2id);
const string & getTypeName(std::map<int, std::string>& id2name, int id);
int getTypeID(std::map<std::string, int>& name2id, const string& name); /* "IDENTIFER" for identifiers. */
bool isContextualNode(Tree* node);  // language-dependent operation 

/** create a parse tree from a file: */
ParseTree* parseFile(const char * fn);

class Terminal;
class NonTerminal;

/* enum type used as "subscripts" of tree attributes. */
typedef enum {
  NODE_VECTOR,			/* the vector */
  NODE_ID,			/* range of node IDs in the serialized tree */
  NODE_TOKEN_ID,		/* range of token IDs */
  NODE_SERIALIZED_NEIGHBOR,
} NodeAttributeName_t;

class Tree {
  public:
    /** the type id of this node */
    int type;  /* need to rely on getTypeName to get its type name */

    /** the child nodes of this node */
    vector<Tree*> children;

    virtual bool isTerminal() { return false;}
    virtual bool isNonTerminal() { return false;}
    virtual Terminal *toTerminal() {return NULL;}
    virtual NonTerminal *toNonTerminal() {return NULL;}

    /** get the left most leaf node */
    virtual Tree* getLeftMostChild() {
      if ( children.size()==0 )
	return this;
      else
	return children.front()->getLeftMostChild();
    }

    /** append a child node */
    virtual void addChild( Tree *t ) {
        children.push_back(t);
    }

    virtual void print() {
        cout << "[ " << type << " ";
        for (int i= 0; i < children.size(); i++) {
            children[i]->print();
        }
        cout << "]";
    }

    virtual void printTok() {
        for (int i= 0; i < children.size(); i++) {
            children[i]->printTok();
        }
     }

    std::map<NodeAttributeName_t, void* > attributes;

    ~Tree()
    {
      /* tree nodes can not be shared: */
        for (int i= 0; i < children.size(); i++) {
	  if ( children[i]!=NULL ) {
            delete children[i];
	    children[i] = NULL;
	  }
        }
	nextSibbling = NULL;
	parent = NULL;

      /* TODO: possible mem leak from the elements in attributes. */
	attributes.clear();
    }
    
    Tree() {
        nextSibbling= NULL;
        parent= NULL;
        type= -1;
    }

    /** calculate the range of line numbers for this tree node, store results in [min, max].
     * For performance concerns, this function should only be called once from the root node. */
    int max, min;
    virtual void lineRange() {
        max=-1;
        min=INT_MAX;
        for (int i= 0; i < children.size(); i++ ) {
            children[i]->lineRange(); // recursion
            if (max < children[i]->max) {
                max= children[i]->max;
            }
            if (min > children[i]->min) {
                min= children[i]->min;
            }
        }
    }

    Tree *nextSibbling;
    Tree *parent;
    int terminal_number; /* The number of terminals under *this* node */

    /** output the nodes and the edges under this tree: */
    long dumpTree(ofstream & out, long n);

    /** get the order number of a tree node under this tree.
     * The order number is based on depth-first traversal and starts with 'n'.
     * It's for use with dumpParseTree. */
    long tree2sn(Tree* nd, long& n);

    /** Recursively compare node kinds. return false iff any difference. */
    friend bool compareTree(Tree* t1, Tree* t2);
};

class Terminal : public Tree {
public:
    Terminal( int type, char *s, int line ) {
        this->type= type;
        value= new string(s);
        this->line= line;
    }
    int line;

    ~Terminal() {
        delete value;
    }

    virtual void lineRange() {
        min= max= line;
    }

    virtual bool isTerminal() {return true;}
    virtual Terminal *toTerminal() {return this;}

    virtual void print()
    {
        cout << "<" << *value << ">";
    }

    virtual void printTok()
    {
        cout << *value << ", " << line << endl;
    }

    string *value;

};

class NonTerminal : public Tree {
public:
    NonTerminal( int type ) {
        this->type= type;
    }

    virtual bool isNonTerminal() {return true;}
    virtual NonTerminal *toNonTerminal() {return this;}
};


/** stuff from Ylex & Bison */
extern FILE *yyin;
void yyrestart( FILE *new_file );
int yyparse();
extern Tree* root;

/** stuff from ptgen */
void id_init();
extern std::map<std::string, int> name2id;
extern std::map<int, std::string> id2name; /*  */

#endif	/* _PARSE_TREE_H_ */

