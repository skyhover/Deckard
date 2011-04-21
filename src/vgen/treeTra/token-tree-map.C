#include "token-tree-map.h"

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

ostream & ClonePointT::
out2xml(ostream & os)
{
  os << "index=\"" << index << "\" ";
  os << "dist=\"" << /*setprecision(2) << */ dist << "\" ";
  os << "file=\"" << filename << "\" ";
  os << "lineno=\"" << begin_line_id << "\" ";
  os << "lineoffset=\"" << line_offset << "\" ";
  os << "nodekind=\"" << node_kind << "\" ";
  os << "nvars=\"" << uni_var_number << "\" ";
  os << "tbid=\"" << tbid << "\" ";
  os << "teid=\"" << teid << "\" ";

  return os;
}

ostream & operator<< (ostream& os, const struct _ClonePointT & cp)
{
  os << cp.linebuf;
  return os;
}

/******************************************
 * Implementation of TokenTreeMap
 *
 *****************************************/
regex_t TokenTreeMap::clone_patterns[ENUM_CLONE_THE_END];

bool TokenTreeMap::
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

TokenTreeMap::
TokenTreeMap() : fn2tree(), clusterbuffer(),
		 contexualNodes(id2name.size(), false)
{
  vecGen_config = new TraGenConfiguration((const char*)NULL); // no actual use; just a dummy
  token_range_counter = new TokenRangeCounter(*vecGen_config);
}

TokenTreeMap::
~TokenTreeMap()
{
  if ( token_range_counter!=NULL ) {
    delete token_range_counter;
    token_range_counter = NULL;
  }
  if ( vecGen_config!=NULL ) {
    delete vecGen_config;
    vecGen_config = NULL;
  }
}

ostream & TokenTreeMap::
outputCluster(ostream & out)
{
  out << "Rank score: " << rank << " * " << clusterbuffer.size() << " =" << rank*clusterbuffer.size()
      << "  buggy score: ";
  for (int i=0; i<NUM_BUGGY_SCORES; i++)
    out << buggy_score[i] << " ";
  out << endl;
  for (int i=0; i<clusterbuffer.size(); i++)
    out << clusterbuffer[i] << endl;
  return out;
}

bool TokenTreeMap::
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

ParseTree* TokenTreeMap::
parseFile(const char * fn)
{
  // NOTE: yyparse is NOT re-entrant, which caused weird bugs when I called yyparse for more than one files!
  yyin = fopen(fn, "r");
  if (!yyin) {
    cerr << "Can't open file: " << fn << endl;
  }
  yyrestart(yyin); // This is maybe unnecessary because FLEX's manual
		   // says this is equivalent to changing yyin directly
		   // when EOF is met. But it doesn't reset start conditions; this is
		   // why we need to use "yywrap" instead of exporting some internal
		   // values in FLEX.
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

bool TokenTreeMap::
parseClonePoint(char * line, PClonePointT pcp)
{
  return pcp->parse(line, clone_patterns);
}

bool TokenTreeMap::
createFN2Tree()			// read lines from stdin
{
  string line;
  int linecount = 0;

  clearBuffer();

  while ( !cin.eof() ) {
    getline(cin, line);  // get a line (read '\n' but not store it)
	if ( line.length()<=0 )  // no more content in the file right before seeing EOF
		break;
    ClonePointT temp;
    linecount++;
    char * charline = new char[line.length()+1];
    strcpy(charline, line.c_str());
    if ( parseClonePoint(charline, &temp) ) {
      clusterbuffer.push_back(temp);
    } else
      cerr << "Warning: error line " << linecount << ": " << line << endl;
    delete charline;
  }
  // set ranks:
  rank = 0;
  for ( int i=0; i<NUM_BUGGY_SCORES; i++ )
    buggy_score[i] = 0;
  rank |= filter1() ? ENUM_RANK_NOTHING : ENUM_RANK_CXT_NODE;
  rank |= filter2() ? ENUM_RANK_NOTHING : ENUM_RANK_CXT_COND;
  rank |= filter3() ? ENUM_RANK_NOTHING : ENUM_RANK_nVARS;
  buggy_score[0] = buggy1();
}

void TokenTreeMap::
clearMap()
{
  fn2tree.clear();
}

bool TokenTreeMap::
isContextual(Tree* node)
{
  assert( node->type >= 0 && node->type < id2name.size() );
  if ( contexualNodes[node->type] == true )
    return true;
  else
    return false;
}

void TokenTreeMap::
clearBuffer()
{
  clusterbuffer.clear();
}

Tree* TokenTreeMap::
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

Tree* TokenTreeMap::
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

Tree* TokenTreeMap::
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

bool TokenTreeMap::
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

list<Tree*>* TokenTreeMap::
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

CloneContextT TokenTreeMap::
getContext1(pair<long, long> tokenrange, string & fn)
{
  CloneContextT rsl;
  ParseTree* pt = NULL;
  map<string, ParseTree*>::iterator fn_itr = fn2tree.find(fn);
  if ( fn_itr == fn2tree.end() ) {
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

CloneContextT TokenTreeMap::
getContext2(pair<long, long> tokenrange, string & fn)
{
  CloneContextT rsl;
  ParseTree* pt = NULL;
  map<string, ParseTree*>::iterator fn_itr = fn2tree.find(fn);
  if ( fn_itr == fn2tree.end() ) {
    pt = parseFile(fn.c_str());
    if ( pt!=NULL ) {
      fn2tree.insert(pair<string, ParseTree*>(fn, pt));
    }
  } else
    pt = (*fn_itr).second;

  rsl.context_node_begin = rsl.context_node_end = NULL;
  rsl.context_node_begin = tokenRange2Tree2(tokenrange, pt);
  //  cerr << "getContext2 type: " << (rsl.context_node_begin ? getTypeName(id2name, rsl.context_node_begin->type) : "-1") << endl;
  if ( rsl.context_node_begin!=NULL ) {
    // if tokenrange is within (actual subrange) context_node_begin, then start from context_node_begin;
    // otherwise, start searching from the parent of context_node_begin;
    map<NodeAttributeName_t, void*>::iterator attr_itr = rsl.context_node_begin->attributes.find(NODE_TOKEN_ID);
    assert ( attr_itr!=rsl.context_node_begin->attributes.end() );
    pair<long, long>* startrange = (pair<long, long>*)(*attr_itr).second;
    Tree* startnode = rsl.context_node_begin;
    if ( tokenrange.first <= startrange->first && tokenrange.second >= startrange->second ) {
      startnode = rsl.context_node_begin->parent;
    }
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

Tree* TokenTreeMap::
getContextNode(Tree* node)
{
  if ( node == NULL )
    return NULL;
  Tree * rsl = node;
  if ( isContextual(rsl) &&
       ( rsl->type == getTypeID(name2id, "simple_if") ||
	 rsl->type == getTypeID(name2id, "do_stmt_start") ||
	 rsl->type == getTypeID(name2id, "WHILE") ||
	 rsl->type == getTypeID(name2id, "FOR") ||
	 rsl->type == getTypeID(name2id, "SWITCH") ) )
    rsl = rsl->parent;
  return rsl;			// assert rsl!=NULL
}

Tree* TokenTreeMap::
getContextParent(Tree* node)
{
  if ( node == NULL )
    return NULL;
  Tree * rsl = getContextNode(node);
  rsl = rsl->parent;
  while ( rsl!=NULL ) {
    if ( isContextual(rsl) ) { // this condition is language-dependant
      break;
    } else
      rsl = rsl->parent;
  }
  return rsl;
}

bool TokenTreeMap::
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

bool TokenTreeMap::
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

Tree* TokenTreeMap::
get_condition_within(Tree* node)
{
  Tree* rsl = NULL;
  if ( node==NULL )
    return rsl;
  // the following relies on C's grammar file and ccontextualNodes.h:
  if ( node->type == getTypeID(name2id, "select_or_iter_stmt") )
    if ( node->children[0]->type == getTypeID(name2id, "simple_if") ) /* simple_if : if_prefix : IF '(' expr */
      rsl = node->children[0]->children[0]->children[2];
    else if ( node->children[0]->type == getTypeID(name2id, "WHILE") )	/* WHILE '(' expr */
      rsl = node->children[2];
    else if ( node->children[0]->type == getTypeID(name2id, "do_stmt_start") )
      if ( node->children.size() >= 3 )
	rsl = node->children[2];
      else /* parsing error */
	rsl = NULL;
    else if ( node->children[0]->type == getTypeID(name2id, "FOR") )
      if ( node->children[3]->children.size() <= 0 )
	rsl = NULL;
      else
	rsl = node->children[3]->children[0];
    else if ( node->children[0]->type == getTypeID(name2id, "SWITCH") )
      rsl = node->children[2];
    else /* shouldn't happen for the C grammar */
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "simple_if") )
    rsl = node->children[0]->children[2];
  else if ( node->type == getTypeID(name2id, "do_stmt_start") )
    if ( node->parent->children.size() >= 3 )
      rsl = node->parent->children[2];
    else /* parsing error */
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "WHILE") )
    rsl = node->parent->children[2];
  else if ( node->type == getTypeID(name2id, "FOR") )
    if ( node->parent->children[3]->children.size() <= 0 )
      rsl = NULL;
    else
      rsl = node->parent->children[3]->children[0];
  else if ( node->type == getTypeID(name2id, "SWITCH") )
    rsl = node->parent->children[2];
  else
    rsl = NULL;

  return rsl;
}

bool TokenTreeMap::
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

    forcompare1 = get_condition_within(forcompare1);
    forcompare2 = get_condition_within(forcompare2);
    return compareTree(forcompare1, forcompare2); // exact match is not good, e.g., an extra pair of () may change the result.
  }
}

Tree* TokenTreeMap::
get_conditional_operator(Tree* node)
{
  // look for the main operator (for C only) in an expr:
  Tree * rsl = NULL;
  if ( node==NULL )
    return rsl;
  if ( node->type == getTypeID(name2id, "expr") )
    return get_conditional_operator(node->children[0]);
  else if ( node->type == getTypeID(name2id, "nonnull_exprlist") )
    if ( node->children.size()==3 ) // nonnull_exprlist ',' expr_no_commas
      return node->children[1];	// ','
    else
      return get_conditional_operator(node->children[0]); // expr_no_commas
  else if ( node->type == getTypeID(name2id, "expr_no_commas") )
    if ( node->children.size()==1 ) // cast_expr
      return get_conditional_operator(node->children[0]);
    else
      return node->children[1];
  else if ( node->type == getTypeID(name2id, "cast_expr") )
    if ( node->children.size()==1 ) // unary_expr
      return get_conditional_operator(node->children[0]);
    else
      return get_conditional_operator(node->children[3]);
  else if ( node->type == getTypeID(name2id, "unary_expr") )
    if ( node->children.size()==1 ) // primary
      return get_conditional_operator(node->children[0]);
    else if ( node->children[0]->type == getTypeID(name2id, "unop") )
      return node->children[0]->children[0];
    else if ( node->children[0]->type == getTypeID(name2id, "extension") )
      return get_conditional_operator(node->children[1]); // cast_expr
    else
      return node->children[0];
  else if ( node->type == getTypeID(name2id, "primary") )
    if ( node->children.size()>1 )
      if ( node->children[1]->type == getTypeID(name2id, "expr") )
	return get_conditional_operator(node->children[1]);
      else
	return node;		// return the primary itself
    else
      return node;		// return the primary itself
  else
    return node;		// return other exprs themselves
  return rsl;
}

bool TokenTreeMap::
isMainOperator(Tree * op1)
{
  if ( op1==NULL )
    return false;
  /* the definition is not so good ...
  else if ( op1->type == getTypeID(name2id, "!") ||
	    op1->type == getTypeID(name2id, "~") ||
	    op1->type == getTypeID(name2id, "-") ||
	    op1->type == getTypeID(name2id, "&") ||
	    op1->type == getTypeID(name2id, "PLUSPLUS") ||
	    op1->type == getTypeID(name2id, "MINUSMINUS") ||
	    op1->type == getTypeID(name2id, "+") ||
	    op1->type == getTypeID(name2id, "*") ||
	    op1->type == getTypeID(name2id, "/") ||
	    op1->type == getTypeID(name2id, "%") ||
	    op1->type == getTypeID(name2id, "LSHIFT") ||
	    op1->type == getTypeID(name2id, "RSHIFT") ||
	    op1->type == getTypeID(name2id, "ARITHCOMPARE") ||
	    op1->type == getTypeID(name2id, "EQCOMPARE") ||
	    op1->type == getTypeID(name2id, "|") ||
	    op1->type == getTypeID(name2id, "^") ||
	    op1->type == getTypeID(name2id, "ANDAND") ||
	    op1->type == getTypeID(name2id, "OROR") ||
	    op1->type == getTypeID(name2id, "?") ||
	    op1->type == getTypeID(name2id, "=") ||
	    op1->type == getTypeID(name2id, "ASSIGN") )
    return true;
  */
  else
    return false;
}

bool TokenTreeMap::
compareConditionalOperators(CloneContextT& context1, CloneContextT& context2)
{
  // only compare the context_node_begin;
  if ( context1.context_node_begin==NULL )
    if ( context2.context_node_begin!=NULL )
      return false;
    else
      return true;
  else if ( context2.context_node_begin==NULL )
    return false;
  else {
    Tree* forcompare1 = context1.context_node_begin;
    Tree* forcompare2 = context2.context_node_begin;

    forcompare1 = get_condition_within(forcompare1);
    forcompare2 = get_condition_within(forcompare2);
    Tree* op1 = get_conditional_operator(forcompare1);
    Tree* op2 = get_conditional_operator(forcompare2);
    /*
    cerr << "cond1: " << forcompare1->type << "=" << ( forcompare1==NULL ? "NULL" : getTypeName(id2name, forcompare1->type) ) << " "
	 << "cond2: " << forcompare2->type << "=" << ( forcompare2==NULL ? "NULL" : getTypeName(id2name, forcompare2->type) ) << endl
	 << "op1: " << op1->type << "=" << ( op1==NULL ? "NULL" : getTypeName(id2name, op1->type) ) << " "
	 << "op2: " << op2->type << "=" << ( op2==NULL ? "NULL" : getTypeName(id2name, op2->type) ) << endl;
    */
    if ( op1==NULL && op2==NULL )
      return true;
    else if ( op1==NULL || op2==NULL )
      return false;
    else if ( op1->type != op2->type )
      return false;
    else if ( isMainOperator(op1) ) /* language dependent...its definition is not so good */
      return compareTree(forcompare1, forcompare2);
    else
      return true;
  }
}

bool TokenTreeMap::
isAnyFiltered()
{
  bool rslflag = false;
  if ( clusterbuffer.size() < 1 ) {
    cerr << "No clone cluster is in the buffer..." << endl;
    rslflag = true;
  } else {
    rslflag = filter1();
    if ( rslflag == false )
      rslflag = filter2();
  }

  return rslflag;
}

bool TokenTreeMap::
isAllFiltered()
{
  bool rslflag = false;
  if ( clusterbuffer.size() < 1 ) {
    cerr << "No clone cluster is in the buffer..." << endl;
    rslflag = true;
  } else {
    rslflag = filter1();
    if ( rslflag == true )
      rslflag = filter2();
  }

  return rslflag;
}

bool TokenTreeMap::
filter1()
{
  // group the clone points; if #groups<=1, filter it;
  vector<ClonePointGroupT> groups;
  GetContextFuncT getContext = &TokenTreeMap::getContext2;
  CompareContextFuncT compareContext = &TokenTreeMap::compareCContext2;
  // when calling a member function through a
  // pointer-to-member-function, we can't use (*ptrMemFun) directly
  // because the address the pointer points to may only be an offset
  // in an ACTUAL object. So, we must use (obj.(*ptrMemFun)) instead.
  // Better to use "functionoid".

  for (int i = 0; i < clusterbuffer.size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      CloneContextT context1 = ((*this).*getContext)(pair<long, long>(groups[j][0]->tbid, groups[j][0]->teid), groups[j][0]->filename);
      CloneContextT context2 = (this->*getContext)(pair<long, long>(clusterbuffer[i].tbid, clusterbuffer[i].teid), clusterbuffer[i].filename);
      if ( (this->*compareContext)(context1, context2) ) {
	groups[j].push_back(&clusterbuffer[i]);
	break;
      }
    }
    if ( j >= tmps ) {
      groups.push_back(ClonePointGroupT());
      groups.back().push_back(&clusterbuffer[i]);
    }
  }

  if ( groups.size() <= 1 )
    return true; // ENUM_RANK_NOTHING
  else
    return false; // ENUM_RANK_CXT_NODE
}

bool TokenTreeMap::
filter2()
{
  // group the clone points based on contexts AND conditions;
  // if #groups<=1 AND #nVARs are different, filter it;
  vector<ClonePointGroupT> groups;
  GetContextFuncT getContext = &TokenTreeMap::getContext2;
  CompareContextFuncT compareCondition = &TokenTreeMap::compareConditionalOperators;

  for (int i = 0; i < clusterbuffer.size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      CloneContextT context1 = ((*this).*getContext)(pair<long, long>(groups[j][0]->tbid, groups[j][0]->teid), groups[j][0]->filename);
      CloneContextT context2 = (this->*getContext)(pair<long, long>(clusterbuffer[i].tbid, clusterbuffer[i].teid), clusterbuffer[i].filename);
      if ( (this->*compareCondition)(context1, context2) ) { // compare contextual conditions.
        groups[j].push_back(&clusterbuffer[i]);
        break;
      }
    }
    if ( j >= tmps ) {
      groups.push_back(ClonePointGroupT());
      groups.back().push_back(&clusterbuffer[i]);
    }
  }

  if ( groups.size() <= 1 ) {
    return true; // ENUM_RANK_NOTHING
  } else
    return false; // ENUM_RANK_CXT_COND
}

bool TokenTreeMap::
filter3()
{
  // filter based on nVARs and set some buggy_scores:
  int min_diff = 10000, max_distance = -10000;
  bool rslflag = true;		// ENUM_RANK_NOTHING
  for ( int i = 1; i < clusterbuffer.size(); i++ ) {
    if ( clusterbuffer[i].uni_var_number != clusterbuffer[0].uni_var_number ) {
      min_diff = min(min_diff, abs(clusterbuffer[i].uni_var_number - clusterbuffer[0].uni_var_number));
      rslflag = false;		// ENUM_RANK_nVARS
    }
    if ( clusterbuffer[i].filename == clusterbuffer[0].filename ) {
      int line_distance;
      if ( clusterbuffer[i].begin_line_id >= clusterbuffer[0].begin_line_id )
	line_distance = clusterbuffer[i].begin_line_id - (clusterbuffer[0].begin_line_id + clusterbuffer[0].line_offset -1);
      else
	line_distance = clusterbuffer[0].begin_line_id - (clusterbuffer[i].begin_line_id + clusterbuffer[i].line_offset -1);
      max_distance = max(max_distance, line_distance);
    } else
      max_distance = 10000;	// TODO: compare absolute filenames
  }
  if ( rslflag==true )
    buggy_score[1] = 0;
  else
    buggy_score[1] = min_diff;
  buggy_score[2] = max_distance;
  return rslflag;
}

int TokenTreeMap::
buggy1()
{
  // filtering heuristic (see: notes.txt)
  int hasCond = 0, hasLoop = 0, hasNone = 0, hasSwitch = 0;
  int condInLoop = 0, ifInLevels = 0;
  GetContextFuncT getContext = &TokenTreeMap::getContext2;

  for (int i = 0; i < clusterbuffer.size(); i++) {
    CloneContextT context2 = (this->*getContext)(pair<long, long>(clusterbuffer[i].tbid, clusterbuffer[i].teid), clusterbuffer[i].filename);
    Tree* forcompare2 = context2.context_node_begin;
    if ( forcompare2 == NULL )
      continue;
    Tree* contextparent = getContextParent(forcompare2);
    /*
    cerr << "Node: " << forcompare2->type << "=" << getTypeName(id2name, forcompare2->type) << " "
	 << "Parent: " << (contextparent==NULL ? -1 : contextparent->type)
	 << "=" << (contextparent==NULL ? "NULL" : getTypeName(id2name, contextparent->type)) << endl;
    */
    if ( forcompare2->type == getTypeID(name2id, "select_or_iter_stmt") )
      forcompare2 = forcompare2->children[0];
    if ( contextparent!=NULL && contextparent->type == getTypeID(name2id, "select_or_iter_stmt") )
      contextparent = contextparent->children[0];

    if ( forcompare2->type == getTypeID(name2id, "simple_if") ||
         forcompare2->type == getTypeID(name2id, "SWITCH") ) {
      hasCond++;
      if ( forcompare2->type == getTypeID(name2id, "SWITCH") )
	hasSwitch++;		// hasSwitch <= hasCond
      else			// simple_if
	if ( contextparent!=NULL &&
	     ( contextparent->type == getTypeID(name2id, "simple_if") ||
	       contextparent->type == getTypeID(name2id, "SWITCH") ||
	       contextparent->type == getTypeID(name2id, "WHILE") ||
	       contextparent->type == getTypeID(name2id, "FOR") ||
	       contextparent->type == getTypeID(name2id, "do_stmt_start") ) )
	  ifInLevels++;		// ifInLevels <= hasCond-hasSwitch
      if ( contextparent!=NULL &&
	   ( contextparent->type == getTypeID(name2id, "WHILE") ||
	     contextparent->type == getTypeID(name2id, "FOR") ||
	     contextparent->type == getTypeID(name2id, "do_stmt_start") ) )
	condInLoop++;
    } else if ( forcompare2->type == getTypeID(name2id, "WHILE") ||
		forcompare2->type == getTypeID(name2id, "FOR") ||
		forcompare2->type == getTypeID(name2id, "do_stmt_start") )
      hasLoop++;
    else
      hasNone++;
  }

 setscores:
  if ( hasLoop>0 && hasCond>0 )
    buggy_score[3] = condInLoop;

  if ( hasCond>0 && hasLoop==0 && hasNone==0 )
    return 4;
  else if ( hasCond>0 && hasLoop==0 && hasNone>0 )
    if ( hasSwitch==hasCond ||	// switch vs. none
	 ifInLevels==hasCond-hasSwitch // all "if"s are deep inside
	 )
      return -5;
    else
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
 * TokenTreeMap For Java
 *
 ****************************/
Tree* TokenTreeMap_Java::
getContextNode(Tree* node)
{
  if ( node == NULL )
    return NULL;
  Tree * rsl = node;
  if ( isContextual(rsl) )
    if ( rsl->type == getTypeID(name2id, "class_declaration")
	 || rsl->type == getTypeID(name2id, "constructor_declaration")
	 || rsl->type == getTypeID(name2id, "finally") )
      rsl = rsl->parent;
    else if ( rsl->type == getTypeID(name2id, "method_declaration")
	      || rsl->type == getTypeID(name2id, "interface_declaration") /* this case needs more care, but should be fine here */
	      || rsl->type == getTypeID(name2id, "abstract_method_declaration") )
      rsl = rsl->parent->parent;
    else if ( rsl->type == getTypeID(name2id, "interface_body") )
      rsl = rsl->parent->parent->parent;
    else if ( rsl->type == getTypeID(name2id, "catch_clause") )
      do {
	rsl = rsl->parent;
      } while ( rsl->type != getTypeID(name2id, "try_statement") );
  return rsl;
}

bool TokenTreeMap_Java::
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

Tree* TokenTreeMap_Java::
get_condition_within(Tree* node)
{
  Tree* rsl = NULL;
  if ( node==NULL )
    return rsl;

  if ( node->type == getTypeID(name2id, "if_then_statement") ||
       node->type == getTypeID(name2id, "if_then_else_statement") ||
       node->type == getTypeID(name2id, "if_then_else_statement_nsi") )
    if ( node->children.size() >=4 )
      rsl = node->children[2];
    else
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "while_statement") ||
	    node->type == getTypeID(name2id, "while_statement_nsi") )
    if ( node->children[0]->type == getTypeID(name2id, "while_expression") )
      rsl = node->children[0]->children[2];
    else if ( node->children.size() >=4 )
      rsl = node->children[2];
    else
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "for_statement") ||
	    node->type == getTypeID(name2id, "for_statement_nsi") )
    if ( node->children.size() == 7 || node->children.size() == 5 )
      rsl = node->children[2];
    else
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "switch_statement") )
    if ( node->children[0]->children.size() >=4 )
      rsl = node->children[0]->children[2];
    else
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "do_statement") )
    rsl = node->children[4];
  else if ( node->type == getTypeID(name2id, "synchronized_statement") )
    if ( node->children.size() >=5 )
      rsl = node->children[2];
    else
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "throw_statement") )
    if ( node->children.size() >=3 )
      rsl = node->children[1];
    else
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "catch_clause") )
    if ( node->children[0]->children.size() ==4 &&
	 node->children[0]->children[2]->type == getTypeID(name2id, "formal_parameter") )
      rsl = node->children[0]->children[2];
    else
      rsl = NULL;
  else
    rsl = NULL;

  return rsl;
}

bool TokenTreeMap_Java::
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

    forcompare1 = get_condition_within(forcompare1);
    forcompare2 = get_condition_within(forcompare2);
    return compareTree(forcompare1, forcompare2); // exact match is not good: an extra pair of () may change the result.
  }
}

Tree* TokenTreeMap_Java::
get_conditional_operator(Tree* node)
{
  // for Java only:
  Tree * rsl = NULL;
  if ( node == NULL )
    return rsl;
  if ( node->type == getTypeID(name2id, "expression")
       || node->type == getTypeID(name2id, "assignment_expression")
       || node->type == getTypeID(name2id, "postfix_expression")
       || node->type == getTypeID(name2id, "name")
       || node->type == getTypeID(name2id, "primary") )
    return get_conditional_operator(node->children[0]);
  else if ( node->type == getTypeID(name2id, "assignment") )
    return get_conditional_operator(node->children[1]);
  else if ( node->type == getTypeID(name2id, "assignment_operator") )
    if ( node->children[0]->type == getTypeID(name2id, "assign_any") )
      return node->children[0]->children[0];
    else
      return node->children[0];
  else if ( node->type == getTypeID(name2id, "conditional_expression")
	    || node->type == getTypeID(name2id, "conditional_or_expression")
	    || node->type == getTypeID(name2id, "conditional_and_expression")
	    || node->type == getTypeID(name2id, "inclusive_or_expression")
	    || node->type == getTypeID(name2id, "exclusive_or_expression")
	    || node->type == getTypeID(name2id, "and_expression")
	    || node->type == getTypeID(name2id, "equality_expression")
	    || node->type == getTypeID(name2id, "relational_expression")
	    || node->type == getTypeID(name2id, "shift_expression")
	    || node->type == getTypeID(name2id, "additive_expression")
	    || node->type == getTypeID(name2id, "multiplicative_expression") )
    if ( node->children.size() > 1 )
      return node->children[1];
    else
      return get_conditional_operator(node->children[0]);
  else if ( node->type == getTypeID(name2id, "unary_expression")
	    || node->type == getTypeID(name2id, "trap_overflow_corner_case")
	    || node->type == getTypeID(name2id, "unary_expression_not_plus_minus") )
    if ( node->children.size() > 1 )
      return node->children[0];
    else
      return get_conditional_operator(node->children[0]);
  else if ( node->type == getTypeID(name2id, "pre_increment_expression")
	    || node->type == getTypeID(name2id, "pre_decrement_expression") )
    return node->children[0];
  else if ( node->type == getTypeID(name2id, "post_increment_expression")
	    || node->type == getTypeID(name2id, "post_decrement_expression") )
    return node->children[1];
  else if ( node->type == getTypeID(name2id, "qualified_name") )
    return node->children[1];
  else if ( node->type == getTypeID(name2id, "formal_parameter")
	    || node->type == getTypeID(name2id, "error")
	    || node->type == getTypeID(name2id, "cast_expression")
	    || node->type == getTypeID(name2id, "simple_name") )
    return node;
  else if ( node->type == getTypeID(name2id, "primary_no_new_array") )
    if ( node->children[0]->type == getTypeID(name2id, "OP_TK") )
      return get_conditional_operator(node->children[1]);
    else if ( node->children.size() > 1 )
      return node->children[1];
    else if ( node->children[0]->type == getTypeID(name2id, "literal") )
      return node->children[0]->children[0];
    else if ( node->children[0]->type == getTypeID(name2id, "THIS_TK")
	      || node->children[0]->type == getTypeID(name2id, "class_instance_creation_expression")
	      || node->children[0]->type == getTypeID(name2id, "method_invocation")
	      || node->children[0]->type == getTypeID(name2id, "array_access") )
      return node->children[0];
    else if ( node->children[0]->type == getTypeID(name2id, "field_access")
	      || node->children[0]->type == getTypeID(name2id, "type_literals") )
      return node->children[0]->children[1];
    else
      return node;
  else if ( node->type == getTypeID(name2id, "array_creation_expression") )
    return node->children[0]->children[0];
  else
    return node;
  return rsl;
}

bool TokenTreeMap_Java::
isMainOperator(Tree * op1)
{
  if ( op1==NULL )
    return false;
  else
    return false;
}

bool TokenTreeMap_Java::
filter1()
{
  vector<PClonePointT> groups;
  GetContextFuncT getContext = &TokenTreeMap::getContext2;
  CompareContextFuncT compareContext = &TokenTreeMap_Java::compareJContext2;

  for (int i = 0; i < clusterbuffer.size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      CloneContextT context1 = ((*this).*getContext)(pair<long, long>(groups[j]->tbid, groups[j]->teid), groups[j]->filename);
      CloneContextT context2 = (this->*getContext)(pair<long, long>(clusterbuffer[i].tbid, clusterbuffer[i].teid), clusterbuffer[i].filename);
      if ( (this->*compareContext)(context1, context2) ) {
	// same context, so no need to store it:
	break;
      }
    }
    if ( j >= tmps ) {
      // a different context:
      groups.push_back(&clusterbuffer[i]);
    }
  }

  if ( groups.size() > 1 )
    return false; // ENUM_RANK_CXT_NODE;
  else
    return true; // ENUM_RANK_NOTHING;
}

bool TokenTreeMap_Java::
filter2()
{
  vector<ClonePointGroupT> groups;
  GetContextFuncT getContext = &TokenTreeMap::getContext2;
  CompareContextFuncT compareCondition = &TokenTreeMap_Java::compareConditionalOperators;

  for (int i = 0; i < clusterbuffer.size(); i++) {
    int j = 0;
    int tmps = groups.size();
    for (; j < tmps; j++) {
      CloneContextT context1 = ((*this).*getContext)(pair<long, long>(groups[j][0]->tbid, groups[j][0]->teid), groups[j][0]->filename);
      CloneContextT context2 = (this->*getContext)(pair<long, long>(clusterbuffer[i].tbid, clusterbuffer[i].teid), clusterbuffer[i].filename);
      if ( (this->*compareCondition)(context1, context2) ) {
        groups[j].push_back(&clusterbuffer[i]);
        break;
      }
    }
    if ( j >= tmps ) {
      groups.push_back(ClonePointGroupT());
      groups.back().push_back(&clusterbuffer[i]);
    }
  }

  if ( groups.size() <= 1 ) {
    return true; // ENUM_RANK_NOTHING
  } else
    return false; // ENUM_RANK_CXT_COND
}

int TokenTreeMap_Java::
buggy1()
{
  int hasCond = 0, hasLoop = 0, hasNone = 0, hasSwitch = 0, hasSync = 0, hasTry = 0;
  int condInLoop = 0, ifInLevels = 0;
  GetContextFuncT getContext = &TokenTreeMap::getContext2;

  for (int i = 0; i < clusterbuffer.size(); i++) {
    CloneContextT context2 = (this->*getContext)(pair<long, long>(clusterbuffer[i].tbid, clusterbuffer[i].teid), clusterbuffer[i].filename);
    Tree* forcompare2 = context2.context_node_begin;
    if ( forcompare2 == NULL )
      continue;
    Tree* contextparent = getContextParent(forcompare2);

    if ( forcompare2->type == getTypeID(name2id, "if_then_statement") ||
         forcompare2->type == getTypeID(name2id, "if_then_else_statement") ||
         forcompare2->type == getTypeID(name2id, "if_then_else_statement_nsi") ||
         forcompare2->type == getTypeID(name2id, "switch_statement") ) {
      hasCond++;
      if ( forcompare2->type == getTypeID(name2id, "switch_statement") )
	hasSwitch++;            // hasSwitch <= hasCond
      else                      // if_*
	if ( contextparent!=NULL &&
	     ( contextparent->type == getTypeID(name2id, "if_then_statement") ||
	       contextparent->type == getTypeID(name2id, "if_then_else_statement") ||
	       contextparent->type == getTypeID(name2id, "if_then_else_statement_nsi") ||
	       contextparent->type == getTypeID(name2id, "switch_statement") ||
	       contextparent->type == getTypeID(name2id, "for_statement") ||
	       contextparent->type == getTypeID(name2id, "for_statement_nsi") ||
	       contextparent->type == getTypeID(name2id, "while_statement") ||
	       contextparent->type == getTypeID(name2id, "while_statement_nsi") ||
	       contextparent->type == getTypeID(name2id, "do_statement") ) )
	  ifInLevels++;         // ifInLevels <= hasCond-hasSwitch
      if ( contextparent!=NULL &&
	   ( contextparent->type == getTypeID(name2id, "for_statement") ||
	     contextparent->type == getTypeID(name2id, "for_statement_nsi") ||
	     contextparent->type == getTypeID(name2id, "while_statement") ||
	     contextparent->type == getTypeID(name2id, "while_statement_nsi") ||
	     contextparent->type == getTypeID(name2id, "do_statement") ) )
	condInLoop++;
    } else if ( forcompare2->type == getTypeID(name2id, "for_statement") ||
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

 setscores:
  if ( hasLoop>0 && hasCond>0 )
    buggy_score[3] = condInLoop;

  if ( hasSync!=0 && hasSync!=clusterbuffer.size() )
    return 3;
  else if ( hasTry!=0 && hasTry!=clusterbuffer.size() )
    return 2;
  else if ( hasCond>0 && hasLoop==0 && hasNone==0 )
    return 4;
  else if ( hasCond>0 && hasLoop==0 && hasNone>0 )
    if ( hasSwitch==hasCond ||  // switch vs. none
	 ifInLevels==hasCond-hasSwitch // all "if"s are deep inside
	 )
      return -5;
    else
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

