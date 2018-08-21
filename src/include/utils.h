/*
 * 
 * Copyright (c) 2007-2018, University of California / Singapore Management University
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
#ifndef _DECKARD_UTILS_H_
#define _DECKARD_UTILS_H_

#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <functional>
#include <locale>
#include <utility>

/** trim from the start of a string */
std::string & ltrim(std::string &s);
/** trim from the end of a string */
std::string & rtrim(std::string &s);
/** trim from both ends of a string */
std::string & trim(std::string &s);

/** convert a string to one number; return INT_MIN when err.
 * - internally used stringstream::operator>>, allowing any character after a space character */
int getOneNumber(std::string &s);
/** convert a string to two numbers (small, big); delimiters in the middle;
 *  return <INT_MAX, INT_MIN> when err */
std::pair<int, int> getTwoNumbers(std::string &s, char delim='-');
/** convert a string to an ordered sequence of numbers (from small to big);
 *  delimiters in the middle; return empty vector when err */
std::vector<int> getNumbers(std::string &s, char delim='-');
/** convert an ordered sequence of numbers into a set.
 * - the input sequence represents single numbers, or ranges of numbers;
 * - the output set contains all numbers represented by the input sequence.
 * - For now, 2012/11/20, only support one single number or one range of numbers.
 */
std::set<int> enumerateNumers(std::vector<int>&);

/** check whether two ordered ranges overlap or not */
template <class Container1, class Container2>
bool is_set_intersection_empty(const Container1& c1, const Container2& c2)
{
   /* code implementation refs:
    * - http://stackoverflow.com/questions/12940522/check-for-empty-intersection-in-stl/
    * - http://stackoverflow.com/questions/1964150/c-test-if-2-sets-are-disjoint
    */
   typename Container1::const_iterator i1 = c1.begin();
   typename Container2::const_iterator i2 = c2.begin();
   while (i1 != c1.end() && i2 != c2.end()) {
      // only use operator<
      if (*i1 < *i2) ++i1;
      else if (*i2 < *i1) ++i2;
      else // (*i1 == *i2)
         return false;
   }
   return true;
}

#endif /* _DECKARD_UTILS_H_ */
