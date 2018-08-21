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
