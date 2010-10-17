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
#ifndef _TREE_ACCESSOR_H_
#define _TREE_ACCESSOR_H_

#include <list>
#include "../../include/ptree.h"
#include "vgen-config.h"
#include "vector-merger.h"

/* wrappers for accessors to tree nodes */
class TreeAccessor {
 public:
  static Tree* get_serialized_next_neighbor(Tree* t);
  static Tree* get_serialized_previous_neighbor(Tree* t);
  static long get_serialized_id(Tree* t);
  static long get_serialized_low_id(Tree* t);
  static TreeVector* get_node_vector(Tree* t);

  static bool is_tree_in_subtree(Tree* t, Tree* p);
  static bool is_tree_node_in_range(Tree* t, VectorMerger & vm);
  /* decide whether the whole subtree t is within the sliding window. */
  static bool is_tree_inrange_complete(Tree* t, VectorMerger & vm);

  static Tree* get_greatest_relevant_ancestor_inrange(Tree* t, VectorMerger & vm);
  static std::list<Tree*> get_all_relevant_ancestors_inrange(Tree* t, VectorMerger & vm);
  static Tree* get_greatest_atomic_relevant_ancestor_inrange(Tree* t, VectorMerger & vm);
  static std::list<Tree*> get_all_relevant_ancestors_in_parsetree(Tree* t, TraGenConfiguration & cfg);
  static Tree* get_greatest_atomic_relevant_ancestor_in_parsetree(Tree* t, TraGenConfiguration & cfg); /* t is also an ancestor of itself.  */
  static Tree* get_least_complete_ancestor_inrange(Tree* t, VectorMerger & vm);
  static Tree* get_greatest_complete_ancestor_inrange(Tree* t, VectorMerger & vm); /* could be t itself */
  static Tree* get_greatest_mergeable_ancestor_inrange(Tree* t, VectorMerger & vm);
  static Tree* get_youngest_unskippable_child(Tree* t, VectorMerger & vm); /* could be t itself */
};

#endif	/* _TREE_ACCESSOR_H_ */
