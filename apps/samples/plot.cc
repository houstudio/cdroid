#include<windows.h>
#include<widget/plotview.h>
#include<cdlog.h>
#include<plstream.h>

static void plot0(plstream*pls);
static void plot1(plstream*pls);
int main(int argc,const char*argv[]){
    setenv("LANG","zh.CN",1);
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    PlotView*plv=new PlotView(640,480);
    plstream*pls=plv->getStream();
    if(argc==1){
        pls->sori( 1 );
        plot0(pls);
    }
    else plot1(pls);
    w->addView(plv);
    Runnable rr([plv](){
        plv->invalidate(true);
    });
    w->postDelayed(rr,100);
    return app.exec();
}

static void plot0(plstream*pls){
    int   i;
    char  text[4];
    PLFLT dtr, theta, dx, dy, r, offset;

    PLFLT *x0 = new PLFLT[361];
    PLFLT *y0 = new PLFLT[361];
    PLFLT *x  = new PLFLT[361];
    PLFLT *y  = new PLFLT[361];

    dtr = M_PI / 180.0;
    for ( i = 0; i <= 360; i++ )
    {
        x0[i] = cos( dtr * i );
        y0[i] = sin( dtr * i );
    }

     pls->env( -1.3, 1.3, -1.3, 1.3, 1, -2 );
    // Draw circles for polar grid
    for ( i = 1; i <= 10; i++ )
    {
        pls->arc( 0.0, 0.0, 0.1 * i, 0.1 * i, 0.0, 360.0, 0.0, 0 );
    }

    pls->col0( 2 );
    for ( i = 0; i <= 11; i++ )
    {
        theta = 30.0 * i;
        dx    = cos( dtr * theta );
        dy    = sin( dtr * theta );

        // Draw radial spokes for polar grid.
        pls->join( 0.0, 0.0, dx, dy );
        sprintf( text, "%d", (int) round( theta ) );

        // Write labels for angle.

        if ( theta < 9.99 )
        {
            offset = 0.45;
        }
        else if ( theta < 99.9 )
        {
            offset = 0.30;
        }
        else
        {
            offset = 0.15;
        }

        //Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
        if ( dx >= -0.00001 )
            pls->ptex( dx, dy, dx, dy, -offset, text );
        else
            pls->ptex( dx, dy, -dx, -dy, 1. + offset, text );
    }

    // Draw the graph.

    for ( i = 0; i <= 360; i++ )
    {
        r    = sin( dtr * ( 5 * i ) );
        x[i] = x0[i] * r;
        y[i] = y0[i] * r;
    }
    pls->col0( 3 );
    pls->line( 361, x, y );

    pls->col0( 4 );
    pls->mtex( "t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh" );

}

void cmap1_init(plstream*pls)
{
    PLFLT i[2];
    PLFLT h[2];
    PLFLT l[2];
    PLFLT s[2];
    bool  rev[2];

    i[0] = 0.0;       // left boundary
    i[1] = 1.0;       // right boundary

    h[0] = 240;       // blue -> green -> yellow ->
    h[1] = 0;         // -> red

    l[0] = 0.6;
    l[1] = 0.6;

    s[0] = 0.8;
    s[1] = 0.8;

    rev[0] = false;       // interpolate on front side of colour wheel.
    rev[1] = false;       // interpolate on front side of colour wheel.

    pls->scmap1n( 256 );
    pls->scmap1l( false, 2, i, h, l, s, rev );
}

static void plot1(plstream*pls){
    int   i, j, k;
    int XPTS=35;
    int YPTS=46;
    int LEVELS=10;
    int opt[]  = { 3, 3 };
    PLFLT alt[] = { 33.0, 17.0 };
    PLFLT az[] = { 24.0, 115.0 };
    const char*title[] = {
       "#frPLplot Example 11 - Alt=33, Az=24, Opt=3",
       "#frPLplot Example 11 - Alt=17, Az=115, Opt=3"
    };

    PLFLT *x = new PLFLT[ XPTS ];
    PLFLT *y = new PLFLT[ YPTS ];
    PLFLT **z;
    PLFLT zmin = 1E9, zmax = -1E-9;

    PLFLT xx, yy;
    int   nlevel  = LEVELS;
    PLFLT *clevel = new PLFLT[LEVELS];
    PLFLT step;



    //pls = new plstream();

    // Parse and process command line arguments.

    //pls->parseopts( &argc, argv, PL_PARSE_FULL );
    // Initialize plplot.

    //pls->init();

    pls->Alloc2dGrid( &z, XPTS, YPTS );

    for ( i = 0; i < XPTS; i++ )
        x[i] = 3. * (PLFLT) ( i - ( XPTS / 2 ) ) / (PLFLT) ( XPTS / 2 );

    for ( j = 0; j < YPTS; j++ )
        y[j] = 3. * (PLFLT) ( j - ( YPTS / 2 ) ) / (PLFLT) ( YPTS / 2 );

    for ( i = 0; i < XPTS; i++ )
    {
        xx = x[i];
        for ( j = 0; j < YPTS; j++ )
        {
            yy      = y[j];
            z[i][j] = 3. * ( 1. - xx ) * ( 1. - xx ) * exp( -( xx * xx ) - ( yy + 1. ) * ( yy + 1. ) ) -
                      10. * ( xx / 5. - pow( xx, 3. ) - pow( yy, 5. ) ) * exp( -xx * xx - yy * yy ) -
                      1. / 3. * exp( -( xx + 1 ) * ( xx + 1 ) - ( yy * yy ) );
            if ( false ) // Jungfraujoch/Interlaken
            {
                if ( z[i][j] < -1. )
                    z[i][j] = -1.;
            }
            if ( zmin > z[i][j] )
                zmin = z[i][j];
            if ( zmax < z[i][j] )
                zmax = z[i][j];
        }
    }

    step = ( zmax - zmin ) / ( nlevel + 1 );
    for ( i = 0; i < nlevel; i++ )
        clevel[i] = zmin + step + step * i;

    cmap1_init(pls);
    for ( k = 0; k < 1; k++ )
    {
        for ( i = 0; i < 4; i++ )
        {
            pls->adv( 0 );
            pls->col0( 1 );
            pls->vpor( 0.0, 1.0, 0.0, 0.9 );
            pls->wind( -1.0, 1.0, -1.0, 1.5 );

            pls->w3d( 1.0, 1.0, 1.2, -3.0, 3.0, -3.0, 3.0, zmin, zmax,
                alt[k], az[k] );
            pls->box3( "bnstu", "x axis", 0.0, 0,
                "bnstu", "y axis", 0.0, 0,
                "bcdmnstuv", "z axis", 0.0, 4 );

            pls->col0( 2 );

            // wireframe plot
            if ( i == 0 )
                pls->mesh( x, y, z, XPTS, YPTS, opt[k] );
            // magnitude colored wireframe plot
            else if ( i == 1 )
                pls->mesh( x, y, z, XPTS, YPTS, opt[k] | MAG_COLOR );

            // magnitude colored wireframe plot with sides
            else if ( i == 2 )
            {
                pls->plot3d( x, y, z, XPTS, YPTS, opt[k] | MAG_COLOR, true );
            }

            // magnitude colored wireframe plot with base contour
            else if ( i == 3 )
                pls->meshc( x, y, z, XPTS, YPTS, opt[k] | MAG_COLOR | BASE_CONT,
                    clevel, nlevel );


            pls->col0( 3 );
            pls->mtex( "t", 1.0, 0.5, 0.5, title[k] );
        }
    }
    pls->Free2dGrid( z, XPTS, YPTS );

}
