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

#include <graphptreemap.h>
#include <cassert>
#include <utils.h>

using namespace std;

/********************************
 * class GraphTreeMapper
 */

// FIX for Potential BUG due to "initialization order fiasco", although the code is NOT thread-safe.
//string GraphTreeMapper::fakeTypeName = "<FAKE_TYPE>";
//int GraphTreeMapper::fakeTypeID = -1;
string& GraphTreeMapper::getFakeTypeName()
{
   static string fakeTypeName = "<FAKE_TYPE>";
   return fakeTypeName;
}
int& GraphTreeMapper::getFakeTypeID()
{
   static int fakeTypeID = -1;
   return fakeTypeID;
}

GraphTreeMapper::GraphTreeMapper(const std::string& attr)
{
   mappingAttr = attr;
}

GraphTreeMapper::~GraphTreeMapper()
{
}

Tree* GraphTreeMapper::graph2tree(Graph* ast)
{
   assert(ast!=NULL);

   GraphNode* astroot = ast->updateEntriesForTree();
   vector<Tree*> trees = copySubtrees(astroot, ast);
   int count = trees.size();
   if ( count<=0 )
      return NULL;
   else if (count==1)
      return trees[0];
   else {
      Tree* fakeroot = new NonTerminal(getFakeTypeID());
      for(vector<Tree*>::const_iterator subitr = trees.begin();
            subitr!=trees.end(); ++subitr) {
         fakeroot->addChild(*subitr);
         (*subitr)->parent = fakeroot;
      }
      fakeroot->min = INT_MAX;
      fakeroot->max = -1;
      fakeroot->lineRangeUpdate();
      return fakeroot;
   }
}

vector<Tree*> GraphTreeMapper::copySubtrees(GraphNode* astroot, Graph* ast)
{
   assert(astroot!=NULL && ast!=NULL);
   
   vector<Tree*> subtrees;
   Tree* root = NULL;

   if ( ast->hasNode(astroot) ) {
      // construct the root Tree node:
      // - set node type:
      string tname = ast->getNodeAttribute("type", astroot);
      if ( tname!=NameMap::getInvalidName() ) {
         stringstream ss(tname);
         int type;
         if ( !(ss>>type) ) {
            type = getFakeTypeID();
            cerr << "Warning: GraphTreeMapper::copySubtrees: node (" << astroot << ") has invalid type: "
                 << tname << endl;
         }
         root = new NonTerminal(type); // treat every node as NonTerminal due to limited info from the .dot file
      } else
         root = new NonTerminal(getFakeTypeID());
      // - set line numbers:
      string attr = ast->getNodeAttribute("line", astroot);
      if ( attr!=NameMap::getInvalidName() ) {
         // set max, min lines in tree node for Deckard
         vector<int> linenums = getNumbers(attr);
         if ( linenums.size()==0 ) {
            root->max = -1;
            root->min = INT_MAX;
            cerr << "Warning: GraphTreeMapper::copySubtrees: node (" << astroot << ") has invalid line numbers: "
                 << attr << endl;
         } else if ( linenums.size()==1 ) {
            root->max = linenums[0];
            root->min = linenums[0];
         } else if ( linenums.size()==2 ) {
            root->max = linenums[1];
            root->min = linenums[0];
         } else {
            root->max = linenums[1];
            root->min = linenums[0];
            cerr << "Warning: GraphTreeMapper::copySubtrees: node (" << astroot << ") has more than two line numbers: "
                 << attr << endl;
         }
      } else {
         root->max = -1;
         root->min = INT_MAX;
      }
   }
   
   /* TODO: check more precisely when 'ast' is a real Tree and prevent possible infinite loops if 'ast' has cycles.
    * currently, only a warning is given:
    */
   if ( astroot->parents.size()>1 ) {
      cerr << "Warning: a supposed Tree node has more than one parent node: " << astroot->nodeName << endl;
      cerr << "-> Continue anyway; may cause undefined behavior." << endl;
   }

   // recursively check/copy every child
   for(vector<GraphNode*>::const_iterator citr = astroot->children.begin();
         citr!=astroot->children.end(); ++citr) {
      vector<Tree*> trees = copySubtrees(*citr, ast);
      if ( root!=NULL ) {
         for(vector<Tree*>::const_iterator subitr = trees.begin();
               subitr!=trees.end(); ++subitr) {
            root->addChild(*subitr);
            (*subitr)->parent = root;
         }
      } else {
         subtrees.insert(subtrees.end(), trees.begin(), trees.end());
      }
   }
   
   // finish up:
   if ( root!=NULL ) {
      root->lineRangeUpdate();
      subtrees.push_back(root); // add this root the returned subtrees (it's the only tree node in subtrees).
   }

   return subtrees;
}

Tree* GraphTreeMapper::graph2tree(Graph* pdg, Graph* ast)
{
   assert(pdg!=NULL && ast!=NULL);

   int id = pdg->getAttributeID(mappingAttr);

   // TODO: to allow 'line' to be a set of ranges or individual numbers. Currently can only be a single number of two numbers.
   // collect all line numbers from the graph:
   set<int> lines;
   for(map<string, GraphNode*>::const_iterator nitr = pdg->graphNodes.begin();
         nitr!=pdg->graphNodes.end(); ++nitr) {
      string attr = nitr->second->getAttribute(id);
      if ( attr!=NameMap::getInvalidName() ) {
         vector<int> nums = getNumbers(attr);
         set<int> nodelines = enumerateNumers(nums);
         lines.insert(nodelines.begin(), nodelines.end());
      }
   }
   
   // Top-down traverse the tree to find "maximal" trees containing all of the lines
   // Or, bottom-up...No need since we assume we want all nodes with the lines so the results should be "maximal"
   GraphNode* astroot = ast->updateEntriesForTree();
   Tree* fakeroot = new NonTerminal(getFakeTypeID());
   vector<Tree*> trees = copySubtrees(astroot, ast, lines);
   for(vector<Tree*>::const_iterator subitr = trees.begin();
         subitr!=trees.end(); ++subitr) {
      fakeroot->addChild(*subitr);
      (*subitr)->parent = fakeroot;
   }
   fakeroot->min = INT_MAX;
   fakeroot->max = -1;
   fakeroot->lineRangeUpdate();
   return fakeroot;
}

vector<Tree*> GraphTreeMapper::copySubtrees(GraphNode* astroot, Graph* ast, set<int>& lines)
{
   assert(astroot!=NULL && ast!=NULL);
   
   vector<Tree*> subtrees;
   Tree* root = NULL;

   if ( ast->hasNode(astroot) ) {
      // check line matches
      string attr = ast->getNodeAttribute(mappingAttr, astroot);
      if ( attr!=NameMap::getInvalidName() ) {
         vector<int> nums = getNumbers(attr);
         set<int> nodelines = enumerateNumers(nums);
         if ( ! is_set_intersection_empty<set<int>, set<int> >(nodelines, lines) ) {
            // when the node is contained in the 'lines', check its type:
            string tname = ast->getNodeAttribute("type", astroot);
            if ( tname!=NameMap::getInvalidName() ) {
               stringstream ss(tname);
               int type;
               if ( !(ss>>type) ) {
                  type = getFakeTypeID();
                  cerr << "Warning: GraphTreeMapper::copySubtrees: node (" << astroot << ") has invalid type: "
                       << tname << endl;
               }
               root = new NonTerminal(type); // treat every node as NonTerminal due to limited info from the .dot file
            } else
               root = new NonTerminal(getFakeTypeID());
            // set max, min lines in tree node for Deckard
            if ( nums.size()==0 ) { // should NOT be reachable
               root->max = -1;
               root->min = INT_MAX;
               cerr << "Error: Unreachable code! GraphTreeMapper::copySubtrees: node (" << astroot << ") has invalid line numbers: "
                    << attr << endl;
            } else if ( nums.size()==1 ) {
               root->max = nums[0];
               root->min = nums[0];
            } else if ( nums.size()==2 ) {
               root->max = nums[1];
               root->min = nums[0];
            } else {
               root->max = nums[1];
               root->min = nums[0];
               cerr << "Warning: GraphTreeMapper::copySubtrees: node (" << astroot << ") has more than two line numbers: "
                    << attr << endl;
            }
         }
      }
   }
   
   /* TODO: check more precisely when 'ast' is a real Tree and prevent possible infinite loops if 'ast' has cycles.
    * currently, only a warning is given:
    */
   if ( astroot->parents.size()>1 ) {
      cerr << "Warning: a supposed Tree node has more than one parent node: " << astroot->nodeName << endl;
      cerr << "-> Continue anyway; may cause undefined behavior." << endl;
   }

   // recursively check/copy every child
   for(vector<GraphNode*>::const_iterator citr = astroot->children.begin();
         citr!=astroot->children.end(); ++citr) {
      vector<Tree*> trees = copySubtrees(*citr, ast, lines);
      if ( root!=NULL ) {
         for(vector<Tree*>::const_iterator subitr = trees.begin();
               subitr!=trees.end(); ++subitr) {
            root->addChild(*subitr);
            (*subitr)->parent = root;
         }
      } else {
         subtrees.insert(subtrees.end(), trees.begin(), trees.end());
      }
   }
   
   // finish up:
   if ( root!=NULL ) {
      root->lineRangeUpdate();
      subtrees.push_back(root); // add this root to the returned subtrees (will be the only root).
   }

   return subtrees;
}
