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
#ifndef _CLONE_CONTEXT_PHP_H_
#define _CLONE_CONTEXT_PHP_H_

#include "token-tree-map.h"

class ContextInconsistency_PHP : public TokenTreeMap {
 public:
  typedef CloneContextT (ContextInconsistency_PHP::*GetContextFuncT)(std::pair<long, long>, std::string &);
  typedef bool (ContextInconsistency_PHP::*CompareContextFuncT)(CloneContextT&, CloneContextT&);

  virtual CloneContextT getContext2(std::pair<long, long> tokenrange, std::string & fn);
  virtual Tree* getContextNode(Tree* node); /* get the highest node which represents the context type of "node"; language dependent */
  virtual Tree* getContextParent(Tree* node); /* get the contextual parent; return NULL if not found. */

  bool comparePHPContext2(CloneContextT& context1, CloneContextT& context2); /* for Php only */
  bool comparePHPConditions(CloneContextT& context1, CloneContextT& context2); /* for Php only, compare conditions of "if" etc. */
  virtual Tree* get_conditional_operator(Tree* node); /* grammar-dependent */
  virtual bool isMainOperator(Tree* node); /* grammar-dependent */
  virtual Tree* get_condition_within(Tree* node); /* grammar-dependent */

  virtual bool filter1();
  virtual bool filter2();
  //  virtual bool filter3();
  virtual int buggy1();
};

#endif
