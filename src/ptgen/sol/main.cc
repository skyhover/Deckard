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
#include <ptree.h>
#include <map>
#include <string>
#include "../../vgen/treeTra/vgen-utils.h"

using namespace std;

map<string,int> name2id;
map<int,string> id2name;

string identifierTypeName = "IDENTIFIER";

int yyparse();

extern Tree *root;

void id_init();

int last_line= 0;

/* A test driver for the solidity parser.
 * Take redirected input from a file via '<'
 * argv[i]=="trees" and/or "tokens" to print trees and/or tokens; case insensitive. 
 */
int main(int argc, char *argv[])
{
    id_init();
    yyparse();
    if (!root) {
        cerr << "failed to parse file" << endl;
        return 1;
    }

	bool printTokens = false;
	bool printTrees = false;
	
	for(int i=1; i<argc; i++) {
		if (compare_string_nocase("trees",argv[i])==0)
			printTrees = true;
		else if (compare_string_nocase("tokens",argv[i])==0)
			printTokens = true;
		else { // report wrong argument
			cerr << "wrong argument: " << argv[i] << endl;
			return 2;
		}
	}
	// print tokens if no parameters are set
	if (!printTokens && !printTrees)
		printTokens = true;

    if (printTokens)
		root->printTok();
    if (printTrees)
		root->print();

//    int c= root->countTerminals();

//    cout << c << endl;

    return 0;
}

