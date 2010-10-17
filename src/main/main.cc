/*
 * 
 * Copyright (c) 2007-2010,
 *   Lingxiao Jiang         <lxjiang@ucdavis.edu>
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
#include <getopt.h>

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

static const char *relNodes[] = {
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

static const char *atomicNodes[] = {
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

static const char *valParents[] = {
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


void initNodes( vector<int> & nodes, const char **nodeconfig)
{
    for (const char **s= nodeconfig; *s != NULL; s++) {
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
    const char * inputfilename = NULL;
    const char * outputfilename = NULL;
    const char * configfilename = NULL;
    int mergeTokens = 30, mergeStride = 1, mergeLists = 3;
    int startline = 0, endline = -1; /* default: all lines */

    // use getopt_long; should also work under cygwin
    static const struct option longOpts[] = {
        { "input-file", required_argument, NULL, 'i' },
        { "output-file", required_argument, NULL, 'o' },
        { "config-file", required_argument, NULL, 'c' },
        { "mim-token-number", required_argument, NULL, 'm' },
        { "stride", required_argument, NULL, 't' },
        { "merge-list-size", required_argument, NULL, 0 },
        { "start-line-number", required_argument, NULL, 0 },
        { "end-line-number", required_argument, NULL, 0 },
        { "help", no_argument, NULL, 'h'},
        { NULL, no_argument, NULL, 0 }
    };

    int c = 0, longIndex = 0;
    while ((c = getopt_long (argc, argv, "i:o:c:m:t:h", longOpts, &longIndex)) != -1) {
      switch (c) {
      case 'i':
        inputfilename = optarg;
        break;
      case 'o':
        outputfilename = optarg;
        break;
      case 'f':
        configfilename = optarg;
        TraGenMain::getParameters(configfilename, mergeTokens, mergeStride, mergeLists);
        break;
      case 'm':
        if ( sscanf(optarg, "%d", &mergeTokens)<=0 ) {
          cerr << "Warning: invalid mergeTokens in option -m (default 30): " << optarg << endl;
          mergeTokens = 30;
        }
        break;
      case 't':
        if ( sscanf(optarg, "%d", &mergeStride)<=0 ) {
          cerr << "Warning: invalid mergeStride in option -t (default 1): " << optarg << endl;
          mergeStride = 1;
        }
        break;
      case 0: /* for long options w/o a short name */
        if( stricmp( "merge-list-size", longOpts[longIndex].name ) == 0 ) {
          if ( sscanf(optarg, "%d", &mergeLists)<=0 ) {
            cerr << "Warning: invalid mergeLists in option --merge-list-size (default 3): " << optarg << endl;
            mergeLists = 3;
          }
        } else if ( stricmp( "start-line-number", longOpts[longIndex].name ) == 0 ) {
          if ( sscanf(optarg, "%d", &startline)<=0 ) {
            cerr << "Warning: invalid startline in option --start-line-number (default: all lines): " << optarg << endl;
            startline = 0;
          }
        } else if ( stricmp( "end-line-number", longOpts[longIndex].name ) == 0 ) {
          if ( sscanf(optarg, "%d", &endline)<=0 ) {
            cerr << "Warning: invalid endline in option --end-line-number (default: same as --start-line-number): " << optarg << endl;
            endline = -1;
          }
        }
        break;
      case 'h':
      case '?':
        /* getopt_long should have already printed an error message. */
        cerr << "Usage: " << argv[0] << " [options] [filename] " << endl;
        cerr << "--input-file <source file name>, or -i " << endl;
        cerr << "--output-file <vector file name>, or -o (default: source name plus '.vec')" << endl;
        cerr << "--config-file <parameter file name>, or -c (default: not use)" << endl;
        cerr << "--mim-token-number <number>, or -m (default: 30)" << endl;
        cerr << "--stride <number>, or -t (default: 1)" << endl;
        cerr << "--merge-list-size <number> (default: not used)" << endl;
        cerr << "--start-line-number <number> (default: all lines)" << endl;
        cerr << "--end-line-number <number> (default: same as start-line-number)" << endl;
        cerr << "--help or -h" << endl;
        cerr << "Later options can override previous options. The actual overriding order is undefined; pls avoid specifying the same parameter twice." << endl;
        return 0;
      default:
        /* unreachable */;
        abort();
      }
    }

    // additional processing of options:
    if(endline<0)
      endline = startline;

    // more vec gen parameters (for backward compatibility only)
    if (optind < argc) {
       cerr << "Warning: unparsed options may be used to overload the parameters: [src file] [config-file | [min-tokens][stride][merge-list]]" << endl;
       inputfilename = argv[optind++];
       if (optind < argc)
         configfilename = argv[optind++];
       if (optind < argc) {
         configfilename = NULL;
         optind--;
         if ( sscanf(argv[optind], "%d", &mergeTokens)<=0 ) {
           cerr << "Error: invalid mergeTokens: " << argv[optind] << endl;
           exit(1);
         }
         optind++;
       } else {
         TraGenMain::getParameters(configfilename, mergeTokens, mergeStride, mergeLists);
       }
       if (optind < argc) {
        if ( sscanf(argv[optind], "%d", &mergeStride)<=0 ) {
          cerr << "Error: invalid mergeStride: " << argv[optind] << endl;
          exit(1);
        }
        optind++;
       }
       if (optind < argc) {
         if ( sscanf(argv[optind], "%d", &mergeLists)<=0 ) {
          cerr << "Error: invalid mergeLists: " << argv[optind] << endl;
          exit(1);
         }
         optind++;
       }
    }
    cerr << "Merging parameters: " << mergeTokens << ", " << mergeStride << ", " << mergeLists << endl;

    // parse the input file
    yyin= fopen(inputfilename,"r");
    if (!yyin) {
        cerr << "invalid filename: " << inputfilename << endl;
        return 1;
    }
    id_init();
    yyparse();
    if (!root) {
        cerr << "failed to parse file: " << inputfilename << endl;
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

    p.filename = inputfilename;

    // for debugging use only in vector generator.
    global_tree_for_debugging = &p;
    cerr << "typeCount before init() = " << p.typeCount() << endl;

    // setup vec file
    string outfilestring = outputfilename==NULL ? string(inputfilename) + ".vec" : outputfilename;
    FILE * outfile = NULL;
    outfile = fopen(outfilestring.c_str(), "w");
    if(outfile==NULL) {
      cerr << "Warning: Can't open file for writing vectors; skip: " << outfilestring << endl;
      return 65;
    }
    TraGenMain t(&p, mergeTokens, mergeStride, mergeLists, outfile);
    t.run(startline, endline);
    fclose(outfile);
    global_tree_for_debugging = NULL;

    //root->printTok();
    //root->print();
    return 0;
}

