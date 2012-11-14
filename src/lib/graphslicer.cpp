
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
   conflicts->graphName = slice->graphName;
   conflicts->graph_functionSig = slice->graph_functionSig;
   
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
   // copy names
   slice->graphName = g->graphName;
   slice->graph_functionSig = g->graph_functionSig;
   // copy attributeIDs
   slice->attributeIDs = g->attributeIDs;
   // copy attributes
   slice->attributes = g->attributes;
   // cannot copy graphNodes

   set<GraphNode*> seen;
   Graph* rsl = depthFirstTraverse(g, node, sc, seen, slice, forward);
   // set graphEntry
   if ( rsl!=NULL && rsl->graphNodes.size()>0 )
      rsl->graphEntry = node;

   return rsl;
}

Graph* GraphSlicer::depthFirstTraverse(Graph* g, GraphNode* node, ISlicingCriteria* sc, std::set<GraphNode*>& seen, Graph* slice, bool forward)
{
   // The result value isn't really useful for now - 2012/11/13

   if ( ! (sc->inSlice(node)) )
      return slice;

   if ( seen.find(node)==seen.end() )
      return slice;
   
   seen.insert(node);
   if ( g->hasNode(node) ) { // this is to work even if 'g' is a graph slice itself
      slice->addNode(node);
      if ( forward ) {
         for(vector<GraphNode*>::const_iterator citr = node->children.begin();
               citr!=node->children.end(); ++citr) {
            slice = depthFirstTraverse(g, *citr, sc, seen, slice, forward);
            /* no need since we're using the pointers to nodes and the edges are kept in the nodes:
            slice->addEdge(node, *citr); */
            // This way (compared with a new copy of GraphNode) saves memory, but be careful with the way we traverse graphs: we need to make sure we don't follow edges that are out of the graph scope.
         }
      } else {
         for(vector<GraphNode*>::const_iterator citr = node->parents.begin();
               citr!=node->parents.end(); ++citr) {
            slice = depthFirstTraverse(g, *citr, sc, seen, slice, forward);
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

