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
#ifndef _DECKARD_GRAPH_H_
#define _DECKARD_GRAPH_H_

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <string.h> // for compatibility on various platforms
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <limits.h>
#include "namemap.h"

class GraphNode;

/** implicitly directed graph */
class Graph {
public:
   Graph();
   Graph(const Graph*);
   /** the default constructor does NOT delete graph nodes */
   ~Graph();
   int deleteGraphNodes();
   
   /** whole-graph level info */

public:
   NameMap attributeIDs;
   GraphNode* graphEntry; /** assumed starting point of a graph; used to identify the root for TREES. TODO: need to be vector for graphs? */
   std::map<std::string, GraphNode*> graphNodes;
   std::string graphName;
   std::string graph_functionSig; /** The signature of the function from which the graph is generated */
   std::map<int, std::string> attributes;
   friend class GraphSlicer;
   friend class GraphTreeMapper;
   
public:
   int nodeCount();
   int edgeCount();
   
   bool hasNode(std::string n);
   bool hasNode(GraphNode* n);
   GraphNode* getNode(std::string);
   bool addNode(GraphNode*);
   bool addNode(std::string);
   /** add an edge into the graph; the nodes are also added into the graph if they are not yet */
   bool addEdge(GraphNode*, GraphNode*);
   /** update the 'graphEntry' based on 'graphNodes': nodes with no parents are entries */
   GraphNode* updateEntries();
   /** update the 'graphNodes' based on 'graphNodes', assuming the graph is a TREE.
    *  Fake root node may be added into 'graphNodes' if multiple entries are found. */
   GraphNode* updateEntriesForTree();
   
   std::string getGraphName();
   bool setGraphName(std::string);
   int getAttributeID(std::string);
   int getOrAddAttributeID(std::string);
   std::string getGraphAttribute(std::string);
   std::string getNodeAttribute(std::string, GraphNode*);
   bool addGraphAttribute(std::string, std::string);
   bool addNodeAttribute(std::string, std::string, GraphNode*);
   
   int printNodes(std::ostream & out=std::cout);
   int printNodeAttributesOnly(GraphNode*, std::ostream & out=std::cout);
   int printEdges(std::ostream & out=std::cout);
   int printGraph(std::ostream & out=std::cout);
   
public: /** auxiliary functions */
   bool mergeGraphAttributes(std::map<int, std::string>*);
   bool mergeNodeAttributes(std::map<int, std::string>*, GraphNode*);
   std::map<int, std::string>* getAttributes();
   int printAttributesWithNames(const std::map<int, std::string>*, std::string prefix="", std::ostream & out=std::cout);
   static int printAttributesWithIDs(const std::map<int, std::string>*, std::string prefix="", std::ostream & out=std::cout);
   bool dumpGraph(const char* ofname=NULL, bool toOverride=false);
   bool outputGraph2Dot(const char* ofname=NULL, bool toOverride=false);
   
public: /** graph operations */
   std::vector<GraphNode*> reorderNodes(std::string attr="line"); /** sort nodes by their line numbers */
   static Graph* combine(Graph*, Graph*); /** union of nodes and edges */
   static Graph* intersect(Graph*, Graph*); /** find a maximal common subgraph */
};

/** node-level info */
class GraphNode {
public:
   GraphNode();
   GraphNode(std::string);
   /** this default destructor does NOT delete parents/children nodes */
   ~GraphNode();

   std::vector<GraphNode*> children; // performance should be ok assuming the sizes of vectors are small
   std::vector<GraphNode*> parents;

private:
   std::map<int, std::string> attributes;
   std::string nodeName;

public:
   std::string getAttribute(int) const;
   bool addAttribute(int, std::string);
   bool mergeAttributes(std::map<int, std::string>*);
   
   bool hasEdge(GraphNode*);
   static bool addEdge(GraphNode*, GraphNode*);
   bool isChildOf(GraphNode*);
   bool isParentOf(GraphNode*);
   bool addChild(GraphNode*);
   bool addParent(GraphNode*);
   
   int printParents(std::string prefix="", std::ostream & out=std::cout); 
   int printChildren(std::string prefix="", std::ostream & out=std::cout);
   int printNode(std::ostream & out=std::cout);
   int printNodeAttributesOnly(std::ostream & out=std::cout);
   
   friend class Graph;
   friend class GraphSlicer;
   friend class GraphTreeMapper;
};

struct CompareGraphNode : public std::binary_function<GraphNode*, GraphNode*, bool> {
public:
   CompareGraphNode(int attrID=-1);
private:
   int compareFieldID; /** the ID of the attribute used for comparison */ 
public:
   bool operator()(const GraphNode*, const GraphNode*);
};
#endif /* _DECKARD_GRAPH_H_ */
