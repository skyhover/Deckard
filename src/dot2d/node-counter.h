#ifndef _DECKARD_NODE_COUNTER_H_
#define _DECKARD_NODE_COUNTER_H_

#include <token-counter.h>

/** count the numbers of tokens represented by AST nodes, and fill in parent pointers for every
   tree node if not filled in yet. */
class NodeTokenCounter : public TokenCounter
{
private:
   /** store the token count for each AST node
    *  - read in from a file;
    *  - the file uses the same format as the nodetype file, except for the third column for the token count. */
   std::map<std::string, int> nodetype_tokencounts;

public:
   NodeTokenCounter(TraGenConfiguration & cfg);
   /** read in token counts for all AST nodes */
   bool init(const char * fn);
   int getTokenCount(std::string);
   int getTokenCount(Tree*);
   
   virtual long evaluateSynthesizedAttribute(Tree* node, Tree* in,
         SynthesizedAttributesList& synl);
   virtual long defaultSynthesizedAttribute(Tree* node, Tree* inh,
         SynthesizedAttributesList& synl);
};

#endif   /* _DECKARD_NODE_COUNTER_H_ */

