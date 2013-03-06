/*
 * 
 * Copyright (c) 2007-2013, University of California / Singapore Management University
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

#include <limits>
#include <utils.h>
#include <iostream>
#include <sstream>

using namespace std;

string & ltrim(string& s)
{
   s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
   return s;
}

// trim from end
string & rtrim(string & s)
{
   s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
   return s;
}

// trim from both ends
string & trim(string & s)
{
   return ltrim(rtrim(s));
}

int getOneNumber(const string &s)
{
   stringstream ss(s); // auto handle spaces
   int num;
   if ( !(ss >> num) )
      num = numeric_limits<int>::min(); // cf. http://www.cplusplus.com/reference/std/limits/numeric_limits/
   return num;
}

pair<int, int> getTwoNumbers(string &s, char delim)
{
   stringstream ss(s);
   string x;
   int lnum, rnum;
   if ( ! getline(ss, x, delim) ) {
      return pair<int, int>(numeric_limits<int>::max(), numeric_limits<int>::min());
   } else {
      stringstream numss(x);
      if ( !(numss>>lnum) ) {
         return pair<int, int>(numeric_limits<int>::max(), numeric_limits<int>::min());
      }
   }
   if ( ! getline(ss, x, delim) ) {
      return pair<int, int>(numeric_limits<int>::max(), numeric_limits<int>::min());
   } else {
      stringstream numss(x);
      if ( !(numss>>rnum) ) {
         return pair<int, int>(numeric_limits<int>::max(), numeric_limits<int>::min());
      }
   }
   if ( lnum<=rnum )
      return pair<int, int>(lnum, rnum);
   else
      return pair<int, int>(rnum, lnum);
}

vector<int> getNumbers(string &s, char delim)
{
   stringstream ss(s);
   string x;
   vector<int> nums;
   while ( getline(ss, x, delim) ) {
      stringstream numss(x);
      int num;
      if ( !(numss>>num) ) {
         // any err => return empty nums:
         nums.clear();
         return nums;
      }
      nums.push_back(num);
   }
   sort(nums.begin(), nums.end());
   return nums;
}

set<int> enumerateNumers(vector<int>& nums)
{
   set<int> rsl;
   if ( nums.size()==1 ) {
      rsl.insert(nums[0]);
   } else if ( nums.size()>=2 ) {
      int low = min(nums[0], nums[1]);
      int high = max(nums[0], nums[1]);
      for(; low<=high; low++)
         rsl.insert(low);
      if ( nums.size()>2 )
         cerr << "Warning: enumerateNumers: 'nums' has more than two numbers. Ignore the extras." << endl;
   }
   return rsl;
}

