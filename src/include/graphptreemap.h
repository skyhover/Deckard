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
