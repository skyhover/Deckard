/*
 * 
 * Copyright (c) 2007-2012,
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
#include "vgen-config.h"

using namespace std;
extern string identifierTypeName;

#define VGDEBUG

/******************************************
 * Implementation for TraGenConfiguration
 *
 *****************************************/
TraGenConfiguration::
TraGenConfiguration(ParseTree* rt)
  : parse_tree(rt), cfgFile(NULL), nodekinds(rt->typeCount()),
    countedNodes(nodekinds, false), outputtedNodes(nodekinds, false),
    atomicNodes(nodekinds, false), mergeableNodes(nodekinds, false)
		   // NOTE: the order of initialization is not guaranteed, but it doesn't matter here.
{
  // default parameters:
  mergeTokens = 30;
  moveStride = 1;
  mergeLists = 3;

  init();
}

TraGenConfiguration::
TraGenConfiguration(ParseTree* rt, int tokens, int strides, int lists)
  : parse_tree(rt), cfgFile(NULL), nodekinds(rt->typeCount()),
    countedNodes(nodekinds, false), outputtedNodes(nodekinds, false),
    atomicNodes(nodekinds, false), mergeableNodes(nodekinds, false)
{
  mergeTokens = tokens;
  moveStride = strides;
  mergeLists = lists;

  init();
}

TraGenConfiguration::
TraGenConfiguration(const char *fn)
{
  // TODO: use syntax similar to C/C++ to ease parsing. Ghassan's trick (but still requires recompilation) ;-)
}

void TraGenConfiguration::
init()
{
#ifdef VGDEBUG
  fprintf(stderr, "typeCount after init() = %d\n", parse_tree->typeCount());
#endif

  // assert rt->relevantNodes[i] is in [0, nodekinds)
  for ( int i=0; i<parse_tree->relevantNodes.size(); i++ ) {
    countedNodes[ parse_tree->relevantNodes[i] ] = true;
#if 0
    cerr << "RelevantNodes====" << parse_tree->relevantNodes[i] << ":" << countedNodes[ parse_tree->relevantNodes[i] ] << endl;
#endif
  }
  for ( int i=0; i<parse_tree->leafNodes.size(); i++ )
    atomicNodes[ parse_tree->leafNodes[i] ] = true;
  for ( int i=0; i<parse_tree->validParents.size(); i++ )
    outputtedNodes[ parse_tree->validParents[i] ] = true;
  for ( int i=0; i<parse_tree->mergeableNodes.size(); i++ )
    mergeableNodes[ parse_tree->mergeableNodes[i] ] = true;

  /***** this is grammar-specific *****/
  identid = parse_tree->getTypeID(identifierTypeName);
}

bool TraGenConfiguration::
isSkippable(Tree* astNode)
{
  assert( astNode->type>=0 && astNode->type<nodekinds );

  if ( countedNodes[astNode->type]==true )
    return false;
  else
    return true;
}

bool TraGenConfiguration::
isOutputtable(Tree* astNode)
{
  assert( astNode->type>=0 && astNode->type<nodekinds );

  if ( outputtedNodes[astNode->type]==true )
    return true;
  else
    return false;
}

bool TraGenConfiguration::
isAtomic(Tree* astNode)
{
  assert( astNode->type>=0 && astNode->type<nodekinds );

  if ( atomicNodes[astNode->type]==true )
    return true;
  else
    return false;
}

bool TraGenConfiguration::
isMergeable(Tree* node)
{
  assert( node->type>=0 && node->type<nodekinds );

  if ( mergeableNodes[node->type]==true )
    return true;
  else
    return false;
}

bool TraGenConfiguration::
increaseVecCounters(Tree* n, TreeVector* tv)
{
  if ( n->type<0 || n->type>=tv->counters.size() )
    return false;

  // increase node counts:
  tv->counters[n->type] += 1;

  // update token range: 
  map<NodeAttributeName_t, void*>::iterator attr_itr = n->attributes.find(NODE_TOKEN_ID);
  assert( attr_itr!=n->attributes.end() );
  if ( tv->token_begin_id<0 || ( ((pair<long, long>*)(*attr_itr).second)->first>=0 && ((pair<long, long>*)(*attr_itr).second)->first<tv->token_begin_id ) )
    tv->token_begin_id = ((pair<long, long>*)(*attr_itr).second)->first;
  if ( tv->token_end_id<0 || ((pair<long, long>*)(*attr_itr).second)->second>tv->token_end_id )
    tv->token_end_id = ((pair<long, long>*)(*attr_itr).second)->second;

  if ( n->isTerminal()==true ) {
    // update line counts:
    Terminal* tn = n->toTerminal();
//    tv->lines.insert(tn->line);
    if ( tv->minLine<=0 || (tn->line>0 && tn->line<tv->minLine) )
      tv->minLine = tn->line;
    if ( tv->maxLine<=0 || tn->line>tv->maxLine )
      tv->maxLine = tn->line;
#ifdef outputcountedlines
    fprintf(stderr, "A line ---%d, id==%s\n", tn->line, n->type==identid? tn->value->c_str() : "NULL");
#endif
  }

  if ( n->type==identid ) {
    // TODO: the condition may be not enough coz we may want to
    // consider recording '+', '-' to help locating bugs.
    // assert( n->isTerminal() );
    Terminal* tn = n->toTerminal();
    map<string, int>::iterator id = tv->name_counters.find(*(tn->value));
    if ( id==tv->name_counters.end() )
      tv->name_counters[*(tn->value)] = 1;
    else
      (*id).second += 1;
    tv->ordered_names.push_back(tn->value);
  }

  return true;
}

