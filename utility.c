//This file has functions used in both server.c and client.c

#include "utility.h"
#include <string.h>
#include <ctype.h>


char* trim(char* str)
{
  char *end;

  // Trim leading space                                                         
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?                                                 
    return str;

  // Trim trailing space                                                        
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator                                                  
  *(end+1) = '\0';

  return str;
}

/* case blind strcmp */
 
/**
 * caseless compare
 * used code found on a forum for this: http://cboard.cprogramming.com/c-programming/63091-strcmp-without-case-sensitivity.html
 */
int strcmpc (const char *p1, const char *p2)
{
  register unsigned char *s1 = (unsigned char *) p1;
  register unsigned char *s2 = (unsigned char *) p2;
  unsigned char c1, c2;
 
  do
    {
      c1 = (unsigned char) toupper((int)*s1++);
      c2 = (unsigned char) toupper((int)*s2++);
      if (c1 == '\0')
	{
	  return c1 - c2;
	}
    }
  while (c1 == c2);
 
  return c1 - c2;
}

