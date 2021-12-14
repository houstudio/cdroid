#include "deltaT.h"
#include "qsastime.h"
#include "dsplint.h"
#include <stdio.h>
int main()
{
    int    iyear;
//  double year, delta, ddelta, d2delta;
    double year, delta;
    for ( iyear = -1000; iyear <= 2000; ++iyear )
    {
        year = (double) iyear;
//    dsplint(xspline, yspline, y2spline, number_of_entries_in_spline_tables, year, &delta, &ddelta, &d2delta);
        dsplint( xspline, yspline, y2spline, number_of_entries_in_spline_tables, year, &delta );
//    printf("%9.0f.%9.0f.%15.6E%15.6E\n", year, delta, ddelta, d2delta);
        printf( "%9.0f.%9.0f.\n", year, delta );
    }
    return 0;
}

