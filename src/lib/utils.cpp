
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

