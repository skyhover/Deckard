#ifndef _DECKARD_UTILS_H_
#define _DECKARD_UTILS_H_

#include <string>
#include <algorithm>
#include <functional>
#include <locale>

/** trim from the start of a string */
std::string & ltrim(std::string &s);
/** trim from the end of a string */
std::string & rtrim(std::string &s);
/** trim from both ends of a string */
std::string & trim(std::string &s);

#endif /* _DECKARD_UTILS_H_ */
