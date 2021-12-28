#include<windows.h>
#include<widget/plotview.h>
#include<cdlog.h>
#include<plstream.h>

static void plot0(plstream*pls);
int main(int argc,const char*argv[]){
    setenv("LANG","zh.CN",1);
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    PlotView*plv=new PlotView(640,480);
    plstream*pls=plv->getStream();
    pls->sori( 1 );
    plot0(pls);
    w->addView(plv);
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
