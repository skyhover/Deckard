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

#include <namemap.h>
#include <utils.h>
#include <cassert>

using namespace std;

/***********************
 * class NameMap
 *
 * ********************/

/* BUG: This simple way of initialization of global variables may lead to initialization order fiasco: http://www.parashift.com/c++-faq-lite/static-init-order.html
 * more: http://stackoverflow.com/questions/2373859/c-static-const-and-initialization-is-there-a-fiasco
 * more: http://www.parashift.com/c++-faq-lite/static-const-with-initializers.html
 * more: http://stackoverflow.com/questions/2373859/c-static-const-and-initialization-is-there-a-fiasco */
//const string NameMap::invalidName = "<none>";
/* The fix makes 'invalidName' to be a static LOCAL variable which is guaranteed to be initialized before the function is called;
 * It requires all references to invalidName to go through this function (sort of singleton pattern, which is mentioned on wikipedia: http://en.wikipedia.org/wiki/Singleton_pattern)
However, the code is not thread-safe in C++: http://blogs.msdn.com/b/oldnewthing/archive/2004/03/08/85901.aspx
(I'd consider this and the whole "initialization order fiasco" as a compiler bug/misfeature:
 - C++ has no guarantee a GLOBAL (or static member) variable is initialized even when an object is loaded
 - Java has such a guarantee...
)
For our case, the code should still almost always "work" even though it's NOT thread-safe and may take more memory/time.
*/
const std::string& NameMap::getInvalidName()
{
   static const string invalidName = "<none>";
   return invalidName;
}

NameMap::NameMap(int startID):
      m_name2id(), m_id2name(), lastUsedID(startID)
{
   if ( DEBUG_LEVEL>1 ) {
      cout << "--> new NameMap: " << m_name2id.size() << "-" << m_id2name.size() << endl;
      printNamesIDs();
      printIDsNames();
      cout << "<--done new NameMap." << endl;
   }
}

NameMap::NameMap(const NameMap& r):
      m_name2id(r.m_name2id), m_id2name(r.m_id2name), lastUsedID(r.lastUsedID)
{
   
}

NameMap& NameMap::operator=(NameMap rhs)
{
   swap(*this, rhs);
   return *this;
}

void swap(NameMap& lhs, NameMap&rhs)
{
   using std::swap;

   lhs.m_name2id.swap(rhs.m_name2id);
   lhs.m_id2name.swap(rhs.m_id2name);
   swap(lhs.lastUsedID, rhs.lastUsedID);
}

//NameMap::~NameMap()
//{ TODO: somehow, it's not the right way to define the destructor
//   m_name2id.~map();
//   m_id2name.~map();
//}

int NameMap::currentLastID()
{
   return lastUsedID;
}

int NameMap::nextAvailableID()
{
   return ++lastUsedID;
}

bool NameMap::hasNameId(string n)
{
   map<string, int>::const_iterator it = m_name2id.find(n);
   if ( it!=m_name2id.end() )
      return true;
   else
      return false;
}

int NameMap::getNameId(string name)
{
   map<string, int>::const_iterator it = m_name2id.find(name);
   if ( it!=m_name2id.end() )
      return it->second;
   else
      return -1;
}

string NameMap::getIDName(int id)
{
   map<int, string>::const_iterator it = m_id2name.find(id);
   if ( it!=m_id2name.end() )
      return it->second;
   else
      return getInvalidName();
}

int NameMap::getOrAddNameId(string n)
{
   if ( hasNameId(n) )
      return getNameId(n);
   int id = nextAvailableID();
   m_name2id[n] = id;
   m_id2name[id] = n;
   return id;
}

bool NameMap::setNameId(string name, int id)
{
   if ( hasNameId(name) ) {
      return false;
   }
   if ( id>lastUsedID )
      lastUsedID = id;
   m_name2id[name] = id;
   m_id2name[id] = name;
   return true;
}

map<string, int> NameMap::getNameIDMap()
{
   return m_name2id;
}

map<int, string> NameMap::getIDNameMap()
{
   return m_id2name;
}

void NameMap::setNameIDMap(map<string, int>& m)
{
   m_name2id = m;
}

void NameMap::setIDNameMap(map<int, string>& m)
{
   m_id2name = m;
}

bool NameMap::isIDValid(int id)
{
   if ( id>=0 && id<=lastUsedID )
      return true;
   else
      return false;
}

int NameMap::printNamesIDs()
{
   for(map<string, int>::const_iterator it = m_name2id.begin();
         it!=m_name2id.end(); ++it) {
      cout << it->first << "\t" << it->second << endl;
   }
   assert(m_name2id.size()==m_id2name.size());
   return m_name2id.size();
}

int NameMap::printIDsNames()
{
   for(map<int, string>::const_iterator it = m_id2name.begin();
         it!=m_id2name.end(); ++it) {
      cout << it->first << "\t" << it->second << endl;
   }
   assert(m_name2id.size()==m_id2name.size());
   return m_id2name.size();
}

NameMap NameMap::combineNameMap(const NameMap& lhs, const NameMap& rhs)
{
   NameMap rsl(lhs);
   // combine m_name2id and m_id2name
   for(map<string, int>::const_iterator ritr = rhs.m_name2id.begin();
         ritr!=rhs.m_name2id.end(); ++ritr) {
      map<string, int>::const_iterator litr = lhs.m_name2id.find(ritr->first);
      if ( litr!=lhs.m_name2id.end() ) {
         if ( litr->second!=ritr->second ) {
            cerr << "Warning: different IDs for the same attribute-" << litr->first << "? Continue anyway..." << endl;
         }
      } else {
         rsl.m_name2id.insert(*ritr);
         rsl.m_id2name.insert(make_pair(ritr->second, ritr->first));
      }
   }
   // combine lastUsedID
   rsl.lastUsedID = max(lhs.lastUsedID, rhs.lastUsedID);
   
   return rsl;
}

NameMap NameMap::readNamesIDs(const char* fn)
{
   NameMap m;
   if ( fn == NULL )
      return m;
   string line;
   ifstream ifs(fn);
   int lcount = 0;
   while ( ifs.good() ) {
      lcount++;
      string name;
      int id;
      getline(ifs, line);
      stringstream ss(line);
      ss >> name;
      if ( !ss ) {
         continue;
      }
      if ( name.find("//")==0 || name.find('#')==0 )
         continue;
      ss >> id;
      if ( !ss ) {
         cerr << "Warning: invalid type id at line " << lcount << ". Default is usded: ";
         id = m.getOrAddNameId(name);
         cerr << id << endl;
      } else {
         if ( ! m.setNameId(name, id) ) {
            cerr << "Warning: failed to set type name id for line " << lcount << ". Skipped." << endl;
         }
      }
   }
   return m;
}

set<string> NameMap::readNames(const char * fn)
{
   set<string> m;
   if ( fn == NULL )
      return m;
   string line;
   ifstream ifs(fn);
   int lcount = 0;
   while ( ifs.good() ) {
      lcount++;
      getline(ifs, line);
      trim(line);
      if ( line.empty() || line.find("//")==0 || line.find('#')==0 )
         continue;
      m.insert(line);
   }
   return m;
}

vector<int> NameMap::name2id(map<string, int>& maps, vector<string>& names)
{
   vector<int> ids;
   for (vector<string>::const_iterator s = names.begin();
         s!=names.end(); ++s) {
      map<string, int>::const_iterator i = maps.find(*s);
      if (i == maps.end()) {
         cerr << "ERROR: NameMap::name2id: unknown node type name: " << *s << endl;
         continue;
      }
      ids.push_back(i->second);
   }
   return ids;
}

vector<string> NameMap::id2name(map<int, string>& maps, vector<int>& ids)
{
   vector<string> names;
   for (vector<int>::const_iterator i = ids.begin();
         i!=ids.end(); ++i) {
      map<int, string>::const_iterator s = maps.find(*i);
      if (s == maps.end()) {
         cerr << "ERROR: NameMap::id2name: unknown node type id: " << *i << endl;
         continue;
      }
      names.push_back(s->second);
   }
   return names;
}

