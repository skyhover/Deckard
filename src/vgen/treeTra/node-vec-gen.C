/*
 * 
 * Copyright (c) 2007-2018, University of California / Singapore Management University
 *   Lingxiao Jiang         <lxjiang@ucdavis.edu> <lxjiang@smu.edu.sg>
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

