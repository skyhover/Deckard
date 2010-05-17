#include <algorithm>
#include "sq-tree.h"
#include "tree-accessor.h"

using namespace std;

#define VGDEBUG

/******************************************
 * Implementation for TreeSerializer:
 *
 *****************************************/
TreeSerializer::
TreeSerializer(TraGenConfiguration & cfg)
  : vecGen_config(cfg), serialized_tree(NULL, NULL),
    previous_node(NULL), id(0), id2node()
{
}

bool TreeSerializer::
skipNode(Tree* astNode, Tree* inh)
{
  return false;
}

bool TreeSerializer::
skipSubTree(Tree* astNode, Tree* inh)
{
  return false;
}

Tree* TreeSerializer::
evaluateInheritedAttribute(Tree* astNode, Tree* inheritedValue)
{
  return astNode;
}

pair<long, long> TreeSerializer::
evaluateSynthesizedAttribute(Tree* node, Tree* in,
			     SynthesizedAttributesList& synl)
{
  // establish serialized tree chains:
  if ( previous_node!=NULL ) {
    map<NodeAttributeName_t, void*>::iterator attr_itr = previous_node->attributes.find(NODE_SERIALIZED_NEIGHBOR);
    assert( attr_itr!=previous_node->attributes.end() );
    ((pair<Tree*, Tree*>*)(*attr_itr).second)->second = node;
    node->attributes.insert(pair<NodeAttributeName_t, pair<Tree*, Tree*>*>(NODE_SERIALIZED_NEIGHBOR, new pair<Tree*, Tree*>(previous_node, NULL)));
  } else {
    // its the first visited node:
    serialized_tree.chain_header = node;
    node->attributes.insert(pair<NodeAttributeName_t, pair<Tree*, Tree*>*>(NODE_SERIALIZED_NEIGHBOR, new pair<Tree*, Tree*>(NULL, NULL)));
  }

  previous_node = node;
  serialized_tree.chain_tail = node;

  // compute node id and its low_id
  long low_id = id;		// It's the id of the current node.
  for (SynthesizedAttributesList::iterator sa_itr = synl.begin();
       sa_itr!=synl.end(); ++sa_itr) {
    if ( (*sa_itr).first>=0 )	// valid id and low_id
      low_id = min(low_id, (*sa_itr).second);
  }

#ifdef outputnodeids
  fprintf(stdout, "Tree node type = %d(%s), #tokens = %d, id = %d, low_id = %d, value=%s\n", node->type, node->terminal_number, id, low_id, node->toTerminal()?node->toTerminal()->value->c_str():"<NULL>");
#endif
  node->attributes.insert(pair<NodeAttributeName_t, pair<long, long>*>(NODE_ID, new pair<long, long>(id, low_id)));
  id2node.insert(pair<long, Tree*>(id, node));
  id++;

  return pair<long, long>(id-1, low_id);
}

// mush pass low_ids from children to parents to accumulate. ids do
// not need to be passed because they are an accumulative attribute.
pair<long, long> TreeSerializer::
defaultSynthesizedAttribute(Tree* node, Tree* inh,
			    SynthesizedAttributesList& synl)
{
  // compute node's low_id
  long low_id = -1;
  int nChildren = 0;

  for (SynthesizedAttributesList::iterator sa_itr = synl.begin();
       sa_itr!=synl.end(); ++sa_itr) {
    if ( (*sa_itr).second>=0 ) {  // valid low_id
      if ( nChildren==0 )
	low_id = (*sa_itr).second;
      else
	low_id = min(low_id, (*sa_itr).second);

      nChildren++;
    }
  }

  // These skipped nodes do not store ids or low_ids to save time and space.
  if ( nChildren<=0 )
    return pair<long, long>(-1, -1); // return invalid ids and invalid low_ids;
  else
    return pair<long, long>(-1, low_id); // return invalid ids and valid low_ids accumulated from children.
}

long TreeSerializer::
sqtree_length()
{
//   long len = 0;
//   Tree* itr = serialized_tree.chain_header;

//   while ( itr!=NULL ) {
//     len++;
//     itr = TreeAccessor::get_serialized_next_neighbor(itr);
//   }

//   return len;

  // alternative: use id2node.size(); should be faster.
  return id2node.size();
}


/***********************************************************
 * Implementation for RelevantNoAtomicParent_TreeSerializer:
 *
 ***********************************************************/
RelevantNoAtomicParent_TreeSerializer::
RelevantNoAtomicParent_TreeSerializer(TraGenConfiguration & cfg)
  : TreeSerializer(cfg)
{
}

bool RelevantNoAtomicParent_TreeSerializer::
skipNode(Tree* astNode, Tree* inh)
{
  // a node is skipped iff it is skippable or has an atomic ancestor (not itself).
  if ( vecGen_config.isSkippable(astNode)==true )
    return true;
  else {
    // TODO: it's possible to improve the tiem complexity from O(logn)
    // to O(1) by assuming that all children of an atomic node are
    // skipped (because of the following skipSubTree).
//     Tree* atomic_anc = TreeAccessor::get_greatest_atomic_relevant_ancestor_in_parsetree(astNode, vecGen_config); // O(logn)
//     if ( atomic_anc!=NULL && atomic_anc!=astNode )
//       return true;
//     else
    // SO, from now on, assume an atomic node in the Sq-Tree is the oldest atomic ancestor of its own.
      return false;
  }
}

bool RelevantNoAtomicParent_TreeSerializer::
skipSubTree(Tree* astNode, Tree* inh)
{
  // skip all children if the node is atomic:
  return vecGen_config.isAtomic(astNode);
}

// Skipped nodes are not linked into the Sq-Tree. We can use the fact to improve performance.
pair<long, long> RelevantNoAtomicParent_TreeSerializer::
defaultSynthesizedAttribute(Tree* node, Tree* inh,
			    SynthesizedAttributesList& synl)
{
  // want to assign each node a unique id, even if the node is
  // skippable, to make is_tree_in_subtree logically clearer.

  long low_id = id;		// ids always valid for this case.

  for (SynthesizedAttributesList::iterator sa_itr = synl.begin();
       sa_itr!=synl.end(); ++sa_itr) {
    low_id = min(low_id, (*sa_itr).second);
  }

#ifdef outputnodeids
  fprintf(stdout, "Skipped tree node type = %d, #tokens = %d, id = %d, low_id = %d\n", node->type, node->terminal_number, id, low_id);
#endif
  node->attributes.insert(pair<NodeAttributeName_t, pair<long, long>*>(NODE_ID, new pair<long, long>(id, low_id)));
  id++;

  return pair<long, long>(id-1, low_id);
}

