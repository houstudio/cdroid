//  Simple vector plot example
//  Copyright (C) 2009  Werner Smekal
//
//
//  This file is part of PLplot.
//
//  PLplot is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  PLplot is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public License
//  along with PLplot; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//

import std.math;
import std.string;

import plplot;

//
// Global transform function for a constriction using data passed in
// This is the same transformation used in constriction.
//
extern ( C ) {
void
transform( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer data )
{
    PLFLT *xmax = cast(PLFLT *) data;

    *xt = x;
    *yt = y / 4.0 * ( 3 - cos( PI * x / *xmax ) );
}
}


//--------------------------------------------------------------------------
// main
//
// Generates several simple vector plots.
//--------------------------------------------------------------------------


class plot {
    //
    // Vector plot of the circulation about the origin
    //
    void circulation()
    {
        const int nx = 20;
        const int ny = 20;

        PLFLT     dx = 1.0;
        PLFLT     dy = 1.0;

        PLFLT     xmin = -nx / 2 * dx;
        PLFLT     xmax = nx / 2 * dx;
        PLFLT     ymin = -ny / 2 * dy;
        PLFLT     ymax = ny / 2 * dy;

        PLcGrid2  cgrid2;
        cgrid2.xg = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            cgrid2.xg[i] = new PLFLT[ny];
        cgrid2.yg = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            cgrid2.yg[i] = new PLFLT[ny];

        PLFLT[][] u = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            u[i] = new PLFLT[ny];
        PLFLT[][] v = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            v[i] = new PLFLT[ny];

        // Create data - circulation around the origin.
        PLFLT x, y;
        for ( int i = 0; i < nx; i++ )
        {
            x = ( i - nx / 2 + 0.5 ) * dx;
            for ( int j = 0; j < ny; j++ )
            {
                y = ( j - ny / 2 + 0.5 ) * dy;
                cgrid2.xg[i][j] = x;
                cgrid2.yg[i][j] = y;
                u[i][j]         = y;
                v[i][j]         = -x;
            }
        }

        // Plot vectors with default arrows
        plenv( xmin, xmax, ymin, ymax, 0, 0 );
        pllab( "(x)", "(y)", "#frPLplot Example 22 - circulation" );
        plcol0( 2 );
        plvect( u, v, 0.0, cgrid2 );
        plcol0( 1 );
    }

    //
    // Vector plot of flow through a constricted pipe
    //
    void
    constriction( int astyle )
    {
        const int nx = 20;
        const int ny = 20;
        string    title;

        PLFLT     dx = 1.0;
        PLFLT     dy = 1.0;

        PLFLT     xmin = -nx / 2 * dx;
        PLFLT     xmax = nx / 2 * dx;
        PLFLT     ymin = -ny / 2 * dy;
        PLFLT     ymax = ny / 2 * dy;

        PLcGrid2  cgrid2;
        cgrid2.xg = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            cgrid2.xg[i] = new PLFLT[ny];
        cgrid2.yg = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            cgrid2.yg[i] = new PLFLT[ny];

        PLFLT[][] u = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            u[i] = new PLFLT[ny];
        PLFLT[][] v = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            v[i] = new PLFLT[ny];

        PLFLT Q = 2.0;
        PLFLT x, y, b, dbdx;
        for ( int i = 0; i < nx; i++ )
        {
            x = ( i - nx / 2 + 0.5 ) * dx;
            for ( int j = 0; j < ny; j++ )
            {
                y = ( j - ny / 2 + 0.5 ) * dy;
                cgrid2.xg[i][j] = x;
                cgrid2.yg[i][j] = y;
                b = ymax / 4.0 * ( 3 - cos( PI * x / xmax ) );
                if ( fabs( y ) < b )
                {
                    dbdx    = ymax / 4.0 * sin( PI * x / xmax ) * PI / xmax * y / b;
                    u[i][j] = Q * ymax / b;
                    v[i][j] = dbdx * u[i][j];
                }
                else
                {
                    u[i][j] = 0.0;
                    v[i][j] = 0.0;
                }
            }
        }

        plenv( xmin, xmax, ymin, ymax, 0, 0 );
        title = format( "#frPLplot Example 22 - constriction (arrow style %d)", astyle );
        pllab( "(x)", "(y)", title );
        plcol0( 2 );
        plvect( u, v, -1.0, cgrid2 );
        plcol0( 1 );
    }


    //
    // Vector plot of flow through a constricted pipe
    // with a coordinate transform
    //
    void
    constriction2()
    {
        const int nx   = 20;
        const int ny   = 20;
        const int nc   = 11;
        const int nseg = 20;

        PLFLT []  clev = new PLFLT[nc];

        PLFLT     dx = 1.0;
        PLFLT     dy = 1.0;

        PLFLT     xmin = -nx / 2 * dx;
        PLFLT     xmax = nx / 2 * dx;
        PLFLT     ymin = -ny / 2 * dy;
        PLFLT     ymax = ny / 2 * dy;

        plstransform( &transform, cast(PLPointer) &xmax );

        PLcGrid2 cgrid2;
        cgrid2.xg = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            cgrid2.xg[i] = new PLFLT[ny];
        cgrid2.yg = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            cgrid2.yg[i] = new PLFLT[ny];

        PLFLT[][] u = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            u[i] = new PLFLT[ny];
        PLFLT[][] v = new PLFLT[][nx];
        for ( int i = 0; i < nx; i++ )
            v[i] = new PLFLT[ny];

        PLFLT Q = 2.0;
        PLFLT x, y, b, dbdx;
        for ( int i = 0; i < nx; i++ )
        {
            x = ( i - nx / 2 + 0.5 ) * dx;
            for ( int j = 0; j < ny; j++ )
            {
                y = ( j - ny / 2 + 0.5 ) * dy;
                cgrid2.xg[i][j] = x;
                cgrid2.yg[i][j] = y;
                b       = ymax / 4.0 * ( 3 - cos( PI * x / xmax ) );
                u[i][j] = Q * ymax / b;
                v[i][j] = 0.0;
            }
        }

        for ( int i = 0; i < nc; i++ )
        {
            clev[i] = Q + i * Q / ( nc - 1 );
        }

        plenv( xmin, xmax, ymin, ymax, 0, 0 );
        pllab( "(x)", "(y)", "#frPLplot Example 22 - constriction with plstransform" );
        plcol0( 2 );
        plshades( u, null, xmin + dx / 2, xmax - dx / 2,
            ymin + dy / 2, ymax - dy / 2,
            clev, 0.0, 1, 1.0, 0 );
        plvect( u, v, -1.0, cgrid2 );
        // Plot edges using plpath (which accounts for coordinate transformation) rather than plline
        plpath( nseg, xmin, ymax, xmax, ymax );
        plpath( nseg, xmin, ymin, xmax, ymin );
        plcol0( 1 );

        plstransform( null, null );
    }

    //--------------------------------------------------------------------------
    // f2mnmx
    //
    // Returns min & max of input 2d array.
//--------------------------------------------------------------------------
    void f2mnmx( PLFLT[][] f, out PLFLT fmn, out PLFLT fmx )
    {
        fmx = f[0][0];
        fmn = fmx;

        for ( int i = 0; i < f.length; i++ )
        {
            for ( int j = 0; j < f[i].length; j++ )
            {
                fmx = fmax( fmx, f[i][j] );
                fmn = fmin( fmn, f[i][j] );
            }
        }
    }


    //
    // Vector plot of the gradient of a shielded potential (see example 9)
    //
    void potential()
    {
        const int nper   = 100;
        const int nlevel = 10;
        const int nr     = 20;
        const int ntheta = 20;

        PLcGrid2  cgrid2;
        cgrid2.xg = new PLFLT[][nr];
        for ( int i = 0; i < nr; i++ )
            cgrid2.xg[i] = new PLFLT[ntheta];
        cgrid2.yg = new PLFLT[][nr];
        for ( int i = 0; i < nr; i++ )
            cgrid2.yg[i] = new PLFLT[ntheta];

        PLFLT[][] u = new PLFLT[][nr];
        for ( int i = 0; i < nr; i++ )
            u[i] = new PLFLT[ntheta];
        PLFLT[][] v = new PLFLT[][nr];
        for ( int i = 0; i < nr; i++ )
            v[i] = new PLFLT[ntheta];
        PLFLT[][] z = new PLFLT[][nr];
        for ( int i = 0; i < nr; i++ )
            z[i] = new PLFLT[ntheta];

        // Potential inside a conducting cylinder (or sphere) by method of images.
        // Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
        // Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
        // Also put in smoothing term at small distances.
        //

        PLFLT rmax = nr;

        PLFLT eps = 2.0;

        PLFLT q1 = 1.0;
        PLFLT d1 = rmax / 4.0;

        PLFLT q1i = -q1 * rmax / d1;
        PLFLT d1i = pow( rmax, 2.0 ) / d1;

        PLFLT q2 = -1.0;
        PLFLT d2 = rmax / 4.0;

        PLFLT q2i = -q2 * rmax / d2;
        PLFLT d2i = pow( rmax, 2.0 ) / d2;

        PLFLT r, theta, x, y;
        PLFLT div1, div1i, div2, div2i;
        for ( int i = 0; i < nr; i++ )
        {
            r = 0.5 + i;
            for ( int j = 0; j < ntheta; j++ )
            {
                theta           = 2.0 * PI / ( ntheta - 1 ) * ( 0.5 + j );
                x               = r * cos( theta );
                y               = r * sin( theta );
                cgrid2.xg[i][j] = x;
                cgrid2.yg[i][j] = y;
                div1            = sqrt( pow( x - d1, 2.0 ) + pow( y - d1, 2.0 ) + pow( eps, 2.0 ) );
                div1i           = sqrt( pow( x - d1i, 2.0 ) + pow( y - d1i, 2.0 ) + pow( eps, 2.0 ) );
                div2            = sqrt( pow( x - d2, 2.0 ) + pow( y + d2, 2.0 ) + pow( eps, 2.0 ) );
                div2i           = sqrt( pow( x - d2i, 2.0 ) + pow( y + d2i, 2.0 ) + pow( eps, 2.0 ) );
                z[i][j]         = q1 / div1 + q1i / div1i + q2 / div2 + q2i / div2i;
                u[i][j]         = -q1 * ( x - d1 ) / pow( div1, 3.0 ) - q1i * ( x - d1i ) / pow( div1i, 3.0 )
                                  - q2 * ( x - d2 ) / pow( div2, 3.0 ) - q2i * ( x - d2i ) / pow( div2i, 3.0 );
                v[i][j] = -q1 * ( y - d1 ) / pow( div1, 3.0 ) - q1i * ( y - d1i ) / pow( div1i, 3.0 )
                          - q2 * ( y + d2 ) / pow( div2, 3.0 ) - q2i * ( y + d2i ) / pow( div2i, 3.0 );
            }
        }

        PLFLT xmin, xmax, ymin, ymax, zmin, zmax;
        f2mnmx( cgrid2.xg, xmin, xmax );
        f2mnmx( cgrid2.yg, ymin, ymax );
        f2mnmx( z, zmin, zmax );

        plenv( xmin, xmax, ymin, ymax, 0, 0 );
        pllab( "(x)", "(y)", "#frPLplot Example 22 - potential gradient vector plot" );

        // Plot contours of the potential
        PLFLT   dz     = ( zmax - zmin ) / nlevel;
        PLFLT[] clevel = new PLFLT[nlevel];
        for ( int i = 0; i < nlevel; i++ )
            clevel[i] = zmin + ( i + 0.5 ) * dz;

        plcol0( 3 );
        pllsty( 2 );
        plcont( z, 1, nr, 1, ntheta, clevel, cgrid2 );
        pllsty( 1 );
        plcol0( 1 );

        // Plot the vectors of the gradient of the potential
        plcol0( 2 );
        plvect( u, v, 25.0, cgrid2 );
        plcol0( 1 );

        // Plot the perimeter of the cylinder
        PLFLT[] px = new PLFLT[nper];
        PLFLT[] py = new PLFLT[nper];
        for ( int i = 0; i < nper; i++ )
        {
            theta = ( 2.0 * PI / ( nper - 1 ) ) * i;
            px[i] = rmax * cos( theta );
            py[i] = rmax * sin( theta );
        }
        plline( px, py );
    }
}


int main( char[][] args )
{
    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    plot myPlot = new plot;

    myPlot.circulation();

    PLINT fill = 0;

    // Pairs of points making the line segments used to plot the user defined arrow
    PLFLT[] arrow_x = [ -0.5, 0.5, 0.3, 0.5, 0.3, 0.5 ];
    PLFLT[] arrow_y = [  0.0, 0.0, 0.2, 0.0, -0.2, 0.0 ];

    // Set arrow style using arrow_x and arrow_y then
    //       plot using these arrows.
    plsvect( arrow_x, arrow_y, fill );
    myPlot.constriction( 1 );

    // Pairs of points making the line segments used to plot the user defined arrow
    PLFLT[] arrow2_x = [ -0.5, 0.3, 0.3, 0.5, 0.3, 0.3 ];
    PLFLT[] arrow2_y = [  0.0, 0.0, 0.2, 0.0, -0.2, 0.0 ];

    // Set arrow style using arrow2_x and arrow2_y then
    //       plot using these filled arrows.
    fill = 1;
    plsvect( arrow2_x, arrow2_y, fill );
    myPlot.constriction( 2 );

    myPlot.constriction2();

    plsvect( null, null, 0 );

    myPlot.potential();

    plend();

    return 0;
}
