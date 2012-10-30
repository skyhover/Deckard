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
   static std::string fakeTypeName;
   static int fakeTypeID;
private:
   std::string mappingAttr;
public:
   // TODO: may (or not) need to modify TraGenConfiguration's constructor to allow construction without a ParseTree object
   
   /** convert a graph to a tree, if possible:
    * - no cyclic paths
    * - each node has a type ID */
   Tree* graph2tree(Graph*);
   
   /** convert a graph to a tree (always with a fake root node) based on line numbers:
    * - one input: pdg-graph, the other: ast-graph
    * - the tree nodes are copied since Deckard's vgen may modify node states; better to separate .  */
   Tree* graph2tree(Graph*, Graph*);
   Graph* tree2graph(Graph*, Graph*); // No need for now.
private:
   std::vector<Tree*> copySubtrees(GraphNode*, Graph* ast, std::set<std::string>& lines);
};

#endif /* _DECKARD_GRAPH_PTREE_MAP_H_ */
