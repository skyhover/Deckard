#include "clone-context-php.h"

/**************************************************
 * Implementation of ContextInconsistency_PHP
 *
 *************************************************/

CloneContextT ContextInconsistency_PHP::
getContext2(std::pair<long, long> tokenrange, std::string & fn)
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
  if ( rsl.context_node_begin!=NULL ) {
    // if tokenrange is within (actual subrange) context_node_begin, then start from context_node_begin;
    // otherwise, start searching from the parent of context_node_begin;
    map<NodeAttributeName_t, void*>::iterator attr_itr = rsl.context_node_begin->attributes.find(NODE_TOKEN_ID);
    assert ( attr_itr!=rsl.context_node_begin->attributes.end() );
    pair<long, long>* startrange = (pair<long, long>*)(*attr_itr).second;
    Tree* startnode = rsl.context_node_begin;
    if ( tokenrange.first <= startrange->first && tokenrange.second >= startrange->second ) {
      startnode = rsl.context_node_begin->parent; // TODO: this may still not guarantee *proper* enclosure
    }
  filter_unticked_stmt:
    while ( startnode!=NULL ) {
      if ( isContextual(startnode) ) { // this condition is language-dependant
	rsl.context_node_begin = startnode;
	break;
      } else
	startnode = startnode->parent;
    }
    if ( startnode!=NULL && startnode->type == getTypeID(name2id, "unticked_statement")
	 && startnode->children[0]->type != getTypeID(name2id, "T_IF")
	 && startnode->children[0]->type != getTypeID(name2id, "T_WHILE")
	 && startnode->children[0]->type != getTypeID(name2id, "T_DO")
	 && startnode->children[0]->type != getTypeID(name2id, "T_FOR")
	 && startnode->children[0]->type != getTypeID(name2id, "T_SWITCH")
	 && startnode->children[0]->type != getTypeID(name2id, "T_FOREACH")
	 && startnode->children[0]->type != getTypeID(name2id, "T_TRY") ) {
      startnode = startnode->parent;
      goto filter_unticked_stmt;
    }
    if ( startnode==NULL ) // at least, rsl.context_node_begin should be the "root":
      rsl.context_node_begin = pt->getRoot();

  } else
    rsl.context_node_begin = pt ? pt->getRoot() : NULL;

  // assert ( rsl.context_node_begin!=NULL && rsl.context_node_end==NULL );
  return rsl;
}

Tree* ContextInconsistency_PHP::
getContextNode(Tree* node)
{
  return node;
}

Tree* ContextInconsistency_PHP::
getContextParent(Tree* node)
{
  if ( node == NULL )
    return NULL;
  Tree * rsl = getContextNode(node);
  rsl = rsl->parent;
 filter_unticked_stmt:
  while ( rsl!=NULL ) {
    if ( isContextual(rsl) ) { // this condition is language-dependant
      break;
    } else
      rsl = rsl->parent;
  }
  if ( rsl!=NULL && rsl->type == getTypeID(name2id, "unticked_statement")
       && rsl->children[0]->type != getTypeID(name2id, "T_IF")
       && rsl->children[0]->type != getTypeID(name2id, "T_WHILE")
       && rsl->children[0]->type != getTypeID(name2id, "T_DO")
       && rsl->children[0]->type != getTypeID(name2id, "T_FOR")
       && rsl->children[0]->type != getTypeID(name2id, "T_SWITCH")
       && rsl->children[0]->type != getTypeID(name2id, "T_FOREACH")
       && rsl->children[0]->type != getTypeID(name2id, "T_TRY") ) {
    rsl = rsl->parent;
    goto filter_unticked_stmt;
  }
  return rsl;
}

bool ContextInconsistency_PHP::
comparePHPContext2(CloneContextT & context1, CloneContextT & context2)
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
    // handle "unticked_statement":
    if ( forcompare1->type == getTypeID(name2id, "unticked_statement") )
      forcompare1 = forcompare1->children[0];
    if ( forcompare2->type == getTypeID(name2id, "unticked_statement") )
      forcompare2 = forcompare2->children[0];
    // treat "for"=="while", "if"=="switch"; it's grammar-specific:
    if ( forcompare1->type == getTypeID(name2id, "elseif_list") ||
	 forcompare1->type == getTypeID(name2id, "new_elseif_list") ||
	 forcompare1->type == getTypeID(name2id, "T_IF") ||
	 forcompare1->type == getTypeID(name2id, "T_SWITCH") )
      if ( forcompare2->type == getTypeID(name2id, "elseif_list") ||
	   forcompare2->type == getTypeID(name2id, "new_elseif_list") ||
	   forcompare2->type == getTypeID(name2id, "T_IF") ||
	   forcompare2->type == getTypeID(name2id, "T_SWITCH") )
	return true;
      else
	return false;
    else if ( forcompare1->type == getTypeID(name2id, "T_WHILE") ||
	      forcompare1->type == getTypeID(name2id, "T_DO") ||
	      forcompare1->type == getTypeID(name2id, "T_FOR") )
      if ( forcompare2->type == getTypeID(name2id, "T_WHILE") ||
	   forcompare2->type == getTypeID(name2id, "T_DO") ||
	   forcompare2->type == getTypeID(name2id, "T_FOR") )
	return true;
      else
	return false;
    else if ( forcompare1->type == forcompare2->type )
      return true;
    else
      return false;
  }
}

Tree* ContextInconsistency_PHP::
get_condition_within(Tree* node)
{
  Tree* rsl = NULL;
  if ( node==NULL )
    return rsl;

  if ( node->type == getTypeID(name2id, "unticked_statement") )
    if ( node->children[0]->type == getTypeID(name2id, "T_IF") )
      rsl = node->children[2];
    else if ( node->children[0]->type == getTypeID(name2id, "T_WHILE") )
      rsl = node->children[2];
    else if ( node->children[0]->type == getTypeID(name2id, "T_DO") )
      rsl = node->children[4];
    else if ( node->children[0]->type == getTypeID(name2id, "T_FOR") ) {
      rsl = node->children[4];	// for_expr
      if ( rsl->children.size()>0 ) {
	rsl = rsl->children[0];
	if ( rsl->children.size()>2 )
	  rsl = rsl->children[2];
	else
	  rsl = rsl->children[0];
      } else
	rsl = NULL;
    } else if ( node->children[0]->type == getTypeID(name2id, "T_SWITCH") )
      rsl = node->children[2];
    else
      rsl = NULL;
  else if ( node->type == getTypeID(name2id, "elseif_list") ||
	    node->type == getTypeID(name2id, "new_elseif_list") )
    if ( node->children.size()>0 )
      rsl = node->children[3];
    else
      rsl = NULL;
  else
    rsl = NULL;

  return rsl;
}

bool ContextInconsistency_PHP::
comparePHPConditions(CloneContextT& context1, CloneContextT& context2)
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
    return compareTree(forcompare1, forcompare2); // exact match is not good: an extra pair of () may change the result.
  }
}

Tree* ContextInconsistency_PHP::
get_conditional_operator(Tree* node)
{
  Tree * rsl = NULL;
  if ( node == NULL )
    return rsl;
  if ( node->type == getTypeID(name2id, "expr")
       || node->type == getTypeID(name2id, "r_variable") )
    return get_conditional_operator(node->children[0]);
  else if ( node->type == getTypeID(name2id, "expr_without_variable") )
    if ( node->children[0]->type == getTypeID(name2id, "T_LIST") )
      return node->children[4];
    else if ( node->children[0]->type == getTypeID(name2id, "variable")
	      || node->children[0]->type == getTypeID(name2id, "rw_variable")
	      || node->children[0]->type == getTypeID(name2id, "expr") )
      return node->children[1];
    else if ( node->children[0]->type == getTypeID(name2id, "'('") )
      return get_conditional_operator(node->children[1]);
    else if ( node->children[0]->type == getTypeID(name2id, "internal_functions_in_yacc") )
      return node->children[0]->children[0];
    else if ( node->children[0]->type == getTypeID(name2id, "scalar") )
      return node->children[0];	// return the "scalar";
    else
      return node->children[0];
  else if ( node->type == getTypeID(name2id, "variable") )
    return node;
  else
    return node;
  return rsl;
}

bool ContextInconsistency_PHP::
isMainOperator(Tree * op1)
{
  if ( op1==NULL )
    return false;
  else
    return false;
}

bool ContextInconsistency_PHP::
filter1()
{
  vector<PClonePointT> groups;
  GetContextFuncT getContext = &ContextInconsistency_PHP::getContext2;
  CompareContextFuncT compareContext = &ContextInconsistency_PHP::comparePHPContext2;

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

bool ContextInconsistency_PHP::
filter2()
{
  vector<ClonePointGroupT> groups;
  GetContextFuncT getContext = &ContextInconsistency_PHP::getContext2;
  CompareContextFuncT compareCondition = &ContextInconsistency_PHP::comparePHPConditions;

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

int ContextInconsistency_PHP::
buggy1()
{
  int hasCond = 0, hasLoop = 0, hasNone = 0, hasSwitch = 0, hasTry = 0;
  int condInLoop = 0, ifInLevels = 0;
  GetContextFuncT getContext = &ContextInconsistency_PHP::getContext2;

  for (int i = 0; i < clusterbuffer.size(); i++) {
    CloneContextT context2 = (this->*getContext)(pair<long, long>(clusterbuffer[i].tbid, clusterbuffer[i].teid), clusterbuffer[i].filename);
    Tree* forcompare2 = context2.context_node_begin;
    if ( forcompare2 == NULL )
      continue;
    Tree* contextparent = getContextParent(forcompare2);

    if ( ( forcompare2->type == getTypeID(name2id, "unticked_statement") &&
	   ( forcompare2->children[0]->type == getTypeID(name2id, "T_IF") ||
	     forcompare2->children[0]->type == getTypeID(name2id, "T_SWITCH") ) ) ||
	 forcompare2->type == getTypeID(name2id, "elseif_list") ||
	 forcompare2->type == getTypeID(name2id, "new_elseif_list") ) {
      hasCond++;
      if ( forcompare2->type == getTypeID(name2id, "unticked_statement") && 
	   forcompare2->children[0]->type == getTypeID(name2id, "T_SWITCH") )
	hasSwitch++;		// hasSwitch <= hasCond
      else			// if_*
	if ( contextparent!=NULL && ( ( contextparent->type == getTypeID(name2id, "unticked_statement") &&
					( contextparent->children[0]->type == getTypeID(name2id, "T_IF") ||
					  contextparent->children[0]->type == getTypeID(name2id, "T_WHILE") ||
					  contextparent->children[0]->type == getTypeID(name2id, "T_DO") ||
					  contextparent->children[0]->type == getTypeID(name2id, "T_FOR") ||
					  contextparent->children[0]->type == getTypeID(name2id, "T_SWITCH") ||
					  contextparent->children[0]->type == getTypeID(name2id, "T_FOREACH") ||
					  contextparent->children[0]->type == getTypeID(name2id, "T_TRY") ) ) ||
				      contextparent->type == getTypeID(name2id, "elseif_list") ||
				      contextparent->type == getTypeID(name2id, "new_elseif_list") ) )
	  ifInLevels++;		// ifInLevels <= hasCond-hasSwitch
      if ( contextparent!=NULL && ( contextparent->type == getTypeID(name2id, "unticked_statement") &&
				    ( contextparent->children[0]->type == getTypeID(name2id, "T_WHILE") ||
				      contextparent->children[0]->type == getTypeID(name2id, "T_DO") ||
				      contextparent->children[0]->type == getTypeID(name2id, "T_FOR") ||
				      contextparent->children[0]->type == getTypeID(name2id, "T_FOREACH") ) ) )
	condInLoop++;
    } else if ( forcompare2->type == getTypeID(name2id, "unticked_statement") &&
		( forcompare2->children[0]->type == getTypeID(name2id, "T_WHILE") ||
		  forcompare2->children[0]->type == getTypeID(name2id, "T_DO") ||
		  forcompare2->children[0]->type == getTypeID(name2id, "T_FOR") ||
		  forcompare2->children[0]->type == getTypeID(name2id, "T_FOREACH") ) )
      hasLoop++;
    else if ( forcompare2->type == getTypeID(name2id, "additional_catch") ||
	      ( forcompare2->type == getTypeID(name2id, "unticked_statement") &&
		forcompare2->children[0]->type == getTypeID(name2id, "T_TRY") ) )
      hasTry++;
    else
      hasNone++;
  }

 setscores:
  if ( hasLoop>0 && hasCond>0 )
    buggy_score[3] = condInLoop;

  if ( hasTry!=0 && hasTry!=clusterbuffer.size() )
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
