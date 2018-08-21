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

#include <graphslicer.h>

using namespace std;

/**********************************
 * GraphSlicer
 */

Graph* GraphSlicer::forwardSlice(Graph* g, GraphNode* node, ISlicingCriteria* sc)
{
   return depthFirstTraverse(g, node, sc);
}

Graph* GraphSlicer::backwardSlice(Graph* g, GraphNode* node, ISlicingCriteria* sc)
{
   return depthFirstTraverse(g, node, sc, false);
}

Graph* GraphSlicer::semanticThread(Graph* g, GraphNode* node, ISlicingCriteria* sc, float gamma)
{
   // assume the nodes are sorted already: g->reorderNodes();
   return depthFirstTraverse(g, node, sc);
}

vector<Graph*> GraphSlicer::semanticThreads(Graph* g, ISlicingCriteria* sc, float gamma)
{
   vector<Graph*> ists;
   set<GraphNode*> seen;
   vector<GraphNode*> orderedNodes = g->reorderNodes("line");
   for(vector<GraphNode*>::const_iterator it = orderedNodes.begin();
         it!=orderedNodes.end(); ++it) {
      if ( seen.find(*it)==seen.end() && sc->inSlice(*it) ) {
         Graph* slice = semanticThread(g, *it, sc, gamma);
         if ( slice->nodeCount()<=0 ) {
            delete slice;
            continue;
         }

         for(map<string, GraphNode*>::const_iterator nitr = slice->graphNodes.begin();
               nitr!=slice->graphNodes.end(); ++nitr) {
            seen.insert(nitr->second);
         }
         ists = addSemanticThread(ists, slice, sc, gamma);

         // to reduce memory leaks:
         bool used = false;
         for(vector<Graph*>::const_iterator sitr = ists.begin();
               sitr!=ists.end(); ++sitr) {
            if ( *sitr==slice ) {
               used = true;
               break;
            }
         }
         if ( !used )
            delete slice;
      }
   }
   return ists;
}

vector<Graph*> GraphSlicer::addSemanticThread(vector<Graph*>& ists, Graph* slice, ISlicingCriteria* sc, float gamma)
{
   if ( slice->nodeCount()<=0 )
      return ists;

   Graph* conflicts = new Graph();
   conflicts->graphName = slice->graphName;
   conflicts->graph_functionSig = slice->graph_functionSig;
   
   vector<Graph*> newists;
   for(vector<Graph*>::const_iterator it = ists.begin();
         it!=ists.end(); ++it) {
      Graph* gi = Graph::intersect(*it, slice);
      if ( gi->nodeCount()>gamma ) {
         Graph* gc = Graph::combine(conflicts, *it);
         delete conflicts;
         delete *it;
         conflicts = gc;
      } else
         newists.push_back(*it);
      delete gi;
   }
   // if no conflicts, newists = old ists + slice:
   if ( conflicts->nodeCount()<=0 ) {
      newists.push_back(slice);
      delete conflicts;
      return newists;
   } else {
      slice = Graph::combine(slice, conflicts);
      delete conflicts;
      newists = addSemanticThread(newists, slice, sc, gamma);
      // to reduce memory leaks:
      bool used = false;
      for(vector<Graph*>::const_iterator sitr = newists.begin();
            sitr!=newists.end(); ++sitr) {
         if ( *sitr==slice ) {
            used = true;
            break;
         }
      }
      if ( !used )
         delete slice;
      return newists;
   }
}

Graph* GraphSlicer::depthFirstTraverse(Graph* g, GraphNode* node, ISlicingCriteria* sc, bool forward)
{
   Graph* slice = new Graph();
   // copy names
   slice->graphName = g->graphName;
   slice->graph_functionSig = g->graph_functionSig;
   // copy attributeIDs
   slice->attributeIDs = g->attributeIDs;
   // copy attributes
   slice->attributes = g->attributes;
   // cannot copy graphNodes

   set<GraphNode*> seen;
   depthFirstTraverse(g, node, sc, seen, slice, forward);
   // set graphEntry
   if ( slice!=NULL && slice->graphNodes.size()>0 )
      slice->graphEntry = node;

   return slice;
}

Graph* GraphSlicer::depthFirstTraverse(Graph* g, GraphNode* node, ISlicingCriteria* sc, std::set<GraphNode*>& seen, Graph* slice, bool forward)
{
   // The result value isn't really useful for now - 2012/11/13

   if ( ! (sc->inSlice(node)) )
      return slice;

   if ( seen.find(node)!=seen.end() )
      return slice;
   
   seen.insert(node);
   if ( g->hasNode(node) ) { // this is to work even if 'g' is a graph slice itself
      slice->addNode(node);
      if ( forward ) {
         for(vector<GraphNode*>::const_iterator citr = node->children.begin();
               citr!=node->children.end(); ++citr) {
            depthFirstTraverse(g, *citr, sc, seen, slice, forward);
            /* no need since we're using the pointers to nodes and the edges are kept in the nodes:
            slice->addEdge(node, *citr); */
            // This way (compared with a new copy of GraphNode) saves memory, but be careful with the way we traverse graphs: we need to make sure we don't follow edges that are out of the graph scope.
         }
      } else {
         for(vector<GraphNode*>::const_iterator citr = node->parents.begin();
               citr!=node->parents.end(); ++citr) {
            depthFirstTraverse(g, *citr, sc, seen, slice, forward);
         }
      }
   }
   
   return slice;
}


/***********************************
 * SlicingCriteriaAcceptAll
 */

bool SlicingCriteriaAcceptAll::inSlice(GraphNode*)
{
   return true;
}

SlicingCriteriaAcceptAll* SlicingCriteriaAcceptAll::instance()
{
   // NOTE: the code is NOT thread-safe
   static SlicingCriteriaAcceptAll scSingleton;

   return &scSingleton;
}

