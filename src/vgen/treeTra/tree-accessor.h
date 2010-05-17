#ifndef _TREE_ACCESSOR_H_
#define _TREE_ACCESSOR_H_

#include <list>
#include "../../include/ptree.h"
#include "vgen-config.h"
#include "vector-merger.h"

/* wrappers for accessors to tree nodes */
class TreeAccessor {
 public:
  static Tree* get_serialized_next_neighbor(Tree* t);
  static Tree* get_serialized_previous_neighbor(Tree* t);
  static long get_serialized_id(Tree* t);
  static long get_serialized_low_id(Tree* t);
  static TreeVector* get_node_vector(Tree* t);

  static bool is_tree_in_subtree(Tree* t, Tree* p);
  static bool is_tree_node_in_range(Tree* t, VectorMerger & vm);
  /* decide whether the whole subtree t is within the sliding window. */
  static bool is_tree_inrange_complete(Tree* t, VectorMerger & vm);

  static Tree* get_greatest_relevant_ancestor_inrange(Tree* t, VectorMerger & vm);
  static std::list<Tree*> get_all_relevant_ancestors_inrange(Tree* t, VectorMerger & vm);
  static Tree* get_greatest_atomic_relevant_ancestor_inrange(Tree* t, VectorMerger & vm);
  static std::list<Tree*> get_all_relevant_ancestors_in_parsetree(Tree* t, TraGenConfiguration & cfg);
  static Tree* get_greatest_atomic_relevant_ancestor_in_parsetree(Tree* t, TraGenConfiguration & cfg); /* t is also an ancestor of itself.  */
  static Tree* get_least_complete_ancestor_inrange(Tree* t, VectorMerger & vm);
  static Tree* get_greatest_complete_ancestor_inrange(Tree* t, VectorMerger & vm); /* could be t itself */
  static Tree* get_greatest_mergeable_ancestor_inrange(Tree* t, VectorMerger & vm);
  static Tree* get_youngest_unskippable_child(Tree* t, VectorMerger & vm); /* could be t itself */
};

#endif	/* _TREE_ACCESSOR_H_ */
