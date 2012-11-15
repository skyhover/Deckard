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
