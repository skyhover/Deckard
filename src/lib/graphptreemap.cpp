
#include <graphptreemap.h>
#include <cassert>

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
      fakeroot->max = 0;
      fakeroot->lineRangeUpdate();
      return fakeroot;
   }
}

vector<Tree*> GraphTreeMapper::copySubtrees(GraphNode* astroot, Graph* ast)
{
   vector<Tree*> subtrees;
   assert(astroot!=NULL);
   
   Tree* root = NULL;

   if ( ast->hasNode(astroot) ) {
      // construct the root Tree node:
      string attr = ast->getNodeAttribute("line", astroot);
      string tname = ast->getNodeAttribute("type", astroot);
      stringstream ss(tname);
      int type = getFakeTypeID();
      if ( !(ss>>type) )
         type = getFakeTypeID();
      root = new NonTerminal(type); // treat every node as NonTerminal due to limited info from the .dot file
      // set max, min lines in tree node for Deckard
      int line = 0;
      stringstream ssline(attr); // could be NameMap::invalidName
      if ( !(ssline>>line) )
         line = 0;
      root->max = line;
      root->min = line;
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

   // collect all line numbers from the graph:
   set<string> lines;
   for(map<string, GraphNode*>::const_iterator nitr = pdg->graphNodes.begin();
         nitr!=pdg->graphNodes.end(); ++nitr) {
      // TODO: check with .dot whether 'line' could be a range. Currently assume each attribute is ONE line number?
      string attr = nitr->second->getAttribute(id);
      if ( attr!=NameMap::getInvalidName() ) {
//         stringstream ss(attr);
//         int line = 0;
//         if ( !(ss>>line) )
//            lines.insert(line);
         lines.insert(attr);
      }
   }
   
   // assume each node only has one line for now.
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
   fakeroot->max = 0;
   fakeroot->lineRangeUpdate();
   return fakeroot;
}

vector<Tree*> GraphTreeMapper::copySubtrees(GraphNode* astroot, Graph* ast, set<string>& lines)
{
   vector<Tree*> subtrees;
   assert(astroot!=NULL);
   
   Tree* root = NULL;
   // TODO: this may have a BUG: if 'astroot' is ignored, all its children are ignored; but we may need to keep their children!
   if ( ast->hasNode(astroot) ) {
      string attr = ast->getNodeAttribute(mappingAttr, astroot);
      if ( attr!=NameMap::getInvalidName() ) {
         set<string>::const_iterator it = lines.find(attr);
         if ( it!=lines.end() ) {
            // when the node is contained in the 'lines', check its type:
            string tname = ast->getNodeAttribute("type", astroot);
            stringstream ss(tname);
            int type = getFakeTypeID();
            if ( !(ss>>type) )
               type = getFakeTypeID();
            root = new NonTerminal(type); // treat every node as NonTerminal due to limited info from the .dot file
            subtrees.push_back(root); // add this root to the returned subtrees (will be the only root).
            // set max, min lines in tree node for Deckard
            int line = 0;
            stringstream ssline(attr);
            if ( !(ssline>>line) )
               line = 0;
            root->max = line;
            root->min = line;
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

   // recursively check/copy each child
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
   }

   return subtrees;
}
