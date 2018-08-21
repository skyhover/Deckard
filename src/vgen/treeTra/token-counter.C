/*
 * 
 * Copyright (c) 2007-2018, University of California / Singapore Management University
 *   Lingxiao Jiang         <lxjiang@ucdavis.edu> <lxjiang@smu.edu.sg>
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
#include "token-counter.h"

using namespace std;

#define VGDEBUG
extern ParseTree* global_tree_for_debugging;

/******************************************
 * Implementation for TokenCounter
 *
 *****************************************/
TokenCounter::
TokenCounter(TraGenConfiguration & cfg)
  : vecGen_config(cfg)
{
}

bool TokenCounter::
skipNode(Tree* astNode, Tree* inh)
{
  return false;
}

bool TokenCounter::
skipSubTree(Tree* astNode, Tree* inh)
{
  return false;
}

Tree* TokenCounter::
evaluateInheritedAttribute(Tree* astNode, Tree* inheritedValue)
{
  // done in Ghassan's code:
//   astNode->parent = inheritedValue;

  return astNode;
}

long TokenCounter::
evaluateSynthesizedAttribute(Tree* node, Tree* inh,
			     SynthesizedAttributesList& synl)
{
  if ( node->isTerminal()==true ) {
    node->terminal_number = 1;
#ifdef outputtokens
    fprintf(stdout, "token = %s\n", node->toTerminal()->value->c_str());
#endif
  } else {
    node->terminal_number = 0;
    for (SynthesizedAttributesList::iterator sa_itr=synl.begin();
	 sa_itr!=synl.end(); ++sa_itr) {
      node->terminal_number += (*sa_itr);
    }
  }

#ifdef outputallnodes
  fprintf(stdout, "Tree node type = %d (%s), #tokens = %d, value=`%s'\n", node->type, global_tree_for_debugging ? global_tree_for_debugging->getTypeName(node->type).c_str() : "<NULL>", node->terminal_number, node->isTerminal() ? node->toTerminal()->value->c_str():"<NULL>");
#endif

  return node->terminal_number;
}

// must pass token numbers from children to parents to accumulate.
long TokenCounter::
defaultSynthesizedAttribute(Tree* node, Tree* inh,
			    SynthesizedAttributesList& synl)
{
  // do not increase terminal numbers.
  if ( node==NULL )
    return 0;

  node->terminal_number = 0;
  if ( node->isNonTerminal()==true ) { // the condition must be redundent.
    for (SynthesizedAttributesList::iterator sa_itr=synl.begin();
	 sa_itr!=synl.end(); ++sa_itr) {
      node->terminal_number += (*sa_itr);
    }
  }

#ifdef outputallnodes
  fprintf(stdout, "Skipped tree node type = %d (%s), #tokens = %d, value=`%s'\n", node->type, global_tree_for_debugging ? global_tree_for_debugging->getTypeName(node->type).c_str() : "<NULL>", node->terminal_number, node->isTerminal() ? node->toTerminal()->value->c_str():"<NULL>");
#endif

  return node->terminal_number;
}


/******************************************
 * Implementation for RelevantTokenCounter
 *
 *****************************************/
RelevantTokenCounter::
RelevantTokenCounter(TraGenConfiguration & cfg)
  : TokenCounter(cfg)
{
}

bool RelevantTokenCounter::
skipNode(Tree* astNode, Tree* inh)
{
  return vecGen_config.isSkippable(astNode);
}

/******************************************
 * Implementation for TokenRangeCounter
 *
 *****************************************/
TokenRangeCounter::
TokenRangeCounter(TraGenConfiguration & cfg)
  : TokenCounter(cfg), tokennumber(0)
{
}

void TokenRangeCounter::
reinit()
{
  tokennumber = 0;
}

long TokenRangeCounter::
evaluateSynthesizedAttribute(Tree* node, Tree* in,
			     SynthesizedAttributesList& synl)
{
  long low_token_id = -1;
  if ( node->isTerminal()==true ) {
    low_token_id = tokennumber++;
  } else {
    for (SynthesizedAttributesList::iterator sa_itr=synl.begin();
	 sa_itr!=synl.end(); ++sa_itr) {
      if ( low_token_id<0 )
	low_token_id = (*sa_itr);
      else if ( (*sa_itr)<0 )
	;
      else
	low_token_id = min(low_token_id, (*sa_itr));
    }
  }

  if ( low_token_id<0 )
    node->attributes.insert(pair<NodeAttributeName_t, pair<long, long>*>(NODE_TOKEN_ID, new pair<long, long>(low_token_id, -1)));
  else
    node->attributes.insert(pair<NodeAttributeName_t, pair<long, long>*>(NODE_TOKEN_ID, new pair<long, long>(low_token_id, tokennumber-1)));

  return low_token_id;
}

long TokenRangeCounter::
defaultSynthesizedAttribute(Tree* node, Tree* inh, 
			    SynthesizedAttributesList& synl)
{
  if ( node==NULL )
    return -1;

  long low_token_id = -1;
  if ( node->isNonTerminal()==true ) { // the condition must be redundent.
    for (SynthesizedAttributesList::iterator sa_itr=synl.begin();
	 sa_itr!=synl.end(); ++sa_itr) {
      if ( low_token_id<0 )
	low_token_id = (*sa_itr);
      else if ( (*sa_itr)<0 )
	;
      else
	low_token_id = min(low_token_id, (*sa_itr));
    }
  }

  if ( low_token_id<0 )
    node->attributes.insert(pair<NodeAttributeName_t, pair<long, long>*>(NODE_TOKEN_ID, new pair<long, long>(low_token_id, -1)));
  else
    node->attributes.insert(pair<NodeAttributeName_t, pair<long, long>*>(NODE_TOKEN_ID, new pair<long, long>(low_token_id, tokennumber-1)));

  return low_token_id;
}
