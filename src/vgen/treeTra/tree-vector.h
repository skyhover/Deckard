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
#ifndef _TREE_VECTOR_H_
#define _TREE_VECTOR_H_

#include <vector>
#include <list>
#include <string>
#include <map>
#include <set>
#include "../../include/ptree.h"
#include "vgen-utils.h"

class TreeVector {
 public:
  std::vector<int> counters;	/* of length `len' */

  /* TODO: these data fields are only good for intra-file vectors for now... */
  const char * filename;	/* the name of the src file; (partially) use singleton to save mem. */
  int nLines;			/* the total number of lines in "filename". */
/*   std::set<int> lines;		/\* use this instead of std::vector<boo> to improve performance. *\/ */
  int minLine, maxLine;		/* use these instead of set<int> to save memory; increase imprecision though. */
  long token_begin_id, token_end_id; /* *estimate* the (inclusive) range of tokens (from the source code) which the vector contains. Initialized to -1 (invalid). */
  Tree* node; /* pointer to the node from which the vector is generated; not always meaningful when merging vectors. */

  /* record identifiers. */
  //  std::map<std::string*, int> name_counters; /* Use string*, instead of strng, to save memory.  Also consistent with ptree.h where string* is used for identifiers. But extra careful should be employed when looking for certain strings (even if they are literally equals, they may be treated as different strings because of pointers. So, it's easier to use "string" directly: */
  std::map<std::string, int> name_counters;
  std::list<std::string*> ordered_names; /* ordered names. It's fine to use string pointers here, because we want to treat literally equivalent strings as different things. */

  TreeVector(int len, int nl=0, const char *fn=NULL);
  TreeVector(ParseTree* rt);
  TreeVector(const TreeVector & cp);

  void clearVector();
  bool isFromSameFile(const TreeVector & cv);

  bool increaseCounters(Tree* n);

  TreeVector operator+(const TreeVector & cv);
  TreeVector & operator+=(const TreeVector & cv);
  TreeVector & operator=(const TreeVector & cv);

  int nNodesContained();	/* the total number of nodes counted in the vector. */

  bool output(FILE * out);

  int nLinesContained();	/* the total number of lines contained in the vector. */
  int minLineContained();	/* the minimal line number. */
  int maxLineContained();	/* the maximal line number. */
  std::pair<long, long> tokenRange(); /* the token range */
  int nodeType();		/* the node type for this vector. TODO */

  int nDiffNamesContained();    /* the number of different names counted in the vector. */
  int nNamesContained();	/* the total number of names counted. */
};


#endif	/* _TREE_VECTOR_H_ */
