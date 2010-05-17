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

