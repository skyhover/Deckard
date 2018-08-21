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
#ifndef _NODE_VEC_GENERATOR_H_
#define _NODE_VEC_GENERATOR_H_

#include "../../include/ptree.h"
#include "tree-traversal.C"
#include "tree-vector.h"
#include "vgen-config.h"

/* This class generates vectors for each tree node first, preparing
   for vector merging later on. Thus, algorithm is more clear than
   integrating merging with basic vector generation. However, this may
   consume more memory than necessary. <--TODO (how to save memory?
   integerate generating and merging in certain way, but
   algorithmically not very clear because of the sliding windows) */
class VecGenerator : public ParseTreeTraversal<Tree*, TreeVector*> /* parent node and node vector */
{
 protected:
  TraGenConfiguration vecGen_config;

 public:
  VecGenerator(TraGenConfiguration & cfg);

  virtual bool skipNode(Tree* astNode, Tree* inh);
  virtual bool skipSubTree(Tree* astNode, Tree* inh);

  virtual Tree* evaluateInheritedAttribute(Tree* astNode, Tree* inh);
  virtual TreeVector* evaluateSynthesizedAttribute(Tree* node, Tree* inh,
						   SynthesizedAttributesList& l);
  virtual TreeVector* defaultSynthesizedAttribute(Tree* node, Tree* inh,
						  SynthesizedAttributesList& synl);
};


#endif	/* _NODE_VEC_GENERATOR_H_ */
