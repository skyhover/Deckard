#include <iostream>
#include <fstream>
#include "tra-gen.h"
#include "tree-accessor.h"

using namespace std;

#define VGDEBUG

/******************************************
 * Implementation for TraGenMain
 *
 *****************************************/
TraGenMain::
TraGenMain(ParseTree* rt, const char * fn, FILE * out)
  : parse_tree(rt), vecGen_config(NULL), token_merger(NULL), list_merger(NULL),
    vecGen_outfile(out), token_counter(NULL), token_range_counter(NULL),
    tree_serializer(NULL), vec_generator(NULL), vec_outputor(NULL)
{
  int mergeTokens = 30, mergeStride = 1, mergeLists = 3;
  getParameters(fn, mergeTokens, mergeStride, mergeLists);

  vecGen_config = new TraGenConfiguration(rt, mergeTokens, mergeStride, mergeLists); 
  token_counter = new TokenCounter(*vecGen_config); // NOT depend on vecGen_config->mergeTokens/moveStride.
  token_range_counter = new TokenRangeCounter(*vecGen_config); // NOT depend on vecGen_config->mergeTokens/moveStride.
  tree_serializer = new RelevantNoAtomicParent_TreeSerializer(*vecGen_config); // NOT depend on vecGen_config->mergeTokens/moveStride.
  vec_generator = new VecGenerator(*vecGen_config); // NOT depend on vecGen_config->mergeTokens/moveStride.
  vec_outputor = new TraVecOutput(*vecGen_config, vecGen_outfile); // DO depend on vecGen_config->mergeTokens, NOT on moveStride.
}

TraGenMain::
TraGenMain(ParseTree* rt, int mergeTokens, int mergeStride, int mergeLists, FILE * out)
  : parse_tree(rt), vecGen_config(NULL), token_merger(NULL), list_merger(NULL),
    vecGen_outfile(out), token_counter(NULL), token_range_counter(NULL),
    tree_serializer(NULL), vec_generator(NULL), vec_outputor(NULL)
{
  vecGen_config = new TraGenConfiguration(rt, mergeTokens, mergeStride, mergeLists); 
  token_counter = new TokenCounter(*vecGen_config); // NOT depend on vecGen_config->mergeTokens/moveStride.
  token_range_counter = new TokenRangeCounter(*vecGen_config); // NOT depend on vecGen_config->mergeTokens/moveStride.
  tree_serializer = new RelevantNoAtomicParent_TreeSerializer(*vecGen_config); // NOT depend on vecGen_config->mergeTokens/moveStride.
  vec_generator = new VecGenerator(*vecGen_config); // NOT depend on vecGen_config->mergeTokens/moveStride.
  vec_outputor = new TraVecOutput(*vecGen_config, vecGen_outfile); // DO depend on vecGen_config->mergeTokens, NOT on moveStride.
}

bool TraGenMain::
getParameters(const char * fn, int & mergeTokens, int & mergeStride, int & mergeLists)
{
  bool flag = false;
  if ( fn!=NULL ) {
    ifstream configfile(fn);
    try {
      if ( configfile.is_open() ) {
        if ( !configfile.eof() )
          configfile >> mergeTokens;
        if ( !configfile.eof() )
          configfile >> mergeStride;
        if ( !configfile.eof() )
          configfile >> mergeLists;
        flag = true;
      } else {
        cerr << "Warning: can't open configuration file  `" << fn << "', use previous parameters." << endl;
      }
    } catch (...) {
      // do nothing.
    }
    // 'finally' not in c++/gcc?
      configfile.close();
  }
  return flag;
}

TraGenMain::
~TraGenMain()
{
  if ( token_merger!=NULL ) {
    delete token_merger;
    token_merger = NULL;
  }
  if ( list_merger!=NULL ) {
    delete list_merger;
    list_merger = NULL;
  }
  if ( vec_outputor!=NULL ) {
    delete vec_outputor;
    vec_outputor = NULL;
  }
  if ( vec_generator!=NULL ) {
    delete vec_generator;
    vec_generator = NULL;
  }
  if ( tree_serializer!=NULL ) {
    delete tree_serializer;
    tree_serializer = NULL;
  }
  if ( token_counter!=NULL ) {
    delete token_counter;
    token_counter = NULL;
  }
  if ( token_range_counter!=NULL ) {
    delete token_range_counter;
    token_range_counter = NULL;
  }
  if ( vecGen_config!=NULL ) {
    delete vecGen_config;
    vecGen_config = NULL;
  }
}

void TraGenMain::
run(int startln, int endln)
{
  Tree* initial_inh = NULL;
  // count token nnumbers and assign token IDs (and, optionally, set parent pointers for e)
  token_counter->traverse(parse_tree->getRoot(), initial_inh);
#ifdef VGDEBUG
  fprintf(stderr, "Total counted terminals:%ld\n", parse_tree->getRoot()->terminal_number);
#endif
  // count token ranges for each node: mainly for bug finding purpose so far
  token_range_counter->traverse(parse_tree->getRoot(), initial_inh);

  // generate basic vectors for nodes:
  vec_generator->traverse(parse_tree->getRoot(), initial_inh);

  // output basic vectors for the lines are specified,
  if(startln>endln || startln<0 || endln<0) {
    cerr << "Error: no vec. Check startln<=endln && startln>=0 && endln>=0: " << startln << ", " << endln << endl;
    cerr << "Also, startln==0 means default vec gen for all lines." << endl;
    return;
  }
  cerr << "Line range used: " << (startln==0 ? "All " : " ") << "[" << startln << ", " << endln << "]" << endl;
  if(startln>0) {
    Tree* line_node = parse_tree->line2Tree(startln, endln);
    if(line_node!=NULL) {
      TreeVector* tv = TreeAccessor::get_node_vector(line_node);
      if (! tv->output(vecGen_outfile) ) {
        cerr << "Warning: no vec." << endl;
      }
    }
    return;
  }

  // otherwise, output basic vectors for all nodes
  if ( vecGen_config->mergeTokens>0 )
    vec_outputor->traverse(parse_tree->getRoot(), initial_inh);
  else if ( vecGen_config->mergeTokens==0 )
    vec_outputor->multipleTraverse(parse_tree->getRoot(), initial_inh);

#ifdef VGDEBUG
  fprintf(stderr, "# basic vectors:%ld\n", vec_outputor->nNodeVectors);
#endif

  if ( vecGen_config->moveStride<=0 )
    return;

  // serialize the whole tree in post order and assign ids to nodes:
  tree_serializer->traverse(parse_tree->getRoot(), initial_inh);
#ifdef VGDEBUG
  fprintf(stderr, "Total nodes > %ld (some children of atomic nodes are skipped), # nodes in the Sq tree:%ld\n", tree_serializer->id, tree_serializer->sqtree_length());
#endif
//#define outputnodeids
#ifdef outputnodeids
  Tree* tree_itr = tree_serializer->serialized_tree.chain_header;
  while ( tree_itr!=NULL ) {
    fprintf(stderr, "Tree %p id=%ld (%s), tokens = %ld, low_id = %ld, value=`%s'\n", tree_itr, TreeAccessor::get_serialized_id(tree_itr), parse_tree->getTypeName(tree_itr->type).c_str(), tree_itr->terminal_number, TreeAccessor::get_serialized_low_id(tree_itr), tree_itr->isTerminal()?tree_itr->toTerminal()->value->c_str():"<NULL>");
    tree_itr = TreeAccessor::get_serialized_next_neighbor(tree_itr);
  }
#endif

  // create different vector mergers:
  token_merger = new VectorMergerOnTokens(tree_serializer->serialized_tree, *vecGen_config); // DO depend on vecGen_config->mergeTokens/moveStride.
  vec_outputor->addMerger(token_merger);
#ifdef useVGDEBUGifwantthis
  list_merger = new VectorMergerOnLists(tree_serializer->serialized_tree, *vecGen_config); // DO depend on vecGen_config->mergeTokens/moveStride.
  vec_outputor->addMerger(list_merger);
#endif

  // output merged vectors
  if ( vecGen_config->mergeTokens==0 )
    vec_outputor->multipleOutputMergedVectors();
  else
    vec_outputor->outputMergedVectors();
#ifdef VGDEBUG
  fprintf(stderr, "# vectors:%ld, including must-gen nodes:%ld\n", vec_outputor->nAllOutputedVectors(), vec_outputor->nNodeVectors);
#endif
}

