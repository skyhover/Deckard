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
  std::vector<bool> countedNodes; /* of length nodekinds, indicating the nodes should be counted. */
  std::vector<bool> outputtedNodes; /* of length nodekinds, indicating the nodes should be outputted. */
  std::vector<bool> atomicNodes; /* of length nodekinds, indicating the nodes should not be counted separately. */
  std::vector<bool> mergeableNodes; /* of length nodekinds, indicating the nodes should be considered for merging (specific to the VectorMergerOnLists???). */

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
