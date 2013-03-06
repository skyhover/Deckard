/*
 * 
 * Copyright (c) 2007-2013, University of California / Singapore Management University
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
#ifndef _TOKEN_COUNTER_H_
#define _TOKEN_COUNTER_H_

#include "../../include/ptree.h"
#include "tree-traversal.C"
#include "vgen-config.h"

/* Count the numbers of tokens and fill in parent pointers for every
   tree node if not filled in yet. */
class TokenCounter : public ParseTreeTraversal<Tree*, long> /* parent pointer, token number */
{
  /* count all tokens (terminal nodes). */
 protected:
  TraGenConfiguration vecGen_config;

 public:
  TokenCounter(TraGenConfiguration & cfg);

  virtual bool skipNode(Tree* astNode, Tree* inh);
  virtual bool skipSubTree(Tree* astNode, Tree* inh);

  virtual Tree* evaluateInheritedAttribute(Tree* astNode, Tree* inheritedValue);
  virtual long evaluateSynthesizedAttribute(Tree* node, Tree* in,
					    SynthesizedAttributesList& synl);
  virtual long defaultSynthesizedAttribute(Tree* node, Tree* inh,
					   SynthesizedAttributesList& synl);
};

class RelevantTokenCounter : public TokenCounter
{
  /* count relevant tokens only. */
 public:
  RelevantTokenCounter(TraGenConfiguration & cfg);

  virtual bool skipNode(Tree* astNode, Tree* inh);
};

class TokenRangeCounter : public TokenCounter /* <Tree*, long> : <parent pointer, lower bound of the token range> */
{
  /* set the token range for each node */
 private:
  long tokennumber; /* number of tokens; initialized to 0. Valid: [0,
		       tokennumber-1]. If "traverse" is called again
		       and again on an object of this class, it's up
		       to callers to decide whether to reset
		       "tokennumber" to 0 or not */

 public:
  TokenRangeCounter(TraGenConfiguration & cfg);
  void reinit();
  virtual long evaluateSynthesizedAttribute(Tree* node, Tree* in,
                                            SynthesizedAttributesList& synl);
  virtual long defaultSynthesizedAttribute(Tree* node, Tree* inh,
                                           SynthesizedAttributesList& synl);
};

#endif	/* _TOKEN_COUNTER_H_ */

