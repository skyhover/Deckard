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
#include <cstdio>
#include "tree-vector.h"

using namespace std;

#define VGDEBUG

/******************************************
 * Implementation for TreeVector
 *
 *****************************************/
TreeVector::
TreeVector(int len, int nl, const char * fn)
  : counters(len, 0), nLines(nl), filename(fn), minLine(0), maxLine(0),
    token_begin_id(-1), token_end_id(-1),
    node(NULL), name_counters(), ordered_names()
{
}

TreeVector::
TreeVector(ParseTree* rt)
  : counters(rt->typeCount(), 0), filename(rt->filename.c_str()), minLine(0), maxLine(0),
    token_begin_id(-1), token_end_id(-1),
    node(NULL), name_counters(), ordered_names()
{
  nLines = rt->getRoot()->max; // casued seg faults: - rt->getRoot()->min + 1;
}

TreeVector::
TreeVector(const TreeVector & cp)
  : counters(cp.counters), nLines(cp.nLines), filename(cp.filename), minLine(cp.minLine), maxLine(cp.maxLine),
    token_begin_id(cp.token_begin_id), token_end_id(cp.token_end_id),
    node(cp.node), name_counters(cp.name_counters), ordered_names(cp.ordered_names)
{
}

void TreeVector::
clearVector()
{
  // clear node counts:
  for (int i=0; i<counters.size(); i++)
    counters[i] = 0;

  // TODO: shall we also clear the filename? seems not.

  // clear line counts:
//   lines.clear();
  minLine = 0; maxLine = 0;
  token_begin_id = -1; token_end_id = -1;

  // clear name counts:
  name_counters.clear();
  ordered_names.clear();
}

bool TreeVector::
isFromSameFile(const TreeVector & cv)
{
  if ( this->filename==cv.filename )
    return true;
  else if ( this->filename==NULL || cv.filename==NULL )
    return false;
  else if ( strcmp(this->filename, cv.filename)==0 )
    return true;
  else
    return false;
}

bool TreeVector::
increaseCounters(Tree* n)
{
  if ( n->type<0 || n->type>=counters.size() )
    return false;

  // increase node counts:
  counters[n->type] += 1;

  // update token range:
  map<NodeAttributeName_t, void*>::iterator attr_itr = n->attributes.find(NODE_TOKEN_ID);
  assert( attr_itr!=n->attributes.end() );
  if ( token_begin_id<0 || ( ((pair<long, long>*)(*attr_itr).second)->first>=0 && ((pair<long, long>*)(*attr_itr).second)->first<token_begin_id ) )
    token_begin_id = ((pair<long, long>*)(*attr_itr).second)->first;
  if ( token_end_id<0 || ((pair<long, long>*)(*attr_itr).second)->second>token_end_id )
    token_end_id = ((pair<long, long>*)(*attr_itr).second)->second;

  if ( n->isTerminal()==true ) {
    // update line counts:
    Terminal* tn = n->toTerminal();
//     lines.insert(tn->line);
    if ( minLine<=0 || (tn->line>0 && tn->line<minLine) )
      minLine = tn->line;
    if ( maxLine<=0 || tn->line>maxLine )
      maxLine = tn->line;

    // increase name counts: TODO, not all names should be recorded...cf. increaseVecCounters in TraGenConfiguration.
    map<string, int>::iterator id = name_counters.find(*(tn->value));
    if ( id==name_counters.end() )
      name_counters[*(tn->value)] = 1;
    else
      (*id).second += 1;
    ordered_names.push_back(tn->value);
  }

  return true;
}

TreeVector TreeVector::
operator+(const TreeVector & cv)
{
  TreeVector rsl(counters.size(), nLines, filename);

  // update token range:
  if ( token_begin_id<0 )
    rsl.token_begin_id = cv.token_begin_id;
  else if ( cv.token_begin_id<0 )
    rsl.token_begin_id = token_begin_id;
  else
    rsl.token_begin_id = min(token_begin_id, cv.token_begin_id);
  rsl.token_end_id = max(token_end_id, cv.token_end_id);

  // merge line ranges:
  if ( isFromSameFile(cv)==false ) {
    // Meaningless maybe. TODO: ignore such cases...
    assert ( false ); // because set<int> lines doesn't support lines from different files...
    string n1(this->filename);
    string n2(cv.filename);
    string n3 = n1+n2;
    rsl.filename = stringclone(n3.c_str());

    rsl.nLines = nLines + cv.nLines;
  } else {
    // assert (rsl.nLines == cv.nLines);
//     set_union(lines.begin(), lines.end(), cv.lines.begin(), cv.lines.end(), inserter(rsl.lines, rsl.lines.begin()));
    if ( minLine<=0 )
      rsl.minLine = cv.minLine;
    else if ( cv.minLine<=0 )
      rsl.minLine = minLine;
    else
      rsl.minLine = min(minLine, cv.minLine);
    rsl.maxLine = max(maxLine, cv.maxLine);
  }

  // merge node vectors:
  for(int i=0; i<counters.size(); i++)
    rsl.counters[i] = counters[i] + cv.counters[i];

  // merge names:
  for (map<string, int>::const_iterator id2 = cv.name_counters.begin();
       id2!=cv.name_counters.end(); ++id2) {
    map<string, int>::iterator id1 = name_counters.find((*id2).first);
    // assert ( (*id1).first==(*id2).first );
    if ( id1==name_counters.end() )
      rsl.name_counters[(*id2).first] = (*id2).second;
    else
      rsl.name_counters[(*id1).first] = (*id1).second + (*id2).second;
  }
  // TODO: the order may not be right at all cases.
  rsl.ordered_names = ordered_names;
  rsl.ordered_names.insert(rsl.ordered_names.end(), cv.ordered_names.begin(), cv.ordered_names.end());

  return rsl;
}

TreeVector & TreeVector::
operator+=(const TreeVector & cv)
{
  // keep the "node" from *this*.

  // update token range:
  if ( token_begin_id<0 )
    token_begin_id = cv.token_begin_id;
  else if ( cv.token_begin_id<0 )
    ;
  else
    token_begin_id = min(token_begin_id, cv.token_begin_id);
  token_end_id = max(token_end_id, cv.token_end_id);

  // merge line ranges:
  if ( isFromSameFile(cv)==false ) {
    // Meaningless maybe. TODO: ignore such cases...
    assert (false);
    string n1(this->filename);
    string n2(cv.filename);
    string n3 = n1+n2;
    filename = stringclone(n3.c_str());

    nLines += cv.nLines;
  } else {
    // assert (rsl.nLines == cv.nLines);
//     set<int> newlines;
//     set_union(lines.begin(), lines.end(), cv.lines.begin(), cv.lines.end(), inserter(newlines, newlines.begin()));
//     lines = newlines;
    if ( minLine<=0 )
      minLine = cv.minLine;
    else if ( cv.minLine<=0 )
      ;
    else
      minLine = min(minLine, cv.minLine);
    maxLine = max(maxLine, cv.maxLine);
  }

  // merge node vectors:
  for (int i=0; i<counters.size(); i++)
    counters[i] += cv.counters[i];

  // merge names:
  for (map<string, int>::const_iterator id2 = cv.name_counters.begin();
       id2!=cv.name_counters.end(); ++id2) {
    map<string, int>::iterator id1 = name_counters.find((*id2).first);
    if ( id1==name_counters.end() )
      name_counters[(*id2).first] = (*id2).second;
    else
      (*id1).second = (*id1).second + (*id2).second;
  }
  // TODO: the order may not be right at all cases.
  ordered_names.insert(ordered_names.end(), cv.ordered_names.begin(), cv.ordered_names.end());

  return *this;
}

TreeVector & TreeVector::
operator=(const TreeVector & cv)
{
  // copy line counts:
  filename = cv.filename;
  nLines = cv.nLines;
  // copy line ranges:
//   lines = cv.lines;
  minLine = cv.minLine;
  maxLine = cv.maxLine;

  // copy token range:
  token_begin_id = cv.token_begin_id;
  token_end_id = cv.token_end_id;

  node = cv.node;
  // copy node counts:
  for (int i=0; i<counters.size(); i++)
    counters[i] = cv.counters[i];

  // copy name counts:
  name_counters = cv.name_counters;
  ordered_names = cv.ordered_names;

  return *this;
}

int TreeVector::
nNodesContained()
{
  int nn = 0;
  for (int i=0; i<counters.size(); i++)
    nn += counters[i];

  return nn;
}

bool TreeVector::
output(FILE * buf)
{
  int minlineno = minLineContained();
  // output iff the vector is from some real file and contains more than one lines.
#ifdef nooutputforemptylines
  if ( minlineno<=0 )
    fprintf(stdout, "==From file `%s', minlineno:%d, total lines:%d (line range=%d), \n", filename, minlineno, nLines, nLinesContained());
  if ( 1 ) {
#else
  if ( filename!=NULL /*&& strcasecmp(strrchr(filename, '.'), ".c")==0*/ && minlineno>0 ) {
#endif
    // output additional info:
    fprintf (buf, "# FILE:%s, ", filename);
    fprintf (buf, "LINE:%d, ", minlineno);
    fprintf (buf, "OFFSET:%d, ", maxLineContained());
    // TODO:
    fprintf (buf, "NODE_KIND:%d, ", node==NULL ? 0 : node->type);
    fprintf (buf, "CONTEXT_KIND:%d, ", 0);
    fprintf (buf, "NEIGHBOR_KIND:%d, ", 0);

    fprintf (buf, "NUM_NODE:%d, ", nNodesContained());
    // TODO
    fprintf (buf, "NUM_DECL:0, NUM_STMT:0, NUM_EXPR:0, ");
    // output token range:
    fprintf(buf, "TBID:%d, TEID:%d, ", token_begin_id, token_end_id);
#define outputidentifiersforbugdetection
#ifdef outputidentifiersforbugdetection
    // output identifiers for bug detection
    fprintf (buf, "VARs:{");
#ifdef outputeachidentifier
    for (map<string, int>::iterator vars = name_counters.begin();
	 vars!=name_counters.end(); ++vars) {
      fprintf (buf, "%s:%d, ", (*vars).first.c_str(), (*vars).second);
    }
#endif
    fprintf (buf, "}%d, ", nDiffNamesContained());
#ifdef outputorderedidentifiers
    // output ordered identifiers for bug detection
    fprintf (buf, "OIDs:{");
    for (list<string*>::iterator oids = ordered_names.begin();
	 oids!=ordered_names.end(); ++oids) {
      fprintf (buf, "%s, ", (*oids)->c_str());
    }
    fprintf (buf, "}%d", nNamesContained());
#endif
#endif
    fprintf (buf, "\n");

    // output the vector itself:
    for (int i=0; i<counters.size(); i++)
      fprintf(buf, "%d ", counters[i]);
    fprintf(buf, "\n");

    return true;
  }

    return false;
}

inline int TreeVector::
nLinesContained()
{
//   return lines.size();
  if ( minLine==0 || maxLine==0 )
    return 0;
  else
    return maxLine-minLine+1;
}

inline int TreeVector::
minLineContained()
{
//   if ( lines.empty()==true )
//     return 0;
//   else
//     return *(lines.begin());	// set is in ascending order
  return minLine;
}

inline int TreeVector::
maxLineContained()
{
//   if ( lines.empty()==true )
//     return 0;
//   else
//     return *(lines.rbegin());	// reverse set is in descending order
  return maxLine;
}

inline pair<long, long> TreeVector::
tokenRange()
{
  return pair<long, long>(token_begin_id, token_end_id);
}

inline int TreeVector::
nDiffNamesContained()
{
  return name_counters.size();
}

inline int TreeVector::
nNamesContained()
{
  return ordered_names.size();
}

