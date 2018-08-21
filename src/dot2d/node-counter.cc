
#include "node-counter.h"

using namespace std;

/******************************************
 * Implementation for NodeTokenCounter
 *
 *****************************************/
NodeTokenCounter::
NodeTokenCounter(TraGenConfiguration & cfg)
  : TokenCounter(cfg)
{
}

bool NodeTokenCounter::init(const char * fn)
{
   nodetype_tokencounts.clear();
   if ( fn == NULL )
      return false;
   
   string line;
   ifstream ifs(fn);
   int lcount = 0;
   while ( ifs.good() ) {
      lcount++;
      string name;
      int id, tcount;
      getline(ifs, line);
      stringstream ss(line);
      ss >> name;
      if ( !ss ) {
         continue;
      }
      if ( name.find("//")==0 || name.find('#')==0 )
         continue;
      ss >> id;
      if ( !ss ) {
         cerr << "Warning: node type " << name << " at line " << lcount << " has invalid id." << endl;
         tcount = 0;
      } else {
         ss >> tcount;
         if ( !ss ) {
            cerr << "Warning: node type " << name << " at line " << lcount << " has invalid token count. Default 0 is used." << endl;
            tcount = 0;
         }
      }
      map<string, int>::const_iterator citr = nodetype_tokencounts.find(name);
      if ( citr!=nodetype_tokencounts.end() ) {
         cerr << "Warning: node type " << name << " has a previous token count: " << citr->second << endl;
      } else {
         nodetype_tokencounts[name] = tcount;
      }
   }
   
   if ( nodetype_tokencounts.empty() ) {
      cerr << "Warning: nodetype_tokencounts empty. Every node has 0 token count?!" << endl;
   }
   return true;
}

int NodeTokenCounter::getTokenCount(string tn)
{
   map<string, int>::const_iterator itr = nodetype_tokencounts.find(tn);
   if ( itr!=nodetype_tokencounts.end() )
      return itr->second;
   else
      return 0;
}

int NodeTokenCounter::getTokenCount(Tree* n)
{
   if ( n==NULL )
      return 0;
   assert(this->vecGen_config.parse_tree!=NULL);
   return getTokenCount(vecGen_config.parse_tree->getTypeName(n->type));
}

long NodeTokenCounter::
evaluateSynthesizedAttribute(Tree* node, Tree* inh,
			     SynthesizedAttributesList& synl)
{
   long c = getTokenCount(node);
#ifdef debugnodetokencount
   cerr << "evaluateSynthesizedAttribute: ";
   cerr << " " << c;
#endif
   for (SynthesizedAttributesList::iterator sa_itr=synl.begin();
         sa_itr!=synl.end(); ++sa_itr) {
      c += (*sa_itr);
#ifdef debugnodetokencount
      cerr << " " << c;
#endif
   }
   node->terminal_number = c;
#ifdef debugnodetokencount
   cerr << endl;
#endif

  return c;
}

long NodeTokenCounter::
defaultSynthesizedAttribute(Tree* node, Tree* inh,
			    SynthesizedAttributesList& synl)
{
   // do not increase terminal numbers.
   if ( node==NULL )
      return 0;

   long c = 0;
#ifdef debugnodetokencount
   cerr << "defaultSynthesizedAttribute: ";
   cerr << " " << c;
#endif
   for (SynthesizedAttributesList::iterator sa_itr=synl.begin();
         sa_itr!=synl.end(); ++sa_itr) {
      c += (*sa_itr);
#ifdef debugnodetokencount
      cerr << " " << c;
#endif
   }
   node->terminal_number = c;
#ifdef debugnodetokencount
   cerr << endl;
#endif

   return c;
}

