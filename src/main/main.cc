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
#include "jrelevantNodes.h"
#else
#ifdef PHP
#include "../ptgen/php5/phprelevantNodes.h"
#else
#include "crelevantNodes.h"
#endif
#endif
};

static char *atomicNodes[] = {
#ifdef JAVA
#include "jatomicNodes.h"
#else
#ifdef PHP
#include "../ptgen/php5/phpatomicNodes.h"
#else
#include "catomicNodes.h"
#endif
#endif
};

static char *valParents[] = {
#ifdef JAVA
#include "jparentNodes.h"
#else
#ifdef PHP
#include "../ptgen/php5/phpparentNodes.h"
#else
#include "cparentNodes.h"
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
        cerr << "usage: %s filename [config_file]" << endl
	     << "\tCurrent config_file contains three natural numbers: " << endl
	     << "\t\t(1) # tokens for merging; " << endl
	     << "\t\t(2) length of a stride; " << endl
	     << "\t\t(3) # lists for merging." << endl;
        return 1;
    }
    yyin= fopen(argv[1],"r");
    if (!yyin) {
        cerr << "invalid filename: %s" << argv[1] << endl;
    }
    id_init();
    yyparse();
    if (!root) {
        cerr << "failed to parse file" << endl;
        return 1;
    }

    root->lineRange();

#if 0
    cerr << "Terminals: " << root->countTerminals() << endl;
#endif

    ParseTree p(root,id2name.size(),&id2name,&name2id);

    initNodes(p.relevantNodes, relNodes);
    initNodes(p.leafNodes, atomicNodes);
    initNodes(p.validParents, valParents);
    //initNodes(p.mergeableNodes, mergeableNodes);

    p.filename= argv[1];

    string outfilename = argv[1];
    outfilename += ".vec";
    const char * configfilename = NULL;
    if ( argc>2 )
      configfilename = argv[2];

    // for debugging use only in vector generator.
    global_tree_for_debugging = &p;
    cerr << "typeCount before init() = " << p.typeCount() << endl;
    TraGenMain t(&p, configfilename, fopen(outfilename.c_str(), "w"));
    t.run();
    global_tree_for_debugging = NULL;

    //root->printTok();
    //root->print();
    return 0;
}

