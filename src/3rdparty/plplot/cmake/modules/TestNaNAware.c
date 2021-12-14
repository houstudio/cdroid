// Test for NaN awareness.
#include <stdio.h>
#if defined(_AIX)
#include "math.h"
#else
#include "nan.h"
#endif

// Return 1 for success and 0 for failure.
int
main()
{
#if defined(_AIX)
   double x = NAN;
#else
   double x = NaN;
#endif
#if defined(_MSC_VER) | defined(__BORLANDC__)
   if (isnan(x)) return 1;
#endif

   if (x != x) return 1;
   else return 0;
}
