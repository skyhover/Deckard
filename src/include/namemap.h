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
#ifndef _DECKARD_NAMEMAP_H_
#define _DECKARD_NAMEMAP_H_

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <string.h> // for compatibility on various platforms
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <set>
#include <limits.h>

/** a map between integers and names;
 *  the intention is to use the ints as indices in other maps. */
class NameMap {
public:
   NameMap(int startID=0);
   NameMap(const NameMap&);
   /** assignment operator. Use copy-swap idiom to implement */
   NameMap& operator=(NameMap);
   friend void swap(NameMap& lhs, NameMap& rhs);

//   ~NameMap();
   const static int DEBUG_LEVEL = 1;
   
   int currentLastID();
   int nextAvailableID();
   bool hasNameId(std::string);
   int getNameId(std::string);
   std::string getIDName(int);
   int getOrAddNameId(std::string);
   bool setNameId(std::string, int);
   std::map<std::string, int> getNameIDMap();
   std::map<int, std::string> getIDNameMap();
   void setNameIDMap(std::map<std::string, int>&);
   void setIDNameMap(std::map<int, std::string>&);

   bool isIDValid(int);
   int printNamesIDs();
   int printIDsNames();
   
   void combineNameMap(const NameMap&);
   static NameMap combineNameMap(const NameMap&, const NameMap&);
   
   /** read in name/id from a file and check their validity.
    *  Each row in the file should be: STRING number */
   static NameMap readNamesIDs(const char*);
   /** read in name/id from a file and check their validity.
    *  Each row in the file should be: number STRING */
   static NameMap readIDsNames(const char*);
   
   /** read in a list of (unique) strings (node type names) from a file.
    * Each row in the file is a type name (allowing spaces) */
   static std::set<std::string> readNames(const char *);
   /** read in a list of (unique) IDs (node ids) */
   static std::set<int> readIDs(const char *);
   
   /** convert names to ids */
   static std::vector<int> name2id(std::map<std::string, int>& maps, std::vector<std::string>&);
   std::vector<int> name2id(std::vector<std::string>&);
   /** convert ids to names */
   static std::vector<std::string> id2name(std::map<int, std::string>& maps, std::vector<int>&);
   std::vector<std::string> id2name(std::vector<int>&);
public:
   static const std::string& getInvalidName();
//   static const std::string invalidName; // C++: can only initialize static members if they are of an integral or enum types AND declared as const. cf. http://www.parashift.com/c++-faq-lite/static-const-with-initializers.html
private:
   std::map<std::string, int> m_name2id;
   std::map<int, std::string> m_id2name;
   int lastUsedID;
};

#endif /* _DECKARD_NAMEMAP_H_ */
