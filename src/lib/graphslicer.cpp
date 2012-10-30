
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

vector<Graph*> GraphSlicer::semanticThread(Graph* g, ISlicingCriteria* sc, float gamma)
{
   vector<Graph*> ists;
   set<GraphNode*> seen;
   vector<GraphNode*> orderedNodes = g->reorderNodes("line");
   for(vector<GraphNode*>::const_iterator it = orderedNodes.begin();
         it!=orderedNodes.end(); ++it) {
      if ( seen.find(*it)==seen.end() && sc->inSlice(*it) ) {
         Graph* slice = semanticThread(g, *it, sc, gamma);
         for(map<string, GraphNode*>::const_iterator nitr = slice->graphNodes.begin();
               nitr!=slice->graphNodes.end(); ++nitr) {
            seen.insert(nitr->second);
         }
         ists = addSemanticThread(ists, slice, sc, gamma);

         // to prevent memory leaks:
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
   Graph* conflicts = new Graph();
   vector<Graph*> newists;
   for(vector<Graph*>::const_iterator it = ists.begin();
         it!=ists.end(); ++it) {
      Graph* gi = Graph::intersect(*it, slice);
      if ( gi->nodeCount()>gamma ) {
         Graph* gc = Graph::combine(conflicts, *it);
         delete conflicts;
         conflicts = gc;
      } else
         newists.push_back(*it);
      delete gi;
   }
   if ( conflicts->nodeCount()<=0 ) {
      newists.push_back(slice);
      return newists;
   }
   slice = Graph::combine(slice, conflicts);
   return addSemanticThread(newists, slice, sc, gamma);
}

Graph* GraphSlicer::depthFirstTraverse(Graph* g, GraphNode* node, ISlicingCriteria* sc, bool forward)
{
   Graph* slice = new Graph();
   slice->graphEntry = node;
   // copy names
   slice->graphName = g->graphName;
   slice->graph_functionSig = g->graph_functionSig;
   // copy attributeIDs
   slice->attributeIDs = g->attributeIDs;
   // copy attributes
   slice->attributes = g->attributes;

   set<GraphNode*> seen;
   return depthFirstTraverse(g, node, sc, seen, slice, forward);
}

Graph* GraphSlicer::depthFirstTraverse(Graph* g, GraphNode* node, ISlicingCriteria* sc, std::set<GraphNode*>& seen, Graph* slice, bool forward)
{
   if ( ! (sc->inSlice(node)) )
      return slice;

   seen.insert(node);
   slice->addNode(node);
   if ( forward ) {
      for(vector<GraphNode*>::const_iterator citr = node->children.begin();
            citr!=node->children.end(); ++citr) {
         if ( g->hasNode(*citr) ) { // this is to work even if 'g' is a graph slice itself
            if ( seen.find(*citr)==seen.end() ) {
               slice = depthFirstTraverse(g, *citr, sc, seen, slice, forward);
               /* no need since we're using the pointers to nodes and the edges are kept in the nodes:
               slice->addEdge(node, *citr); */
               // This way (compared with a new copy of GraphNode) saves memory, but be careful with the way we traverse graphs: we need to make sure we don't follow edges that are out of the graph scope.
            }
         } else {
            seen.insert(*citr);
         }
      }
   } else {
      for(vector<GraphNode*>::const_iterator citr = node->parents.begin();
            citr!=node->parents.end(); ++citr) {
         if ( g->hasNode(*citr) && seen.find(*citr)==seen.end() ) {
            slice = depthFirstTraverse(g, *citr, sc, seen, slice, forward);
         } else {
            seen.insert(*citr);
         }
      }
   }
   return slice;
}


/***********************************
 * SlicingCriteriaAcceptAll
 */

SlicingCriteriaAcceptAll* SlicingCriteriaAcceptAll::scSingleton = NULL;

bool SlicingCriteriaAcceptAll::inSlice(GraphNode*)
{
   return true;
}

SlicingCriteriaAcceptAll* SlicingCriteriaAcceptAll::instance()
{
   if ( scSingleton==NULL )
      scSingleton = new SlicingCriteriaAcceptAll();
   return scSingleton;
}

