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
#include <map>
#include <string>
#include <string.h>
#include <algorithm>
#include <cctype>
#include <ptree.h>

using namespace std;

map<string,int> name2id;
map<int,string> id2name;
#ifdef JAVA
string identifierTypeName = "ID_TK";
#else
#ifdef PHP
string identifierTypeName = "T_VARIABLE";
#else
#ifdef SOLIDITY
string identifierTypeName = "IDENTIFIER";
#else
string identifierTypeName = "IDENTIFIER";
#endif
#endif
#endif

/** get input of type T from a string */
template <class T> bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}

int main( int argc, char **argv )
{
  if ( argc<4 ) {
    cerr << "Usage: " << argv[0] << " <filename> <start token id> <end token id> [contextual node level [override files]]" << endl;
    return 1;
  }

  FILE * fin = NULL;
  if ( (fin=fopen(argv[1], "r"))==NULL ) {
    cerr << "Can't open file: " << argv[1] << endl;
    return 2;
  }
  fclose(fin);
  fin = NULL;

#ifdef JAVA
  const char * file_suffix = ".java";
#else
#ifdef PHP
  const char * file_suffix = ".php";
#else
#ifdef SOLIDITY
  const char * file_suffix = ".sol";
#else
  const char * file_suffix = ".c";
#endif
#endif
#endif
  string fn = string(argv[1]);
  unsigned int sn = fn.rfind(file_suffix);
  if ( sn==string::npos || sn+strlen(file_suffix)<fn.length() ) {
    cerr << "Warning: file suffix error. Should be '" << file_suffix << "'. Continue anyway..." << endl;
  }

  long tbid, teid;
  if(!from_string(tbid, string(argv[2]), dec) || tbid<0 ) {
    cerr << "Error: start token id incorrect: " << argv[2] << "-->" << tbid << endl;
    return 1;
  }
  if(!from_string(teid, string(argv[3]), dec) || teid<=0 ) {
    cerr << "Error: end token id incorrect: " << argv[3] << "-->" << teid << endl;
    return 1;
  }
  
  long contextlevel = 0;  // 0 means no context 
  if( argc>=5 ) {
    if( !from_string(contextlevel, string(argv[4]), dec) || contextlevel<0 ) {
      cerr << "Error: context level incorrect: " << argv[4] << "-->" << contextlevel << endl;
      return 1;
    } else if ( contextlevel>1 ) {
        cerr << "Warning: only 1-level context is valid for now" << endl;
	contextlevel = 1;
    }
  }

  id_init();

  ParseTree* pt = parseFile(argv[1]);
  if ( pt==NULL ) {
    cerr << "Error: no parse tree created for file: " << argv[1] << endl;
    return 1;
  }

  if(argc>=6) {
    pt->dumpParseTree(NULL, true);  // to override existing file
  } else {
    pt->dumpParseTree(NULL, false);
  }

  Tree* node = pt->tokenRange2Tree(tbid, teid);
  long i = pt->tree2sn(node);
  if(i<=0) {
    cerr << "Warning: incorrect tree node order number: " << endl;
  }
  cout << pt->filename << " " << tbid << " " << teid << " " << i;
  if( contextlevel>0 ) {
    Tree* contextnode = pt->getContextualNode(node);
    long ci = pt->tree2sn(contextnode);
    cout << " " << ci;
  }
  cout << endl;

  return 0;
}

