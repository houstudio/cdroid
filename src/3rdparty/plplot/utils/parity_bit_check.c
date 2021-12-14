
#include <stdio.h>
#include <ctype.h>

// Checks whether the eighth (parity) bit is set on any character of stdin.
// Returns the first such character found or zero if no such character
// found.
int
main( int argc, char *argv[] )
{
    int c;
    (void) argc;
    (void) argv;
    while ( ( c = getchar() ) != EOF )
    {
        if ( c & 0x80 )
            return c;
    }
    return 0;
}
