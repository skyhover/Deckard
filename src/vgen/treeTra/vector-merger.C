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
#include "vector-merger.h"
#include "tree-accessor.h"

using namespace std;

#define VGDEBUG

/******************************************
 * Implementation for VectorMerger
 *
 *****************************************/
VectorMerger::
VectorMerger(SerializedTree & head, TraGenConfiguration & cfg)
  : vecGen_config(cfg), serialized_tree(head), front(head.chain_header), tail(NULL), 
    ok_for_merge(false)
{
  mergeUnit = 1;		// useful only in child classes.
  if ( front!=NULL )		// this is possible when the parse tree has no nodes.
    tail = TreeAccessor::get_serialized_next_neighbor(head.chain_header);	// point to the next node; could be NULL when pointing to the end.
}

bool VectorMerger::
reset()
{
  front = serialized_tree.chain_header;
  if ( front!=NULL )
    tail = TreeAccessor::get_serialized_next_neighbor(front);
  else
    tail = NULL;
  ok_for_merge = false;

  return true;
}

long VectorMerger::
outputAllMergedVectors(FILE * out)
{
  TreeVector tv(vecGen_config.parse_tree);
  long moved_steps = 0;	// total numbers of steps the sliding window moved.
  long nOutputedVectors = 0;	// total number of vectors outputed.

  // handle the case when the serialized tree is too short, but is
  // this really needed? TODO
  if ( moveForward()==false ) {
    mergeTreeVectors(&tv);
    tv.output(out);
    moved_steps++;
    nOutputedVectors++;
    goto return_check;
  }

  do {
    if ( moved_steps % vecGen_config.moveStride == 0 ) { // TODO: maybe not a good condition
      tv.clearVector();
      mergeTreeVectors(&tv);
      tv.output(out);
      nOutputedVectors++;
    }
    moved_steps++;
  }  while ( moveForward()==true );

 return_check:
#ifdef VGDEBUG
  fprintf(stderr, "Moved steps = %ld, merged vectors = %ld\n", moved_steps, nOutputedVectors);
#endif

  return nOutputedVectors;
}


/******************************************
 * Implementation for VectorMergerOnTokens
 *
 *****************************************/
VectorMergerOnTokens::
VectorMergerOnTokens(SerializedTree & head, TraGenConfiguration & cfg)
  : VectorMerger(head, cfg)
{
  mergeUnit = cfg.mergeTokens;
}

bool VectorMergerOnTokens::
mergeTreeVectors(TreeVector * mv) /* merge tree vectors into mv */
{
  assert ( mv!=NULL );

  if ( ok_for_merge==true ) {
    int mergeable_counts = 0;
    Tree * chain_itr = front;

#ifdef detectthebugcausedbyduplicatenodeids
    static int mvstep = 0;
    mvstep++;
    while ( chain_itr!=tail ) {
      assert ( chain_itr!=NULL );
      fprintf(stdout, "Node %p of type:%d, id:%d, low_id:%d, tokens:%d\n", chain_itr, chain_itr->type, TreeAccessor::get_serialized_id(chain_itr), TreeAccessor::get_serialized_low_id(chain_itr), chain_itr->terminal_number);
      Tree * mergeable_ancestor = TreeAccessor::get_greatest_mergeable_ancestor_inrange(chain_itr, *this);
      if ( mergeable_ancestor!=NULL ) {
	assert ( TreeAccessor::is_tree_node_in_range(mergeable_ancestor, *this)==true );
	chain_itr = mergeable_ancestor;
      }
      chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
    }
    fprintf(stdout, "==== %d steps ====\n", mvstep);
    chain_itr = front;
#endif

    // NOTE#1: the sq-tree (instances of class
    // RelevantNoAtomicParent_TreeSerializer) doesn't contain children
    // of an atomic node or any skippable nodes.
#if 0
    while ( chain_itr!=tail /* hack to avoid seg faults:  && chain_itr!=NULL */ ) {
      if ( vecGen_config.isSkippable(chain_itr)==false ) {
	Tree * mergeable_ancestor = TreeAccessor::get_greatest_mergeable_ancestor_inrange(chain_itr, *this);
	if ( mergeable_ancestor!=NULL ) {
	  chain_itr = mergeable_ancestor; // may be still the same chain_itr;
	}

	mv->operator+=(*TreeAccessor::get_node_vector(chain_itr));
	mergeable_counts += chain_itr->terminal_number;
      }	// end if unskippable
      chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
    }
#endif	// Old implementation.
    while ( chain_itr!=tail ) {
      Tree* mergeable_ancestor = NULL;
      if ( vecGen_config.isAtomic(chain_itr)==true || TreeAccessor::is_tree_inrange_complete(chain_itr, *this)==true ) {
	mergeable_ancestor = TreeAccessor::get_greatest_complete_ancestor_inrange(chain_itr, *this);
	assert ( mergeable_ancestor!=NULL ); // In general, mergeable_ancestor may be NULL, but not here.

	mv->operator+=(*TreeAccessor::get_node_vector(mergeable_ancestor));
	mergeable_counts += mergeable_ancestor->terminal_number;
	chain_itr = mergeable_ancestor;
      }
      chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
    }

#ifdef checkmergedtokens
    fprintf(stderr, "Merging %d tokens\n", mergeable_counts);
#endif
    return true;
  } else
    return false;
}

inline int VectorMergerOnTokens::
enoughForMerge()	/* decide whether the sliding window contains enough stuff for merging. */
{
  int mergeable_counts = 0;
  Tree * chain_itr = front;

#if 0
  while ( chain_itr!=tail /* hack to avoid seg faults: && chain_itr==NULL */ ) {
    if ( vecGen_config.isSkippable(chain_itr)==false ) {
      Tree* mergeable_ancestor = TreeAccessor::get_greatest_mergeable_ancestor_inrange(chain_itr, *this);
				// Here, it seems need to use get_greatest_atomic_ancestor_in_parsetree first (TODO). 
	                        // However, thanks to moveForwardOneStep, the greatest_atomic_ancestor_in_parsetree 
                                // should be (almost) always within range.

      if ( mergeable_ancestor!=NULL )
	chain_itr = mergeable_ancestor;
      //assert ( chain_itr!=NULL );
      mergeable_counts += chain_itr->terminal_number;

      if ( mergeable_counts >= mergeUnit ) {
	ok_for_merge = true;
	return mergeable_counts-mergeUnit;
      }
    } // end if unskippable
    chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
  }
#endif	// old implementation.
  while ( chain_itr!=tail ) {
    Tree* mergeable_ancestor = NULL;
    if ( vecGen_config.isAtomic(chain_itr)==true || TreeAccessor::is_tree_inrange_complete(chain_itr, *this)==true ) {
      mergeable_ancestor = TreeAccessor::get_greatest_complete_ancestor_inrange(chain_itr, *this);
      assert ( mergeable_ancestor!=NULL ); // In general, mergeable_ancestor may be NULL, but not here.

      mergeable_counts += mergeable_ancestor->terminal_number;
      if ( mergeable_counts >= mergeUnit ) {
	ok_for_merge = true;
	return mergeable_counts-mergeUnit;
      }
      chain_itr = mergeable_ancestor;
    }
    chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
  }

  // assert (mergeable_counts < mergeUnit);
  ok_for_merge = false;
  return mergeable_counts-mergeUnit; // return the number of token lacked
}

inline bool VectorMergerOnTokens::
moveForwardOneStep(Tree* & t) /* definition of "one step" is on a piece of paper. */
// cf. NOTE#1.  Return false iff t is not really moved forward; return
// true iff t is moved forward at least one token.  Also, need to
// guarantee tail is always after front.
{
  assert ( t==front || t==tail );
  if ( t==NULL )
    return false;

  bool flag = false;

  // logical programming here ;-)
  if ( t==front ) {
    // assert ( t!=NULL && front!=NULL );
    Tree* chain_itr = TreeAccessor::get_serialized_next_neighbor(t);
    while ( chain_itr!=NULL ) {
      if ( chain_itr->terminal_number>0 && 
	   (vecGen_config.isAtomic(chain_itr)==true || chain_itr->children.size()==0) ) {
	t = chain_itr;
	flag = true;
	goto check_tail_is_after_front;
      }
      chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
    }

    t = NULL;
    goto check_tail_is_after_front;

  } else if ( t==tail ) {
    // assert ( t!=NULL && tail!=NULL );
    Tree* chain_itr = TreeAccessor::get_serialized_next_neighbor(t);
    while ( chain_itr!=NULL ) {
      if ( chain_itr->terminal_number>0 && (vecGen_config.isAtomic(chain_itr)==true || 
					    TreeAccessor::get_serialized_low_id(chain_itr)>TreeAccessor::get_serialized_id(t)) ) {
	t = chain_itr;
	flag = true;
	goto check_tail_is_after_front;
      }
      chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
    }

    if ( t->terminal_number>0 && ( vecGen_config.isAtomic(t)==true || t->isTerminal()==true ) )
      // The condition is used to prevent certain duplicate vectors (whose differences are only at nonterminals) and could be omitted.
      flag = true; // the node pointed to by the tail is not counted, but it will be counted after the following assignment.
    t = NULL;
    goto check_tail_is_after_front;
  }

  // check whether tail is after front:
 check_tail_is_after_front:
  if ( tail==NULL )
    return flag;
  else if ( front==NULL ) {
    tail = NULL;
    return false;		// no longer moveable;
  } else if ( TreeAccessor::get_serialized_id(front) < TreeAccessor::get_serialized_id(tail) )
    return flag;
  else {
    tail = TreeAccessor::get_serialized_next_neighbor(front);
    return true;
  }
}

inline bool VectorMergerOnTokens::
moveBackwardOneStep(Tree* & t)
{
  // logic is not easy unless I simplify the serialized tree to
  // contain leafNodes only (Ghassan's).
  return false;
}

bool VectorMergerOnTokens::
moveForward()
// move forward until the next mergeable sliding window.
{
  Tree* chain_itr = NULL;

  if ( ok_for_merge==true ) {
    // move front forward one step:
    moveForwardOneStep(front);
  }

#ifdef outputmovementofslidingwindow
  fprintf(stdout, "SW from id %d to id %d\n", front==NULL ? -1 : TreeAccessor::get_serialized_id(front), tail==NULL ? -1 : TreeAccessor::get_serialized_id(tail));
#endif
  // move tail forward until the next mergeable sliding window or end.
  while ( enoughForMerge()<0 ) {
    if ( moveForwardOneStep(tail)==false )
      return false;
#ifdef outputmovementofslidingwindow
    fprintf(stdout, "SW from id %d to id %d\n", front==NULL ? -1 : TreeAccessor::get_serialized_id(front), tail==NULL ? -1 : TreeAccessor::get_serialized_id(tail));
#endif
  }

  if ( ok_for_merge==true )
    return true;
  else
    return false;
}

bool VectorMergerOnTokens::
moveBackward()			// TODO: logic is not right yet.
{
  Tree* chain_itr = NULL;
  int lackoftokens = enoughForMerge();

  if ( lackoftokens>=0 ) {
    // move tail/front backward one token:
    chain_itr = tail;
    if ( chain_itr==NULL )
      chain_itr = serialized_tree.chain_tail;

    while ( chain_itr!=NULL && ( vecGen_config.isSkippable(chain_itr)==true || chain_itr->isNonTerminal()==true ) ) {
      chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
    }
    tail == chain_itr;

    chain_itr = front;		// assert ( front!=NULL );
    while ( vecGen_config.isSkippable(chain_itr)==true || chain_itr->isNonTerminal()==true ) {
      chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);

      if ( chain_itr==NULL ) { // couldn't move backward anymore
	ok_for_merge = false;
	return false;
      }
    }
    front = chain_itr;

    ok_for_merge = true;
    return true;
  } else {
    // move front backward until enough for merging or return false
    int forward_tokens = 0;
    lackoftokens = -lackoftokens;

    chain_itr = front;		// assert ( front!=NULL );
    while ( chain_itr!=NULL && forward_tokens<lackoftokens ) {
      if ( vecGen_config.isSkippable(chain_itr)==false && chain_itr->isTerminal()==true )
	forward_tokens++;

      chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
    }

    if ( chain_itr==NULL )
      front = serialized_tree.chain_header;
    else
      front = chain_itr;

    if ( forward_tokens>=lackoftokens ) {
      ok_for_merge = true;
      return true;
    } else {
      ok_for_merge = false;
      return false;
    }
  }
}

bool VectorMergerOnTokens::
reset()		/* re-initialize front/tail, and mergeUnit. */
{
  VectorMerger::reset();
  mergeUnit = vecGen_config.mergeTokens;

  return true;
}



/******************************************
 * Implementation for VectorMergerOnLists
 *
 *****************************************/
VectorMergerOnLists::
VectorMergerOnLists(SerializedTree & head, TraGenConfiguration & cfg)
  : VectorMerger(head, cfg), list_for_merge()
{
  mergeUnit = cfg.mergeLists;

  if ( front!=NULL && vecGen_config.isMergeable(front)==true && vecGen_config.isSkippable(front)==false )
    list_for_merge.push_back(front);
}

bool VectorMergerOnLists::
mergeTreeVectors(TreeVector * mv) /* merge tree vectors into mv */
{
  assert ( mv!=NULL );

  if ( ok_for_merge==true || /* at the end */(front==NULL && list_for_merge.size()>0) ) {
    int mergeable_counts = 0;
    for ( list<Tree*>::iterator itr = list_for_merge.begin(); itr!=list_for_merge.end(); ++itr) {
      mv->operator+=(*(TreeAccessor::get_node_vector(*itr)));
      mergeable_counts += (*itr)->terminal_number;
    }

#ifdef checkmergedtokens
    fprintf(stderr, "Merging %d tokens\n", mergeable_counts);
#endif

    return true;
  } else
    return false;
}

Tree* VectorMergerOnLists::
get_previous_mergeable_node(Tree* t)
// could return NULL or t itself
{
  Tree* chain_itr = t;

  // search for a missed node backward. Again, a logic programming ;-)
  while ( chain_itr!=NULL ) {
    if ( vecGen_config.isMergeable(chain_itr)==true ) {
      if ( vecGen_config.isSkippable(chain_itr)==false )
	break;
      else if ( vecGen_config.isAtomic(chain_itr)==true ) {
	chain_itr = TreeAccessor::get_youngest_unskippable_child(chain_itr, *this);
	chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
      }
    } else if ( vecGen_config.isAtomic(chain_itr)==true ) {
      chain_itr = TreeAccessor::get_youngest_unskippable_child(chain_itr, *this);
      chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
    } else
      chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
  }

  return chain_itr;
}

int VectorMergerOnLists::
add_mergeable_nodes_before(Tree* t, int missed_nodes, list<Tree*> & node_store)
// assert ( t==front && node_store==list_for_merge );
{
  if ( t==NULL || missed_nodes<=0 )
    return -1;

  Tree* chain_itr = t;
  int added_nodes = 0;

  do {
    chain_itr = get_previous_mergeable_node(chain_itr);

    if ( chain_itr==NULL )
      return added_nodes;

    node_store.push_front(chain_itr);
    missed_nodes--;
    added_nodes++;

    chain_itr = TreeAccessor::get_youngest_unskippable_child(chain_itr, *this); // assert ( chain_itr!=NULL && chain_itr is in the ST);
    chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
  } while ( missed_nodes>0 && chain_itr!=NULL );

  return added_nodes;
}

Tree* VectorMergerOnLists::
get_next_mergeable_node(Tree* t)
{
  Tree* chain_itr = t;

  // logic programming again. Getting systematic but bored ;-)
  while ( chain_itr!=NULL ) {
    if ( vecGen_config.isMergeable(chain_itr)==true ) {
      if ( vecGen_config.isSkippable(chain_itr)==true )
	chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
      // didn't try to move to chain_itr's greatest atomic ancestor
      // because atomic nodes are few, thus no too much time saving.
      else {
	Tree* ancestor = TreeAccessor::get_greatest_atomic_relevant_ancestor_in_parsetree(chain_itr, this->vecGen_config);
	if ( ancestor==NULL || ancestor==chain_itr )
	  return chain_itr;
	else
	  chain_itr = ancestor;
      }
    } else
      chain_itr = TreeAccessor::get_serialized_next_neighbor(chain_itr);
  }

  return chain_itr;
}

inline int VectorMergerOnLists::
enoughForMerge()	/* decide whether the sliding window contains enough stuff for merging. */
{
  int listsize = list_for_merge.size();

  if ( listsize < mergeUnit ) {
    ok_for_merge = false;
    return listsize-mergeUnit;
  } else {
    int tokens = 0;
    for ( list<Tree*>::iterator itr = list_for_merge.begin(); itr!=list_for_merge.end(); ++itr)
      tokens += (*itr)->terminal_number;

    if ( tokens < vecGen_config.mergeTokens ) {
      // even all the lists are too small
      ok_for_merge = false;
      return -1-mergeUnit;
    } else {
      ok_for_merge = true;
      return listsize-mergeUnit;
    }
  }
}

bool VectorMergerOnLists::
reset()		/* re-initialize front/tail, list_for_merge, and mergeUnit. */
{
  list_for_merge.clear();
  VectorMerger::reset();
  mergeUnit = vecGen_config.mergeLists;

  if ( front!=NULL && vecGen_config.isMergeable(front)==true && vecGen_config.isSkippable(front)==false ) {
    list_for_merge.push_back(front);
  }

  return true;
}

inline bool VectorMergerOnLists::
moveForwardOneStep(Tree* & t)
/* definition of "one step" is one mergeableNodes (be careful with "parents"). */
// return false iff t is not really moved forward.
// The sliding window is ended at t, inclusively (different from the tail in [front, tail) ).
// assert ( t==front );
{
  if ( t==NULL )
    return false;

  Tree* chain_itr;
  if ( t==serialized_tree.chain_header && ok_for_merge==false && list_for_merge.size()==0 )
    // this is the first time moveForwardOneStep is called.
    chain_itr = t;
  else
    chain_itr = TreeAccessor::get_serialized_next_neighbor(t);

  chain_itr = get_next_mergeable_node(chain_itr);

  if ( chain_itr!=NULL ) {
    bool flag_makeup_passed_nodes = false;

    for (list<Tree*>::iterator itr = list_for_merge.begin(); itr!=list_for_merge.end(); ++itr) {
      // delete trees which are children of chain_itr:
      if ( TreeAccessor::is_tree_in_subtree(*itr, chain_itr)==true ) {
	// now, nodes in [itr, list_for_merge.end()) are all children of chain_itr:

	if ( (*itr)->terminal_number<chain_itr->terminal_number )
	  flag_makeup_passed_nodes = true; // may need to make up some previously missed nodes.

	list_for_merge.erase(itr, list_for_merge.end());

	break;
      }
    }

    t = chain_itr; tail = t;
    // add the chain_itr into list_for_merge
    list_for_merge.push_back(t);

    // try to make up the previously missed nodes:
    int missed_nodes = list_for_merge.size()-mergeUnit;
    // assert ( t==chain_itr && chain_itr!=NULL && list_for_merge.size()>=1 );
    if ( flag_makeup_passed_nodes==true && missed_nodes>0 ) {
      chain_itr = TreeAccessor::get_youngest_unskippable_child(list_for_merge.front(), *this);
      chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
      // add missed mergeable nodes from chain_itr (include it) into list_for_merge.
      add_mergeable_nodes_before(chain_itr, missed_nodes, list_for_merge);
    }

    return true;
  } else {
    t = NULL; tail = NULL;
    return false;
  }
}

inline bool VectorMergerOnLists::
moveBackwardOneStep(Tree* & t)	// logic not meaningful
// assert ( t==front )
{
  /* finishing... */
  if ( t==NULL ) {
    t = serialized_tree.chain_tail;
    if ( ok_for_merge==false ) {
      int missed_nodes = mergeUnit-list_for_merge.size();
      if ( missed_nodes<=0 )
	missed_nodes = 1;

      Tree* chain_itr = TreeAccessor::get_youngest_unskippable_child(list_for_merge.front(), *this);
      chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
      add_mergeable_nodes_before(chain_itr, missed_nodes, list_for_merge);
    }
  } else {
    if ( ok_for_merge==false ) {
      int missed_nodes = mergeUnit-list_for_merge.size();
      if ( missed_nodes<=0 )
	missed_nodes = 1;

      Tree* chain_itr = TreeAccessor::get_youngest_unskippable_child(list_for_merge.front(), *this);
      chain_itr = TreeAccessor::get_serialized_previous_neighbor(chain_itr);
      add_mergeable_nodes_before(chain_itr, missed_nodes, list_for_merge);
    } else {
      Tree* chain_itr = get_previous_mergeable_node(t);
      if ( chain_itr==NULL )
	return false;

      list_for_merge.clear();
      add_mergeable_nodes_before(chain_itr, mergeUnit, list_for_merge);
      t = chain_itr;
    }
  }

  return false;
}

bool VectorMergerOnLists::
moveForward()
// move forward until the next mergeable sliding window.
{
  Tree* chain_itr = NULL;

  // if already ok_for_merge==true, delete one node in list_for_merge
  // to move sliding window forward:
  if ( ok_for_merge==true )
    list_for_merge.pop_front();

  while ( enoughForMerge()<0 ) {
    if ( moveForwardOneStep(front)==false )
      return false;
  }

  if ( ok_for_merge==true )
    return true;
  else
    return false;
}

bool VectorMergerOnLists::
moveBackward()
{
  return false;
}

