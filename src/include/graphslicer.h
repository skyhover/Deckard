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
#ifndef _DECKARD_GRAPH_SLICER_H_
#define _DECKARD_GRAPH_SLICER_H_

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

class ISlicingCriteria;

class GraphSlicer {
public:
   GraphSlicer();
   ~GraphSlicer();
public:
   Graph* graph; /** not useful yet */
public:
   static Graph* forwardSlice(Graph*, GraphNode*, ISlicingCriteria* sc);
   static Graph* backwardSlice(Graph*, GraphNode*, ISlicingCriteria* sc);
   static Graph* bidirectionalSlice(Graph*, Graph*, ISlicingCriteria* sc);
   static Graph* weaklyConnectedSlice(Graph*, Graph*, ISlicingCriteria* sc);
   static Graph* forwardSlice(Graph*, std::vector<GraphNode*>*, ISlicingCriteria* sc);
   static Graph* backwardSlice(Graph*, std::vector<GraphNode*>*, ISlicingCriteria* sc);
   static Graph* bidirectionalSlice(Graph*, std::vector<GraphNode*>*, ISlicingCriteria* sc);
   static Graph* weaklyConnectedSlice(Graph*, std::vector<GraphNode*>*, ISlicingCriteria* sc);
   static Graph* semanticThread(Graph*, GraphNode*, ISlicingCriteria* sc, float gamma=3.0);
   static Graph* semanticThread(Graph*, std::vector<GraphNode*>*, ISlicingCriteria* sc, float gamma=3.0);
   /** find a set of STs */
   static std::vector<Graph*> semanticThreads(Graph*, ISlicingCriteria* sc, float gamma=3.0);
   static std::vector<Graph*> addSemanticThread(std::vector<Graph*>&, Graph*, ISlicingCriteria* sc, float gamma=3.0);
   
   static Graph* depthFirstTraverse(Graph*, GraphNode*, ISlicingCriteria* sc, bool forward=true);
   static Graph* breadthFirstTraverse(Graph*, GraphNode*, ISlicingCriteria* sc);
private:
   static Graph* depthFirstTraverse(Graph*, GraphNode*, ISlicingCriteria* sc, std::set<GraphNode*>& seen, Graph*, bool forward=true);
};


class ISlicingCriteria {
public:
   virtual ~ISlicingCriteria() { }
   virtual bool inSlice(GraphNode*) = 0;
};

class SlicingCriteriaAcceptAll : public ISlicingCriteria {
private:
   /* Replace this with a static local var in 'instance()' to avoid possible "initialization order fiasco"
   static SlicingCriteriaAcceptAll* scSingleton;
   * Also, more discussion about singleton pattern in C++:
   * - possibly mem leak if using pointers, unless some clean up 'atexit'
   * - cf. stackoverflow.com/questions/2496918/singleton-pattern-in-c
   */
   /** prevent external calls to the big three: */
   SlicingCriteriaAcceptAll() { }
   SlicingCriteriaAcceptAll(const SlicingCriteriaAcceptAll&) { }
   SlicingCriteriaAcceptAll& operator=(const SlicingCriteriaAcceptAll&) { }
public:
   virtual bool inSlice(GraphNode*);
   static SlicingCriteriaAcceptAll* instance();
};

#endif /* _DECKARD_GRAPH_SLICER_H_ */
