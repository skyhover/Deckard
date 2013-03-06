/*
 * 
 * Copyright (c) 2007-2013, University of California / Singapore Management University
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
#ifndef _DECKARD_GRAPH_PTREE_MAP_H_
#define _DECKARD_GRAPH_PTREE_MAP_H_

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <string.h> // for compatibility on various platforms
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <set>
#include <limits.h>
#include "graph.h"
#include "ptree.h"

/** map a graph (or semantic thread) to a tree (possible fake root node would be added to connect subtrees */
class GraphTreeMapper {
public:
   GraphTreeMapper(const std::string& mappingAttr="line");
   ~GraphTreeMapper();
public:
//   static std::string fakeTypeName;
//   static int fakeTypeID;
   /* Potential BUG: due to "initialization order fiasco", replace the above static members with static local variables in functions
    * return non-const reference to allow changes to the static local variables.
    */
   static std::string& getFakeTypeName();
   static int& getFakeTypeID();
private:
   std::string mappingAttr;
public:
   
   /** convert a graph to a tree, if possible:
    * - no cyclic paths (TODO: add validity check)
    * - each node has a type ID.
    * Additional fake root would be added only if there are more than one entry in the graph. */
   Tree* graph2tree(Graph*);
   
   /** convert a graph to a tree (always with a fake root node) based on line numbers:
    * - frist input: pdg-graph, the second: ast-graph. The first is mapped onto second, and a new Tree is constructed.
    * - the tree nodes are copied since Deckard's vgen may modify node states; better to separate .  */
   Tree* graph2tree(Graph*, Graph*);
   Graph* tree2graph(Graph*, Graph*); // No need for now.
private:
   /** construct a Tree rooted at 'astroot' in 'ast'. Every node in 'ast' is kept. */
   std::vector<Tree*> copySubtrees(GraphNode* astroot, Graph* ast);
   /** construct a Tree rooted at 'astroot' in 'ast'. Nodesin 'ast' are kept only if they are contained in 'lines'. */
   std::vector<Tree*> copySubtrees(GraphNode* astroot, Graph* ast, std::set<int>& lines);
};

#endif /* _DECKARD_GRAPH_PTREE_MAP_H_ */
