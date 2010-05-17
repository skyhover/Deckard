
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
