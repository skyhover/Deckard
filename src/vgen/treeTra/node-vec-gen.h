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
