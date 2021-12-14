// Shade plot demo.

// Copyright (C) 1993-2001 Maurice LeBrun
// Copyright (C) 2000-2018 Alan W. Irwin
// Copyright (C) 2004 Rafael Laboissiere
// Copyright (C) 2007-2011 Andrew Ross
// Copyright (C) 2010 Hezekiah M. Carty

#include "plcdemos.h"

#define XPTS    35              // Data points in x
#define YPTS    46              // Data points in y

PLFLT           z[XPTS][YPTS], zmin, zmax;
PLFLT_NC_VECTOR zIliffe[XPTS];
// Function prototypes

static void     plot1( void );
static void     plot2( void );
static void     plot3( void );
//static void     cmap1_init1( void );
static void     cmap1_init2( void );


//--------------------------------------------------------------------------
// main
//
// Does a variety of shade plots.
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
    int   i, j;
    PLFLT xx, yy;

// Parse and process command line arguments

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

// Set up color map 0
//
//  plscmap0n(3);
//
// Set up color map 1

    cmap1_init2();

// Initialize plplot

    plinit();

// Calculate z data array that is statically allocated above.

    for ( i = 0; i < XPTS; i++ )
    {
        xx = (PLFLT) ( i - ( XPTS / 2 ) ) / (PLFLT) ( XPTS / 2 );
        for ( j = 0; j < YPTS; j++ )
        {
            yy = (PLFLT) ( j - ( YPTS / 2 ) ) / (PLFLT) ( YPTS / 2 ) - 1.0;

            z[i][j] = xx * xx - yy * yy + ( xx - yy ) / ( xx * xx + yy * yy + 0.1 );
        }
    }

    // Calculate corresponding Iliffe vector, maximum, and minimum.
    plStatic2dGrid( (PLFLT_NC_MATRIX) zIliffe, (PLFLT_VECTOR) ( &z[0][0] ), XPTS, YPTS );
    plMinMax2dGrid( (PLFLT_MATRIX) zIliffe, XPTS, YPTS, &zmax, &zmin );

    plot1();
    plot2();
    plot3();

    plend();
    exit( 0 );
}

//--------------------------------------------------------------------------
// cmap1_init1
//
// Initializes color map 1 in HLS space.
//--------------------------------------------------------------------------

//static void
//cmap1_init1( void )
//{
//    PLFLT i[4], h[4], l[4], s[4];
//
//    i[0] = 0;           // left boundary
//    i[1] = 0.45;        // just before center
//    i[2] = 0.55;        // just after center
//    i[3] = 1;           // right boundary
//
//    h[0] = 260;         // hue -- low: blue-violet
//    h[1] = 260;         // only change as we go over vertex
//    h[2] = 20;          // hue -- high: red
//    h[3] = 20;          // keep fixed
//
//#if 1
//    l[0] = 0.5;         // lightness -- low
//    l[1] = 0.0;         // lightness -- center
//    l[2] = 0.0;         // lightness -- center
//    l[3] = 0.5;         // lightness -- high
//#else
//    plscolbg( 255, 255, 255 );
//    l[0] = 0.5;         // lightness -- low
//    l[1] = 1.0;         // lightness -- center
//    l[2] = 1.0;         // lightness -- center
//    l[3] = 0.5;         // lightness -- high
//#endif
//    s[0] = 1;           // maximum saturation
//    s[1] = 1;           // maximum saturation
//    s[2] = 1;           // maximum saturation
//    s[3] = 1;           // maximum saturation
//
//    c_plscmap1l( 0, 4, i, h, l, s, NULL );
//}

//--------------------------------------------------------------------------
// cmap1_init2
//
// Initializes color map 1 in HLS space.
//--------------------------------------------------------------------------

static void
cmap1_init2( void )
{
    PLFLT i[4], h[4], l[4], s[4];

    i[0] = 0;           // left boundary
    i[1] = 0.45;        // just before center
    i[2] = 0.55;        // just after center
    i[3] = 1;           // right boundary

    h[0] = 260;         // hue -- low: blue-violet
    h[1] = 260;         // only change as we go over vertex
    h[2] = 20;          // hue -- high: red
    h[3] = 20;          // keep fixed

#if 1
    l[0] = 0.6;         // lightness -- low
    l[1] = 0.0;         // lightness -- center
    l[2] = 0.0;         // lightness -- center
    l[3] = 0.6;         // lightness -- high
#else
    plscolbg( 255, 255, 255 );
    l[0] = 0.5;         // lightness -- low
    l[1] = 1.0;         // lightness -- center
    l[2] = 1.0;         // lightness -- center
    l[3] = 0.5;         // lightness -- high
#endif
    s[0] = 1;           // saturation -- low
    s[1] = 0.5;         // saturation -- center
    s[2] = 0.5;         // saturation -- center
    s[3] = 1;           // saturation -- high

    c_plscmap1l( 0, 4, i, h, l, s, NULL );
}

//--------------------------------------------------------------------------
// plot1
//
// Illustrates a single shaded region.
//--------------------------------------------------------------------------

static void
plot1( void )
{
    PLFLT shade_min, shade_max, sh_color;
    PLINT sh_cmap   = 0;
    PLINT min_color = 0, max_color = 0;
    PLFLT sh_width, min_width = 0., max_width = 0.;

    pladv( 0 );
    plvpor( 0.1, 0.9, 0.1, 0.9 );
    plwind( -1.0, 1.0, -1.0, 1.0 );

// Plot using identity transform

    shade_min = zmin + ( zmax - zmin ) * 0.4;
    shade_max = zmin + ( zmax - zmin ) * 0.6;
    sh_color  = 7;
    sh_width  = 2.;
    min_color = 9;
    max_color = 2;
    min_width = 2.;
    max_width = 2.;

    plpsty( 8 );
    plshade( (PLFLT_MATRIX) zIliffe, XPTS, YPTS, NULL, -1., 1., -1., 1.,
        shade_min, shade_max,
        sh_cmap, sh_color, sh_width,
        min_color, min_width, max_color, max_width,
        plfill, 1, NULL, NULL );

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
    pllab( "distance", "altitude", "Bogon flux" );
}

//--------------------------------------------------------------------------
// plot2
//
// Illustrates multiple adjacent shaded regions, using different fill
// patterns for each region.
//--------------------------------------------------------------------------

static void
plot2( void )
{
    PLFLT        shade_min, shade_max, sh_color;
    PLINT        sh_cmap   = 0;
    PLINT        min_color = 0, max_color = 0;
    PLFLT        sh_width, min_width = 0., max_width = 0.;
    int          i;
    static PLINT nlin[10]   = { 1, 1, 1, 1, 1, 2, 2, 2, 2, 2 };
    static PLINT inc[10][2] = { { 450,    0 }, { -450,    0 }, { 0,   0 }, { 900,   0 },
                                { 300,    0 }, {  450, -450 }, { 0, 900 }, {   0, 450 },
                                { 450, -450 }, {    0,  900 } };
    static PLINT del[10][2] = { { 2000, 2000 }, { 2000, 2000 }, { 2000, 2000 },
                                { 2000, 2000 }, { 2000, 2000 }, { 2000, 2000 },
                                { 2000, 2000 }, { 2000, 2000 }, { 4000, 4000 },
                                { 4000, 2000 } };

    sh_width = 2.;

    pladv( 0 );
    plvpor( 0.1, 0.9, 0.1, 0.9 );
    plwind( -1.0, 1.0, -1.0, 1.0 );

// Plot using identity transform

    for ( i = 0; i < 10; i++ )
    {
        shade_min = zmin + ( zmax - zmin ) * i / 10.0;
        shade_max = zmin + ( zmax - zmin ) * ( i + 1 ) / 10.0;
        sh_color  = i + 6;
        plpat( nlin[i], inc[i], del[i] );

        plshade( (PLFLT_MATRIX) zIliffe, XPTS, YPTS, NULL, -1., 1., -1., 1.,
            shade_min, shade_max,
            sh_cmap, sh_color, sh_width,
            min_color, min_width, max_color, max_width,
            plfill, 1, NULL, NULL );
    }

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
    pllab( "distance", "altitude", "Bogon flux" );
}

//--------------------------------------------------------------------------
// plot3
//
// Illustrates shaded regions in 3d, using a different fill pattern for
// each region.
//--------------------------------------------------------------------------

static void
plot3( void )
{
    static PLFLT xx[2][5] = { { -1.0, 1.0, 1.0, -1.0, -1.0 },
                              { -1.0, 1.0, 1.0, -1.0, -1.0 } };
    static PLFLT yy[2][5] = { {  1.0,  1.0, 0.0, 0.0,  1.0 },
                              { -1.0, -1.0, 0.0, 0.0, -1.0 } };
    static PLFLT zz[2][5] = { { 0.0, 0.0, 1.0, 1.0, 0.0 },
                              { 0.0, 0.0, 1.0, 1.0, 0.0 } };

    pladv( 0 );
    plvpor( 0.1, 0.9, 0.1, 0.9 );
    plwind( -1.0, 1.0, -1.0, 1.0 );
    plw3d( 1., 1., 1., -1.0, 1.0, -1.0, 1.0, 0.0, 1.5, 30, -40 );

// Plot using identity transform

    plcol0( 1 );
    plbox3( "bntu", "X", 0.0, 0, "bntu", "Y", 0.0, 0, "bcdfntu", "Z", 0.5, 0 );
    plcol0( 2 );
    pllab( "", "", "3-d polygon filling" );

    plcol0( 3 );
    plpsty( 1 );
    plline3( 5, xx[0], yy[0], zz[0] );
    plfill3( 4, xx[0], yy[0], zz[0] );
    plpsty( 2 );
    plline3( 5, xx[1], yy[1], zz[1] );
    plfill3( 4, xx[1], yy[1], zz[1] );
}
