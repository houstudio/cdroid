//--------------------------------------------------------------------------
// Copyright (C) 2004  Andrew Ross
// Copyright (C) 2004-2014 Alan W. Irwin
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
// Implementation of PLplot example 20 in C++.
//--------------------------------------------------------------------------

#include "plc++demos.h"
#include "plevent.h"
#include <fstream>

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x20 {
public:
    x20( int, char ** );
    void save_plot( const char * );
    void gray_cmap( PLINT );
    int read_img( const char *, PLFLT ***, int *, int *, int * );
    int get_clip( PLFLT *, PLFLT *, PLFLT *, PLFLT * );

private:
    // Class data
    plstream             *pls;

    static const int     XDIM;
    static const int     YDIM;

    static int           dbg;
    static int           nosombrero;
    static int           nointeractive;
    static char          *f_name;

    static PLOptionTable options[];
};

const int x20::XDIM                = 260;
const int x20::YDIM                = 220;
int       x20::      dbg           = 0;
int       x20::      nosombrero    = 0;
int       x20::      nointeractive = 0;
char      *x20::    f_name         = NULL;

struct stretch_data
{
    PLFLT xmin, xmax, ymin, ymax;
    PLFLT stretch;
};

PLOptionTable x20::options[] = {
    {
        "dbg",                  // extra debugging plot
        NULL,
        NULL,
        &dbg,
        PL_OPT_BOOL,
        "-dbg",
        "Extra debugging plot"
    },
    {
        "nosombrero",                   // Turns on test of xor function
        NULL,
        NULL,
        &nosombrero,
        PL_OPT_BOOL,
        "-nosombrero",
        "No sombrero plot"
    },
    {
        "nointeractive",                // Turns on test of xor function
        NULL,
        NULL,
        &nointeractive,
        PL_OPT_BOOL,
        "-nointeractive",
        "No interactive selection"
    },
    {
        "save",                 // For saving in postscript
        NULL,
        NULL,
        &f_name,
        PL_OPT_STRING,
        "-save filename",
        "Save sombrero plot in color postscript `filename'"
    },
    {
        NULL,                   // option
        NULL,                   // handler
        NULL,                   // client data
        NULL,                   // address of variable to set
        0,                      // mode flag
        NULL,                   // short syntax
        NULL
    }                           // long syntax
};

// Transformation function
static void
mypltr( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, PLPointer pltr_data )
{
    struct stretch_data *s = (struct stretch_data *) pltr_data;
    PLFLT x0, y0, dy;
    x0  = ( s->xmin + s->xmax ) * 0.5;
    y0  = ( s->ymin + s->ymax ) * 0.5;
    dy  = ( s->ymax - s->ymin ) * 0.5;
    *tx = x0 + ( x0 - x ) * ( 1.0 - s->stretch * cos( ( y - y0 ) / dy * M_PI * 0.5 ) );
    *ty = y;
}

x20::x20( int argc, char ** argv )
{
    PLFLT *x = new PLFLT[XDIM];
    PLFLT               *y = new PLFLT[YDIM];
    PLFLT               **z, **r;
    PLFLT               xi, yi, xe, ye;
    int                 i, j, width, height, num_col;
    PLFLT               **img_f;
    PLFLT               img_min;
    PLFLT               img_max;
    struct stretch_data stretch;
    PLcGrid2            cgrid2;
    PLFLT               xx, yy;

    // plplot initialization

    pls = new plstream();

    // Parse and process command line arguments.
    pls->MergeOpts( options, "x20c options", NULL );
    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize PLplot.
    pls->init();

    pls->Alloc2dGrid( &z, XDIM, YDIM );

    // view image border pixels
    if ( dbg )
    {
        pls->env( 1., (PLFLT) XDIM, 1., (PLFLT) YDIM, 1, 1 ); // no plot box

        // build a one pixel square border, for diagnostics
        for ( i = 0; i < XDIM; i++ )
            z[i][YDIM - 1] = 1.; // right
        for ( i = 0; i < XDIM; i++ )
            z[i][0] = 1.;        // left

        for ( i = 0; i < YDIM; i++ )
            z[0][i] = 1.;        // top
        for ( i = 0; i < YDIM; i++ )
            z[XDIM - 1][i] = 1.; // botton

        pls->lab( "...around a blue square.", " ", "A red border should appear..." );

        pls->image( z, XDIM, YDIM,
            1., (PLFLT) XDIM, 1., (PLFLT) YDIM, 0., 0.,
            1., (PLFLT) XDIM, 1., (PLFLT) YDIM );
    }

    // sombrero-like demo
    if ( !nosombrero )
    {
        pls->Alloc2dGrid( &r, XDIM, YDIM );
        pls->col0( 2 ); // draw a yellow plot box, useful for diagnostics! :(
        pls->env( 0., 2. * M_PI, 0, 3. * M_PI, 1, -1 );

        for ( i = 0; i < XDIM; i++ )
            x[i] = i * 2. * M_PI / ( XDIM - 1 );
        for ( i = 0; i < YDIM; i++ )
            y[i] = i * 3. * M_PI / ( YDIM - 1 );

        for ( i = 0; i < XDIM; i++ )
            for ( j = 0; j < YDIM; j++ )
            {
                r[i][j] = sqrt( x[i] * x[i] + y[j] * y[j] ) + 1e-3;
                z[i][j] = sin( r[i][j] ) / ( r[i][j] );
            }

        pls->lab( "No, an amplitude clipped \"sombrero\"", "", "Saturn?" );
        pls->ptex( 2., 2., 3., 4., 0., "Transparent image" );
        pls->image( z, XDIM, YDIM, 0., 2. * M_PI, 0, 3. * M_PI, 0.05, 1.,
            0., 2. * M_PI, 0, 3. * M_PI );
        pls->Free2dGrid( r, XDIM, YDIM );

        // save the plot
        if ( f_name )
            save_plot( f_name );
    }

    pls->Free2dGrid( z, XDIM, YDIM );

    // read Chloe image
    if ( read_img( "Chloe.pgm", &img_f, &width, &height, &num_col ) )
    {
        if ( read_img( "../Chloe.pgm", &img_f, &width, &height, &num_col ) )
        {
            if ( read_img( "../../Chloe.pgm", &img_f, &width, &height, &num_col ) )
            {
                cout << "No such file - aborting" << endl;
                delete pls;
                exit( 1 );
            }
        }
    }

    // set gray colormap
    gray_cmap( num_col );

    // display Chloe
    pls->env( 1., width, 1., height, 1, -1 );

    if ( !nointeractive )
        pls->lab( "Set and drag Button 1 to (re)set selection, Button 2 to finish.", " ", "Chloe..." );
    else
        pls->lab( "", " ", "Chloe..." );

    pls->image( img_f, width, height, 1., width, 1., height, 0., 0.,
        1., width, 1., height );

    // selection/expansion demo
    if ( !nointeractive )
    {
        xi = 25.; xe = 130.;
        yi = 235.; ye = 125.;

        if ( get_clip( &xi, &xe, &yi, &ye ) ) // get selection rectangle
        {
            delete pls;
            exit( 0 );
        }

        //
        // I'm unable to continue, clearing the plot and advancing to the next
        // one, without hiting the enter key, or pressing the button... help!
        //
        // Forcing the xwin driver to leave locate mode and destroying the
        // xhairs (in GetCursorCmd()) solves some problems, but I still have
        // to press the enter key or press Button-2 to go to next plot, even
        // if a pladv() is not present!  Using plbop() solves the problem, but
        // it shouldn't be needed!
        //

        // pls->bop();

        //
        // spause(false), adv(0), spause(true), also works,
        // but the above question remains.
        // With this approach, the previous pause state is lost,
        // as there is no API call to get its current state.
        //

        pls->spause( false );
        pls->adv( 0 );

        // display selection only
        pls->image( img_f, width, height, 1., width, 1., height, 0., 0., xi, xe, ye, yi );

        pls->spause( true );

        // zoom in selection
        pls->env( xi, xe, ye, yi, 1, -1 );
        pls->image( img_f, width, height, 1., width, 1., height, 0., 0., xi, xe, ye, yi );
    }

    // Base the dynamic range on the image contents.
    pls->MinMax2dGrid( img_f, width, height, &img_max, &img_min );

    // Draw a saturated version of the original image.  Only use the middle 50%
    // of the image's full dynamic range.
    pls->col0( 2 );
    pls->env( 0, width, 0, height, 1, -1 );
    pls->lab( "", "", "Reduced dynamic range image example" );
    pls->imagefr( img_f, width, height, 0., width, 0., height, 0., 0., img_min + img_max * 0.25, img_max - img_max * 0.25, NULL, NULL );

    // Draw a distorted version of the original image, showing its full dynamic range.
    pls->env( 0, width, 0, height, 1, -1 );
    pls->lab( "", "", "Distorted image example" );

    stretch.xmin    = 0;
    stretch.xmax    = width;
    stretch.ymin    = 0;
    stretch.ymax    = height;
    stretch.stretch = 0.5;

    // In C / C++ the following would work, with plimagefr directly calling
    // mypltr. For compatibilty with other language bindings the same effect
    // can be achieved by generating the transformed grid first and then
    // using pltr2.
    //
    // plimagefr(img_f, width, height, 0., width, 0., height, 0., 0., img_min, img_max, mypltr, (PLPointer) &stretch);

    pls->Alloc2dGrid( &cgrid2.xg, width + 1, height + 1 );
    pls->Alloc2dGrid( &cgrid2.yg, width + 1, height + 1 );
    cgrid2.nx = width + 1;
    cgrid2.ny = height + 1;

    for ( i = 0; i <= width; i++ )
    {
        for ( j = 0; j <= height; j++ )
        {
            mypltr( i, j, &xx, &yy, ( PLPointer ) & stretch );
            cgrid2.xg[i][j] = xx;
            cgrid2.yg[i][j] = yy;
        }
    }

    pls->imagefr( img_f, width, height, 0., width, 0., height, 0., 0., img_min, img_max, plcallback::tr2, &cgrid2 );

    pls->Free2dGrid( cgrid2.xg, width + 1, height + 1 );
    pls->Free2dGrid( cgrid2.yg, width + 1, height + 1 );
    pls->Free2dGrid( img_f, width, height );

    delete pls;

    delete[] y;
    delete[] x;
}

// read image from file in binary ppm format
int x20::read_img( const char *fname, PLFLT ***img_f, int *width, int *height, int *num_col )
{
    ifstream      ifs( fname, ios::out | ios::binary );
    unsigned char *img;
    char          ver[80];
    char          c;
    int           i, j, w, h;
    PLFLT         **imf;

    // naive grayscale binary ppm reading. If you know how to, improve it
    if ( !ifs.is_open() )
        return 1;

    ifs.getline( ver, 80 );
    //cout << "version: " << ver << endl;

    if ( strcmp( ver, "P5" ) ) // I only understand this!
        return 1;

    ifs.read( &c, 1 );
    while ( c == '#' )
    {
        ifs.getline( ver, 80 ); // comments
        //cout << ver << endl;
        ifs.read( &c, 1 );
    }
    ifs.putback( c );

    ifs >> w >> h >> *num_col;
    ifs.getline( ver, 80 );
    //cout << "width=" << w << " height=" << h << " num_col=" << *num_col << endl;

    img = new unsigned char[w * h];
    pls->Alloc2dGrid( &imf, w, h );

    ifs.read( (char *) img, w * h );
    ifs.close();

    for ( i = 0; i < w; i++ )
        for ( j = 0; j < h; j++ )
            imf[i][j] = img[( h - 1 - j ) * w + i];
    // flip image up-down

    delete[] img;

    *width  = w;
    *height = h;
    *img_f  = imf;
    return 0;
}

// save plot
void x20::save_plot( const char *fname )
{
    plstream *pls2;

    pls2 = new plstream();       // create a new one

    pls2->sdev( "psc" );         // new device type. Use a known existing driver
    pls2->sfnam( fname );        // file name

    pls2->cpstrm( *pls, false ); // copy old stream parameters to new stream
    pls2->replot();              // do the save

    delete pls2;                 // close new device
}

//  get selection square interactively
int x20::get_clip( PLFLT *xi, PLFLT *xe, PLFLT *yi, PLFLT *ye )
{
    PLGraphicsIn gin;
    PLFLT        xxi   = *xi, yyi = *yi, xxe = *xe, yye = *ye, t;
    PLINT        start = 0;
    bool         st;

    pls->xormod( true, &st ); // enter xor mode to draw a selection rectangle

    if ( st )                 // driver has xormod capability, continue
    {
        while ( 1 )
        {
            PLFLT sx[5], sy[5];

            pls->xormod( false, &st );
            pls->GetCursor( &gin );
            pls->xormod( true, &st );

            if ( gin.button == 1 )
            {
                xxi = gin.wX; yyi = gin.wY;
                if ( start )
                    pls->line( 5, sx, sy ); // clear previous rectangle

                start = 0;

                sx[0] = xxi; sy[0] = yyi;
                sx[4] = xxi; sy[4] = yyi;
            }

            if ( gin.state & 0x100 )
            {
                xxe = gin.wX; yye = gin.wY;
                if ( start )
                    pls->line( 5, sx, sy ); // clear previous rectangle

                start = 1;

                sx[2] = xxe; sy[2] = yye;
                sx[1] = xxe; sy[1] = yyi;
                sx[3] = xxi; sy[3] = yye;
                pls->line( 5, sx, sy ); // draw new rectangle
            }

            if ( gin.button == 3 || gin.keysym == PLK_Return || gin.keysym == 'Q' )
            {
                if ( start )
                    pls->line( 5, sx, sy ); // clear previous rectangle
                break;
            }
        }
        pls->xormod( false, &st ); // leave xor mod

        if ( xxe < xxi )
        {
            t = xxi; xxi = xxe; xxe = t;
        }

        if ( yyi < yye )
        {
            t = yyi; yyi = yye; yye = t;
        }

        *xe = xxe; *xi = xxi;
        *ye = yye; *yi = yyi;

        return ( gin.keysym == 'Q' );
    }
    else  // driver has no xormod capability, just do nothing
        return 0;
}

// set gray colormap
void x20::gray_cmap( PLINT num_col )
{
    PLFLT r[2], g[2], b[2], pos[2];

    r[0] = g[0] = b[0] = 0.0;
    r[1] = g[1] = b[1] = 1.0;

    pos[0] = 0.0;
    pos[1] = 1.0;

    pls->scmap1n( num_col );
    pls->scmap1l( true, 2, pos, r, g, b, NULL );
}

int main( int argc, char ** argv )
{
    x20 *x = new x20( argc, argv );

    delete x;
}


//--------------------------------------------------------------------------
//                              End of x20.cc
//--------------------------------------------------------------------------
