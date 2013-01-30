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
#ifndef _TRA_GEN_H_
#define _TRA_GEN_H_

#include <cstdio>
#include "../../include/ptree.h"
#include "vgen-config.h"
#include "token-counter.h"
#include "sq-tree.h"
#include "node-vec-gen.h"
#include "vector-output.h"
#include "vector-merger.h"

class TraGenMain {
protected:
  ParseTree * parse_tree;
  TraGenConfiguration * vecGen_config;
  TokenCounter * token_counter;
  TokenRangeCounter * token_range_counter;
  TreeSerializer * tree_serializer;
  VecGenerator * vec_generator;
  TraVecOutput * vec_outputor;
  VectorMerger * token_merger;
  VectorMerger * list_merger; /* TODO: being debugged; too inefficient. */
  FILE * vecGen_outfile;

 public:
  TraGenMain(ParseTree* rt, const char * fn, FILE * out);
 public:
  TraGenMain(ParseTree* rt, int mergeTokens, int mergeStride, int mergeLists, FILE * out);

  static bool getParameters(const char * fn, int & mergeTokens, int & mergeStride, int & mergeLists);

  virtual ~TraGenMain();

  virtual void run(int startln=0, int endln=0);
};

#endif /* _TRA_GEN_H_ */

