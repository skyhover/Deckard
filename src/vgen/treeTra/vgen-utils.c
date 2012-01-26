/*
 * 
 * Copyright (c) 2007-2012,
 *   Lingxiao Jiang         <lxjiang@ucdavis.edu>
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

#include "vgen-utils.h"

char *stringclone(const char *var)
{
  char * tmp = NULL;
  assert( var!=NULL );
  tmp = (char *)malloc(sizeof(char)*(strlen(var)+1));
  strcpy(tmp, var);
  return tmp;
}

char *stringnclone(const char *var, unsigned int len)
{
  char *tmp = NULL;
  assert( var!=NULL );
  tmp = (char *)malloc(sizeof(char)*(len+1));
  strncpy(tmp, var, len);
  tmp[len] = '\0';
  return tmp;
}

/* a wrapper of strcmp. */
int compare_string(const void *c1, const void *c2) /* a wrapper of strcmp. */
{
  return strcmp((const char *)c1, (const char *)c2);
}

/* ad-hoc comparison of strings without case; can't handle unicode etc. */
int compare_string_nocase(const void *c1, const void *c2)
{
  const char * s1 = (const char *)c1;
  const char * s2 = (const char *)c2;
  unsigned i;
  for (i = 0; s1[i]!=NULL && s2[i]!=NULL; i++) {
    char uc1 = toupper(s1[i]);
    char uc2 = toupper(s2[i]);
    if ( uc1!=uc2 )
      return uc1 - uc2;
  }
  if (s1[i]==NULL && s2[i]==NULL)
    return 0;
  else if (s1[i]==NULL)
    return 0 - s2[i];
  else
    return s1[i];
}

