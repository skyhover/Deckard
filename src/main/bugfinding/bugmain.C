#include <map>
#include <string>
#include <algorithm>
#include <cctype>
#include <ptree.h>
#include "clone-cluster.C"

using namespace std;

map<string,int> name2id;
map<int,string> id2name;
#ifdef JAVA
string identifierTypeName = "ID_TK";
#else
string identifierTypeName = "IDENTIFIER";
#endif

static const char *cxtNodes[] = {
#ifdef JAVA 
#include "jcontextualNodes.h"
#else
#include "ccontextualNodes.h"
#endif
};

/* For debugging use only */
ParseTree* global_tree_for_debugging;

class CompareClusterRanks {
public:
  bool operator()(const CloneClusterT & c1, const CloneClusterT & c2) {
    if ( c1.rank*c1.cluster->size() > c2.rank*c2.cluster->size() )
      return true;
    else if ( c1.buggy_score > c2.buggy_score )
      return true;
    else
      return false;
  }

  static bool filtered(CloneClusterT & cls) {
    if ( cls.rank==0 || cls.buggy_score==0 )
      return true;
    else
      return false;
  }
};


int main( int argc, char **argv )
{
  if ( argc!=2 ) {
    cerr << "Usage: " << argv[0] << " <file containing all clone clusters>" << endl;
    return 1;
  }

  FILE * fin = NULL;
  if ( (fin=fopen(argv[1], "r"))==NULL ) {
    cerr << "Can't open file: " << argv[1] << endl;
    return 2;
  }

  id_init();
  CloneClusters::init_shared_data();
#ifdef JAVA
  CloneClusters_Java tt;
#else
  CloneClusters tt;
#endif
  tt.initNodes(cxtNodes);
  tt.createAllClusters(fin);
  tt.setAllRanks();
  tt.sortClusters(CompareClusterRanks());
  for(int i=0; i<tt.clusters.size(); i++) {
    if ( CompareClusterRanks::filtered(tt.clusters[i])==false ) {
      tt.outputOneCluster(cout, tt.clusters[i]);
      cout << endl;
    }
  }

  return 0;
}

