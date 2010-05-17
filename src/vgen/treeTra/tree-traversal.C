/* The file defines the generic tree traversal interface, shameless
   adapted from ROSE: a compiler infrastructure from Dan Quinlan at
   LLNL. The difference is mainly in the tree node structure, so maybe
   we can have a tree traversal mechanism generator for different tree
   node structures ;-) */
#ifndef _TREE_TRAVERSAL_C_
#define _TREE_TRAVERSAL_C_

#include <vector>
#include "../../include/ptree.h"

typedef enum {
  TT_PREORDER,
  TT_INORDER,
  TT_POSTORDER
} t_traverseOrder;

template <class InheritedAttributeType, class SynthesizedAttributeType>
class ParseTreeTraversal {
 public:
  typedef std::vector<SynthesizedAttributeType> SynthesizedAttributesList;
  SynthesizedAttributeType traverse(Tree* basenode,
                                    InheritedAttributeType inheritedValue,
                                    t_traverseOrder travOrder=TT_PREORDER);

  ParseTreeTraversal/*<InheritedAttributeType, SynthesizedAttributeType>*/();
  virtual ~ParseTreeTraversal();

  /* TODO: how to recognize whether a synthesized attribute is from defaultSynthesizedAttribute? */

  virtual bool skipNode(Tree* astNode, InheritedAttributeType inh)=0; /* skip astNode (not affect the subtree under astNode) */
  virtual bool skipSubTree(Tree* astNode, InheritedAttributeType inh)=0; /* skip the subtree under astNode (not include astNode) */
  virtual InheritedAttributeType evaluateInheritedAttribute(Tree* astNode,
                                                            InheritedAttributeType inheritedValue)=0;
  virtual SynthesizedAttributeType evaluateSynthesizedAttribute(Tree* node,
                                                                InheritedAttributeType in,
                                                                SynthesizedAttributesList& l)=0;
  virtual InheritedAttributeType defaultInheritedAttribute(Tree* node,
							   InheritedAttributeType inh);
  virtual SynthesizedAttributeType defaultSynthesizedAttribute(Tree* node,
							       InheritedAttributeType inh,
							       SynthesizedAttributesList& synl);

};


//bad using namespace std;

template <class InheritedAttributeType, class SynthesizedAttributeType>
ParseTreeTraversal<InheritedAttributeType, SynthesizedAttributeType>::
ParseTreeTraversal( )
{
}

template<class InheritedAttributeType, class SynthesizedAttributeType>
ParseTreeTraversal<InheritedAttributeType, SynthesizedAttributeType>::
~ParseTreeTraversal( )
{
}

template <class InheritedAttributeType, class SynthesizedAttributeType>
SynthesizedAttributeType ParseTreeTraversal<InheritedAttributeType, SynthesizedAttributeType>::
traverse(Tree* node, InheritedAttributeType inheritedValue, t_traverseOrder treeTraversalOrder)
{
  SynthesizedAttributeType returnValue; //=defaultSynthesizedAttribute(inheritedValue);
  std::vector<SynthesizedAttributeType> subtreeSynthesizedAttributes;

  if ( treeTraversalOrder==TT_PREORDER ) {
    if ( node && skipNode(node, inheritedValue)==false )
      inheritedValue = evaluateInheritedAttribute(node, inheritedValue);
    else
      inheritedValue = defaultInheritedAttribute(node, inheritedValue);
  }

  if ( node && skipSubTree(node, inheritedValue)==false ) {
    // Visit the children
    int i=0;
    for (std::vector<Tree*>::iterator iter=node->children.begin(); iter!=node->children.end(); (++iter, ++i)) {
      // NOTE: because this for loop doesn't check the validity of
      // children's synthesized attributes, users of this functions
      // should check them. The same for inherited attributes from
      // parents.
      if (*iter) {
	SynthesizedAttributeType subtreeReturnValue = traverse(*iter, inheritedValue, treeTraversalOrder);
	subtreeSynthesizedAttributes.push_back(subtreeReturnValue);
      } else {
	SynthesizedAttributeType defaultValue=defaultSynthesizedAttribute(NULL, inheritedValue, subtreeSynthesizedAttributes);
	subtreeSynthesizedAttributes.push_back(defaultValue);
      }
    }
  } else
    ;			     // let subtreeSynthesizedAttributes be empty;

  if( treeTraversalOrder!=TT_PREORDER ) {
    if ( node && skipNode(node, inheritedValue)==false )
      inheritedValue = evaluateInheritedAttribute(node, inheritedValue);
    else
      inheritedValue = defaultInheritedAttribute(node, inheritedValue);
  }

  if ( node && skipNode(node, inheritedValue)==false )
    returnValue = evaluateSynthesizedAttribute(node, inheritedValue, subtreeSynthesizedAttributes);
  else
    returnValue = defaultSynthesizedAttribute(node, inheritedValue, subtreeSynthesizedAttributes);

  return returnValue;
}

template <class InheritedAttributeType, class SynthesizedAttributeType>
InheritedAttributeType ParseTreeTraversal<InheritedAttributeType, SynthesizedAttributeType>::
defaultInheritedAttribute(Tree* node, InheritedAttributeType inh)
{
  return inh;
}

template <class InheritedAttributeType, class SynthesizedAttributeType>
SynthesizedAttributeType ParseTreeTraversal<InheritedAttributeType, SynthesizedAttributeType>::
defaultSynthesizedAttribute(Tree* node, InheritedAttributeType inh, SynthesizedAttributesList& synl)
{
  SynthesizedAttributeType s;
  return s;
}

#endif	// _TREE_TRAVERSAL_C_

