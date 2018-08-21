#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <string.h> // for compatibility on various platforms
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <set>
#include <limits.h>

/** A Tree Node or a tree */
class Tree;

/** stuff from Ylex & Bison */
extern FILE *yyin;
void yyrestart( FILE *new_file );
int yyparse();
extern Tree* root;

/** stuff from ptgen */
void id_init();
extern std::map<std::string, int> name2id;
extern std::map<int, std::string> id2name; /*  */

/** A whole parse tree */
class ParseTree {
   const static int DEBUG_LEVEL = 1;
  public:
    ParseTree(Tree *root, int nTypes, std::map<int, std::string> *typeNames, std::map<std::string,int> *typeIds);
    /** recursively delete all tree nodes */
    ~ParseTree();

    Tree *getRoot();

    int typeCount();

    /** Valid type values range from 0 to typeCount-1 */
    const std::string & getTypeName(int);

    int getTypeID( const std::string& );  /* "IDENTIFER" for identifiers. */

    std::string filename;

    /** relevantNodes, are those that should be counted within the vector */
    std::vector<int> relevantNodes;

    /** leafNodes are the smallest nodes, also named atomicNodes, which are used to advance the
     * sliding window */
    std::vector<int> leafNodes;

    /** validParents, also named parentNodes, are the nodes from which we will generate vectors if they
     * have the required counts */
    std::vector<int> validParents;

    /** this's something similar to a combination of relevantNodes and atomicNodes just because of a different vector merging strategy,
	 * VectorMergerOnLists??? Not really used. */
    std::vector<int> mergeableNodes;

    /** dump the whole tree in a graph-like format; output filename is the 'filename'+'.grp' */ 
    bool dumpParseTree(const char* fn, bool toOveride);
    /** dump the tree in the dot format; output filename is the 'filename'+'.dot' */
    bool outputParseTree2Dot(const char* fn, bool toOveride);

    /** return the smallest tree containing all elements from the line number */
    Tree* line2Tree(int ln);
    /** return the smallest tree containing all elements from the line range */
    Tree* line2Tree(int startln, int endln);

    /** return the smallest common ancestor in the parse tree that contains all the tokens in the range: */
    Tree* tokenRange2Tree(long startTokenId, long endTokenId); 
    /** return the "contextual" node above the given node, i.e., a node that can indicate the type of the surrounding context containing this node: */
    Tree* getContextualNode(Tree* node);
    Tree* getContextualNode(long startTokenId, long endTokenId);

    /** set node ids from a given set of node names */
    static int setNodeIDs(std::vector<int>&, const std::set<std::string>&);
    
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
    std::map<int, std::string> *typeNames;

    /** map node type names to type ids */
    std::map<std::string, int> *typeIDs;
};

/* Valid type ids range from 0 to typeCount-1 */
int typeCount(std::map<int, std::string>& id2name);
int typeCount(std::map<std::string, int>& name2id);
const std::string & getTypeName(std::map<int, std::string>& id2name, int id);
int getTypeID(std::map<std::string, int>& name2id, const std::string& name); /* "IDENTIFER" for identifiers. */
bool isContextualNode(Tree* node);  // language-dependent operation 
bool setContextualNodes(const std::set<std::string>& nodenames);

/** create a parse tree from a file: */
ParseTree* parseFile(const char * fn);

class Terminal;
class NonTerminal;

/* enum type used as "subscripts" of tree attributes. */
typedef enum {
  NODE_VECTOR,			/** pointer to the tree vector */
  NODE_ID,			/** pointer to the min/max range of node IDs in the serialized tree */
  NODE_TOKEN_ID,		/** pointer to the min/max range of token IDs */
  NODE_SERIALIZED_NEIGHBOR, /** pointer to pointers to the previous and next nodes in the serialized chain */
} NodeAttributeName_t;

class Tree {
  public:
    /** the type id of this node */
    int type;  /* need to rely on getTypeName to get its type name */

    /** the child nodes of this node */
    std::vector<Tree*> children;

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
        std::cout << "[ " << getTypeName(id2name,type) << " ";
        for (int i= 0; i < children.size(); i++) {
            children[i]->print();
        }
        std::cout << "]";
    }

    virtual void printTok() {
        for (int i= 0; i < children.size(); i++) {
            children[i]->printTok();
        }
     }

    std::map<NodeAttributeName_t, void* > attributes;

    virtual ~Tree();
    
    Tree() {
       nextSibbling= NULL;
       parent= NULL;
       type= -1;
    }

    /** recursively calculate the range of line numbers for this tree node from bottom-up, and store results in [min, max].
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

    /** update the range of line numbers for this tree node based its direct children (i.e., no recursive updates),
     *  assuming every node has previously set max/min correctly already. */
    virtual void lineRangeUpdate()
    {
       for (int i= 0; i < children.size(); i++ ) {
          // no recursion needed, assuming every node has previously set max/min
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
    long dumpTree(std::ofstream & out, long n);
    /** output the nodes and the edges under this tree to a dot file: */
    long outputTree2Dot(std::ofstream & out, long n);

    /** get the order number of a tree node under this tree.
     * The order number is based on depth-first traversal and starts with 'n'.
     * It's for use with dumpParseTree. */
    long tree2sn(Tree* nd, long& n);

    /** Recursively compare node kinds. return false iff any difference. */
    friend bool compareTree(Tree* t1, Tree* t2);
};

class Terminal : public Tree {
public:
    Terminal( int type, const char *s, int line ) {
        this->type= type;
        value= new std::string(s);
        this->line= line;
    }
    int line;

    virtual ~Terminal() {
        delete value;
    }

    virtual void lineRange() {
        min= max= line;
    }

    virtual bool isTerminal() {return true;}
    virtual Terminal *toTerminal() {return this;}

    virtual void print()
    {
       std::cout << "<" << *value << ">";
    }

    virtual void printTok()
    {
       std::cout << *value << ", " << line << std::endl;
    }

    std::string *value;

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

