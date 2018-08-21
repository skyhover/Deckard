#ifndef _DOT_VEC_GEN_H_
#define _DOT_VEC_GEN_H_

#include <cstdio>
#include <tra-gen.h>

class ASTTraGenMain : public TraGenMain {
public:
   ASTTraGenMain(ParseTree* rt, const char * fn, const char * nodetokencounts, FILE * out);
   ASTTraGenMain(ParseTree* rt, int mergeTokens, int mergeStride, int mergeLists, const char * nodetokencounts, FILE * out);
};

#endif /* _DOT_VEC_GEN_H_ */

