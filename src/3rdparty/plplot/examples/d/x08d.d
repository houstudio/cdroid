//      3-d plot demo.
//
// Copyright (C) 2008  Werner Smekal
// Copyright (C) 2004  Alan W. Irwin
// Copyright (C) 2004  Rafael Laboissiere
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

import plplot;
import std.string;
import std.math;
import std.c.stdlib;

//--------------------------------------------------------------------------
// cmap1_init1
//
// Initializes color map 1 in HLS space.
// Basic grayscale variation from half-dark (which makes more interesting
// looking plot compared to dark) to light.
// An interesting variation on this:
//	s[1] = 1.0
//--------------------------------------------------------------------------
void cmap1_init( bool gray )
{
    PLFLT[2] i, h, l, s;

    i[] = [ 0.0, 1.0 ];         // boundaries

    if ( gray )
    {
        h[] = [ 0.0, 0.0 ];     // hue -- low: red (arbitrary if s=0), high: red (arbitrary if s=0)
        l[] = [ 0.5, 1.0];      // lightness -- low: half-dark, high: light
        s[] = [ 0.0, 0.0 ];     // minimum saturation
    }
    else
    {
        h[] = [ 240.0, 0.0 ]; // blue -> green -> yellow -> red
        l[] = [ 0.6, 0.6 ];
        s[] = [ 0.8, 0.8 ];
    }

    plscmap1n( 256 );
    c_plscmap1l( 0, 2, cast(PLFLT*) i, cast(PLFLT*) h, cast(PLFLT*) l,
        cast(PLFLT*) s, null );
}


//--------------------------------------------------------------------------
// main
//
// Does a series of 3-d plots for a given data set, with different
// viewing options in each plot.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    const       nlevel = 10;
    const       XPTS   = 35;    // Data points in x
    const       YPTS   = 45;    // Data points in y

    PLFLT       xx, yy, r;
    bool        rosen = false;

    const PLFLT dx = 2. / cast(PLFLT) ( XPTS - 1 );
    const PLFLT dy = 2. / cast(PLFLT) ( YPTS - 1 );

    const       indexxmin = 0;
    const       indexxmax = XPTS;
    // parameters of ellipse (in x, y index coordinates) that limits the data.
    // x0, y0 correspond to the exact floating point centre of the index
    // range.
    const PLFLT x0 = 0.5 * cast(PLFLT) ( XPTS - 1 );
    const PLFLT a  = 0.9 * x0;
    const PLFLT y0 = 0.5 * cast(PLFLT) ( YPTS - 1 );
    const PLFLT b  = 0.7 * y0;
    PLFLT       square_root;

    PLFLT[]     alt = [ 60.0, 40.0 ];
    PLFLT[]     az  = [ 30.0, -30.0 ];

    string title[] = [ "#frPLplot Example 8 - Alt=60, Az=30",
                       "#frPLplot Example 8 - Alt=40, Az=-30" ];

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    // Allocate data structures
    PLFLT[XPTS] x;
    PLFLT[YPTS] y;
    PLINT[XPTS] indexymin;
    PLINT[XPTS] indexymax;

    PLFLT[][] z        = new PLFLT[][XPTS];
    PLFLT[][] zlimited = new PLFLT[][XPTS];
    for ( int i = 0; i < XPTS; i++ )
    {
        z[i]        = new PLFLT[YPTS];
        zlimited[i] = new PLFLT[YPTS];
    }

    for ( int i = 0; i < XPTS; i++ )
    {
        x[i] = -1. + cast(PLFLT) i * dx;
        if ( rosen )
            x[i] *= 1.5;
    }

    for ( int i = 0; i < YPTS; i++ )
    {
        y[i] = -1. + cast(PLFLT) i * dy;
        if ( rosen )
            y[i] += 0.5;
    }

    for ( size_t i = 0; i < XPTS; i++ )
    {
        xx = x[i];
        for ( size_t j = 0; j < YPTS; j++ )
        {
            yy = y[j];
            if ( rosen )
            {
                z[i][j] = pow( 1. - xx, 2. ) + 100. * pow( yy - pow( xx, 2. ), 2. );
                // The log argument may be zero for just the right grid.
                if ( z[i][j] > 0. )
                    z[i][j] = log( z[i][j] );
                else
                    z[i][j] = -5.; // -MAXFLOAT would mess-up up the scale
            }
            else
            {
                r       = sqrt( xx * xx + yy * yy );
                z[i][j] = exp( -r * r ) * cos( 2.0 * PI * r );
            }
        }
    }

    for ( size_t i = indexxmin; i < indexxmax; i++ )
    {
        square_root = sqrt( 1. - fmin( 1., pow( ( cast(PLFLT) i - x0 ) / a, 2. ) ) );
        // Add 0.5 to find nearest integer and therefore preserve symmetry
        // with regard to lower and upper bound of y range.
        indexymin[i] = cast(PLINT) fmax( 0, cast(PLINT) ( 0.5 + y0 - b * square_root ) );
        // indexymax calculated with the convention that it is 1
        // greater than highest valid index.
        indexymax[i] = cast(PLINT) fmin( YPTS, 1 + cast(PLINT) ( 0.5 + y0 + b * square_root ) );

        for ( size_t j = indexymin[i]; j < indexymax[i]; j++ )
            zlimited[i][j] = z[i][j];
    }

    PLFLT zmin, zmax;
    f2mnmx( z, zmin, zmax );

    PLFLT step = ( zmax - zmin ) / ( nlevel + 1 );
    PLFLT[nlevel] clevel;
    for ( size_t i = 0; i < nlevel; i++ )
        clevel[i] = zmin + step + step * i;

    pllightsource( 1., 1., 1. );

    for ( size_t k = 0; k < 2; k++ )
    {
        for ( size_t ifshade = 0; ifshade < 5; ifshade++ )
        {
            pladv( 0 );
            plvpor( 0.0, 1.0, 0.0, 0.9 );
            plwind( -1.0, 1.0, -0.9, 1.1 );
            plcol0( 3 );
            plmtex( "t", 1.0, 0.5, 0.5, title[k] );
            plcol0( 1 );
            if ( rosen )
                plw3d( 1.0, 1.0, 1.0, -1.5, 1.5, -0.5, 1.5, zmin, zmax, alt[k], az[k] );
            else
                plw3d( 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, zmin, zmax, alt[k], az[k] );

            plbox3( "bnstu", "x axis", 0.0, 0,
                "bnstu", "y axis", 0.0, 0,
                "bcdmnstuv", "z axis", 0.0, 0 );
            plcol0( 2 );

            switch ( ifshade )
            {
            case 0:
                // diffuse light surface plot
                cmap1_init( 1 );
                plsurf3d( x, y, z, 0 );
                break;
            case 1:
                // magnitude colored plot
                cmap1_init( 0 );
                plsurf3d( x, y, z, MAG_COLOR );
                break;
            case 2:
                //  magnitude colored plot with faceted squares
                cmap1_init( 0 );
                plsurf3d( x, y, z, MAG_COLOR | FACETED );
                break;
            case 3:
                // magnitude colored plot with contours
                cmap1_init( 0 );
                plsurf3d( x, y, z, MAG_COLOR | SURF_CONT | BASE_CONT, clevel );
                break;
            default:
                // magnitude colored plot with contours and index limits.
                cmap1_init( 0 );
                plsurf3dl( x, y, zlimited, MAG_COLOR | SURF_CONT | BASE_CONT, clevel, indexxmin, indexxmax, indexymin, indexymax );
                break;
            }
        }
    }

    plend();
    return 0;
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
