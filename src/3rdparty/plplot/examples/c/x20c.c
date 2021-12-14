//      plimage demo
//
//

#include "plcdemos.h"
#include "plevent.h"
#include <math.h>

#define XDIM    260
#define YDIM    220

void save_plot( char * );
void gray_cmap( PLINT );
int read_img( PLCHAR_VECTOR, PLFLT ***, int *, int *, int * );
int get_clip( PLFLT *, PLFLT *, PLFLT *, PLFLT * );

int  dbg           = 0;
int  nosombrero    = 0;
int  nointeractive = 0;
char *f_name       = NULL;

struct stretch_data
{
    PLFLT xmin, xmax, ymin, ymax;
    PLFLT stretch;
};

static PLOptionTable options[] = {
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

int
main( int argc, char *argv[] )
{
    PLFLT               x[XDIM], y[YDIM], **z, **r;
    PLFLT               xi, yi, xe, ye;
    int                 i, j, width, height, num_col;
    PLFLT               **img_f;
    PLFLT               img_min;
    PLFLT               img_max;
    struct stretch_data stretch;
    PLcGrid2            cgrid2;
    PLFLT               xx, yy;

    // Bugs in plimage():
    // -at high magnifications, the left and right edge are ragged, try
    //    ./x20c -dev xwin -wplt 0.3,0.3,0.6,0.6 -ori 0.5
    // AWI comment as of 2016-02-04.  This bug appears to no longer exist.

    //
    // Bugs in x20c.c:
    // -if the window is resized after a selection is made on "Chloe", when
    //  making a new selection the old one will re-appear.

    //  AWI comment as of 2016-02-04.  I confirm that both the old
    // and new selection areas are outlined, but only the new one
    // is actually used for the selection so this is relatively harmless
    // for this application.  Nevertheless, it should be looked into as a
    // likely rendering issue with plbuf.

    // Parse and process command line arguments

    plMergeOpts( options, "x20c options", NULL );
    plparseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize plplot

    plinit();

    plAlloc2dGrid( &z, XDIM, YDIM );

    // view image border pixels
    if ( dbg )
    {
        plenv( 1., (PLFLT) XDIM, 1., (PLFLT) YDIM, 1, 1 ); // no plot box

        // build a one pixel square border, for diagnostics
        for ( i = 0; i < XDIM; i++ )
            z[i][YDIM - 1] = 1.; // right
        for ( i = 0; i < XDIM; i++ )
            z[i][0] = 1.;        // left

        for ( i = 0; i < YDIM; i++ )
            z[0][i] = 1.;        // top
        for ( i = 0; i < YDIM; i++ )
            z[XDIM - 1][i] = 1.; // botton

        pllab( "...around a blue square.", " ", "A red border should appear..." );

        plimage( (PLFLT_MATRIX) z, XDIM, YDIM,
            1., (PLFLT) XDIM, 1., (PLFLT) YDIM, 0., 0.,
            1., (PLFLT) XDIM, 1., (PLFLT) YDIM );
    }

    // sombrero-like demo
    if ( !nosombrero )
    {
        plAlloc2dGrid( &r, XDIM, YDIM );
        plcol0( 2 ); // draw a yellow plot box, useful for diagnostics! :(
        plenv( 0., 2. * M_PI, 0, 3. * M_PI, 1, -1 );

        for ( i = 0; i < XDIM; i++ )
            x[i] = i * 2. * M_PI / (PLFLT) ( XDIM - 1 );
        for ( i = 0; i < YDIM; i++ )
            y[i] = i * 3. * M_PI / (PLFLT) ( YDIM - 1 );

        for ( i = 0; i < XDIM; i++ )
            for ( j = 0; j < YDIM; j++ )
            {
                r[i][j] = sqrt( x[i] * x[i] + y[j] * y[j] ) + 1e-3;
                z[i][j] = sin( r[i][j] ) / ( r[i][j] );
            }

        pllab( "No, an amplitude clipped \"sombrero\"", "", "Saturn?" );
        plptex( 2., 2., 3., 4., 0., "Transparent image" );
        plimage( (PLFLT_MATRIX) z, XDIM, YDIM, 0., 2. * M_PI, 0., 3. * M_PI, 0.05, 1.,
            0., 2. * M_PI, 0., 3. * M_PI );
        plFree2dGrid( r, XDIM, YDIM );

        // save the plot
        if ( f_name )
            save_plot( f_name );
    }

    plFree2dGrid( z, XDIM, YDIM );

    // read Chloe image
    // Note we try three different locations to cover the case where this
    // examples is being run from the test_c.sh script or directly on Windows
    if ( read_img( "Chloe.pgm", &img_f, &width, &height, &num_col ) )
    {
        if ( read_img( "../Chloe.pgm", &img_f, &width, &height, &num_col ) )
        {
            if ( read_img( "../../Chloe.pgm", &img_f, &width, &height, &num_col ) )
            {
                fprintf( stderr, "No such file" );
                plend();
                exit( 1 );
            }
        }
    }

    // set gray colormap
    gray_cmap( num_col );

    // display Chloe
    plenv( 1., width, 1., height, 1, -1 );

    if ( !nointeractive )
        pllab( "Set and drag Button 1 to (re)set selection, Button 2 to finish.", " ", "Chloe..." );
    else
        pllab( "", " ", "Chloe..." );

    plimage( (PLFLT_MATRIX) img_f, width, height, 1., width, 1., height, 0., 0.,
        1., width, 1., height );

    // plend();exit(0);

    // selection/expansion demo
    if ( !nointeractive )
    {
        xi = 25.; xe = 130.;
        yi = 235.; ye = 125.;

        if ( get_clip( &xi, &xe, &yi, &ye ) ) // get selection rectangle
        {
            plend();
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

        // plbop();

        //
        // plspause(0), pladv(0), plspause(1), also works,
        // but the above question remains.
        // With this approach, the previous pause state is lost,
        // as there is no API call to get its current state.
        //

        plspause( 0 );
        pladv( 0 );

        // display selection only
        plimage( (PLFLT_MATRIX) img_f, width, height, 1., width, 1., height, 0., 0., xi, xe, ye, yi );

        plspause( 1 );

        // zoom in selection
        plenv( xi, xe, ye, yi, 1, -1 );
        plimage( (PLFLT_MATRIX) img_f, width, height, 1., width, 1., height, 0., 0., xi, xe, ye, yi );
    }

    // Base the dynamic range on the image contents.
    plMinMax2dGrid( (PLFLT_MATRIX) img_f, width, height, &img_max, &img_min );

    // Draw a saturated version of the original image.  Only use the middle 50%
    // of the image's full dynamic range.
    plcol0( 2 );
    plenv( 0, width, 0, height, 1, -1 );
    pllab( "", "", "Reduced dynamic range image example" );
    plimagefr( (PLFLT_MATRIX) img_f, width, height, 0., width, 0., height, 0., 0., img_min + img_max * 0.25, img_max - img_max * 0.25, NULL, NULL );

    // Draw a distorted version of the original image, showing its full dynamic range.
    plenv( 0, width, 0, height, 1, -1 );
    pllab( "", "", "Distorted image example" );

    stretch.xmin    = 0;
    stretch.xmax    = width;
    stretch.ymin    = 0;
    stretch.ymax    = height;
    stretch.stretch = 0.5;

    // In C / C++ the following would work, with plimagefr directly calling
    // mypltr. For compatibilty with other language bindings the same effect
    // can be achieved by generating the transformed grid first and then
    // using pltr2.
    // plimagefr((const PLFLT **) img_f, width, height, 0., width, 0., height, 0., 0., img_min, img_max, mypltr, (PLPointer) &stretch);

    plAlloc2dGrid( &cgrid2.xg, width + 1, height + 1 );
    plAlloc2dGrid( &cgrid2.yg, width + 1, height + 1 );
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

    plimagefr( (PLFLT_MATRIX) img_f, width, height, 0., width, 0., height, 0., 0., img_min, img_max, pltr2, &cgrid2 );

    plFree2dGrid( cgrid2.xg, width + 1, height + 1 );
    plFree2dGrid( cgrid2.yg, width + 1, height + 1 );
    plFree2dGrid( img_f, width, height );

    plend();
    exit( 0 );
}

// read image from file in binary ppm format
int read_img( PLCHAR_VECTOR fname, PLFLT ***img_f, int *width, int *height, int *num_col )
{
    FILE          *fp;
    unsigned char *img;
    char          ver[80];
    int           i, j, w, h;
    PLFLT         **imf;

    // naive grayscale binary ppm reading. If you know how to, improve it
    if ( ( fp = fopen( fname, "rb" ) ) == NULL )
        return 1;

    if ( fscanf( fp, "%s\n", ver ) != 1 ) // version
    {
        fclose( fp );
        return 1;
    }
    // printf("version: %s\n", ver);

    if ( strcmp( ver, "P5" ) ) // I only understand this!
    {
        fclose( fp );
        return 1;
    }

    while ( ( i = fgetc( fp ) ) == '#' )
    {
        if ( fgets( ver, 80, fp ) == NULL ) // comments
        {
            fclose( fp );
            return 1;
        }
        // printf("%s", ver);
    }
    ungetc( i, fp );

    if ( fscanf( fp, "%d%d%d\n", &w, &h, num_col ) != 3 ) // width, height num colors
    {
        fclose( fp );
        return 1;
    }
    // printf("width=%d height=%d num_col=%d\n", w, h, *num_col);

    img = (unsigned char *) malloc( (size_t) ( w * h ) * sizeof ( char ) );
    plAlloc2dGrid( &imf, w, h );

    if ( (int) fread( img, sizeof ( char ), (size_t) ( w * h ), fp ) != w * h )
    {
        fclose( fp );
        free( img );
        plFree2dGrid( imf, w, h );
        return 1;
    }
    fclose( fp );

    for ( i = 0; i < w; i++ )
        for ( j = 0; j < h; j++ )
            imf[i][j] = img[( h - 1 - j ) * w + i];
    // flip image up-down

    free( img );

    *width  = w;
    *height = h;
    *img_f  = imf;
    return 0;
}

// save plot
void save_plot( char *fname )
{
    PLINT cur_strm, new_strm;

    plgstrm( &cur_strm );    // get current stream
    plmkstrm( &new_strm );   // create a new one

    plsdev( "psc" );         // new device type. Use a known existing driver
    plsfnam( fname );        // file name

    plcpstrm( cur_strm, 0 ); // copy old stream parameters to new stream
    plreplot();              // do the save
    plend1();                // close new device

    plsstrm( cur_strm );     // and return to previous one
}

//  get selection square interactively
int get_clip( PLFLT *xi, PLFLT *xe, PLFLT *yi, PLFLT *ye )
{
    PLGraphicsIn gin;
    PLFLT        xxi = *xi, yyi = *yi, xxe = *xe, yye = *ye, t;
    PLINT        st, start = 0;

    plxormod( 1, &st ); // enter xor mode to draw a selection rectangle

    if ( st )           // driver has xormod capability, continue
    {
        while ( 1 )
        {
            PLFLT sx[5], sy[5];

            plxormod( 0, &st );
            plGetCursor( &gin );
            plxormod( 1, &st );

            if ( gin.button == 1 )
            {
                xxi = gin.wX; yyi = gin.wY;
                if ( start )
                    plline( 5, sx, sy ); // clear previous rectangle

                start = 0;

                sx[0] = xxi; sy[0] = yyi;
                sx[4] = xxi; sy[4] = yyi;
            }

            if ( gin.state & 0x100 )
            {
                xxe = gin.wX; yye = gin.wY;
                if ( start )
                    plline( 5, sx, sy ); // clear previous rectangle

                start = 1;

                sx[2] = xxe; sy[2] = yye;
                sx[1] = xxe; sy[1] = yyi;
                sx[3] = xxi; sy[3] = yye;
                plline( 5, sx, sy ); // draw new rectangle
            }

            if ( gin.button == 3 || gin.keysym == PLK_Return || gin.keysym == 'Q' )
            {
                if ( start )
                    plline( 5, sx, sy ); // clear previous rectangle
                break;
            }
        }
        plxormod( 0, &st ); // leave xor mod

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
    else // driver has no xormod capability, just do nothing
        return 0;
}

// set gray colormap
void gray_cmap( PLINT num_col )
{
    PLFLT r[2], g[2], b[2], pos[2];

    r[0] = g[0] = b[0] = 0.0;
    r[1] = g[1] = b[1] = 1.0;

    pos[0] = 0.0;
    pos[1] = 1.0;

    plscmap1n( num_col );
    plscmap1l( 1, 2, pos, r, g, b, NULL );
}
