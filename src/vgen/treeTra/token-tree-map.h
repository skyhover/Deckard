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
#ifndef _TOKEN_TREE_MAP_H_
#define _TOKEN_TREE_MAP_H_

#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <regex.h>
#include "../../include/ptree.h"
#include "vgen-config.h"
#include "token-counter.h"

extern std::map<std::string, int> name2id;
extern std::map<int, std::string> id2name; /*  */
extern Tree *root;
extern FILE *yyin;

void yyrestart( FILE *new_file ); /* switch input file for flex. */
int yyparse();
void id_init();

typedef enum {
  ENUM_CLONE_INDEX, ENUM_CLONE_DIST, ENUM_CLONE_FILE, ENUM_CLONE_LINE, ENUM_CLONE_OFFSET,
  ENUM_CLONE_TBID, ENUM_CLONE_TEID, ENUM_CLONE_NODE_KIND, ENUM_CLONE_NODE_NUM, ENUM_CLONE_nVARs,
  ENUM_CLONE_THE_END
} pattern_enum_t;

typedef struct _ClonePointT {
  std::string linebuf;
  long index;			/* the ID of the vector. */
  float dist;
  std::string filename;
  long begin_line_id, line_offset;
  long tbid, teid;
  int node_kind, node_number;
  int uni_var_number;

  bool parse(char * line, regex_t patterns[], int dim=ENUM_CLONE_THE_END);
  std::ostream & out2html(std::ostream & os);
  std::ostream & out2html0(std::ostream & os);
  std::ostream & out2xml(std::ostream & os);
  friend std::ostream & operator<< (std::ostream& os, const struct _ClonePointT & cp);
} ClonePointT, *PClonePointT;
typedef std::vector<ClonePointT*> ClonePointGroupT, *PClonePointGroupT;	/* Q: why can't use "ClonePointT&"? */

typedef enum {
  ENUM_RANK_NOTHING = 0,
  ENUM_RANK_CXT_NODE = 1,
  ENUM_RANK_CXT_COND = 2,
  ENUM_RANK_nVARS = 4,
  ENUM_RANK_THE_END = 8
} bug_rank_enum_t;

typedef struct _CloneContextT {
  /* Definitions of "context":
     1. the smallest common ancestor + one node (or a path of node) above;
     2. the smallest common ancestor which is a "if"/"while"/"switch", etc
   */
  Tree * context_node_begin;
  Tree * context_node_end;
} CloneContextT, *PCloneContextT;

class TokenTreeMap {
 public:
  static regex_t clone_patterns[ENUM_CLONE_THE_END];
  static bool init_shared_data();

  std::map<std::string, ParseTree*> fn2tree; /* map finenames to parse tree roots. */
  std::vector<ClonePointT> clusterbuffer; /* outputs for a clone cluster from LSH. */
  unsigned int rank;		/* the higher, more buggy */
#define NUM_BUGGY_SCORES 5
  int buggy_score[NUM_BUGGY_SCORES]; /* the higher, more buggy */
  /* buggy_score[0]: context types
     buggy_score[1]: nVARs difference
     buggy_score[2]: location distance
     buggy_score[3]: cond. in loop.
     buggy_score[4]: unused
   */

  TraGenConfiguration * vecGen_config; /* only used as a dummy for TokenRangeCounter. */
  std::vector<bool> contexualNodes; /* contexualNodes[i]==true iff the node kind is considered as contexts */
  TokenRangeCounter * token_range_counter;
 public:
  TokenTreeMap();		/* all nodes are contexual by default. */
  ~TokenTreeMap();
  std::ostream & outputCluster(std::ostream & out);

  bool initNodes(const char ** nodeconfig);
  ParseTree* parseFile(const char * fn);
  // use #define instead before we have a good general way of handling this:  bool parseClonePointAux(char * line, PCloneContextT pcp, const regex_t *preg, size_t nmatch, regmatch_t pmatch[], int eflags);
  bool parseClonePoint(char * line, PClonePointT pcp); /* parse a line from the clone report into ClonePointT */
  virtual bool createFN2Tree(); /* get each filename, parse the file, assign ID to each token, save . */
  void clearMap(); /* in order to save memory; called manually some times by users. */
  virtual bool isContextual(Tree* node);
 private:
  void clearBuffer();		/* automatically called in createFN2Tree to save memory and seperate a clone cluster from the previous one. */
  Tree* tokenRange2TreeAux1(std::pair<long, long> tokenrange, Tree* node);
  bool root2TokenAux(long tid, Tree* node, std::list<Tree*>&);
 public:
  Tree* tokenRange2Tree1(std::pair<long, long> tokenrange, ParseTree* pt); /* return the smallest common ancestor, but a bug in it - TODO. */
  Tree* tokenRange2Tree2(std::pair<long, long> tokenrange, ParseTree* pt); /* return the smallest common ancestor, different alg, OK. */
  std::list<Tree*>* root2Token(long tid, ParseTree* pt); /* return the path from the root to the terminal. */

  /* NOTE: "pointer-to-member" function has a different type from normal C function pointer. cf. C++ FAQ LITE, Marshall Cline.
     better to use "functionoid"!
   */
  typedef CloneContextT (TokenTreeMap::*GetContextFuncT)(std::pair<long, long>, std::string &);
  typedef bool (TokenTreeMap::*CompareContextFuncT)(CloneContextT&, CloneContextT&);
  typedef bool (TokenTreeMap::*ClusterFilterT)();

  virtual CloneContextT getContext1(std::pair<long, long> tokenrange, std::string & fn);	/* see the definitions of "context" above. */
  virtual CloneContextT getContext2(std::pair<long, long> tokenrange, std::string & fn);
  virtual Tree* getContextNode(Tree* node); /* get the highest node which represents the context type of "node"; language dependent */
  virtual Tree* getContextParent(Tree* node); /* get the contextual parent; return NULL if not found. */

  bool compareContext1(CloneContextT& context1, CloneContextT& context2); /* return true iff the contexts are the same. */
  bool compareCContext2(CloneContextT& context1, CloneContextT& context2); /* for C only */
  bool compareCConditions(CloneContextT& context1, CloneContextT& context2); /* for C only, compare conditions of "if" etc. */
  virtual Tree* get_conditional_operator(Tree* node); /* grammar-dependent */
  virtual bool isMainOperator(Tree * op1); /* define what are "first" "main" operators. grammar-dependent. */
  virtual Tree* get_condition_within(Tree* node); /* grammar-dependent */
  virtual bool compareConditionalOperators(CloneContextT& context1, CloneContextT& context2); /* compare the "first" "main" operators in conditions. */

  virtual bool isAnyFiltered(); /* check "clusterbuffer" to see whether it may be filtered by ANY filter. TODO: make it generic to be able to accept any filter.
			   May need a list of function pointers. */
  virtual bool isAllFiltered();	/* check "clusterbuffer" to see whether it may be filtered by ALL filter. */
#define isFilteredI(ttm, i, rslflag) /* check "clusterbuffer" to see whether it may be filtered by filter##i(). */ \
    {                        \
      rslflag = false;  \
      if ( ttm.clusterbuffer.size() < 1 ) { \
        cerr << "No clone cluster is in the buffer..." << endl; \
        rslflag = true; \
      } else { \
        rslflag = ttm.filter##i (); \
      } \
    }

  virtual bool filter1();
  virtual bool filter2();
  virtual bool filter3();
  virtual int buggy1();
  //  virtual int buggy2();
};

class TokenTreeMap_Java : public TokenTreeMap {
 public:
  typedef bool (TokenTreeMap_Java::*CompareContextFuncT)(CloneContextT&, CloneContextT&);

  virtual Tree* getContextNode(Tree* node); /* get the highest node which represents the context type of "node"; language dependent */

  bool compareJContext2(CloneContextT& context1, CloneContextT& context2); /* for Java only */
  bool compareJConditions(CloneContextT& context1, CloneContextT& context2); /* for Java only, compare conditions of "if" etc. */
  virtual Tree* get_conditional_operator(Tree* node); /* grammar-dependent */
  virtual bool isMainOperator(Tree* node); /* grammar-dependent */
  virtual Tree* get_condition_within(Tree* node); /* grammar-dependent */

  virtual bool filter1();
  virtual bool filter2();
  //  virtual bool filter3();
  virtual int buggy1();
};

#endif
