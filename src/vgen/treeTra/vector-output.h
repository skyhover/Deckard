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
#ifndef _VECTOR_OUTPUT_H_
#define _VECTOR_OUTPUT_H_

#include <vector>
#include <list>
#include "../../include/ptree.h"
#include "tree-traversal.C"
#include "vgen-config.h"
#include "vector-merger.h"

class TraVecOutput : public ParseTreeTraversal<Tree*, long> /* parent node and no use. */
{
 public:
  FILE * vecGen_outfile;
  TraGenConfiguration vecGen_config;

  long nNodeVectors;
  std::vector<long> nMergedVectors;
  std::list<VectorMerger*> vector_mergers; /* helpers for merging vectors. */

  int tokenSizeUsed1, tokenSizeUsed2, tokenSizeBound, strideUsed;

 public:
  TraVecOutput(TraGenConfiguration & cfg, FILE * out);

  virtual bool skipNode(Tree* astNode, Tree* inh);
  virtual bool skipSubTree(Tree* astNode, Tree* inh);

  virtual Tree* evaluateInheritedAttribute(Tree* astNode, Tree* inh);
  virtual long evaluateSynthesizedAttribute(Tree* node, Tree* inh,
					    SynthesizedAttributesList& l);
  /* "traverse" are used to output vectors for stride 0. */

  bool addMerger(VectorMerger* vm);
  long outputMergedVectors();	/* output vectors for stride >0 */
  long nAllOutputedVectors();

  /* traverse the tree several times using different token sizes: */
  long multipleTraverse(Tree* basenode, Tree* inheritedValue,
			t_traverseOrder travOrder=TT_PREORDER);
  long multipleOutputMergedVectors(); /* output vectors for a list of strides >0 */
};

#endif	/* _VECTOR_OUTPUT_H_ */

