
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

