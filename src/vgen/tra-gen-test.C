#include "../include/ptree.h"
#include "treeTra/tra-gen.h"
#include <cstdio>

int main() {
  ParseTree * t;

  TraGenMain tg(t, NULL, fopen("test", "w"));

  tg.run();

  return 0;
}
