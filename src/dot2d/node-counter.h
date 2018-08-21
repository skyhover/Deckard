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
#ifndef _DECKARD_NODE_COUNTER_H_
#define _DECKARD_NODE_COUNTER_H_

#include <token-counter.h>

/** count the numbers of tokens represented by AST nodes, and fill in parent pointers for every
   tree node if not filled in yet. */
class NodeTokenCounter : public TokenCounter
{
private:
   /** store the token count for each AST node
    *  - read in from a file;
    *  - the file uses the same format as the nodetype file, except for the third column for the token count. */
   std::map<std::string, int> nodetype_tokencounts;

public:
   NodeTokenCounter(TraGenConfiguration & cfg);
   /** read in token counts for all AST nodes */
   bool init(const char * fn);
   int getTokenCount(std::string);
   int getTokenCount(Tree*);
   
   virtual long evaluateSynthesizedAttribute(Tree* node, Tree* in,
         SynthesizedAttributesList& synl);
   virtual long defaultSynthesizedAttribute(Tree* node, Tree* inh,
         SynthesizedAttributesList& synl);
};

#endif   /* _DECKARD_NODE_COUNTER_H_ */

