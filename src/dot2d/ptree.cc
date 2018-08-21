#include <ptree.h>
#include <assert.h>
#include <vgen-config.h>
#include <token-counter.h>
#include <namemap.h>

using namespace std;

/*******************************
 * class ParseTree
 */
ParseTree::ParseTree(Tree *root, int nTypes,
      map<int,string> *typeNames, map<string,int> *typeIds )
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
    assert(id < nTypes && id >= 0);
    map<int,string>::iterator i= typeNames->find(id);
    if (i== typeNames->end()) {
        throw "node type id not found";
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

bool ParseTree::dumpParseTree(const char* fn, bool toOveride)
{
   ifstream inp;
   ofstream out;
   string outputfn = (fn==NULL ? filename : string(fn)) + ".grp";

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
   cerr << "# Dumping the parse tree to file: " << outputfn << endl;
   out << "# " << filename << endl;
   long ncount = 1;
   ncount = root->dumpTree(out, ncount);

   // close the file:
   out.close();
   return true;
}

bool ParseTree::outputParseTree2Dot(const char* fn, bool toOveride)
{
   bool flag = true;
   ifstream inp;
   ofstream out;
   string outputfn = (fn==NULL ? filename : string(fn)) + ".dot";

   // prepare the output file:
   if(!toOveride) {
      inp.open(outputfn.c_str(), ifstream::in);
      inp.close();
      if(!inp.fail()) {
         cerr << "Warning: parse tree dot file exists already: " << outputfn << " ...skip" << endl;
         return false;
      }
      inp.clear(ios::failbit);
   }
   out.open(outputfn.c_str(), ofstream::out);
   if(out.fail()) {
      cerr << "Error: cannot open dot file: " << outputfn << endl;
      return false;
   }

   // Write the tree to the dot file:
   cerr << "# Writing the parse tree to dot file: " << outputfn << endl;
   out << "# This graph is supposed to be a parse tree." << endl;
   out << "digraph " << filename << " {" << endl;
   flag = flag && root->outputTree2Dot(out, 1);
   out << "}" << endl;

   // close the file:
   out.close();
   return flag;
}

Tree* ParseTree::line2Tree(int ln)
{
   // precondition: the line range of each node is set, and the line range of a parent node contains the line ranges of all of its children
   // Note that there may be more than one token in the same line
   return line2Tree(ln, ln);
}

Tree* ParseTree::line2Tree(int startln, int endln)
{
   // precondition: the line range of each node is set, and the line range of a parent node contains the line ranges of all of its children
   if(startln>endln || startln<0)
      return NULL;
   if(startln==0)
      return root;
   //else
   Tree* itr = root;
   while(itr!=NULL) {
//    cerr << "children size: " << itr->children.size() << ". Comparing with line range: " << itr->min << ":" << itr->max << endl;
      if(itr->min<=endln && startln<=itr->max) {
         int inRangeCount = 0;
         Tree* inRangeNode = NULL;
         for(int i=0; i<itr->children.size(); i++) {
//          cerr << "comparing with children " << i << "'s line range: " << itr->children[i]->min << ":" << itr->children[i]->max << endl;
            if(itr->children[i]->min<=endln && startln<=itr->children[i]->max) {
               inRangeCount++;
               if(inRangeCount>=2)
                  break;
               inRangeNode = itr->children[i];
            }
         }
         if(inRangeCount==1)
            itr = inRangeNode;
         else
            break;
      } else
         itr = NULL;
   }
   return itr;
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

int ParseTree::setNodeIDs(vector<int>& nids, const set<string>& nnames)
{
   int c = 0;
   for (set<string>::const_iterator nitr = nnames.begin();
         nitr!=nnames.end(); ++nitr) {
       map<string, int>::iterator i= name2id.find(*nitr);
       if (i == name2id.end()) {
           cerr << "ERROR: ParseTree::setNodeIDs: unknown node type name: " << *nitr << endl;
           if ( DEBUG_LEVEL>0 ) {
              for(map<string, int>::const_iterator it = name2id.begin();
                    it!=name2id.end(); ++it) {
                 cerr << it->first << "\t" << it->second << endl;
              }
              assert(name2id.size()>0);
              assert(name2id.size()==id2name.size());
           }
           continue;
       }
       nids.push_back(i->second);
       c++;
   }
   return c;
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


/*******************************************
 * class Tree
 */

Tree::~Tree()
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

   /* clear up the attributes, but the types used may depend on other files, increasing chances of circular dependency.
    * We need to include "tree vector" since we use its 'delete' operator, and simple forward declaration isn't enough of the types used.
    * TODO: break the circular dependence in a better way.
    */
   std::map<NodeAttributeName_t, void*>::iterator attr_itr;
   attr_itr = attributes.find(NODE_VECTOR);
   if ( attr_itr!=attributes.end() ) {
      TreeVector* attr = (TreeVector*)(*attr_itr).second;
      if ( attr!=NULL )
         delete attr;
   }
   attr_itr = attributes.find(NODE_ID);
   if ( attr_itr!=attributes.end() ) {
      std::pair<long, long>* attr = (std::pair<long, long>*)(*attr_itr).second;
      if ( attr!=NULL )
         delete attr;
   }
   attr_itr = attributes.find(NODE_TOKEN_ID);
   if ( attr_itr!=attributes.end() ) {
      std::pair<long, long>* attr = (std::pair<long, long>*)(*attr_itr).second;
      if ( attr!=NULL )
         delete attr;
   }
   attr_itr = attributes.find(NODE_SERIALIZED_NEIGHBOR);
   if ( attr_itr!=attributes.end() ) {
      std::pair<Tree*, Tree*>* attr = (std::pair<Tree*, Tree*>*)(*attr_itr).second;
      if ( attr!=NULL )
         delete attr;
   }
   attributes.clear();
}

long Tree::dumpTree(ofstream & out, long n)
{
    long c = n++;
    out << "n " << c << " " << getTypeName(id2name, type)
        << " [min,max]=[" << min << "," << max << "] TC=" << terminal_number << endl;
    for (int i= 0; i < children.size(); i++) {
        out << "e " << c << " " << n << endl;
        n = children[i]->dumpTree(out, n);
    }
    return n;
}

long Tree::outputTree2Dot(ofstream & out, long n)
{
   long c = n++;
   out << "nodeid" << c << " [ type=" << type << ", typeName=\"" << getTypeName(id2name, type)
       << "\", min=" << min << ", max=" << max << ", tokenCount=" << terminal_number << " ] ;" << endl;
   for (int i= 0; i < children.size(); i++) {
      out << "nodeid" << c << " -> " << "nodeid" << n << endl;
      n = children[i]->outputTree2Dot(out, n);
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
//   return id2name.size();
   // find the max id in the map
   int c = -1;
   for(map<int, string>::const_iterator it = id2name.begin();
         it!=id2name.end(); ++it) {
      if ( it->first > c )
         c = it->first;
   }
   return c+1;
}

int typeCount(map<string, int>& name2id)
{
//  return name2id.size();
   // find the max id in the map
   int c = -1;
   for(map<string, int>::const_iterator it = name2id.begin();
         it!=name2id.end(); ++it) {
      if ( it->second > c )
         c = it->second;
   }
   return c+1;
}

const string & getTypeName(map<int, string>& id2name, int id)
{
  assert( id < typeCount(id2name) && id >= 0 );
  map<int,string>::iterator i= id2name.find(id);
  if (i == id2name.end()) {
    throw "node type id not found";
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

static vector<bool> ctxNodes; /* internal use only: ctxNodes[i]==true iff the node kind is considered as contexts */

bool setContextualNodes(const set<string>& nodenames)
{
  bool errflag = false;
  assert ( ! name2id.empty() );
  ctxNodes = vector<bool>(typeCount(id2name), false);
  for (set<string>::const_iterator s  = nodenames.begin();
        s != nodenames.end(); ++s) {
     map<string, int>::const_iterator i= name2id.find(*s);
    if (i == name2id.end()) {
      cerr << "ERROR: setContextualNodes: unknown node type name when setting contextual nodes: " << *s << endl;
      errflag = true;
      continue;
    }
    ctxNodes[i->second] = true;
  }
  return errflag;
}

bool isContextualNode(Tree* node)
{
  assert( node->type >= 0 && node->type < typeCount(id2name) );
  if ( ctxNodes.empty() ) {
     throw "contextual nodes have not been set. Stop.";
  }
  return ctxNodes[node->type];
}

