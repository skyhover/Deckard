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
#include <ptree.h>
#include <map>
#include <string>
#include <token-tree-map.h>
#include <algorithm>
#include <cctype>

using namespace std;

// These are not really used by "out2html"; just to resolve dependancy.
map<string,int> name2id;
map<int,string> id2name;
string identifierTypeName = "no need for such name";

/* read in a file line-by-line:
   if the line is a valid clone point, transform it to html;
   otherwise, simply output the line. */
int main( int argc, char **argv )
{
  if ( argc!=2 ) {
    cerr << "usage: [a clone cluster file]" << endl;
    return 1;
  }

  id_init();
  TokenTreeMap::init_shared_data();
  ClonePointT tt;
  ifstream inf(argv[1], ios::in);
  if ( ! inf.is_open() ) {
    cerr << "Can't open file: " << argv[1] << endl;
    return 1;
  }

  string line;
  int linecount = 0;

  while ( !inf.eof() ) {
    getline(inf, line);
    linecount++;
    char * charline = new char[line.length()+1];
    strcpy(charline, line.c_str());
    if ( strcmp(charline, "")==0 || strncmp(charline, "\n", 1)==0 )
      cout << "<p></p>" << endl << "<br />" << endl;
    else if ( tt.parse(charline, TokenTreeMap::clone_patterns) ) {
      tt.out2html0(cout);
    } else
      cout << tt << "<br /> " << endl;
    delete charline;
  }
  inf.close();

  return 0;
}

