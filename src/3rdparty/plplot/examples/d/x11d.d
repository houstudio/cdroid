//      Mesh plot demo.
//
// Copyright (C) 2008  Werner Smekal
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
// along with PLplot; if not, write to the Free So  ftware
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//

import plplot;
import std.string;
import std.math;


void cmap1_init()
{
    PLFLT[2] i, h, l, s;

    i[] = [ 0.0, 1.0 ];         // boundaries
    h[] = [ 240.0, 0.0 ];       // blue -> green -> yellow -> red
    l[] = [ 0.6, 0.6 ];
    s[] = [ 0.8, 0.8 ];

    plscmap1n( 256 );
    c_plscmap1l( 0, 2, cast(PLFLT*) i, cast(PLFLT*) h, cast(PLFLT*) l,
        cast(PLFLT*) s, null );
}


//--------------------------------------------------------------------------
// main
//
// Does a series of mesh plots for a given data set, with different
// viewing options in each plot.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    const   nlevel = 10;
    const   XPTS   = 35;        // Data points in x
    const   YPTS   = 46;        // Data points in y

    int[]   opt = [ DRAW_LINEXY, DRAW_LINEXY ];

    PLFLT[] alt = [ 33.0, 17.0 ];
    PLFLT[] az  = [ 24.0, 115.0 ];

    string title[] = [ "#frPLplot Example 11 - Alt=33, Az=24, Opt=3",
                       "#frPLplot Example 11 - Alt=17, Az=115, Opt=3" ];

    PLFLT xx, yy;

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    PLFLT[XPTS] x;
    PLFLT[YPTS] y;

    PLFLT[][] z = new PLFLT[][XPTS];
    for ( int i = 0; i < XPTS; i++ )
        z[i] = new PLFLT[YPTS];

    for ( int i = 0; i < XPTS; i++ )
        x[i] = 3. * cast(PLFLT) ( i - ( XPTS / 2 ) ) / cast(PLFLT) ( XPTS / 2 );

    for ( int i = 0; i < YPTS; i++ )
        y[i] = 3. * cast(PLFLT) ( i - ( YPTS / 2 ) ) / cast(PLFLT) ( YPTS / 2 );

    for ( size_t i = 0; i < XPTS; i++ )
    {
        xx = x[i];
        for ( size_t j = 0; j < YPTS; j++ )
        {
            yy      = y[j];
            z[i][j] = 3. * ( 1. - xx ) * ( 1. - xx ) * exp( -( xx * xx ) - ( yy + 1. ) * ( yy + 1. ) ) -
                      10. * ( xx / 5. - pow( xx, 3. ) - pow( yy, 5. ) ) * exp( -xx * xx - yy * yy ) -
                      1. / 3. * exp( -( xx + 1 ) * ( xx + 1 ) - ( yy * yy ) );

            if ( 0 ) // Jungfraujoch/Interlaken
            {
                if ( z[i][j] < -1. )
                    z[i][j] = -1.;
            }
        }
    }

    PLFLT zmin, zmax;
    f2mnmx( z, zmin, zmax );

    PLFLT step = ( zmax - zmin ) / ( nlevel + 1 );
    PLFLT[nlevel] clevel;
    for ( size_t i = 0; i < nlevel; i++ )
        clevel[i] = zmin + step + step * i;

    cmap1_init();
    for ( size_t k = 0; k < 2; k++ )
    {
        for ( size_t i = 0; i < 4; i++ )
        {
            pladv( 0 );
            plcol0( 1 );
            plvpor( 0.0, 1.0, 0.0, 0.9 );
            plwind( -1.0, 1.0, -1.0, 1.5 );
            plw3d( 1.0, 1.0, 1.2, -3.0, 3.0, -3.0, 3.0, zmin, zmax, alt[k], az[k] );
            plbox3( "bnstu", "x axis", 0.0, 0,
                "bnstu", "y axis", 0.0, 0,
                "bcdmnstuv", "z axis", 0.0, 4 );

            plcol0( 2 );
            switch ( i )
            {
            case 0:
                // wireframe plot
                plmesh( x, y, z, opt[k] );
                break;
            case 1:
                // magnitude colored wireframe plot
                plmesh( x, y, z, opt[k] | MAG_COLOR );
                break;
            case 2:
                // magnitude colored wireframe plot with sides
                plot3d( x, y, z, opt[k] | MAG_COLOR, 1 );
                break;
            case 3:
                // magnitude colored wireframe plot with base contour
                plmeshc( x, y, z, opt[k] | MAG_COLOR | BASE_CONT, clevel );
                break;
            default:
                break;
            }

            plcol0( 3 );
            plmtex( "t", 1.0, 0.5, 0.5, title[k] );
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
