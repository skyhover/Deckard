
/** This is the main entry for vector generation for one source file.
 *  With different defines, the file can be compiled for different languages.
 *  
 *  Need to use with scripts if wanting vector generation for many source files.
 */

#include <ptree.h>
#include <map>
#include <string>
#include <tra-gen.h>
#include <algorithm>
#include <cctype>

using namespace std;

map<string,int> name2id;
map<int,string> id2name;
#ifdef JAVA
string identifierTypeName = "ID_TK";
#else
#ifdef PHP
string identifierTypeName = "T_VARIABLE";
#else
string identifierTypeName = "IDENTIFIER";
#endif
#endif

int yyparse();

extern Tree *root;

void id_init();

extern FILE *yyin;

static char *relNodes[] = {
#ifdef JAVA 
#include "../ptgen/java/jrelevantNodes.h"
#else
#ifdef PHP
#include "../ptgen/php5/phprelevantNodes.h"
#else
#include "../ptgen/gcc/crelevantNodes.h"
#endif
#endif
};

static char *atomicNodes[] = {
#ifdef JAVA
#include "../ptgen/java/jatomicNodes.h"
#else
#ifdef PHP
#include "../ptgen/php5/phpatomicNodes.h"
#else
#include "../ptgen/gcc/catomicNodes.h"
#endif
#endif
};

static char *valParents[] = {
#ifdef JAVA
#include "../ptgen/java/jparentNodes.h"
#else
#ifdef PHP
#include "../ptgen/php5/phpparentNodes.h"
#else
#include "../ptgen/gcc/cparentNodes.h"
#endif
#endif
};


void initNodes( vector<int> & nodes, char **nodeconfig)
{
    for (char **s= nodeconfig; *s != NULL; s++) {
        map<string,int>::iterator i= name2id.find(*s);
        if (i == name2id.end()) {
            cerr << "unknown node type: " << *s << endl;
            continue;
        }
        nodes.push_back(i->second);
    }
}

/* For debugging use only */
ParseTree* global_tree_for_debugging;

int main( int argc, char **argv )
{
    if ( argc<2 ) {
        cerr << "usage: %s filename [config_file | [3 parameters]]" << endl
	     << "\tCurrent config_file contains three natural numbers: " << endl
	     << "\t\t(1) # tokens for merging; " << endl
	     << "\t\t(2) length of a stride; " << endl
	     << "\t\t(3) # lists for merging." << endl;
        return 1;
    }

    // parse the input file
    yyin= fopen(argv[1],"r");
    if (!yyin) {
        cerr << "invalid filename: %s" << argv[1] << endl;
        return 1;
    }
    id_init();
    yyparse();
    if (!root) {
        cerr << "failed to parse file: " << argv[1] << endl;
        return 65;
    }

    root->lineRange();

#if 0
    cerr << "Terminal count: " << root->countTerminals() << endl;
#endif

    // prepare for vector generation
    ParseTree p(root,id2name.size(),&id2name,&name2id);

    initNodes(p.relevantNodes, relNodes);
    initNodes(p.leafNodes, atomicNodes);
    initNodes(p.validParents, valParents);
    //initNodes(p.mergeableNodes, mergeableNodes);

    p.filename= argv[1];

    // vec gen parameters:
    int mergeTokens = 30, mergeStride = 1, mergeLists = 3;
    const char * configfilename = NULL;
    if ( argc==3 ) {
      configfilename = argv[2];
      if ( !TraGenMain::getParameters(configfilename, mergeTokens, mergeStride, mergeLists) )
        /* use whatever parameters set so far */ ;
    } else {
      if ( argc>=3 ) {
        if ( sscanf(argv[2], "%d", &mergeTokens)<=0 ) {
          cerr << "Can't get mergeTokens from argv[2]: " << argv[2] << endl;
          mergeTokens = 30;
        }
      }
      if ( argc>=4 ) {
        if ( sscanf(argv[3], "%d", &mergeStride)<=0 ) {
          cerr << "Can't get mergeStride from argv[3]: " << argv[3] << endl;
          mergeStride = 1;
        }
      }
      if ( argc>=5 ) {
        if ( sscanf(argv[4], "%d", &mergeLists)<=0 ) {
          cerr << "Can't get mergeLists from argv[4]: " << argv[4] << endl;
          mergeLists = 3;
        }
      }
      cerr << "Merging parameters: " << mergeTokens << ", " << mergeStride << ", " << mergeLists << endl;
    }

    // for debugging use only in vector generator.
    global_tree_for_debugging = &p;
    cerr << "typeCount before init() = " << p.typeCount() << endl;

    // setup vec file
    string outfilename = argv[1];
    outfilename += ".vec";
    FILE * outfile = NULL;
    outfile = fopen(outfilename.c_str(), "w");
    if(outfile==NULL) {
      cerr << "Can't open file for writing vectors; skip: " << outfilename << endl;
      return 65;
    }
    TraGenMain t(&p, mergeTokens, mergeStride, mergeLists, outfile);
    t.run();
    fclose(outfile);
    global_tree_for_debugging = NULL;

    //root->printTok();
    //root->print();
    return 0;
}

