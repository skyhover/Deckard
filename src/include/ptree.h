#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <limits.h>

/* better NOT use this in header files for the sake of large programs with lots of components. */
using namespace std;

class Tree;

class ParseTree {
public:
    ParseTree(Tree *root, int nTypes, map<int,string> *typeNames, map<string,int> *typeIds);
    ~ParseTree();

    Tree *getRoot();
    int typeCount();
    /* Valid type values range from 0 to typeCount-1 */
    const string &getTypeName(int);
    int getTypeID( const string& ); /* "IDENTIFER" for identifiers. */

    string filename;

    /* relevantNodes, are those that shouold be counted  within the vector */
    vector<int> relevantNodes;
    /* leafNodes are the smallest nodes which are used to advance the
     * sliding window */
    vector<int> leafNodes;
    /* validParents are the nodes from which we will generate vectors if they
     * have the required counts */
    vector<int> validParents;

    /* something similar to relevantNodes??? just because of a different vector merging strategy. */
    vector<int> mergeableNodes;
private:
    ParseTree();
    ParseTree(const ParseTree &);
    const ParseTree &operator=( const ParseTree &rhs);

    int nTypes;
    Tree *root;
    map<int,string> *typeNames;
    map<string,int> *typeIDs;
};

/* Valid type values range from 0 to typeCount-1 */
int typeCount(std::map<int, std::string>& id2name);
int typeCount(std::map<std::string, int>& name2id);
const string &getTypeName(std::map<int, std::string>& id2name, int id);
int getTypeID(std::map<std::string, int>& name2id, const string& name); /* "IDENTIFER" for identifiers. */


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

    int type;
    vector<Tree*> children;

    virtual bool isTerminal() { return false;}
    virtual bool isNonTerminal() { return false;}
    virtual Terminal *toTerminal() {return NULL;}
    virtual NonTerminal *toNonTerminal() {return NULL;}

    virtual Tree* getLeftMostChild() {
      if ( children.size()==0 )
	return this;
      else
	return children.front()->getLeftMostChild();
    }

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
      /* TODO: */
      /* tree nodes can not be shared: */
        for (int i= 0; i < children.size(); i++) {
	  if ( children[i]!=NULL ) {
            delete children[i];
	    children[i] = NULL;
	  }
        }
	nextSibbling = NULL;
	parent = NULL;

	/* TODO: not good, extra dependancies and singletons (make "delete" less convenient)......
	std::map<NodeAttributeName_t, void*>::iterator attr_itr = attributes.find(NODE_VECTOR);
	if ( attr_itr!=attributes.end() )
	  delete (TreeVector*)(*attr_itr).second;
	attr_itr = attributes.find(NODE_ID);
	if ( attr_itr!=attributes.end() )
	  delete (pair<long, long>*)(*attr_itr).second;
	attr_itr = attributes.find(NODE_TOKEN_ID);
	if ( attr_itr!=attributes.end() )
	  delete (pair<long, long>*)(*attr_itr).second;
	if ( attr_itr!=attributes.end() )
	  delete (pair<Tree*, Tree*>*)(*attr_itr).second; */
	attributes.clear();
    }
    
    Tree() {
        nextSibbling= NULL;
        parent= NULL;
        type= -1;
    }

    int max, min;
    virtual void lineRange() {
        max=-1;
        min=INT_MAX;
        for (int i= 0; i < children.size(); i++ ) {
            children[i]->lineRange();
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

    friend bool compareTree(Tree* t1, Tree* t2); /* recursively compare node kinds. return false iff any difference. */
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


#endif	/* _PARSE_TREE_H_ */

