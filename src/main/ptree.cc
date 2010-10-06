#include <ptree.h>
#include <assert.h>
#include <vgen-config.h>
#include <token-counter.h>

using namespace std;

ParseTree::ParseTree(Tree *root, int nTypes,
	map<int,string> *typeNames,
        map<string,int> *typeIds )
{
    this->root= root;
    this->nTypes= nTypes;
    this->typeNames= typeNames;
    this->typeIDs= typeIds;
}

ParseTree::~ParseTree()
{
  if ( root!=NULL ) {
    delete root;
    root = NULL;
  }
  // TODO: possible mem leak on typeNames, typeIds
}

Tree *ParseTree::getRoot()
{
    return root;
}

int ParseTree::typeCount()
{
    return nTypes;
}

const string & ParseTree::getTypeName(int id)
{
    assert(id<nTypes && id >=0);
    map<int,string>::iterator i= typeNames->find(id);
    if (i== typeNames->end()) {
        throw "not found";
    } else {
        return i->second;
    }
}

int ParseTree::getTypeID( const string &name)
{
    map<string,int>::iterator i= typeIDs->find(name);

    if (i== typeIDs->end()) {
        return -1;
    } else {
        return i->second;
    }
}

bool ParseTree::dumpParseTree(bool toOveride)
{
    ifstream inp;
    ofstream out;
    string outputfn = filename + ".grp";

    // prepare the output file:
    if(!toOveride) {
        inp.open(outputfn.c_str(), ifstream::in);
	inp.close();
	if(!inp.fail()) {
	    cerr << "Warning: parse tree dump file exists already: " << outputfn << " ...skip" << endl;
	    return false;
	}
	inp.clear(ios::failbit);
    }
    out.open(outputfn.c_str(), ofstream::out);
    if(out.fail()) {
        cerr << "Error: cannot open dump file: " << outputfn << endl;
	return false;
    }

    // dump the tree to the file:
    out << "# " << filename << endl;
    long ncount = 1;
    ncount = root->dumpTree(out, ncount);

    // close the file:
    out.close();
    return true;
}

Tree* ParseTree::tokenRange2Tree(long startTokenId, long endTokenId)
{
  if ( root == NULL )
    return NULL;

  list<Tree*>* path1 = root2Token(startTokenId);
  list<Tree*>* path2 = root2Token(endTokenId);
  Tree * rsl = NULL;

  if ( path1==NULL || path2==NULL )
    rsl = root;
  else {
    // compare the two paths starting from the root:
    list<Tree*>::iterator itr1 = path1->begin();
    list<Tree*>::iterator itr2 = path2->begin();
    for (; itr1 != path1->end() && itr2 != path2->end(); ++itr1, ++itr2) {
      if ( (*itr1) != (*itr2) ) // find the smallest common ancestor
	break;
      rsl = (*itr1);
    }
    // print the paths for debugging:
#define print_path_list(path_name) \
    for (list<Tree*>::iterator path_itr = (path_name)->begin(); \
         path_itr != (path_name)->end(); ++path_itr) \
      cerr << "NODE: %X" << (*path_itr) << " type:" << getTypeName(id2name, (*path_itr)->type) << endl;
    /*    print_path_list(path1);
    cerr << endl;
    print_path_list(path2); */
  }
  if ( rsl == NULL )
    rsl = root;

  if ( path1!=NULL )
    delete path1;
  if ( path2!=NULL )
    delete path2;

  return rsl; // shouldn't return NULL here.
}

Tree* ParseTree::getContextualNode(Tree* node)
{
   // assert 'node' in this parse tree
   if ( node==NULL )
      return root;

   map<NodeAttributeName_t, void*>::iterator attr_itr = node->attributes.find(NODE_TOKEN_ID);
   assert ( attr_itr != node->attributes.end() );
   pair<long, long>* startrange = (pair<long, long>*)(*attr_itr).second;
   if (node->parent==NULL)
      return root;

   Tree* startnode = node->parent;
   while ( startnode!=NULL ) {
      if ( isContextualNode(startnode) ) { // this condition is language-dependant
	 break;
      } else
	 startnode = startnode->parent;
   }
   if ( startnode==NULL )
      return root;
   return startnode;
}

Tree* ParseTree::getContextualNode(long startTokenId, long endTokenId)
{
   Tree* node = tokenRange2Tree(startTokenId, endTokenId);
   return getContextualNode(node);
}

list<Tree*>* ParseTree::root2Token(long tid)
{
  list<Tree*>* path = new list<Tree*>();
  root2TokenAux(tid, root, *path);
  if ( path->empty() ) {
    delete path;
    path = NULL;
  }
  return path;
}

bool ParseTree::root2TokenAux(long tid, Tree* node, list<Tree*>& path)
{
  if ( node == NULL )
    return false;

  map<NodeAttributeName_t, void*>::iterator attr_itr = node->attributes.find(NODE_TOKEN_ID);
  assert( attr_itr!=node->attributes.end() );
  const pair<long, long>* tokens = (pair<long, long>*)(*attr_itr).second;
  if ( tid >= tokens->first && tid <= tokens->second ) {
    // add the containing node to the path:
    path.push_back(node);
    // select a child with the containing range:
    for (int i=0; i < node->children.size(); i++) {
      if ( root2TokenAux(tid, node->children[i], path) )
	return true;		// within range.
    } // the path must exist and be unique (it's true for well-formed
      // ASTs), otherwise the code may return a wrong list.
    return false; // out of range. should NOT be reachable for well-formed ASTs.
  } else
    return false;		// out of range.
}

long ParseTree::tree2sn(Tree* nd)
{
    if (root == NULL || nd == NULL)
        return -1;

    long n = 1;
    n = root->tree2sn(nd, n);

    return n;
}


long Tree::dumpTree(ofstream & out, long n)
{
    long c = n++;
    out << "n " << c << " " << getTypeName(id2name, type) << endl;
    for (int i= 0; i < children.size(); i++) {
        out << "e " << c << " " << n << endl;
	n = children[i]->dumpTree(out, n);
    }
    return n;
}

long Tree::tree2sn(Tree* t, long& n)
{
    long c = n++;
    if(this==t)
       return c;

    for (int i= 0; i < children.size(); i++) {
	c = children[i]->tree2sn(t, n);
	if(c>0)
            return c;
    }
    return -1;
}

/* Valid type ids range from 0 to typeCount-1 */
int typeCount(map<int, string>& id2name)
{
  return id2name.size();
}

int typeCount(map<string, int>& name2id)
{
  return name2id.size();
}

const string & getTypeName(map<int, string>& id2name, int id)
{
  assert( id<id2name.size() && id >=0 );
  map<int,string>::iterator i= id2name.find(id);
  if (i == id2name.end()) {
    throw "not found";
  } else {
    return i->second;
  }
}

int getTypeID(map<string, int>& name2id, const string& name)
{
  map<string,int>::iterator i= name2id.find(name);
  if (i == name2id.end()) {
    return -1;
  } else {
    return i->second;
  }
}


bool compareTree(Tree* t1, Tree * t2)
{
  if ( t1==NULL && t2==NULL )
    return true;
  else if ( t1==NULL || t2==NULL )
    return false;
  else if ( t1->type != t2->type )
    return false;
  else if ( t1->children.size() != t2->children.size() )
    return false;
  else {
    for ( int i=0; i < t1->children.size(); i++ ) {
      if ( compareTree(t1->children[i], t2->children[i]) )
	continue;
      else
	return false;
    }
    return true;
  }
}

ParseTree* parseFile(const char * fn)
{
  // NOTE: yyparse is NOT re-entrant, which may cause unexpected behavior if called for more than one files.
  yyin = fopen(fn, "r");
  if (!yyin) {
    cerr << "Error: Can't open file for yyin: " << fn << endl;
  }
  yyrestart(yyin); // This may be unnecessary because BISON's manual
		   // says this is equivalent to (but I doubt it)
		   // changing yyin directly.
  yyparse();
  fclose(yyin);
  yyin = NULL;
  if (!root) {
    cerr << "Error: failed to parse file: " << fn << endl;
    return NULL;
  }

  Tree* initial_inh = NULL;
  root->lineRange();
  ParseTree* pt = new ParseTree(root, id2name.size(), &id2name, &name2id);
  TraGenConfiguration * vecGen_config = new TraGenConfiguration((const char*)NULL); // no actual use; just a dummy
  TokenRangeCounter * token_range_counter = new TokenRangeCounter(*vecGen_config);
  token_range_counter->reinit();
  token_range_counter->traverse(root, initial_inh);
  pt->filename = fn;
  return pt;
}

static vector<bool> ctxNodes; /* internal use only: ctxNodes[i]==true iff the node kind is considered as contexts */
static const char * contextualNodes[] = {
#ifdef JAVA 
#include "../ptgen/java/jcontextualNodes.h"
#else
#ifdef PHP
#include "../ptgen/php5/phpcontextualNodes.h"
#else
#include "../ptgen/gcc/ccontextualNodes.h"
#endif
#endif
};

bool setContextualNodes() /* internal use only */
{
  bool errflag = false;
  assert ( name2id.size() > 0 );
  ctxNodes = vector<bool>(id2name.size(), false);
  for (const char **s= contextualNodes; *s != NULL; s++) {
    map<string,int>::iterator i= name2id.find(*s);
    if (i == name2id.end()) {
      cerr << "unknown node type when setting contextual nodes: " << *s << endl;
      errflag = true;
      continue;
    }
    ctxNodes[i->second] = true;
  }
  return errflag;
}

bool isContextualNode(Tree* node)
{
  assert( node->type >= 0 && node->type < id2name.size() );
  if ( ctxNodes.empty() )
    setContextualNodes();
  return ctxNodes[node->type];
}

