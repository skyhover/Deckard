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
#ifndef _SERIALIZED_TREE_H_
#define _SERIALIZED_TREE_H_

#include <map>
#include <utility>
#include "../../include/ptree.h"
#include "tree-traversal.C"
#include "vgen-config.h"

struct SerializedTree {
  Tree *chain_header;
  Tree *chain_tail;
  SerializedTree(Tree* h, Tree* t) {
    chain_header = h;
    chain_tail = t;
  }
};

/* serialize the tree in post order. */
class TreeSerializer : public ParseTreeTraversal<Tree*, std::pair<long, long> >
  /* <parent node, pair<its id (in the serialized tree; no use as a synthesized attribute), low_id> > */
{
  /* all nodes are serialized in their post order. */
 protected:
  TraGenConfiguration vecGen_config;

 public:
  SerializedTree serialized_tree; /* head/tail of the tree chain */
  Tree *previous_node;
  long id; /* The number of (valid, depending on implementation) nodes; initialized to 0. NOTE: it is better to put "id" here than in class TokenCounter: it's easier to make sure that nodes with valid ids are those nodes in the serialized tree (but code is maybe more interweaved). */
  std::map<long, Tree*> id2node; /* keep a map between ids and nodes to make traversal over the sq-tree faster. TODO. */

 public:
  TreeSerializer(TraGenConfiguration & cfg);

  virtual bool skipNode(Tree* astNode, Tree* inh);
  virtual bool skipSubTree(Tree* astNode, Tree* inh);

  virtual Tree* evaluateInheritedAttribute(Tree* astNode, Tree* inheritedValue);
  virtual std::pair<long, long> evaluateSynthesizedAttribute(Tree* node, Tree* in,
							     SynthesizedAttributesList& synl);
  virtual std::pair<long, long> defaultSynthesizedAttribute(Tree* node, Tree* inh,
							    SynthesizedAttributesList& synl);
  virtual long sqtree_length();
};

class RelevantNoAtomicParent_TreeSerializer : public TreeSerializer
{
  /* relevant nodes without atomic ancestors in the parse tree are serialized in their post order. */
 public:
  RelevantNoAtomicParent_TreeSerializer(TraGenConfiguration & cfg);

  virtual bool skipNode(Tree* astNode, Tree* inh);
  virtual bool skipSubTree(Tree* astNode, Tree* inh);
  /* use unique id for different nodes, even if they are skippable. */
  virtual std::pair<long, long> defaultSynthesizedAttribute(Tree* node, Tree* inh,
							    SynthesizedAttributesList& synl);
};

#endif	/* _SERIALIZED_TREE_H_ */

