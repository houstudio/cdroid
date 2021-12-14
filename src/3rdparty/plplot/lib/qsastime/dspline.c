//
// Copyright (C) 2009 Alan W. Irwin
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// Provenance: This code was originally developed under the GPL as part of
// the FreeEOS project (revision 121).  This code has been converted from
// Fortran to C with the aid of f2c and relicensed for PLplot under the LGPL
// with the permission of the FreeEOS copyright holder (Alan W. Irwin).
//

#include "dspline.h"

int dspline( double *x, double *y, int n,
             int if1, double cond1, int ifn, double condn, double *y2 )
{
    int    i__1, i__, k;
    double p, u[2000], qn, un, sig;

//      input parameters:
//      x(n) are the spline knot points
//      y(n) are the function values at the knot points
//      if1 = 1 specifies cond1 is the first derivative at the
//        first knot point.
//      if1 = 2 specifies cond1 is the second derivative at the
//        first knot point.
//      ifn = 1 specifies condn is the first derivative at the
//        nth knot point.
//      ifn = 2 specifies condn is the second derivative at the
//        nth knot point.
//      output values:
//      y2(n) is the second derivative of the spline evaluated at
//        the knot points.
    // Parameter adjustments
    --y2;
    --y;
    --x;

    // Function Body
    if ( n > 2000 )
    {
        return 1;
    }
//      y2(i) = u(i) + d(i)*y2(i+1), where
//      d(i) is temporarily stored in y2(i) (see below).
    if ( if1 == 2 )
    {
//        cond1 is second derivative at first point.
//        these two values assure that for above equation with d(i) temporarily
//        stored in y2(i)
        y2[1] = 0.;
        u[0]  = cond1;
    }
    else if ( if1 == 1 )
    {
//        cond1 is first derivative at first point.
//        special case (Press et al 3.3.5 with A = 1, and B=0)
//        of equations below where
//        a_j = 0
//        b_j = -(x_j+1 - x_j)/3
//        c_j = -(x_j+1 - x_j)/6
//        r_j = cond1 - (y_j+1 - y_j)/(x_j+1 - x_j)
//        u(i) = r(i)/b(i)
//        d(i) = -c(i)/b(i)
//        N.B. d(i) is temporarily stored in y2.
        y2[1] = -.5;
        u[0]  = 3. / ( x[2] - x[1] ) * ( ( y[2] - y[1] ) / ( x[2] - x[1] ) - cond1 );
    }
    else
    {
        return 2;
    }
//      if original tri-diagonal system is characterized as
//      a_j y2_j-1 + b_j y2_j + c_j y2_j+1 = r_j
//      Then from Press et al. 3.3.7, we have the unscaled result:
//      a_j = (x_j - x_j-1)/6
//      b_j = (x_j+1 - x_j-1)/3
//      c_j = (x_j+1 - x_j)/6
//      r_j = (y_j+1 - y_j)/(x_j+1 - x_j) - (y_j - y_j-1)/(x_j - x_j-1)
//      In practice, all these values are divided through by b_j/2 to scale
//      them, and from now on we will use these scaled values.

//      forward elimination step: assume y2(i-1) = u(i-1) + d(i-1)*y2(i).
//      When this is substituted into above tridiagonal equation ==>
//      y2(i) = u(i) + d(i)*y2(i+1), where
//      u(i) = [r(i) - a(i) u(i-1)]/[b(i) + a(i) d(i-1)]
//      d(i) = -c(i)/[b(i) + a(i) d(i-1)]
//      N.B. d(i) is temporarily stored in y2.
    i__1 = n - 1;
    for ( i__ = 2; i__ <= i__1; ++i__ )
    {
//        sig is scaled a(i)
        sig = ( x[i__] - x[i__ - 1] ) / ( x[i__ + 1] - x[i__ - 1] );
//        p is denominator = scaled a(i) d(i-1) + scaled  b(i), where scaled
//        b(i) is 2.
        p = sig * y2[i__ - 1] + 2.;
//        propagate d(i) equation above.  Note sig-1 = -c(i)
        y2[i__] = ( sig - 1. ) / p;
//        propagate scaled u(i) equation above
        u[i__ - 1] = ( ( ( y[i__ + 1] - y[i__] ) / ( x[i__ + 1] - x[i__] ) - ( y[i__]
                                                                               - y[i__ - 1] ) / ( x[i__] - x[i__ - 1] ) ) * 6. / ( x[i__ + 1] -
                                                                                                                                   x[i__ - 1] ) - sig * u[i__ - 2] ) / p;
    }
    if ( ifn == 2 )
    {
//        condn is second derivative at nth point.
//        These two values assure that in the equation below.
        qn = 0.;
        un = condn;
    }
    else if ( ifn == 1 )
    {
//        specify condn is first derivative at nth point.
//        special case (Press et al 3.3.5 with A = 0, and B=1)
//        implies a_n y2(n-1) + b_n y2(n) = r_n, where
//        a_n = (x_n - x_n-1)/6
//        b_n = (x_n - x_n-1)/3
//        r_n = cond1 - (y_n - y_n-1)/(x_n - x_n-1)
//        use same propagation equation as above, only with c_n = 0
//        ==> d_n = 0 ==> y2(n) = u(n) =>
//        y(n) = [r(n) - a(n) u(n-1)]/[b(n) + a(n) d(n-1)]
//        qn is scaled a_n
        qn = .5;
//        un is scaled r_n (N.B. un is not u(n))!  Sorry for the mixed notation.
        un = 3. / ( x[n] - x[n - 1] ) * ( condn - ( y[n] - y[n - 1] ) / ( x[n]
                                                                          - x[n - 1] ) );
    }
    else
    {
        return 3;
    }
//      N.B. d(i) is temporarily stored in y2, and everything is
//      scaled by b_n.
//     qn is scaled a_n, 1.d0 is scaled b_n, and un is scaled r_n.
    y2[n] = ( un - qn * u[n - 2] ) / ( qn * y2[n - 1] + 1. );
//      back substitution.
    for ( k = n - 1; k >= 1; --k )
    {
        y2[k] = y2[k] * y2[k + 1] + u[k - 1];
    }
    return 0;
}

