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

