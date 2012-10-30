
#include <graphptreemap.h>

using namespace std;

/********************************
 * class GraphTreeMapper
 */

string GraphTreeMapper::fakeTypeName = "<FAKE_TYPE>";
int GraphTreeMapper::fakeTypeID = -1;

GraphTreeMapper::GraphTreeMapper(const std::string& attr)
{
   mappingAttr = attr;
}

GraphTreeMapper::~GraphTreeMapper()
{
}

// TODO: need to set filename, line range etc for each tree node for Deckard to work as it was designed

Tree* GraphTreeMapper::graph2tree(Graph* pdg, Graph* ast)
{
   int id = pdg->getAttributeID(mappingAttr);

   // collect all line numbers from the graph:
   set<string> lines;
   for(map<string, GraphNode*>::const_iterator nitr = pdg->graphNodes.begin();
         nitr!=pdg->graphNodes.end(); ++nitr) {
      // TODO: check with .dot whether 'line' could be a range?
      string attr = nitr->second->getAttribute(id);
      if ( attr!=NameMap::invalidName ) {
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
   Tree* fakeroot = new NonTerminal(fakeTypeID);
   GraphNode* astroot = ast->updateEntries();
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
   
   if ( !(ast->hasNode(astroot)) )
      return subtrees;
   
   Tree* root = NULL;
   string attr = ast->getNodeAttribute(mappingAttr, astroot);

   if ( attr!=NameMap::invalidName ) {
      set<string>::const_iterator it = lines.find(attr);
      if ( it!=lines.end() ) {
         // when the node is contained.
         string tname = ast->getNodeAttribute("type", astroot);
         stringstream ss(tname);
         int type = fakeTypeID;
         if ( !(ss>>type) )
            type = fakeTypeID;
         root = new NonTerminal(type); // treat every node as NonTerminal due to limited info from the .dot file
         subtrees.push_back(root);
         // set max, min lines in tree node for Deckard
         int line = 0;
         stringstream ssline(attr);
         if ( !(ssline>>line) )
            line = 0;
         root->max = line;
         root->min = line;
      }
   }
   
   // recursively check/copy each child
   for(vector<GraphNode*>::const_iterator citr = astroot->children.begin();
         citr!=astroot->children.end(); ++citr) {
      if ( ast->hasNode(*citr) ) {
         vector<Tree*> trees = copySubtrees(*citr, ast, lines);
         if ( root!=NULL ) {
            for(vector<Tree*>::const_iterator subitr = trees.begin();
                  subitr!=trees.end(); ++subitr) {
               root->addChild(*subitr);
               (*subitr)->parent = root;
            }
            root->lineRangeUpdate();
         } else {
            subtrees.insert(subtrees.end(), trees.begin(), trees.end());
         }
      }
   }

   return subtrees;
}
