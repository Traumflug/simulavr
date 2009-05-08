#ifndef STRING_TO_TYPE
  #error "STRING_TO_TYPE not defined"
#endif

#ifndef STRING_TO_NAME
  #error "STRING_TO_NAME not defined"
#endif

#ifndef STRING_TO_METHOD
  #error "STRING_TO_METHOD not defined"
#endif

#ifndef STRING_TO_MAX
  #error "STRING_TO_MAX not defined"
#endif

bool STRING_TO_NAME (
  const char      *s,
  STRING_TO_TYPE  *n,
  char           **endptr,
  int              base
)
{
  STRING_TO_TYPE  result;
  char           *end;

  if ( !n )
    return false;

  errno = 0;
  *n    = 0;

  result = STRING_TO_METHOD( s, &end, base );

  if ( endptr )
    *endptr = end; 

  /* nothing was converted */
  if ( end == s )
    return false;

  /* there was a conversion error */
  if ( (result == 0) && errno )
    return false;

  /* there was an overflow */
  if ( (result == LONG_MAX) && (errno == ERANGE))
    return false;

#ifdef STRING_TO_MIN
  /* there was an underflow */
  if ( (result == STRING_TO_MIN) && (errno == ERANGE))
    return false;
#endif

  *n = result;
  return true;
}

