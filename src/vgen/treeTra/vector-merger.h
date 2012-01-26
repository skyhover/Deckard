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
#ifndef _VECTOR_MERGER_H_
#define _VECTOR_MERGER_H_

#include <list>
#include "../../include/ptree.h"
#include "sq-tree.h"

/* Interface for different vector merging algorithms based on
   different sliding windows. They are based on serialized trees. */
class VectorMerger {
  /* This class implements vector merging based on token counts. */
 protected:
  SerializedTree serialized_tree;
  Tree *front, *tail;		/* determine the boundary of the sliding window [front, tail). */
  int mergeUnit;
  bool ok_for_merge;		/* flag whether the number of trees reaches mergeUnit. */
 public:
  TraGenConfiguration vecGen_config;

 public:
  VectorMerger(SerializedTree & head, TraGenConfiguration & cfg);

  virtual bool mergeTreeVectors(TreeVector * mv)=0; /* merge tree vectors into mv */

  /* make sure ok_for_merge is consistent with the mergeUnit and the silding window. */
  virtual int enoughForMerge()=0;	/* decide whether the sliding window contains enough stuff for merging; return value: 1-enough, 0-just enough, -n-lack of n tokens. */
  virtual bool moveForwardOneStep(Tree* & t)=0; /* definition of "one step" is on a piece of paper. */
  virtual bool moveBackwardOneStep(Tree* & t)=0;
  virtual bool moveForward()=0; /* move the sliding window forward to the next position ok for merging. */
  virtual bool moveBackward()=0; /* move the sliding window backward to the previous position ok for merging. */
  virtual bool reset();		/* re-initialize front/tail. */

  virtual long outputAllMergedVectors(FILE * out); /* return number of outputed vectors */

  friend class TreeAccessor;
};

class VectorMerger1 {
  /* This class is basically a circulated queue. old interface, not flexible enough. Removed. */
 public:
  int mergeUnit;		/* >0 */
  Tree** subtrees;		/* of length at most mergeUnit. */
  TreeVector* treevectors;	/* of length the same as subtrees. */
  int front, tail;		/* in the range of [0, mergeUnit) */
  bool ok_for_merge;		/* flag whether the number of trees reaches mergeUnit. */

  VectorMerger1(int mu);

  bool mergeTreeVectors(TreeVector * mv); /* merge treevectors into mv */
  bool addTree(Tree* t, TreeVector * v); /* add a tree into this merger. */
  bool removeTree(unsigned int n); /* remove the n oldest trees in the merger; n<=mergeUnit. */
  bool removeAllTrees();	/* empty the merger. */
};

class VectorMergerOnTokens : public VectorMerger {
  /* This class implements vector merging based on token counts. */
 public:
  VectorMergerOnTokens(SerializedTree & head, TraGenConfiguration & cfg);

  virtual bool mergeTreeVectors(TreeVector * mv); /* merge tree vectors into mv */

  /* make sure ok_for_merge is consistent with the mergeUnit and the silding window. */
  virtual int enoughForMerge();	/* decide whether the sliding window contains enough stuff for merging; return value: 1-enough, 0-just enough, -n-lack of n tokens. */
  virtual bool moveForwardOneStep(Tree* & t); /* definition of "one step" is on a piece of paper. */
  virtual bool moveBackwardOneStep(Tree* & t);
  virtual bool moveForward(); /* move the sliding window forward to the next position ok for merging. */
  virtual bool moveBackward(); /* move the sliding window backward to the previous position ok for merging. */
  virtual bool reset();		/* re-initialize front/tail, and mergeUnit. */

  friend class TreeAccessor;
};

class VectorMergerOnLists : public VectorMerger {
  /* This class will implement vector merging for statement_list etc. TODO: not used yet */
 protected:
  std::list<Tree*> list_for_merge; /* a list of nodes for merging, should be in the post-order because of the ordered identifiers used later for bug finding. */

 public:
  VectorMergerOnLists(SerializedTree & head, TraGenConfiguration & cfg);

  virtual bool mergeTreeVectors(TreeVector * mv); /* merge tree vectors into mv */

 private:
  Tree* get_previous_mergeable_node(Tree* t); /* go back from t (include t) to get the last mergeable node. could return NULL. */
  int add_mergeable_nodes_before(Tree* t, int missed_nodes, std::list<Tree*> & node_store); /* go back from t (include t) to get missed_nodes number mergeable nodes. */
  Tree* get_next_mergeable_node(Tree* t); /* go forward from t (include t) to get the next mergeable node. could return NULL. */
 public:
  /* make sure ok_for_merge is consistent with the mergeUnit and the silding window. */
  virtual int enoughForMerge();	/* decide whether the sliding window contains enough stuff for merging; return value: 1-enough, 0-just enough, -n-lack of n tokens. */
  virtual bool moveForwardOneStep(Tree* & t); /* definition of "one step" is on a piece of paper. */
  virtual bool moveBackwardOneStep(Tree* & t);
  virtual bool moveForward(); /* move the sliding window forward to the next position ok for merging. */
  virtual bool moveBackward(); /* move the sliding window backward to the previous position ok for merging. */
  virtual bool reset();		/* re-initialize front/tail, list_for_merge, and mergeUnit. */

  friend class TreeAccessor;
};

#endif	/* _VECTOR_MERGER_H_ */
