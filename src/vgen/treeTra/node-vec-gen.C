#include "node-vec-gen.h"

using namespace std;

/******************************************
 * Implementation for VecGenerator:
 *     A basic vector counter.
 *
 *****************************************/
VecGenerator::
VecGenerator(TraGenConfiguration & cfg)
  : vecGen_config(cfg)
{
}

bool VecGenerator::
skipNode(Tree* astNode, Tree* inh)
{
  // skippable nodes are not counted in vectors:
  return vecGen_config.isSkippable(astNode);
}

bool VecGenerator::
skipSubTree(Tree* astNode, Tree* inh)
{
  return false;
}

Tree* VecGenerator::
evaluateInheritedAttribute(Tree* astNode, Tree* inh)
{
  return astNode;
}

TreeVector* VecGenerator::
evaluateSynthesizedAttribute(Tree* astNode, Tree* inh,
			     SynthesizedAttributesList& synl)
{
  assert( astNode->type>=0 && astNode->type<vecGen_config.nodekinds );

  TreeVector * tv = new TreeVector(vecGen_config.parse_tree);

  // increase vector counters.
//   assert ( tv->increaseCounters(astNode)==true );
  assert ( vecGen_config.increaseVecCounters(astNode, tv)==true );

  // merge vectors from its children.
  for (SynthesizedAttributesList::iterator sa_itr=synl.begin();
       sa_itr!=synl.end(); ++sa_itr)
    if ( *sa_itr!=NULL )	// valid TreeVector from the child
      tv->operator+=(*(*sa_itr));

  tv->node = astNode;
  astNode->attributes.insert(pair<NodeAttributeName_t, TreeVector*>(NODE_VECTOR, tv));

  return tv;
}

// must pass vectors from children to parents to accumulate vectors
TreeVector* VecGenerator::
defaultSynthesizedAttribute(Tree* node, Tree* inh,
			    SynthesizedAttributesList& synl)
{
  TreeVector * tv = new TreeVector(vecGen_config.parse_tree);

  // only merge vectors from its children:
  int nChildren = 0;
  for (SynthesizedAttributesList::iterator sa_itr=synl.begin();
       sa_itr!=synl.end(); ++sa_itr)
    if ( *sa_itr!=NULL ) {
      if ( nChildren==0 )
	tv->operator= (*(*sa_itr));
      else
	tv->operator+=(*(*sa_itr));

      nChildren++;
    }

  if ( nChildren<=0 ) {
    delete tv;
    return NULL;
  } else {
    node->attributes.insert(pair<NodeAttributeName_t, TreeVector*>(NODE_VECTOR, tv));
    return tv;
  }
}

