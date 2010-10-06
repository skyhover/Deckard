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

  ~TraGenMain();

  void run();
};

#endif /* _TRA_GEN_H_ */

