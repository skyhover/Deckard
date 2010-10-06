#include "clone-cluster.h"

/**************************************************
 * Implementation of ClonePointT
 *
 *************************************************/
bool ClonePointT::
parse(char * line, regex_t patterns[], int dim)
{
  // up to me to make sure patterns is of length dim.
  int a, b;
  regmatch_t pmatch[2];
  bool rslflag = true;

  if ( line==NULL || strcmp(line, "")==0 || strncmp(line, "\n", 1)==0 )
    return false;

  linebuf = line;

  if ( regexec(&(patterns[ENUM_CLONE_FILE]), line, 2, pmatch, 0)==0 &&
       (a = pmatch[1].rm_so) != -1 ) {
    b = pmatch[1].rm_eo;
    char t = line[b];
    line[b] = '\0';
    filename = line+a;
    line[b] = t;
  } else
    rslflag = false;

#define parseAux(buf, pat, fld, func) {                 \
    if ( regexec(&(pat), buf, 2, pmatch, 0)==0 &&       \
         (a = pmatch[1].rm_so) != -1 ) {                \
      b = pmatch[1].rm_eo;                              \
      char t = buf[b];                                  \
      buf[b] = '\0';                                    \
      fld = func(buf+a);                                \
      buf[b] = t;                                       \
    } else {                                            \
      fld = func("0");                                  \
      rslflag = false;                                  \
    }                                                   \
  }

  // relax return flag: as long as we have "FILE", then return true:
  bool fileflag = rslflag;
  parseAux(line, patterns[ENUM_CLONE_LINE], begin_line_id, atol);
  parseAux(line, patterns[ENUM_CLONE_INDEX], index, atol);
  parseAux(line, patterns[ENUM_CLONE_DIST], dist, atof);
  parseAux(line, patterns[ENUM_CLONE_OFFSET], line_offset, atol);
  parseAux(line, patterns[ENUM_CLONE_TBID], tbid, atol);
  parseAux(line, patterns[ENUM_CLONE_TEID], teid, atol);
  parseAux(line, patterns[ENUM_CLONE_NODE_KIND], node_kind, atoi);
  parseAux(line, patterns[ENUM_CLONE_NODE_NUM], node_number, atoi);
  parseAux(line, patterns[ENUM_CLONE_nVARs], uni_var_number, atoi);
  return fileflag;
}

ostream & ClonePointT::
out2html(ostream & os)
{
  //  os.width(9); os.fill('0');
  string showfilename = filename;
#define MAXLENFORSHOW 56
  if ( showfilename.length() > MAXLENFORSHOW )
    showfilename = "..." + showfilename.substr(showfilename.length() - MAXLENFORSHOW);
  os << index << "\t\tdist:" << /*setprecision(2) <<*/ dist << "\t\t"
     << "FILE <a href=\"" << filename << ".html\">" << showfilename << "</a> "
     << "LINE:<a href=\"" << filename << ".html#line" << begin_line_id << "\">" << begin_line_id << "</a>:<a href=\"" << filename << ".html#line" << (begin_line_id + line_offset -1) << "\">" << line_offset << "</a> "
     << "NODE_KIND:" << node_kind << " nVARs:" << uni_var_number << " NUM_NODE:" << node_number << " TBID:" << tbid << " TEID:" << teid << "<br />" << endl;
  return os;
}

ostream & ClonePointT::
out2html0(ostream & os)
{
  string showfilename = filename;
#define MAXLENFORSHOW 56
  if ( showfilename.length() > MAXLENFORSHOW )
    showfilename = "..." + showfilename.substr(showfilename.length() - MAXLENFORSHOW);
  string::size_type loc = linebuf.find("FILE ");
  if ( loc!=string::npos )
    os << linebuf.substr(0, loc);
  os << "FILE <a href=\"" << filename << ".html\">" << showfilename << "</a> "
     << "LINE:<a href=\"" << filename << ".html#line" << begin_line_id << "\">"
     << begin_line_id << "</a>:<a href=\"" << filename << ".html#line" << (begin_line_id + line_offset -1) << "\">" << line_offset << "</a> ";
  loc = linebuf.find("NODE_KIND:");
  if ( loc!=string::npos )
    os << linebuf.substr(loc);
  os << "<br />" << endl;
  return os;
}

ostream & operator<< (ostream& os, const struct _ClonePointT & cp)
{
  os << cp.linebuf;
  return os;
}

/******************************************
 * Implementation of CloneClusters
 *
 *****************************************/
regex_t CloneClusters::clone_patterns[ENUM_CLONE_THE_END];
#define MAX_FILES_IN_MEM 250

bool CloneClusters::
init_shared_data()
{
  int errcode; char errmsg[100];
  if ( 0 != (errcode = regcomp(&clone_patterns[ENUM_CLONE_INDEX], "^([0-9]+)", REG_EXTENDED | REG_NEWLINE)) ) {
    regerror(errcode, &clone_patterns[ENUM_CLONE_INDEX], errmsg, 100);
    cerr << "error code: " << errcode << endl << errmsg << endl;
  }
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_DIST], "dist:([0-9\\.]+)", REG_EXTENDED) );
  //NOTE: filenames can contain spaces...
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_FILE], "FILE (.*) LINE", REG_EXTENDED) );
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_LINE], "LINE:([0-9]+):", REG_EXTENDED) );
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_OFFSET], "LINE:[0-9]+:([0-9]+)", REG_EXTENDED) );
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_TBID], "TBID:([-]?[0-9]+)", REG_EXTENDED) );
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_TEID], "TEID:([-]?[0-9]+)", REG_EXTENDED) );
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_NODE_KIND], "NODE_KIND:([0-9]+)", REG_EXTENDED) );
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_NODE_NUM], "NUM_NODE:([0-9]+)", REG_EXTENDED) );
  assert ( 0 == regcomp(&clone_patterns[ENUM_CLONE_nVARs], "nVARs:([0-9]+)", REG_EXTENDED) );
  return true;
}

CloneClusters::
CloneClusters() : fn2tree(), clusters(), contexualNodes(id2name.size(), false)
{
  clusterbuffer = new vector<PClonePointT>();
  vecGen_config = new TraGenConfiguration((const char*)NULL); // no actual use; just a dummy
  token_range_counter = new TokenRangeCounter(*vecGen_config);
}

CloneClusters::
~CloneClusters()
{
  for(int i = 0; i<clusters.size(); i++) {
    for (int j=0; j<clusters[i].cluster->size(); j++)
      delete (*(clusters[i].cluster))[j];
    delete clusters[i].cluster;
  }
  if ( clusterbuffer!=NULL )
    delete clusterbuffer;
  if ( token_range_counter!=NULL )
    delete token_range_counter;
  if ( vecGen_config!=NULL )
    delete vecGen_config;
}

bool CloneClusters::
initNodes(const char ** nodeconfig)
{
  bool errflag = false;
  assert ( name2id.size() > 0 );
  contexualNodes = vector<bool>(id2name.size(), false);
  for (const char **s= nodeconfig; *s != NULL; s++) {
    map<string,int>::iterator i= name2id.find(*s);
    if (i == name2id.end()) {
      cerr << "unknown node type: " << *s << endl;
      errflag = true;
      continue;
    }
    contexualNodes[i->second] = true;
  }
  return errflag;
}

ParseTree* CloneClusters::
parseFile(const char * fn)
{
  // NOTE: yyparse is NOT re-entrant, which caused weird bugs when I called yyparse for more than one files!
  yyin = fopen(fn, "r");
  if (!yyin) {
    cerr << "Can't open file: " << fn << endl;
  }
  yyrestart(yyin); // This is maybe unnecessary because BISON's manual
		   // says this is equivalent to (but I doubt it)
		   // changing yyin directly.
  yyparse();
  fclose(yyin);
  yyin = NULL;
  if (!root) {
    cerr << "failed to parse file: " << fn << endl;
    return NULL;
  } else {
    Tree* initial_inh = NULL;
    root->lineRange();
    ParseTree* pt = new ParseTree(root, id2name.size(), &id2name, &name2id);
    token_range_counter->reinit(); // this is necessary to correctly match terminal IDs.
    token_range_counter->traverse(pt->getRoot(), initial_inh);
    return pt;
  }
}

#define parseClonePointAux(line, pcp, reg, field, func) {	\
    if ( regexec(&(reg), line, 2, pmatch, 0)==0 &&		\
	 (a = pmatch[1].rm_so) != -1 ) {			\
      b = pmatch[1].rm_eo;					\
      char t = line[b];						\
      line[b] = '\0';						\
      pcp->field = func(line+a);				\
      line[b] = t;						\
    } else							\
      rslflag = false;						\
  } // no use any more, coz I re-organized the code.

bool CloneClusters::
parseClonePoint(char * line, PClonePointT pcp)
{
  return pcp->parse(line, clone_patterns);
}

int CloneClusters::
createOneCluster(FILE * fin)
{
  char *line = NULL;
  size_t bufferLength = 0;
  ssize_t lineLength;
  int linecount = 0;

  while ( (lineLength = getline(&line, &bufferLength, fin)) > 0) {
    linecount++;
    if ( strcmp(line,"")==0 || strcmp(line, "\n")==0 ) // clone clusters are separated by empty lines.
      return clusterbuffer->size();
    PClonePointT temp = new ClonePointT();
    if ( parseClonePoint(line, temp) ) {
      clusterbuffer->push_back(temp);
    } else
      cerr << "Warning: error line " << linecount << ": " << line << endl;
  }
  return -1;
}

int CloneClusters::
createAllClusters(FILE * fin)
{
  int pointcount = 0;

  clearBuffer();

  while ( (pointcount=createOneCluster(fin))>=0 ) {
    if ( pointcount>0 ) {
      CloneClusterT temp;
      temp.cluster = clusterbuffer;
      temp.rank = 0; temp.buggy_score = 0;
      clusters.push_back(temp);
      clearBuffer();
    }
  }
  return clusters.size();
}

bool CloneClusters::
setOneRank(CloneClusterT & cls)
{
  cls.rank = 0; cls.buggy_score = 0;
  cls.rank |= rank1(cls);
  cls.rank |= rank2(cls);
  cls.rank |= rank3(cls);
  cls.buggy_score = buggy1(cls);
  return true;
}

int CloneClusters::
setAllRanks()
{
  for(int i=0; i<clusters.size(); i++) {
    setOneRank(clusters[i]);
  }
  return clusters.size();
}

template<class StrictWeakOrdering>int CloneClusters::
sortClusters(StrictWeakOrdering comp)
{
  sort(clusters.begin(), clusters.end(), comp);
  return clusters.size();
}

bool CloneClusters::
outputOneCluster(ostream & out, CloneClusterT & cls)
{
  out << "Rank score: " << cls.rank << " * " << cls.cluster->size() << " =" << cls.rank*cls.cluster->size()
       << "  buggy score: " << cls.buggy_score << endl;
  for (int i=0; i<cls.cluster->size(); i++)
    out << *((*(cls.cluster))[i]);
}

int CloneClusters::
outputAllClusters(ostream & out)
{
  for(int i=0; i<clusters.size(); i++) {
    outputOneCluster(out, clusters[i]);
    out << endl;
  }
  return clusters.size();
}

void CloneClusters::
clearMap()
{
  fn2tree.clear();
}

bool CloneClusters::
isContextual(Tree* node)
{
  assert( node->type >= 0 && node->type < id2name.size() );
  if ( contexualNodes[node->type] == true )
    return true;
  else
    return false;
}

void CloneClusters::
clearBuffer()
{
  clusterbuffer = new vector<PClonePointT>();
}

Tree* CloneClusters::
tokenRange2TreeAux1(pair<long, long> tokenrange, Tree* node)
// This function can't find the smallest subtree containing the tokenrange because of certain inconsistent logical error.
{
  Tree* rsl = NULL;
  map<NodeAttributeName_t, void*>::iterator attr_itr = node->attributes.find(NODE_TOKEN_ID);
  assert( attr_itr!=node->attributes.end() );
  const pair<long, long>* tokens = (pair<long, long>*)(*attr_itr).second;
  // assert ( tokens->first <= tokens->second );
  // assert ( tokenrange.first <= tokenrange.second );
  if ( tokens->first < tokenrange.first ) // logical bug here. "<=" instead of "<"
    if ( tokens->second < tokenrange.first )
      return NULL;
    else if ( tokens->second <= tokenrange.second ) // logical bug here. "<" instead of "<="
      return node;
    else {			// recursion: 
      int num_nullchild = 0;
      for (int i = 0; i < node->children.size(); i++) {
	Tree* rslchild = tokenRange2TreeAux1(tokenrange, node->children[i]);
	if ( rslchild!=NULL ) {
	  num_nullchild++;
	  if ( num_nullchild>1 )
	    return node;
	  else
	    rsl = rslchild;
	}
      }
      return rsl;
    }
  else if ( tokens->first <= tokenrange.second ) // logical bug here. TODO: may cut this branch...
    return node;
  else
    return NULL;
}

Tree* CloneClusters::
tokenRange2Tree1(std::pair<long, long> tokenrange, ParseTree* pt)
{
  if ( pt==NULL )
    return NULL;

  Tree * root = pt->getRoot();
  if ( root == NULL )
    return NULL;
  else
    return tokenRange2TreeAux1(tokenrange, root); // shouldn't return NULL here.
}

Tree* CloneClusters::
tokenRange2Tree2(std::pair<long, long> tokenrange, ParseTree* pt)
{
  if ( pt==NULL )
    return NULL;

  Tree * root = pt->getRoot();
  if ( root == NULL )
    return NULL;

  list<Tree*>* path1 = root2Token(tokenrange.first, pt);
  list<Tree*>* path2 = root2Token(tokenrange.second, pt);
  Tree * rsl = NULL;

  if ( path1==NULL || path2==NULL )
    rsl = root;
  else {			// compare the two paths
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

bool CloneClusters::
root2TokenAux(long tid, Tree* node, list<Tree*>& path)
{
  if ( node == NULL )
    return false;

  map<NodeAttributeName_t, void*>::iterator attr_itr = node->attributes.find(NODE_TOKEN_ID);
  assert( attr_itr!=node->attributes.end() );
  const pair<long, long>* tokens = (pair<long, long>*)(*attr_itr).second;
  if ( tid >= tokens->first && tid <= tokens->second ) {
    // TODO: try to make it tail recursive:

    // add the current node to the path:
    path.push_back(node);
    // select a child with the correct range:
    for (int i=0; i < node->children.size(); i++) {
      if ( root2TokenAux(tid, node->children[i], path) )
	return true;		// within range.
    } // the path must exist and be unique (it's true for well-formed
      // ASTs), otherwise the code may return a wrong list.
    return false; // out of range. should NOT be here for well-formed ASTs.
  } else
    return false;		// out of range.
}

list<Tree*>* CloneClusters::
root2Token(long tid, ParseTree* pt)
{
  list<Tree*>* path = new list<Tree*>();
  root2TokenAux(tid, pt->getRoot(), *path);
  if ( path->empty() ) {
    delete path;
    path = NULL;
  }
  return path;
}

CloneContextT CloneClusters::
getContext1(pair<long, long> tokenrange, string & fn)
{
  CloneContextT rsl;
  ParseTree* pt = NULL;
  map<string, ParseTree*>::iterator fn_itr = fn2tree.find(fn);
  if ( fn_itr == fn2tree.end() ) {
    // save memory - delete all existing trees if too many files:
    if ( fn2tree.size() > MAX_FILES_IN_MEM ) {
      for ( fn_itr = fn2tree.begin(); fn_itr!=fn2tree.end(); fn_itr++) {
        delete (*fn_itr).second;
      }
      fn2tree.clear();
    }
    // create the new tree:
    pt = parseFile(fn.c_str());
    if ( pt!=NULL ) {
      fn2tree.insert(pair<string, ParseTree*>(fn, pt));
    }
  } else
    pt = (*fn_itr).second;

  rsl.context_node_begin = rsl.context_node_end = NULL;
  rsl.context_node_begin = tokenRange2Tree2(tokenrange, pt);
  if ( rsl.context_node_begin!=NULL )
    rsl.context_node_end = rsl.context_node_begin->parent;
  else
    rsl.context_node_begin = pt ? pt->getRoot() : NULL;

  // assert ( rsl.context_node_begin!=NULL );
  return rsl;
}

CloneContextT CloneClusters::
getContext2(pair<long, long> tokenrange, string & fn)
{
  CloneContextT rsl;
  ParseTree* pt = NULL;
  map<string, ParseTree*>::iterator fn_itr = fn2tree.find(fn);
  if ( fn_itr == fn2tree.end() ) {
    // save memory - delete all existing trees if too many files:
    if ( fn2tree.size() > MAX_FILES_IN_MEM ) {
      for ( fn_itr = fn2tree.begin(); fn_itr!=fn2tree.end(); fn_itr++) {
	delete (*fn_itr).second;
      }
      fn2tree.clear();
    }
    // create the new tree:
    pt = parseFile(fn.c_str());
    if ( pt!=NULL ) {
      fn2tree.insert(pair<string, ParseTree*>(fn, pt));
    }
  } else
    pt = (*fn_itr).second;

  rsl.context_node_begin = rsl.context_node_end = NULL;
  rsl.context_node_begin = tokenRange2Tree2(tokenrange, pt);
  if ( rsl.context_node_begin!=NULL ) {
    map<NodeAttributeName_t, void*>::iterator attr_itr = rsl.context_node_begin->attributes.find(NODE_TOKEN_ID);
    assert ( attr_itr!=rsl.context_node_begin->attributes.end() );
    pair<long, long>* startrange = (pair<long, long>*)(*attr_itr).second;
    Tree* startnode = rsl.context_node_begin;
    // if tokenrange is within (actual subrange) context_node_begin, then start from context_node_begin;
    // otherwise, start searching from the parent of context_node_begin;
    if ( tokenrange.first <= startrange->first && tokenrange.second >= startrange->second )
      startnode = rsl.context_node_begin->parent;
    while ( startnode!=NULL ) {
      if ( isContextual(startnode) ) { // this condition is language-dependant
	rsl.context_node_begin = startnode;
	break;
      } else
	startnode = startnode->parent;
    }
    if ( startnode==NULL ) // at least, rsl.context_node_begin should be the "root":
      rsl.context_node_begin = pt->getRoot();
  } else
    rsl.context_node_begin = pt ? pt->getRoot() : NULL;

  // assert ( rsl.context_node_begin!=NULL && rsl.context_node_end==NULL );
  return rsl;
}

bool CloneClusters::
compareContext1(CloneContextT& context1, CloneContextT& context2)
{
  // pair-wise comparison of node kinds: better to use a loop instead (TODO)
  if ( context1.context_node_begin==NULL )
    if ( context2.context_node_begin!=NULL )
      return false;
    else if ( context1.context_node_end==NULL )
      if ( context2.context_node_end!=NULL )
	return false;
      else
	return true;
    else if ( context2.context_node_end==NULL )
      return false;
    else if ( context1.context_node_end->type != context2.context_node_end->type )
      return false;
    else
      return true;
  else if ( context2.context_node_begin==NULL )
    return false;
  else if ( context1.context_node_begin->type != context2.context_node_begin->type )
    return false;
  else if ( context1.context_node_end==NULL )
    if ( context2.context_node_end!=NULL )
      return false;
    else
      return true;
  else if ( context2.context_node_end==NULL )
    return false;
  else if ( context1.context_node_end->type != context2.context_node_end->type )
    return false;
  else
    return true;
}

bool CloneClusters::
compareCContext2(CloneContextT& context1, CloneContextT& context2)
{
  // only compare the context_node_begin;
  if ( context1.context_node_begin==NULL )
    if ( context2.context_node_begin!=NULL )
      return false;
    else
      return true;
  else if ( context2.context_node_begin==NULL )
    return false;
  else {			// actual comparison here:
    Tree* forcompare1 = context1.context_node_begin;
    Tree* forcompare2 = context2.context_node_begin;
    // because of non-clear structure in C's grammar file, we need the "if" hack:
    if ( forcompare1->type == getTypeID(name2id, "select_or_iter_stmt") )
      forcompare1 = forcompare1->children[0];
    if ( forcompare2->type == getTypeID(name2id, "select_or_iter_stmt") )
      forcompare2 = forcompare2->children[0];
    assert ( forcompare1!=NULL && forcompare2!=NULL );
    // may be better to let "for"=="while", "if"=="switch"
    if ( forcompare1->type == getTypeID(name2id, "simple_if") ||
	 forcompare1->type == getTypeID(name2id, "SWITCH") )
      if ( forcompare2->type == getTypeID(name2id, "simple_if") ||
	   forcompare2->type == getTypeID(name2id, "SWITCH") )
	return true;
      else
	return false;
    else if ( forcompare1->type == getTypeID(name2id, "WHILE") ||
	      forcompare1->type == getTypeID(name2id, "FOR") )
      if ( forcompare2->type == getTypeID(name2id, "WHILE") ||
	   forcompare2->type == getTypeID(name2id, "FOR") )
	return true;
      else
	return false;
    else if ( forcompare1->type == forcompare2->type )
      return true;
    else
      return false;
  }
}

bool CloneClusters::
compareCConditions(CloneContextT& context1, CloneContextT& context2)
{
  // only compare the context_node_begin;
  if ( context1.context_node_begin==NULL )
    if ( context2.context_node_begin!=NULL )
      return false;
    else
      return true;
  else if ( context2.context_node_begin==NULL )
    return false;
  else {                        // actual condition comparison here:
    Tree* forcompare1 = context1.context_node_begin;
    Tree* forcompare2 = context2.context_node_begin;
    // the following relies on C's grammar file and ccontextualNodes.h:
#define look_for_C_condition(node) {				   \
      if ( node->type == getTypeID(name2id, "select_or_iter_stmt") )	\
	if ( node->children[0]->type == getTypeID(name2id, "simple_if") ) /* simple_if : if_prefix : IF '(' expr */ \
	  node = node->children[0]->children[0]->children[2];		\
	else if ( node->children[0]->type == getTypeID(name2id, "WHILE") )	/* WHILE '(' expr */ \
	  node = node->children[2];					\
	else if ( node->children[0]->type == getTypeID(name2id, "do_stmt_start") ) \
	  if ( node->children.size() >= 3 )				\
	    node = node->children[2];					\
	  else /* parsing error */					\
	    node = NULL;						\
	else if ( node->children[0]->type == getTypeID(name2id, "FOR") ) \
	  if ( node->children[3]->children.size() <= 0 )		\
	    node = NULL;						\
	  else								\
	    node = node->children[3]->children[0];			\
	else if ( node->children[0]->type == getTypeID(name2id, "SWITCH") ) \
	  node = node->children[2];					\
	else /* shouldn't happen for the C grammar */			\
	  node = NULL;							\
      else if ( node->type == getTypeID(name2id, "simple_if") )		\
	node = node->children[0]->children[2];				\
      else if ( node->type == getTypeID(name2id, "do_stmt_start") )	\
	if ( node->parent->children.size() >= 3 )			\
	  node = node->parent->children[2];				\
	else /* parsing error */					\
	  node = NULL;							\
      else if ( node->type == getTypeID(name2id, "WHILE") )		\
	node = node->parent->children[2];				\
      else if ( node->type == getTypeID(name2id, "FOR") )		\
	if ( node->parent->children[3]->children.size() <= 0 )		\
	  node = NULL;							\
	else								\
	  node = node->parent->children[3]->children[0];		\
      else if ( node->type == getTypeID(name2id, "SWITCH") )		\
        node = node->parent->children[2];				\
      else								\
	node = NULL;							\
    }

    look_for_C_condition(forcompare1);
    look_for_C_condition(forcompare2);
    return compareTree(forcompare1, forcompare2); // exact match is not good: an extra pair of () may change the result.
  }
}

int CloneClusters::
rank1(CloneClusterT & cls)
{
  // group the clone points based on context node kind;
  vector<PClonePointT> groups;
  GetContextFuncT getContext = &CloneClusters::getContext2;
  CompareContextFuncT compareContext = &CloneClusters::compareCContext2;
  // when calling a member function through a
  // pointer-to-member-function, we can't use (*ptrMemFun) directly
  // because the address the pointer points to may only be an offset
  // in an ACTUAL object. So, we must use (obj.(*ptrMemFun)) instead.
  // Better to use "functionoid".

  for (int i = 0; i < cls.cluster->size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      CloneContextT context1 = ((*this).*getContext)(pair<long, long>(groups[j]->tbid, groups[j]->teid), groups[j]->filename);
      CloneContextT context2 = (this->*getContext)(pair<long, long>((*(cls.cluster))[i]->tbid, (*(cls.cluster))[i]->teid), (*(cls.cluster))[i]->filename);
      if ( (this->*compareContext)(context1, context2) ) {
	// same context, so no need to store it:
	break;
      }
    }
    if ( j >= tmps ) {
      // a different context:
      groups.push_back((*(cls.cluster))[i]);
    }
  }

  if ( groups.size() > 1 )
    return ENUM_RANK_CXT_NODE;
  else
    return ENUM_RANK_NOTHING;
}

int CloneClusters::
rank2(CloneClusterT & cls)
{
  // group the clone points based on contexts AND conditions;
  // if #groups<=1 AND #nVARs are different, filter it;
  vector<PClonePointT> groups;
  GetContextFuncT getContext = &CloneClusters::getContext2;
  CompareContextFuncT compareCondition = &CloneClusters::compareCConditions;

  for (int i = 0; i < cls.cluster->size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      CloneContextT context1 = ((*this).*getContext)(pair<long, long>(groups[j]->tbid, groups[j]->teid), groups[j]->filename);
      CloneContextT context2 = (this->*getContext)(pair<long, long>((*(cls.cluster))[i]->tbid, (*(cls.cluster))[i]->teid), (*(cls.cluster))[i]->filename);
      if ( (this->*compareCondition)(context1, context2) ) { // compare contextual conditions.
	// same condition:
        break;
      }
    }
    if ( j >= tmps ) {
      groups.push_back((*(cls.cluster))[i]);
    }
  }

  if ( groups.size() > 1 ) {
    return ENUM_RANK_CXT_COND;
  } else
    return ENUM_RANK_NOTHING;
}

int CloneClusters::
rank3(CloneClusterT & cls)
{
  // group the clone points based on nVARs:
  vector<PClonePointT> groups;

  for (int i = 0; i < cls.cluster->size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      if ( groups[j]->uni_var_number == (*(cls.cluster))[i]->uni_var_number ) {
	// same nVARs:
        break;
      }
    }
    if ( j >= tmps ) {
      groups.push_back((*(cls.cluster))[i]);
    }
  }

  if ( groups.size() > 1 ) {
    return ENUM_RANK_nVARS;
  } else
    return ENUM_RANK_NOTHING;
}

int CloneClusters::
buggy1(CloneClusterT & cls)
{
  // filtering heuristic (see: notes.txt)
  int hasCond = 0, hasLoop = 0, hasNone = 0;
  GetContextFuncT getContext = &CloneClusters::getContext2;

  for (int i = 0; i < cls.cluster->size(); i++) {
    CloneContextT context2 = (this->*getContext)(pair<long, long>((*(cls.cluster))[i]->tbid, (*(cls.cluster))[i]->teid), (*(cls.cluster))[i]->filename);
    Tree* forcompare2 = context2.context_node_begin;
    if ( forcompare2->type == getTypeID(name2id, "select_or_iter_stmt") )
      forcompare2 = forcompare2->children[0];
    if ( forcompare2->type == getTypeID(name2id, "simple_if") ||
         forcompare2->type == getTypeID(name2id, "SWITCH") )
      hasCond++;
    else if ( forcompare2->type == getTypeID(name2id, "WHILE") ||
              forcompare2->type == getTypeID(name2id, "FOR") ||
	      forcompare2->type == getTypeID(name2id, "do_stmt_start") )
      hasLoop++;
    else
      hasNone++;
  }

  if ( hasCond>0 && hasLoop==0 && hasNone==0 )
    return 4;
  else if ( hasCond>0 && hasLoop==0 && hasNone>0 )
    return 6;
  else if ( hasCond>0 && hasLoop>0 && hasNone==0 )
    return 1;
  else if ( hasCond>0 && hasLoop>0 && hasNone>0 )
    return 0;
  else if ( hasCond==0 && hasLoop>0 && hasNone>0 )
    return -4;
  else if ( hasCond==0 && hasLoop>0 && hasNone==0 )
    return -2;
  else
    return -8;
}


/*****************************
 * CloneClusters For Java
 *
 ****************************/
bool CloneClusters_Java::
compareJContext2(CloneContextT & context1, CloneContextT & context2)
{
  // only compare the context_node_begin;
  if ( context1.context_node_begin==NULL )
    if ( context2.context_node_begin!=NULL )
      return false;
    else
      return true;
  else if ( context2.context_node_begin==NULL )
    return false;
  else {                        // actual comparison here:
    Tree* forcompare1 = context1.context_node_begin;
    Tree* forcompare2 = context2.context_node_begin;
    // treat "for"=="while", "if"=="switch"
    // because of the particular grammar for Java, we need the ad-hoc hack:
    if ( forcompare1->type == getTypeID(name2id, "if_then_statement") ||
	 forcompare1->type == getTypeID(name2id, "if_then_else_statement") ||
	 forcompare1->type == getTypeID(name2id, "if_then_else_statement_nsi") ||
	 forcompare1->type == getTypeID(name2id, "switch_statement") )
      if ( forcompare2->type == getTypeID(name2id, "if_then_statement") ||
	   forcompare2->type == getTypeID(name2id, "if_then_else_statement") ||
	   forcompare2->type == getTypeID(name2id, "if_then_else_statement_nsi") ||
	   forcompare2->type == getTypeID(name2id, "switch_statement") )
	return true;
      else
	return false;
    else if ( forcompare1->type == getTypeID(name2id, "for_statement") ||
	      forcompare1->type == getTypeID(name2id, "for_statement_nsi") ||
	      forcompare1->type == getTypeID(name2id, "while_statement") ||
	      forcompare1->type == getTypeID(name2id, "while_statement_nsi") )
      if ( forcompare2->type == getTypeID(name2id, "for_statement") ||
	   forcompare2->type == getTypeID(name2id, "for_statement_nsi") ||
	   forcompare2->type == getTypeID(name2id, "while_statement") ||
	   forcompare2->type == getTypeID(name2id, "while_statement_nsi") )
	return true;
      else
	return false;
    else if ( forcompare1->type == getTypeID(name2id, "catch_clause") ||
	      forcompare1->type == getTypeID(name2id, "finally") )
      if ( forcompare2->type == getTypeID(name2id, "catch_clause") ||
	   forcompare2->type == getTypeID(name2id, "finally") )
	return true;
      else
	return false;
    else if ( forcompare1->type == forcompare2->type )
      return true;
    else
      return false;
  }
}

bool CloneClusters_Java::
compareJConditions(CloneContextT& context1, CloneContextT& context2)
{
  // only compare the context_node_begin;
  if ( context1.context_node_begin==NULL )
    if ( context2.context_node_begin!=NULL )
      return false;
    else
      return true;
  else if ( context2.context_node_begin==NULL )
    return false;
  else {                        // actual condition comparison here:
    Tree* forcompare1 = context1.context_node_begin;
    Tree* forcompare2 = context2.context_node_begin;
    // the following relies on the particular grammar for Java and jcontextualNodes.h:
#define look_for_J_condition(node) {					\
      if ( node->type == getTypeID(name2id, "if_then_statement") ||	\
	   node->type == getTypeID(name2id, "if_then_else_statement") || \
	   node->type == getTypeID(name2id, "if_then_else_statement_nsi") ) \
	if ( node->children.size() >=4 )				\
	  node = node->children[2];					\
	else								\
	  node = NULL;							\
      else if ( node->type == getTypeID(name2id, "while_statement") ||	\
		node->type == getTypeID(name2id, "while_statement_nsi") ) \
	if ( node->children[0]->type == getTypeID(name2id, "while_expression") ) \
	  node = node->children[0]->children[2];			\
	else if ( node->children.size() >=4 )				\
	  node = node->children[2];					\
	else								\
	  node = NULL;							\
      else if ( node->type == getTypeID(name2id, "for_statement") ||	\
		node->type == getTypeID(name2id, "for_statement_nsi") ) \
	if ( node->children.size() == 7 ||				\
	     node->children.size() == 5 )				\
	  node = node->children[2];					\
	else								\
	  node = NULL;							\
      else if ( node->type == getTypeID(name2id, "switch_statement") )	\
	if ( node->children[0]->children.size() >=4 )			\
	  node = node->children[0]->children[2];			\
	else								\
	  node = NULL;							\
      else if ( node->type == getTypeID(name2id, "do_statement") )	\
	node = node->children[4];					\
      else if ( node->type == getTypeID(name2id, "synchronized_statement") ) \
	if ( node->children.size() >=5 )				\
	  node = node->children[2];					\
	else								\
	  node = NULL;							\
      else if ( node->type == getTypeID(name2id, "throw_statement") )	\
	if ( node->children.size() >=3 )				\
	  node = node->children[1];					\
	else								\
	  node = NULL;							\
      else if ( node->type == getTypeID(name2id, "catch_clause") )	\
	if ( node->children[0]->children.size() ==4 &&			\
	     node->children[0]->children[2]->type == getTypeID(name2id, "formal_parameter") ) \
	  node = node->children[0]->children[2];				\
	else								\
	  node = NULL;							\
      else								\
        node = NULL;							\
    }

    look_for_J_condition(forcompare1);
    look_for_J_condition(forcompare2);

    return compareTree(forcompare1, forcompare2); // exact match is not good: an extra pair of () may change the result.
  }
}

int CloneClusters_Java::
rank1(CloneClusterT & cls)
{
  vector<PClonePointT> groups;
  GetContextFuncT getContext = &CloneClusters::getContext2;
  CompareContextFuncT compareContext = &CloneClusters_Java::compareJContext2;

  for (int i = 0; i < cls.cluster->size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      CloneContextT context1 = ((*this).*getContext)(pair<long, long>(groups[j]->tbid, groups[j]->teid), groups[j]->filename);
      CloneContextT context2 = (this->*getContext)(pair<long, long>((*(cls.cluster))[i]->tbid, (*(cls.cluster))[i]->teid), (*(cls.cluster))[i]->filename);
      if ( (this->*compareContext)(context1, context2) ) {
	// same context, so no need to store it:
	break;
      }
    }
    if ( j >= tmps ) {
      // a different context:
      groups.push_back((*(cls.cluster))[i]);
    }
  }

  if ( groups.size() > 1 )
    return ENUM_RANK_CXT_NODE;
  else
    return ENUM_RANK_NOTHING;
}

int CloneClusters_Java::
rank2(CloneClusterT & cls)
{
  vector<PClonePointT> groups;
  GetContextFuncT getContext = &CloneClusters::getContext2;
  CompareContextFuncT compareCondition = &CloneClusters_Java::compareJConditions;

  for (int i = 0; i < cls.cluster->size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      CloneContextT context1 = ((*this).*getContext)(pair<long, long>(groups[j]->tbid, groups[j]->teid), groups[j]->filename);
      CloneContextT context2 = (this->*getContext)(pair<long, long>((*(cls.cluster))[i]->tbid, (*(cls.cluster))[i]->teid), (*(cls.cluster))[i]->filename);
      if ( (this->*compareCondition)(context1, context2) ) { // compare conditions.
	// same conditions:
        break;
      }
    }
    if ( j >= tmps ) {
      groups.push_back((*(cls.cluster))[i]);
    }
  }

  if ( groups.size() > 1 ) {
    return ENUM_RANK_CXT_COND;
  } else
    return ENUM_RANK_NOTHING;
}

int CloneClusters_Java::
buggy1(CloneClusterT & cls)
{
  int hasCond = 0, hasLoop = 0, hasNone = 0, hasSync = 0, hasTry = 0;
  GetContextFuncT getContext = &CloneClusters::getContext2;

  for (int i = 0; i < cls.cluster->size(); i++) {
    CloneContextT context2 = (this->*getContext)(pair<long, long>((*(cls.cluster))[i]->tbid, (*(cls.cluster))[i]->teid), (*(cls.cluster))[i]->filename);
    Tree* forcompare2 = context2.context_node_begin;
    if ( forcompare2->type == getTypeID(name2id, "if_then_statement") ||
         forcompare2->type == getTypeID(name2id, "if_then_else_statement") ||
         forcompare2->type == getTypeID(name2id, "if_then_else_statement_nsi") ||
         forcompare2->type == getTypeID(name2id, "switch_statement") )
      hasCond++;
    else if ( forcompare2->type == getTypeID(name2id, "for_statement") ||
              forcompare2->type == getTypeID(name2id, "for_statement_nsi") ||
              forcompare2->type == getTypeID(name2id, "while_statement") ||
              forcompare2->type == getTypeID(name2id, "while_statement_nsi") ||
	      forcompare2->type == getTypeID(name2id, "do_statement") )
      hasLoop++;
    else if ( forcompare2->type == getTypeID(name2id, "synchronized_statement") )
      hasSync++;
    else if ( forcompare2->type == getTypeID(name2id, "throw_statement") ||
	      forcompare2->type == getTypeID(name2id, "try_statement") ||
	      forcompare2->type == getTypeID(name2id, "catch_clause") ||
	      forcompare2->type == getTypeID(name2id, "finally") )
      hasTry++;
    else
      hasNone++;
  }

  if ( hasSync!=0 && hasSync!=cls.cluster->size() )
    return 3;
  else if ( hasTry!=0 && hasTry!=cls.cluster->size() )
    return 2;
  else if ( hasCond>0 && hasLoop==0 && hasNone==0 )
    return 4;
  else if ( hasCond>0 && hasLoop==0 && hasNone>0 )
    return 6;
  else if ( hasCond>0 && hasLoop>0 && hasNone==0 )
    return 1;
  else if ( hasCond>0 && hasLoop>0 && hasNone>0 )
    return 0;
  else if ( hasCond==0 && hasLoop>0 && hasNone>0 )
    return -4;
  else if ( hasCond==0 && hasLoop>0 && hasNone==0 )
    return -2;
  else
    return -8;
}
