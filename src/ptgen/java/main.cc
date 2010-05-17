#include <ptree.h>
#include <map>
#include <string>

using namespace std;

map<string,int> name2id;
map<int,string> id2name;

int yyparse();

extern Tree *root;

void id_init();

int main()
{
    id_init();
    yyparse();
    if (!root) {
        cerr << "failed to parse file" << endl;
        return 1;
    }

    root->printTok();
    //root->print();

//    int c= root->countTerminals();

//    cout << c << endl;

    return 0;
}

