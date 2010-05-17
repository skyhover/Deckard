#include <ptree.h>
#include <assert.h>

using namespace std;

ParseTree::ParseTree(Tree *root, int nTypes, map<int,string> *typeNames,
        map<string,int> *typeIds )
{
    this->root= root;
    this->nTypes= nTypes;
    this->typeNames= typeNames;
    this->typeIDs= typeIds;
}

ParseTree::~ParseTree()
{
  // TODO:
  if ( root!=NULL ) {
    delete root;
    root = NULL;
  }
    //    delete typeNames;
    //    delete typeIds;
}

Tree *ParseTree::getRoot()
{
    return root;
}

int ParseTree::typeCount()
{
    return nTypes;
}

const string &ParseTree::getTypeName(int id)
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

/* Valid type values range from 0 to typeCount-1 */
int typeCount(map<int, string>& id2name)
{
  return id2name.size();
}

int typeCount(map<string, int>& name2id)
{
  return name2id.size();
}

const string &getTypeName(map<int, string>& id2name, int id)
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
