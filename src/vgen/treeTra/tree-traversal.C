/*
 * 
 * Copyright (c) 2007-2012,
 *   Lingxiao Jiang         <lxjiang@ucdavis.edu>
 *   Ghassan Misherghi      <ghassanm@ucdavis.edu>
 *   Zhendong Su            <su@ucdavis.edu>
 *   Stephane Glondu        <steph@glondu.net>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
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

