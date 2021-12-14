#include "wxPLplot_nanosec.h"
#if defined ( PLPLOT_WX_DEBUG_OUTPUT ) && defined ( PLPLOT_WX_NANOSEC )

#include <stdint.h>     // for uint64 definition
#include <time.h>       // for clock_gettime
#define BILLION    1000000000L

void
pl_wx_nanosec( const char *string )
{
    uint64_t        timestamp;
    struct timespec timenano;
    // Determine seconds since the epoch and nanosecs since the epoch of
    // the last second.
    clock_gettime( CLOCK_MONOTONIC, &timenano );
    timestamp = BILLION * ( timenano.tv_sec ) + timenano.tv_nsec;
    wxLogDebug( "nanosecs since epoch = %llu: %s", (long long unsigned int) timestamp, string );
}
#endif //#if defined(PLPLOT_WX_DEBUG_OUTPUT) && defined(PLPLOT_WX_NANOSEC)
