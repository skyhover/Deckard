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

#include <graph.h>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <utils.h>

using namespace std;

/********************
 * class Graph
 *
 * *****************/
Graph::Graph():
      attributeIDs(), graphEntry(NULL), graphNodes(),
      graphName("no name"), graph_functionSig("no signature"),
      attributes() {
}

Graph::Graph(const Graph* init):
      attributeIDs(init->attributeIDs), graphEntry(init->graphEntry), graphNodes(init->graphNodes), 
      graphName(init->graphName), graph_functionSig(init->graph_functionSig),
      attributes(init->attributes) {
}

Graph::~Graph() {
   // this default destructor does NOT delete graph nodes.
   // destructors for member objects and base class objects are automagically called.
}

int Graph::deleteGraphNodes()
{
   int c = 0;
   for(map<string, GraphNode*>::const_iterator it = graphNodes.begin();
         it!=graphNodes.end(); ++it) {
      delete it->second;
      c++;
   }
   return c;
}

int Graph::nodeCount()
{
   return graphNodes.size();
}

int Graph::edgeCount()
{
   int c = 0;
   for(map<string, GraphNode*>::const_iterator it = graphNodes.begin();
         it!=graphNodes.end(); ++it) {
      c += it->second->children.size();
   }
   return c;
}

bool Graph::hasNode(string n)
{
   map<string, GraphNode*>::const_iterator it = graphNodes.find(n);
   if ( it==graphNodes.end() )
      return false;
   else
      return true;
}

bool Graph::hasNode(GraphNode* n)
{
   if ( n==NULL )
      return false;
   return hasNode(n->nodeName);
}

GraphNode* Graph::getNode(string n)
{
   map<string, GraphNode*>::const_iterator it = graphNodes.find(n);
   if ( it==graphNodes.end() )
      return NULL;
   else
      return it->second;
}

bool Graph::addNode(GraphNode* n) {
   assert(n!=NULL);
   if ( hasNode(n) )
      return false;
   graphNodes.insert(make_pair(n->nodeName, n));
   return true;
}

bool Graph::addNode(string n) {
   if ( hasNode(n) )
      return false;
   graphNodes.insert(make_pair(n, new GraphNode(n)));
   return true;
}

bool Graph::addEdge(GraphNode* lhs, GraphNode* rhs) {
   assert(lhs!=NULL && rhs!=NULL);
   bool flag = GraphNode::addEdge(lhs, rhs);
   bool b1 = addNode(lhs);
   bool b2 = addNode(rhs);
   return flag || b1 || b2;
}

GraphNode* Graph::updateEntries()
{
   int count = 0;
   for(map<string, GraphNode*>::const_iterator nitr = graphNodes.begin();
         nitr!=graphNodes.end(); ++nitr) {
      if ( nitr->second->parents.size()<=0 ) {
         count++;
         if ( count==1 ) {
            if ( graphEntry!=nitr->second ) {
               graphEntry = nitr->second;
            }
         }
      }
   }
   if ( count>1 ) {
      cerr << "Warning: " << count << " graph entries found in Graph::updateEntries(). Only the first one will be used for AST." << endl;
   } else if ( count<=0 ) {
      cerr << "Warning: NO graph entry found in Graph::updateEntries(). The graph may not be an AST." << endl;
      graphEntry = NULL;
   }

   return graphEntry;
}

GraphNode* Graph::updateEntriesForTree()
{
   vector<GraphNode*> entries;
   for(map<string, GraphNode*>::const_iterator nitr = graphNodes.begin();
         nitr!=graphNodes.end(); ++nitr) {
      if ( nitr->second->parents.size()<=0 ) {
         entries.push_back(nitr->second);
      }
   }
   int count = entries.size();
   if ( count<=0 ) {
      cerr << "Warning: NO graph entry found in Graph::updateEntries(). The graph may not be an AST." << endl;
      graphEntry = NULL;
   } else if ( count==1 ) {
      graphEntry = entries[0];
   } else if ( count>1 ) {
      cerr << "Warning: " << count << " graph entries found in Graph::updateEntriesForTree(). Fake root node would be added to connect all entries." << endl;
      GraphNode* newroot = new GraphNode("<FakeASTRoot>");
      graphEntry = newroot;
      for(vector<GraphNode*>::const_iterator eitr = entries.begin();
            eitr!=entries.end(); ++eitr) {
         addEdge(newroot, *eitr);
      }
   }

   return graphEntry;
}

string Graph::getGraphName()
{
	return graphName;
}

bool Graph::setGraphName(string n) {
   graphName = n;
   return true;
}

int Graph::getAttributeID(string n) {
   return attributeIDs.getNameId(n);
}

int Graph::getOrAddAttributeID(string n)
{
   return attributeIDs.getOrAddNameId(n);
}

string Graph::getGraphAttribute(string n) {
   int id = getAttributeID(n);
   if ( attributeIDs.isIDValid(id) ) {
      map<int, string>::const_iterator attr = attributes.find(id);
      if ( attr!=attributes.end() )
         return attr->second;
   }
   return NameMap::getInvalidName();
}

string Graph::getNodeAttribute(string name, GraphNode* node) {
   int id = getAttributeID(name);
   return node->getAttribute(id);
}

bool Graph::addGraphAttribute(string name, string value) {
   int id = getOrAddAttributeID(name);
   map<int, string>::const_iterator attr = attributes.find(id);
   if ( attr!=attributes.end() )
      return false;
   attributes[id] = value;
   return true;
}

bool Graph::addNodeAttribute(string name, string attr, GraphNode* node) {
   int id = getOrAddAttributeID(name);
   return node->addAttribute(id, attr);
}

int Graph::printNodes(ostream & out)
{
   for(map<string, GraphNode*>::const_iterator it = graphNodes.begin();
         it!=graphNodes.end(); ++it) {
      out << "NODE: " << it->second->nodeName;
      printNodeAttributesOnly(it->second, out);
      out << endl;
   }
   return graphNodes.size();
}

int Graph::printNodeAttributesOnly(GraphNode* n, ostream & out)
{
   assert(n!=NULL);
   if ( n->attributes.size()<1 )
      return 0;
   out << " [ ";
   for(map<int, string>::const_iterator it = n->attributes.begin();
         it!=n->attributes.end(); ++it) {
      out << attributeIDs.getIDName(it->first);
      out << "=" << it->second << " ";
   }
   out << " ] ";
   return n->attributes.size();
}

int Graph::printEdges(ostream & out)
{
   int c = 0;
   for(map<string, GraphNode*>::const_iterator it = graphNodes.begin();
         it!=graphNodes.end(); ++it) {
      for(vector<GraphNode*>::const_iterator e = it->second->children.begin();
            e!=it->second->children.end(); ++e) {
         out << "EDGE: " << it->second->nodeName << " -> " << (*e)->nodeName << endl;
      }
      c += it->second->children.size();
   }
   return c;
}

int Graph::printGraph(ostream & out)
{
   out << "GRAPH: " << graphName << endl;
   printAttributesWithNames(&attributes, "   ", out);
   int c = printNodes(out);
   c += printEdges(out);
   return c;
}

bool Graph::mergeGraphAttributes(map<int, string>* attrs) {
   assert(attrs!=NULL);
   attributes.insert(attrs->begin(), attrs->end());
   return true;
}

bool Graph::mergeNodeAttributes(map<int, string>* attrs, GraphNode* node) {
   return node->mergeAttributes(attrs);
}

std::map<int, std::string>* Graph::getAttributes()
{
   return &attributes;
}

int Graph::printAttributesWithNames(const map<int, string>* attrs, std::string prefix, ostream & out)
{
   for(map<int, string>::const_iterator it = attrs->begin();
         it != attrs->end(); ++it) {
      out << prefix << attributeIDs.getIDName(it->first) << "=" << it->second << endl;
   }
   return attrs->size();
}

int Graph::printAttributesWithIDs(const map<int, string>* attrs, std::string prefix, ostream & out)
{
   for(map<int, string>::const_iterator it = attrs->begin();
         it != attrs->end(); ++it) {
      out << prefix << it->first << "=" << it->second << endl;
   }
   return attrs->size();
}

bool Graph::dumpGraph(const char * ofname, bool toOverride)
{
   ifstream inp;
   ofstream out;
   string outputfn = (ofname==NULL ? graph_functionSig : ofname) + ".grp.dot";

   // prepare the output file:
   if(!toOverride) {
      inp.open(outputfn.c_str(), ifstream::in);
      inp.close();
      if(!inp.fail()) {
         cerr << "Warning: graph dump file exists already: " << outputfn << " ...skip" << endl;
         return false;
      }
      inp.clear(ios::failbit);
   }
   out.open(outputfn.c_str(), ofstream::out);
   if(out.fail()) {
      cerr << "Error: cannot open graph dump file: " << outputfn << endl;
      return false;
   }

   // print the graph to the file:
   cerr << "# Dumping the graph into file: " << outputfn << endl;
   out << "# " << graph_functionSig << endl;
   printGraph(out);

   // close the file:
   out.close();
   return true;
}

bool Graph::outputGraph2Dot(const char * ofname, bool toOverride)
{
   ifstream inp;
   ofstream out;
   string outputfn = (ofname==NULL ? graph_functionSig : ofname) + ".dot";

   // prepare the output file:
   if(!toOverride) {
      inp.open(outputfn.c_str(), ifstream::in);
      inp.close();
      if(!inp.fail()) {
         cerr << "Warning: graph dot file exists already: " << outputfn << " ...skip" << endl;
         return false;
      }
      inp.clear(ios::failbit);
   }
   out.open(outputfn.c_str(), ofstream::out);
   if(out.fail()) {
      cerr << "Error: cannot open graph dot file: " << outputfn << endl;
      return false;
   }

   // print the graph to the file:
   cerr << "# Writing the graph into dot file: " << outputfn << endl;
   out << "# This graph is supposed to be a digraph: " << graphName << " " << graph_functionSig << endl;
   out << "digraph " << graphName << " {" << endl;
   // output graph attributes:
   if ( attributes.size()>0 ) {
      out << "graph [ ";
      for(map<int, string>::const_iterator it = attributes.begin();
            it != attributes.end(); ++it) {
         out << "attrID=" << it->first << " " << attributeIDs.getIDName(it->first) << "=" << it->second << " ";
      }
      out << "] ;" << endl;
   }
   // output graph nodes:
   for(map<string, GraphNode*>::const_iterator it = graphNodes.begin();
         it!=graphNodes.end(); ++it) {
      GraphNode* n = it->second;
      if ( hasNode(n) ) {
         out << n->nodeName;
         if ( attributes.size()>0 ) {
            out << " [ ";
            for(map<int, string>::const_iterator ait = n->attributes.begin();
                  ait!=n->attributes.end(); ++ait) {
               out << "attrID=" << ait->first << " "
                   << attributeIDs.getIDName(ait->first) << "=" << ait->second << " ";
            }
            out << " ] ";
         }
         out << endl;
      }
   }
   // output graph edges:
   for(map<string, GraphNode*>::const_iterator it = graphNodes.begin();
         it!=graphNodes.end(); ++it) {
      GraphNode* n = it->second;
      if ( hasNode(n) ) {
         for(vector<GraphNode*>::const_iterator e = n->children.begin();
            e!=n->children.end(); ++e) {
            if ( hasNode(*e) ) {
               out << n->nodeName << " -> " << (*e)->nodeName << endl;
            }
         }
      }
   }

   out << "}" << endl;

   // close the file:
   out.close();
   return true;
}

vector<GraphNode*> Graph::reorderNodes(string attr)
{
   int id = getAttributeID(attr);
   vector<GraphNode*> nodes;
   for(map<string, GraphNode*>::const_iterator nitr = graphNodes.begin();
         nitr!=graphNodes.end(); ++nitr) {
      nodes.push_back(nitr->second);
   }
   CompareGraphNode comp(id);
   sort(nodes.begin(), nodes.end(), comp);
   return nodes;
}

Graph* Graph::combine(Graph* lhs, Graph* rhs)
{
   Graph* rsl = new Graph();
   // set graph names
   if ( lhs->graphName!=rhs->graphName ) {
      cerr << "Warning: different graph names: " << lhs->graphName << " vs. " << rhs->graphName << endl
           << " -> Really can combine? Continue anyway..." << endl;
   }
   rsl->setGraphName(lhs->graphName);
   if ( lhs->graph_functionSig!=rhs->graph_functionSig ) {
      cerr << "Warning: different function signature: " << lhs->graph_functionSig << " vs. " << rhs->graph_functionSig << endl
           << " -> Really can combine? Continue anyway..." << endl;
   }
   rsl->graph_functionSig = lhs->graph_functionSig;
   // combine graphNodes
   rsl->graphNodes = lhs->graphNodes;
   for(map<string, GraphNode*>::const_iterator ritr = rhs->graphNodes.begin();
         ritr!=rhs->graphNodes.end(); ++ritr) {
      GraphNode* n = rsl->getNode(ritr->first);
      if ( n!=NULL ) {
         if ( n!=ritr->second ) {
            cerr << "Warning: different graph node instances for the same name when combing? Continue anyway..." << endl;
         }
      } else {
         rsl->graphNodes.insert(*ritr);
      }
   }
   // combine attributeIDs
   rsl->attributeIDs = NameMap::combineNameMap(lhs->attributeIDs, rhs->attributeIDs);
   // combine attributes
   rsl->attributes = lhs->attributes;
   for(map<int, string>::const_iterator ritr = rhs->attributes.begin();
         ritr!=rhs->attributes.end(); ++ritr) {
      map<int, string>::const_iterator aitr = lhs->attributes.find(ritr->first);
      if ( aitr!=lhs->attributes.end() ) {
         if ( aitr->second!=ritr->second ) {
            cerr << "Warning: different attribute values when combing attribute ID-" << aitr->first << ": " << aitr->second << " vs. " << ritr->second << endl
                 << " -> Really can combine? Continue anyway..." << endl;
         }
      } else {
         rsl->attributes.insert(*ritr);
      }
   }
   // ignore graphEntry for now and leave it to updateEntries... if needed. TODO: optimize it.
   
   return rsl;
}

Graph* Graph::intersect(Graph* lhs, Graph* rhs)
{
   Graph* rsl = new Graph();
   // set graph names
   if ( lhs->graphName!=rhs->graphName ) {
      cerr << "Warning: different graph names: " << lhs->graphName << " vs. " << rhs->graphName << endl
           << " -> Really can intersect? Continue anyway..." << endl;
   }
   rsl->setGraphName(lhs->graphName);
   if ( lhs->graph_functionSig!=rhs->graph_functionSig ) {
      cerr << "Warning: different function signature: " << lhs->graph_functionSig << " vs. " << rhs->graph_functionSig << endl
           << " -> Really can intersect? Continue anyway..." << endl;
   }
   rsl->graph_functionSig = lhs->graph_functionSig;
   // intersect graphNodes
   for(map<string, GraphNode*>::const_iterator litr = lhs->graphNodes.begin();
         litr!=lhs->graphNodes.end(); ++litr) {
      map<string, GraphNode*>::const_iterator ritr = rhs->graphNodes.find(litr->first);
      if ( ritr==rhs->graphNodes.end() )
         continue;
      if ( litr->second!=ritr->second ) {
         cerr << "Warning: different graph node instances for the same name when intersecting? Continue anyway..." << endl;
      }
      rsl->graphNodes.insert(*litr);
   }
   // combine attributeIDs
   rsl->attributeIDs = NameMap::combineNameMap(lhs->attributeIDs, rhs->attributeIDs);
   // intersect attributes
   for(map<int, string>::const_iterator litr = lhs->attributes.begin();
         litr!=lhs->attributes.end(); ++litr) {
      map<int, string>::const_iterator ritr = rhs->attributes.find(litr->first);
      if ( ritr==rhs->attributes.end() )
         continue;
      if ( litr->second!=ritr->second ) {
         cerr << "Warning: different attribute values for attribute ID-" << litr->first << ": " << litr->second << " vs. " << ritr->second << endl
              << " -> Really can intersect? Continue anyway..." << endl;
      }
      rsl->attributes.insert(*litr);
   }
   // ignore graphEntry for now and leave it to updateEntries... if needed. TODO: optimize it.
   
   return rsl;
}

/********************************
 * class GraphNode
 * ************************/
GraphNode::GraphNode():children(), parents(), attributes(), nodeName("no name node")
{
}

GraphNode::GraphNode(string n):children(), parents(), attributes(), nodeName(n)
{
}

GraphNode::~GraphNode()
{
   // leave things to default destruction
}

string GraphNode::getAttribute(int id) const
{
   map<int, string>::const_iterator attr = attributes.find(id);
   if ( attr!=attributes.end() )
      return attr->second;
   return NameMap::getInvalidName();
}

bool GraphNode::addAttribute(int id, string n)
{
   map<int, string>::const_iterator attr = attributes.find(id);
   if ( attr!=attributes.end() )
      return false;
   attributes[id] = n;
   return true;
}

bool GraphNode::mergeAttributes(map<int, string>* attrs)
{
   assert(attrs!=NULL);
   attributes.insert(attrs->begin(), attrs->end());
   return true;
}

bool GraphNode::hasEdge(GraphNode* n)
{
   if ( n==NULL )
      return false;
   for(vector<GraphNode*>::const_iterator it = children.begin();
         it!=children.end(); ++it) {
      // pointer equivalence is ok, assuming node names are unique
      if ( *it==n )
         return true;
   }
   for(vector<GraphNode*>::const_iterator it = parents.begin();
         it!=parents.end(); ++it) {
      if ( *it==n )
         return true;
   }
   return false;
}

bool GraphNode::isChildOf(GraphNode* n)
{
   if ( n==NULL )
      return false;
   for(vector<GraphNode*>::const_iterator it = parents.begin();
         it!=parents.end(); ++it) {
      if ( *it==n )
         return true;
   }
   return false;
}

bool GraphNode::isParentOf(GraphNode* n)
{
   if ( n==NULL )
      return false;
   for(vector<GraphNode*>::const_iterator it = children.begin();
         it!=children.end(); ++it) {
      if ( *it==n )
         return true;
   }
   return false;
}

bool GraphNode::addChild(GraphNode* n)
{
   assert(n!=NULL);
   if ( isParentOf(n) )
      return false;
   children.push_back(n);
   return true;
}

bool GraphNode::addParent(GraphNode* n)
{
   assert(n!=NULL);
   if ( isChildOf(n) )
      return false;
   parents.push_back(n);
   return true;
}

bool GraphNode::addEdge(GraphNode* lhs, GraphNode* rhs)
{
   assert(lhs!=NULL && rhs!=NULL);
   bool flag = lhs->addChild(rhs);
   bool b = rhs->addParent(lhs);
   return flag || b;
}

int GraphNode::printParents(string prefix, ostream & out)
{
   for(vector<GraphNode*>::const_iterator it = parents.begin();
         it!=parents.end(); ++it) {
      out << prefix << (*it)->nodeName << endl;
   }
   return parents.size();
}

int GraphNode::printChildren(string prefix, ostream & out)
{
   for(vector<GraphNode*>::const_iterator it = children.begin();
         it!=children.end(); ++it) {
      out << prefix << (*it)->nodeName << endl;
   }
   return children.size();
}

int GraphNode::printNode(ostream & out)
{
   int c = 1;
   out << "NODE: " << nodeName << endl;
   c += printParents("<-", out);
   c += printChildren("->", out);
   return c;
}

int GraphNode::printNodeAttributesOnly(ostream & out)
{
   if ( attributes.size()<1 )
      return 0;
   out << " [ ";
   for(map<int, string>::const_iterator it = attributes.begin();
         it!=attributes.end(); ++it) {
      out << it->first << "=" << it->second << " ";
   }
   out << " ] ";
   return attributes.size();
}

/*********************************
 * CompareGraphNode
 */
CompareGraphNode::CompareGraphNode(int attrID)
{
   compareFieldID = attrID;
}

bool CompareGraphNode::operator()(const GraphNode* lhs, const GraphNode* rhs)
{
   /* NOTE: 'x==y' is implemented by '!(x<y) && !(y<x)'
    * So, this operator must be strict:
    * - (x<y)==true => (y<x)==false
    * - (x==y)==true => (x<y)==false && (y<x)==false
    * otherwise, there may be subtle bugs caused if 'sort' is used.
    */
   /* NULL < invalidName < invalid lines < valid lines;
    * Valid lines can be a single number, or a sequence of numbers,
    * and are compared in the dictionary order.
    */
   if(lhs==NULL && rhs==NULL) return false;
   else if (lhs==NULL) return true;
   else if (rhs==NULL) return false;
   else {
      string lstr = lhs->getAttribute(compareFieldID);
      string rstr = rhs->getAttribute(compareFieldID);
      if ( lstr==NameMap::getInvalidName() && rstr==NameMap::getInvalidName() )
         return false;
      else if ( lstr==NameMap::getInvalidName() )
         return true;
      else if ( rstr==NameMap::getInvalidName() )
         return false;
      else {
         // reply on other parts of code to get line numbers to make it more modular
         vector<int> lnums = getNumbers(lstr);
         vector<int> rnums = getNumbers(rstr);
         if ( lnums.empty() && rnums.empty() )
            return lstr < rstr;
         else if ( lnums.empty() ) return true;
         else if ( rnums.empty() ) return false;
         else {
            vector<int>::const_iterator litr, ritr;
            for( litr=lnums.begin(), ritr=rnums.begin();
                  litr!=lnums.end() && ritr!=rnums.end(); ++litr, ++ritr) {
               if ( *litr < *ritr ) return true;
               else if ( *litr > *ritr ) return false;
            }
            if ( litr==lnums.end() && ritr==rnums.end() ) return false;
            else if ( litr==lnums.end() ) return true;
            else if ( ritr==rnums.end() ) return false;
            else {
               // impossible case
               cerr << "Error: CompareGraphNode::operator() sees impossible cases." << endl;
               throw "CompareGraphNode::operator() sees impossible cases.";
            }
         }
      }
   }
}

