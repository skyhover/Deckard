#ifndef _CLONE_CLUSTER_H_
#define _CLONE_CLUSTER_H_

#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <utility>
#include <cstdio>
#include <regex.h>
#include "../../include/ptree.h"
#include "../../vgen/treeTra/vgen-config.h"
#include "../../vgen/treeTra/token-counter.h"

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
  ostream & out2html(ostream & os);
  ostream & out2html0(ostream & os);
  friend ostream & operator<< (ostream& os, const struct _ClonePointT & cp);
} ClonePointT, *PClonePointT;

typedef enum {
  ENUM_RANK_NOTHING = 0,
  ENUM_RANK_CXT_NODE = 1,
  ENUM_RANK_CXT_COND = 2,
  ENUM_RANK_nVARS = 4,
  ENUM_RANK_THE_END = 8
} bug_rank_enum_t;

typedef struct _CloneClusterT_tag {
  unsigned int rank;		/* the higher, more buggy */
  int buggy_score;		/* the higher, more buggy */
  std::vector<PClonePointT>* cluster; /* Q: why can't use "ClonePointT&" as a template argument? */
} CloneClusterT, *PCloneClusterT;

typedef struct _CloneContextT {
  /* Definitions of "context":
     1. the smallest common ancestor + one node (or a path of node) above;
     2. the smallest common ancestor which is a "if"/"while"/"switch", etc
   */
  Tree * context_node_begin;
  Tree * context_node_end;
} CloneContextT, *PCloneContextT;

class CloneClusters {
 public:
  static regex_t clone_patterns[ENUM_CLONE_THE_END];
  static bool init_shared_data();

  std::map<std::string, ParseTree*> fn2tree; /* map finenames to parse tree roots. */
  std::vector<CloneClusterT> clusters; /* store all clone clusters */
  std::vector<PClonePointT>* clusterbuffer; /* store a clone cluster (tempararily). */
  TraGenConfiguration * vecGen_config; /* only used as a dummy for TokenRangeCounter. */
  std::vector<bool> contexualNodes; /* contexualNodes[i]==true iff the node kind is considered as contexts */
  TokenRangeCounter * token_range_counter;
 public:
  CloneClusters();		/* all nodes are contexual by default. */
  ~CloneClusters();
  bool initNodes(const char ** nodeconfig);
  ParseTree* parseFile(const char * fn);
  // use #define instead before we have a good general way of handling this:  bool parseClonePointAux(char * line, PCloneContextT pcp, const regex_t *preg, size_t nmatch, regmatch_t pmatch[], int eflags);
  bool parseClonePoint(char * line, PClonePointT pcp); /* parse a line from the clone report into ClonePointT */
  virtual int createOneCluster(FILE * fin); /* get each filename (from stdin), parse the file, assign ID to each token, save. Input clusters are separated by empty lines.
					       return -1 when reaching the end of the input file, otherwise return the number of clone points in the buffer. */
  virtual int createAllClusters(FILE * fin); /* return the number of clusters. */
  bool setOneRank(CloneClusterT & cls);
  virtual int setAllRanks();
  template<class StrictWeakOrdering>int sortClusters(StrictWeakOrdering comp);
  bool outputOneCluster(ostream & out, CloneClusterT & cls);
  int outputAllClusters(ostream & out);

  void clearMap(); /* in order to save memory; called manually some times by users. */
  bool isContextual(Tree* node);
 private:
  void clearBuffer(); /* automatically called in createClusters to save memory and seperate a clone cluster from the previous one. */
  Tree* tokenRange2TreeAux1(std::pair<long, long> tokenrange, Tree* node);
  bool root2TokenAux(long tid, Tree* node, std::list<Tree*>&);
 public:
  Tree* tokenRange2Tree1(std::pair<long, long> tokenrange, ParseTree* pt); /* return the smallest common ancestor, but a bug in it - TODO. */
  Tree* tokenRange2Tree2(std::pair<long, long> tokenrange, ParseTree* pt); /* return the smallest common ancestor, different alg, OK. */
  std::list<Tree*>* root2Token(long tid, ParseTree* pt); /* return the path from the root to the terminal. */

  /* NOTE: "pointer-to-member" function has a different type from normal C function pointer. cf. C++ FAQ LITE, Marshall Cline.
     better to use "functionoid"!
   */
  typedef CloneContextT (CloneClusters::*GetContextFuncT)(std::pair<long, long>, std::string &);
  typedef bool (CloneClusters::*CompareContextFuncT)(CloneContextT&, CloneContextT&);
  typedef bool (CloneClusters::*ClusterFilterT)();

  CloneContextT getContext1(std::pair<long, long> tokenrange, std::string & fn);	/* see the definitions of "context" above. */
  CloneContextT getContext2(std::pair<long, long> tokenrange, std::string & fn);

  bool compareContext1(CloneContextT& context1, CloneContextT& context2); /* return true iff the contexts are the same. */
  bool compareCContext2(CloneContextT& context1, CloneContextT& context2); /* for C only */
  bool compareCConditions(CloneContextT& context1, CloneContextT& context2); /* for C only, compare conditions of "if" etc. */


  virtual int rank1(CloneClusterT & cls);
  virtual int rank2(CloneClusterT & cls);
  virtual int rank3(CloneClusterT & cls);
  virtual int buggy1(CloneClusterT & cls);
};

class CloneClusters_Java : public CloneClusters {
 public:
  typedef bool (CloneClusters_Java::*CompareContextFuncT)(CloneContextT&, CloneContextT&);
  bool compareJContext2(CloneContextT& context1, CloneContextT& context2); /* for Java only */
  bool compareJConditions(CloneContextT& context1, CloneContextT& context2); /* for Java only, compare conditions of "if" etc. */

  virtual int rank1(CloneClusterT & cls);
  virtual int rank2(CloneClusterT & cls);
  // same:  virtual int rank3(PCloneClusterT cls);
  virtual int buggy1(CloneClusterT & cls);
};

#endif
