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
#ifndef _VEC_GEN_CONFIG_H_
#define _VEC_GEN_CONFIG_H_

#include <vector>
#include "../../include/ptree.h"
#include "tree-vector.h"

/* Represent the configurable options from configuration files. */
class TraGenConfiguration {
 public:
  ParseTree* parse_tree;	/* for backup ;-) */

  const char * cfgFile;		/* Name of the configuration file. TODO: we don't have flexible syntax for configuration yet. */

  int nodekinds; /* The number of kinds of nodes: 0<=type<nodekinds. */
  std::vector<bool> countedNodes; /* of length nodekinds, indicating the nodes should be counted; same as relevantNodes in ptree.h. */
  std::vector<bool> outputtedNodes; /* of length nodekinds, indicating the nodes should be outputted; same as validParents in ptree.h. */
  std::vector<bool> atomicNodes; /* of length nodekinds, indicating the nodes that should not be broken up by moving sliding windows; same as leafNodes in ptree.h. */
  std::vector<bool> mergeableNodes; /* of length nodekinds, indicating the nodes should be considered for merging (similar to a combination of atomicNodes and relevantNodes; specific to the VectorMergerOnLists???. Not really used.) */

  /* parameters for vector merging. */
  int mergeTokens; /* The minimal number of tokens should be outputted. If <0, disable it; if ==0, use a list: 30 50 80 130 210 340 550 890 */
  int mergeLists; /* The maximal number of statements (or other kinds of lists) should be outputted. TODO: not used yet. */ 
  int moveStride; /* The minimal distance the sliding window is moved each time. If <=0, disable it (no meaning to run a list if we can use stride 1). If mergeTokens==0, use stride 2. */

 public:
  /* Helper functions for generating file names. TODO: not useful for now. */
  const char * getHeaderName( );
  const char * getCppName( );
  const char * getVecName( );

 public:
  TraGenConfiguration(const char * fn);	/* Read in a configuration file. */
  TraGenConfiguration(ParseTree* rt); /* configuration based on a parse tree. */
  TraGenConfiguration(ParseTree* rt, int tokens, int strides, int lists);
 private:
  void init();

 public:
  bool isSkippable(Tree* node);
  bool isOutputtable(Tree* node);
  bool isAtomic(Tree* node);
  bool isMergeable(Tree* node);

  bool increaseVecCounters(Tree* n, TreeVector* tv/*, ParseTree* pt=parse_tree not allowed in C++.*/);

  int identid; /* node type representing identifiers; it's grammar-specific */

  friend class VectorMerger;
  friend class VectorMergerOnTokens;
  friend class VectorMergerOnLists;
  friend class VecGenerator;
  friend class TraVecOutput;
};

#endif	/* _VEC_GEN_CONFIG_H_ */
