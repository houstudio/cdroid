//--------------------------------------------------------------------------
// Copyright (C) 2004  Andrew Ross
// Copyright (C) 2004-2014  Alan W. Irwin
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
// Implementation of PLplot example 8 in C++.
//--------------------------------------------------------------------------

#include "plc++demos.h"

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x08 {
public:
    x08( int, char ** );
    void cmap1_init( int );

private:
    plstream             *pls;

    static const int     XPTS;
    static const int     YPTS;
    static PLFLT         alt[];
    static PLFLT         az[];
    static const char    *title[];

    static PLBOOL        rosen;

    static PLOptionTable options[];

    PLFLT MIN( PLFLT x, PLFLT y ) { return ( x < y ? x : y ); };
    PLFLT MAX( PLFLT x, PLFLT y ) { return ( x > y ? x : y ); };
};


// These values must be odd, for the middle
// of the index range to be an integer, and thus
// to correspond to the exact floating point centre
// of the sombrero.
const int     x08::    XPTS      = 35;
const int     x08::    YPTS      = 45;
PLFLT         x08::        alt[] = { 60.0, 40.0 };
PLFLT         x08::        az[] = { 30.0, -30.0 };

const char    *x08::  title[] = {
    "#frPLplot Example 8 - Alt=60, Az=30",
    "#frPLplot Example 8 - Alt=40, Az=-30",
};

PLOptionTable x08::options[] = {
    {
        "rosen",             // Turns on use of Rosenbrock function
        NULL,
        NULL,
        &x08::rosen,
        PL_OPT_BOOL,
        "-rosen",
        "Use the log_e of the \"Rosenbrock\" function"
    },
    {
        NULL,                     // option
        NULL,                     // handler
        NULL,                     // client data
        NULL,                     // address of variable to set
        0,                        // mode flag
        NULL,                     // short syntax
        NULL
    }                             // long syntax
};


PLBOOL x08::rosen = 0;

// cmap1_init1

// Initializes color map 1 in HLS space.
// Basic grayscale variation from half-dark (which makes more interesting
// looking plot compared to dark) to light.
// An interesting variation on this:
// s[1] = 1.0

void x08::cmap1_init( int gray )
{
    PLFLT *i   = new PLFLT[2];
    PLFLT *h   = new PLFLT[2];
    PLFLT *l   = new PLFLT[2];
    PLFLT *s   = new PLFLT[2];
    bool  *rev = new bool[2];

    i[0] = 0.0;       // left boundary
    i[1] = 1.0;       // right boundary

    if ( gray == 1 )
    {
        h[0] = 0.0;     // hue -- low: red (arbitrary if s=0)
        h[1] = 0.0;     // hue -- high: red (arbitrary if s=0)

        l[0] = 0.5;     // lightness -- low: half-dark
        l[1] = 1.0;     // lightness -- high: light

        s[0] = 0.0;     // minimum saturation
        s[1] = 0.0;     // minimum saturation
    }
    else
    {
        h[0] = 240; // blue -> green -> yellow ->
        h[1] = 0;   // -> red

        l[0] = 0.6;
        l[1] = 0.6;

        s[0] = 0.8;
        s[1] = 0.8;
    }

    rev[0] = false;       // interpolate on front side of colour wheel.
    rev[1] = false;       // interpolate on front side of colour wheel.

    pls->scmap1n( 256 );
    pls->scmap1l( false, 2, i, h, l, s, rev );

    delete[] i;
    delete[] h;
    delete[] l;
    delete[] s;
    delete[] rev;
}

// Does a series of 3-d plots for a given data set, with different viewing
// options in each plot.

x08::x08( int argc, char **argv )
{
    int       i, j, k;
    const int LEVELS = 10;

    PLFLT     *x = new PLFLT[ XPTS ];
    PLFLT     *y = new PLFLT[ YPTS ];
    PLFLT     **z;
    PLFLT     *clevel = new PLFLT[LEVELS];
    PLFLT     dx      = 2. / (PLFLT) ( XPTS - 1 );
    PLFLT     dy      = 2. / (PLFLT) ( YPTS - 1 );

    PLFLT     xx, yy, r;
    PLFLT     zmin = 0.0, zmax = 0.0;

    int       ifshade;

    PLINT     indexxmin  = 0;
    PLINT     indexxmax  = XPTS;
    PLINT     *indexymin = new PLINT[ XPTS ];
    PLINT     *indexymax = new PLINT[ XPTS ];
    PLFLT     **zlimited;

    // parameters of ellipse (in x, y index coordinates) that limits the data.
    // x0, y0 correspond to the exact floating point centre of the index
    // range.
    PLFLT x0 = 0.5 * (PLFLT) ( XPTS - 1 );
    PLFLT a  = 0.9 * x0;
    PLFLT y0 = 0.5 * (PLFLT) ( YPTS - 1 );
    PLFLT b  = 0.7 * y0;
    PLFLT square_root;

    pls = new plstream();

    // Parse and process command line arguments.
    pls->MergeOpts( options, "x08c options", NULL );
    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize plplot.

    pls->init();

    pls->Alloc2dGrid( &z, XPTS, YPTS );

    for ( i = 0; i < XPTS; i++ )
    {
        x[i] = -1. + (PLFLT) i * dx;
        if ( rosen )
            x[i] *= 1.5;
    }

    for ( j = 0; j < YPTS; j++ )
    {
        y[j] = -1. + (PLFLT) j * dy;
        if ( rosen )
            y[j] += 0.5;
    }

    for ( i = 0; i < XPTS; i++ )
    {
        xx = x[i];
        for ( j = 0; j < YPTS; j++ )
        {
            yy = y[j];
            if ( rosen )
            {
                z[i][j] = pow( ( 1. - xx ), 2. ) + 100 * pow( ( yy - pow( xx, 2. ) ), 2. );
                // The log argument might be zero for just the right grid.
                if ( z[i][j] > 0. )
                    z[i][j] = log( z[i][j] );
                else
                    z[i][j] = -5.; // -MAXFLOAT would mess-up up the scale
            }
            else
            {
                r       = sqrt( xx * xx + yy * yy );
                z[i][j] = exp( -r * r ) * cos( 2.0 * M_PI * r );
            }
            if ( i == 0 && j == 0 )
            {
                zmin = z[i][j];
                zmax = zmin;
            }
            if ( zmin > z[i][j] )
                zmin = z[i][j];
            if ( zmax < z[i][j] )
                zmax = z[i][j];
        }
    }

    //  Allocate and calculate y index ranges and corresponding zlimited.
    pls->Alloc2dGrid( &zlimited, XPTS, YPTS );

    for ( i = indexxmin; i < indexxmax; i++ )
    {
        square_root = sqrt( 1. - MIN( 1., pow( ( i - x0 ) / a, 2. ) ) );
        // Add 0.5 to find nearest integer and therefore preserve symmetry
        // with regard to lower and upper bound of y range.
        indexymin[i] = MAX( 0, (PLINT) ( 0.5 + y0 - b * square_root ) );
        // indexymax calculated with the convention that it is 1
        // greater than highest valid index.
        indexymax[i] = MIN( YPTS, 1 + (PLINT) ( 0.5 + y0 + b * square_root ) );
        for ( j = indexymin[i]; j < indexymax[i]; j++ )
            zlimited[i][j] = z[i][j];
    }

    PLFLT step = ( zmax - zmin ) / ( LEVELS + 1 );
    for ( i = 0; i < LEVELS; i++ )
        clevel[i] = zmin + step * ( i + 1 );

    pls->lightsource( 1., 1., 1. );
    for ( k = 0; k < 2; k++ )
    {
        for ( ifshade = 0; ifshade < 5; ifshade++ )
        {
            pls->adv( 0 );
            pls->vpor( 0.0, 1.0, 0.0, 0.9 );
            pls->wind( -1.0, 1.0, -0.9, 1.1 );
            pls->col0( 3 );
            pls->mtex( "t", 1.0, 0.5, 0.5, title[k] );
            pls->col0( 1 );
            if ( rosen )
                pls->w3d( 1.0, 1.0, 1.0, -1.5, 1.5, -0.5, 1.5, zmin, zmax,
                    alt[k], az[k] );
            else
                pls->w3d( 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, zmin, zmax,
                    alt[k], az[k] );
            pls->box3( "bnstu", "x axis", 0.0, 0,
                "bnstu", "y axis", 0.0, 0,
                "bcdmnstuv", "z axis", 0.0, 0 );

            pls->col0( 2 );

            switch ( ifshade )
            {
            case 0: // diffuse light surface plot
                cmap1_init( 1 );
                pls->surf3d( x, y, z, XPTS, YPTS, 0, NULL, 0 );
                break;
            case 1: // magnitude colored plot
                cmap1_init( 0 );
                pls->surf3d( x, y, z, XPTS, YPTS, MAG_COLOR, NULL, 0 );
                break;
            case 2: //  magnitude colored plot with faceted squares
                cmap1_init( 0 );
                pls->surf3d( x, y, z, XPTS, YPTS, MAG_COLOR | FACETED, NULL, 0 );
                break;
            case 3: // magnitude colored plot with contours
                cmap1_init( 0 );
                pls->surf3d( x, y, z, XPTS, YPTS, MAG_COLOR | SURF_CONT | BASE_CONT, clevel, LEVELS );
                break;
            case 4:  // magnitude colored plot with contours and index limits.
                cmap1_init( 0 );
                pls->surf3dl( x, y, (const PLFLT * const *) zlimited, XPTS, YPTS, MAG_COLOR | SURF_CONT | BASE_CONT, clevel, LEVELS, indexxmin, indexxmax, indexymin, indexymax );
            }
        }
    }

    pls->Free2dGrid( z, XPTS, YPTS );

    delete[] x;
    delete[] y;
    delete[] clevel;
    delete pls;
}


int main( int argc, char **argv )
{
    x08 *x = new x08( argc, argv );

    delete x;
}


//--------------------------------------------------------------------------
//                              End of x08.cc
//--------------------------------------------------------------------------
