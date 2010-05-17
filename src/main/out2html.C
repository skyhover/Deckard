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
  FILE * inf = fopen(argv[1], "r");
  if ( inf==NULL ) {
    cerr << "Can't open file: " << argv[1] << endl;
    return 1;
  }

  char *line = NULL;
  size_t bufferLength = 0;
  ssize_t lineLength;
  int linecount = 0;

  while ( (lineLength = getline(&line, &bufferLength, inf)) > 0) {
    linecount++;
    if ( strcmp(line, "")==0 || strncmp(line, "\n", 1)==0 )
      cout << "<p></p>" << endl << "<br />" << endl;
    else if ( tt.parse(line, TokenTreeMap::clone_patterns) ) {
      tt.out2html0(cout);
    } else
      cout << tt << "<br /> " << endl;
  }

  return 0;
}

