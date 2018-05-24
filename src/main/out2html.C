#include <ptree.h>
#include <map>
#include <string>
#include <string.h>
#include <token-tree-map.h>
#include <algorithm>
#include <cctype>

using namespace std;

// These are not really used by "out2html"; just to resolve dependency. TODO: refactor, decouple bug detection from clone detection
map<string,int> name2id;
map<int,string> id2name;
string identifierTypeName = "dummy only; no need for such name";

/* read in a file line-by-line:
   if the line is a valid clone point, transform it to HTML;
   otherwise, simply output the line. */
int main( int argc, char **argv )
{
  if ( argc!=2 ) {
    cerr << "usage: [a clone cluster file]" << endl;
    cerr << "output: an HTML version of the clone cluster file." << endl;
    return 1;
  }

  id_init();
  TokenTreeMap::init_shared_data();
  ClonePointT tt;
  ifstream inf(argv[1], ios::in);
  if ( ! inf.is_open() ) {
    cerr << "Can't open file: " << argv[1] << endl;
    return 1;
  }

  string line;
  int linecount = 0;

  while ( !inf.eof() ) {
    getline(inf, line);
    linecount++;
    char * charline = new char[line.length()+1];
    strcpy(charline, line.c_str());
    if ( strcmp(charline, "")==0 || strncmp(charline, "\n", 1)==0 )
      cout << "<p></p>" << endl << "<br />" << endl;
    else if ( tt.parse(charline, TokenTreeMap::clone_patterns) ) {
      tt.out2html0(cout);
    } else
      cout << tt << "<br /> " << endl;
    delete charline;
  }
  inf.close();

  return 0;
}

