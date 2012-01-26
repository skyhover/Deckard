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
#include "vector-output.h"
#include "tree-accessor.h"

using namespace std;

/******************************************
 * Implementation for TraVecOutput
 *
 *****************************************/
TraVecOutput::
TraVecOutput(TraGenConfiguration & cfg, FILE * out)
  : vecGen_config(cfg), vecGen_outfile(out), vector_mergers(),
    nNodeVectors(0), nMergedVectors(),
    tokenSizeUsed1(30), tokenSizeUsed2(50),
    tokenSizeBound(100), strideUsed(2)
{
}

bool TraVecOutput::
skipNode(Tree* astNode, Tree* inh)
{
  return astNode->terminal_number < vecGen_config.mergeTokens
    || vecGen_config.isSkippable(astNode)==true
    || vecGen_config.isOutputtable(astNode)==false;
}

bool TraVecOutput::
skipSubTree(Tree* astNode, Tree* inh)
{
  return vecGen_config.isAtomic(astNode);
}

Tree* TraVecOutput::
evaluateInheritedAttribute(Tree* astNode, Tree* inh)
{
  TreeVector* tv = TreeAccessor::get_node_vector(astNode);
  tv->output(vecGen_outfile);
  nNodeVectors++;

  return astNode;
}

long TraVecOutput::
evaluateSynthesizedAttribute(Tree* node, Tree* in,
			     SynthesizedAttributesList& l)
{
  return 0;			// no use here.
}

bool TraVecOutput::
addMerger(VectorMerger* vm)
{
  if ( vm!=NULL ) {
    vector_mergers.push_back(vm);
    return true;
  } else
    return false;
}

long TraVecOutput::
outputMergedVectors()
{
  long n = 0;
  for (list<VectorMerger*>::iterator vm_itr = vector_mergers.begin(); vm_itr!=vector_mergers.end(); ++vm_itr) {
    long tmp = (*vm_itr)->outputAllMergedVectors(vecGen_outfile);
    n += tmp;
    nMergedVectors.push_back( tmp );
  }

  return n;
}

long TraVecOutput::
nAllOutputedVectors()
{
  long n = nNodeVectors;
  for (vector<long>::iterator i=nMergedVectors.begin(); i!=nMergedVectors.end(); ++i)
    n += (*i);

  return n;
}

/* traverse the tree several times using different configuration values: */
long TraVecOutput::
multipleTraverse(Tree* basenode, Tree* inheritedValue,
		 t_traverseOrder travOrder)
{
  int tokensize1=tokenSizeUsed1, tokensize2=tokenSizeUsed2, tokensize=tokensize1;
  int oldtokensize = vecGen_config.mergeTokens;
  long ret;

  // should only use to smallest token size to traverse once to avoid duplication:
  vecGen_config.mergeTokens = tokensize;
  ret = this->traverse(basenode, inheritedValue, travOrder);
  vecGen_config.mergeTokens = oldtokensize;

  return ret;
}

/* traverse the tree several times using different configuration values: */
long TraVecOutput::
multipleOutputMergedVectors()
{
  int tokensize1=tokenSizeUsed1, tokensize2=tokenSizeUsed2, tokensize=tokensize1;
  int selfoldtokensize = vecGen_config.mergeTokens; // should have no effect on vectors from stride >0.
  int selfoldstride = vecGen_config.moveStride;	// should have no effect, coz internally use VectorMerger's configuration.
  long n = 0;

  vecGen_config.moveStride = strideUsed;
  while ( tokensize<=tokenSizeBound ) {
    int oldtokensize;
    int oldstride;
    vecGen_config.mergeTokens = tokensize;
    for (list<VectorMerger*>::iterator vm_itr = vector_mergers.begin(); vm_itr!=vector_mergers.end(); ++vm_itr) {
      long tmp;
      oldtokensize = (*vm_itr)->vecGen_config.mergeTokens;
      oldstride = (*vm_itr)->vecGen_config.moveStride;
      (*vm_itr)->vecGen_config.mergeTokens = tokensize;
      (*vm_itr)->vecGen_config.moveStride = strideUsed; 
      (*vm_itr)->reset();
      tmp = (*vm_itr)->outputAllMergedVectors(vecGen_outfile);
      n += tmp;
      nMergedVectors.push_back( tmp );
      (*vm_itr)->vecGen_config.mergeTokens = oldtokensize;
      (*vm_itr)->vecGen_config.moveStride = oldstride;
    }
    tokensize1 = tokensize2;
    tokensize2 = tokensize+tokensize2;
    tokensize = tokensize1;
  }
  vecGen_config.moveStride = selfoldstride;
  vecGen_config.mergeTokens = selfoldtokensize;

  return n;
}
  
