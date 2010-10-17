/*
 * 
 * Copyright (c) 2007-2010,
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
#include "tree-accessor.h"

using namespace std;

/******************************************
 * Implementation for TreeAccessor
 *
 *****************************************/
Tree* TreeAccessor::
get_serialized_next_neighbor(Tree* t)
{
  map<NodeAttributeName_t, void*>::iterator attr_itr = t->attributes.find(NODE_SERIALIZED_NEIGHBOR);
  assert ( attr_itr!=t->attributes.end() );

  return ((pair<Tree*, Tree*>*)(*attr_itr).second)->second;
}

Tree* TreeAccessor::
get_serialized_previous_neighbor(Tree* t)
{
  map<NodeAttributeName_t, void*>::iterator attr_itr = t->attributes.find(NODE_SERIALIZED_NEIGHBOR);
  assert ( attr_itr!=t->attributes.end() );

  return ((pair<Tree*, Tree*>*)(*attr_itr).second)->first;
}

long TreeAccessor::
get_serialized_id(Tree* t)
{
  map<NodeAttributeName_t, void*>::iterator attr_itr = t->attributes.find(NODE_ID);
  assert ( attr_itr!=t->attributes.end() );

  return ((pair<long, long>*)(*attr_itr).second)->first;
}

long TreeAccessor::
get_serialized_low_id(Tree* t)
{
  map<NodeAttributeName_t, void*>::iterator attr_itr = t->attributes.find(NODE_ID);
  assert ( attr_itr!=t->attributes.end() );

  return ((pair<long, long>*)(*attr_itr).second)->second;
}

TreeVector* TreeAccessor::
get_node_vector(Tree* t)
{
  map<NodeAttributeName_t, void*>::iterator attr_itr = t->attributes.find(NODE_VECTOR);
  assert ( attr_itr!=t->attributes.end() );

  return (TreeVector*)(*attr_itr).second;
}

bool TreeAccessor::
is_tree_in_subtree(Tree* t, Tree* p)
{
  // assert ( t!=NULL && p!=NULL );
  map<NodeAttributeName_t, void*>::iterator attr_id = t->attributes.find(NODE_ID),
    attr_id2 = p->attributes.find(NODE_ID);
  assert ( attr_id!=t->attributes.end() && attr_id2!=p->attributes.end() );

  pair<long, long> * t_id = (pair<long, long>*)(*attr_id).second, * p_id = (pair<long, long>*)(*attr_id2).second;
  if ( p_id->second<=t_id->first && p_id->first>=t_id->first )
    return true;
  else
    return false;
}

bool TreeAccessor::
is_tree_node_in_range(Tree* t, VectorMerger & vm)
{
  if ( t==NULL )
    return false;

  // assert ( front!=NULL );
  map<NodeAttributeName_t, void*>::iterator attr_itr = t->attributes.find(NODE_ID),
    front_attr_itr = vm.front->attributes.find(NODE_ID);
  assert( attr_itr!=t->attributes.end() && front_attr_itr!=vm.front->attributes.end() );
  pair<long, long> * t_id = (pair<long, long>*)(*attr_itr).second, * front_id = (pair<long, long>*)(*front_attr_itr).second;

  if ( vm.tail==NULL ) {
    if ( t_id->first>=front_id->first )
      return true;
//     goto ad_hoc_check_to_avoid_bug_caused_by_duplicated_ids;
    else
      return false;
  } else {
    map<NodeAttributeName_t, void*>::iterator tail_attr_itr = vm.tail->attributes.find(NODE_ID);
    assert ( tail_attr_itr!=vm.tail->attributes.end() );
    pair<long, long> * tail_id = (pair<long, long>*)(*tail_attr_itr).second;

    if ( t_id->first>=front_id->first && t_id->first<=tail_id->first ) {
      return true;
//       goto ad_hoc_check_to_avoid_bug_caused_by_duplicated_ids;
    }
    else
      return false;
  }
#if 0
 ad_hoc_check_to_avoid_bug_caused_by_duplicated_ids:
  // better to separate computing node ids from couting tokens.
  Tree* chain_itr = vm.front;
  while ( chain_itr!=vm.tail && chain_itr!=NULL ) {
    if ( chain_itr==t )
      return true;
    chain_itr = get_serialized_next_neighbor(chain_itr);
  }
  return false;
#endif
}

bool TreeAccessor::
is_tree_inrange_complete(Tree* t, VectorMerger & vm)
{
  // check t in range or not.
  if ( is_tree_node_in_range(t, vm)==false )
    return false;

  // check whether t is complete in the sliding window.
  map<NodeAttributeName_t, void*>::iterator attr_itr = t->attributes.find(NODE_ID),
    front_attr_itr = vm.front->attributes.find(NODE_ID);
  assert( attr_itr!=t->attributes.end() && front_attr_itr!=vm.front->attributes.end() );
  pair<long, long> * t_id = (pair<long, long>*)(*attr_itr).second, * front_id = (pair<long, long>*)(*front_attr_itr).second;

  if ( vm.tail==NULL ) {
    if ( t_id->second>=front_id->first )
      return true;
    else
      return false;
  } else {
    map<NodeAttributeName_t, void*>::iterator tail_attr_itr = vm.tail->attributes.find(NODE_ID);
    assert ( tail_attr_itr!=vm.tail->attributes.end() );
    pair<long, long> * tail_id = (pair<long, long>*)(*tail_attr_itr).second;

    if ( t_id->second>=front_id->first && t_id->first<=tail_id->first )
      return true;
    else if ( t_id->first>=front_id->first && t_id->first<=tail_id->first
	      && vm.vecGen_config.isAtomic(t) )
      return true;
    else
      return false;
  }
}

Tree* TreeAccessor::
get_greatest_relevant_ancestor_inrange(Tree* t, VectorMerger & vm)
// maybe return t itself; return NULL iff t is outside the sliding window.
{
  list<Tree*> ancestors = get_all_relevant_ancestors_inrange(t, vm);

  if ( ancestors.size()<=0 )
    return NULL;
  else
    return ancestors.front();
}

std::list<Tree*> TreeAccessor::
get_all_relevant_ancestors_inrange(Tree* t, VectorMerger & vm)
// return all unskippable ancestors in range, including t itself.
{
  list<Tree*> ancestors;
  Tree* p = t;

  // get all ancestors in range; may include t itself.
  // p is not within range ==> p->parent is not within range.
  while ( p!=NULL && is_tree_node_in_range(p, vm)==true ) {
    if ( vm.vecGen_config.isSkippable(p)==false )
      ancestors.push_front(p);
    p = p->parent;
  }

  return ancestors;
}

Tree* TreeAccessor::
get_greatest_atomic_relevant_ancestor_inrange(Tree* t, VectorMerger & vm)
{
  list<Tree*> ancestors = get_all_relevant_ancestors_inrange(t, vm);

  // find the oldest atomic ancestor (may be itself).
  for (list<Tree*>::iterator anc_itr = ancestors.begin(); anc_itr!=ancestors.end(); ++anc_itr)
    if ( vm.vecGen_config.isAtomic(*anc_itr)==true )
      return *anc_itr;

  return NULL;			// no atomic ancestor in range.
}

list<Tree*> TreeAccessor::
get_all_relevant_ancestors_in_parsetree(Tree* t, TraGenConfiguration & cfg)
{
  list<Tree*> ancestors;
  Tree* p = t;

  // get all ancestors in the whole tree; may include t itself.
  while ( p!=NULL ) {
    if ( cfg.isSkippable(p)==false )
      ancestors.push_front(p);
    p = p->parent;
  }

  return ancestors;
}

Tree* TreeAccessor::
get_greatest_atomic_relevant_ancestor_in_parsetree(Tree* t, TraGenConfiguration & cfg)
{
  list<Tree*> ancestors = get_all_relevant_ancestors_in_parsetree(t, cfg);

  // find the oldest atomic ancestor (may be itself).
  for (list<Tree*>::iterator anc_itr = ancestors.begin(); anc_itr!=ancestors.end(); ++anc_itr)
    if ( cfg.isAtomic(*anc_itr)==true )
      return *anc_itr;

  return NULL;		      // no atomic ancestor in the whole tree.
}

Tree* TreeAccessor::
get_greatest_complete_ancestor_inrange(Tree* t, VectorMerger & vm)
{
  // linearly search backward to find the oldest complete ancestor of
  // t. Could be t itself.
  // assert ( t is within [front, tail) );

  Tree* chain_itr = NULL;

  if ( t==NULL || vm.front==NULL )
    return NULL;
  if ( vm.tail==NULL )
    chain_itr = vm.serialized_tree.chain_tail;
  else
    chain_itr = get_serialized_previous_neighbor(vm.tail);

  long front_id = get_serialized_id(vm.front);
  long t_low_id = get_serialized_low_id(t);
  long t_id = get_serialized_id(t);

  while ( chain_itr!=vm.front ) {
    long tmp_id = get_serialized_id(chain_itr);
    long tmp_low_id = get_serialized_low_id(chain_itr);

    if ( tmp_low_id<=t_low_id && tmp_low_id>=front_id )
      return chain_itr;
    else
      chain_itr = get_serialized_previous_neighbor(chain_itr);
  }
  if ( t==vm.front && t_low_id>=front_id ) // t is a terminal
    return t;
  else
    return NULL;
}

Tree* TreeAccessor::
get_greatest_mergeable_ancestor_inrange(Tree* t, VectorMerger & vm)
{
  // definition for "mergeable" is one a piece of paper...

  list<Tree*> ancestors = get_all_relevant_ancestors_inrange(t, vm);
  for (list<Tree*>::iterator anc_itr = ancestors.begin(); anc_itr!=ancestors.end(); ++anc_itr) {
    Tree * chain_itr = (*anc_itr);
    assert( is_tree_node_in_range(chain_itr, vm)==true );

    // An example of "logical" programming which determines whether a
    // node is "mergeable" in the sliding window ;-)
    if ( vm.vecGen_config.isAtomic(chain_itr)==true ) {
      if ( vm.vecGen_config.isSkippable(chain_itr)==false )
	return chain_itr;
      else continue;
    } else {
      if ( vm.vecGen_config.isSkippable(chain_itr)==false && is_tree_inrange_complete(chain_itr, vm)==true )
	return chain_itr;
      else continue;
    }
  } // may return t itself.

  return NULL; // nodes above t (including t itself) are all
	       // unmergeable; then could move vm.front to the node
	       // after t's greatest_ancestor_inrange.
}

Tree* TreeAccessor::
get_youngest_unskippable_child(Tree* t, VectorMerger & vm) /* could be t itself */
{
  Tree* chain_itr = t;
  Tree* atomic_anc = NULL;
  Tree* result = NULL;

  if ( t==NULL )
    return NULL;

  // too slow now:
  while ( chain_itr!=NULL && is_tree_in_subtree(chain_itr, t)==true ) {
    if ( vm.vecGen_config.isSkippable(chain_itr)==true )
      ;
    else if ( vm.vecGen_config.isMergeable(chain_itr)==false )
      ;
    else if ( atomic_anc!=NULL && is_tree_in_subtree(chain_itr, atomic_anc) )
      ;
    else
      result = chain_itr;

    if ( vm.vecGen_config.isAtomic(chain_itr) ) {
      if ( atomic_anc==NULL || is_tree_in_subtree(chain_itr, atomic_anc)==false )
	atomic_anc = chain_itr;
    }
    chain_itr = get_serialized_previous_neighbor(chain_itr);
  }

  return t;
}

