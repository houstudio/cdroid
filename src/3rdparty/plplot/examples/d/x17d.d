// Plots a simple stripchart with four pens.
//

import std.string;
import std.math;
import std.stdio;
import std.c.stdlib;
import plplot;


//--------------------------------------------------------------------------
// main program
//--------------------------------------------------------------------------
int main( char[][] args )
{
    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // If db is used the plot is much more smooth. However, because of the
    // async X behaviour, one does not have a real-time scripcharter.
    // plsetopt("db", "");
    // plsetopt("np", "");

    // User sets up plot completely except for window and data
    // Eventually settings in place when strip chart is created will be
    // remembered so that multiple strip charts can be used simultaneously.
    //

    // Specify some reasonable defaults for ymin and ymax
    // The plot will grow automatically if needed (but not shrink)
    PLFLT ymin = -0.1;
    PLFLT ymax = 0.1;

    // Specify initial tmin and tmax -- this determines length of window.
    // Also specify maximum jump in t
    // This can accomodate adaptive timesteps
    PLFLT tmin  = 0.;
    PLFLT tmax  = 10.;
    PLFLT tjump = 0.3;  // percentage of plot to jump

    // Axes options same as plbox.
    // Only automatic tick generation and label placement allowed
    // Eventually I'll make this fancier
    PLINT colbox = 1;
    PLINT collab = 3;
    PLINT[4] styline = [ 2, 3, 4, 5 ];  // pens color and line style
    PLINT[4] colline = [ 2, 3, 4, 5 ];

    // pens legend
    string[4] legline = [ "sum", "sin", "sin*noi", "sin+noi" ];

    PLFLT xlab = 0.; // legend position
    PLFLT ylab = 0.25;

    PLINT autoy = 1;    // autoscale y
    PLINT acc   = 1;    // don't scrip, accumulate

    // Initialize plplot
    plinit();

    pladv( 0 );
    plvsta();

    // Register our error variables with PLplot
    // From here on, we're handling all errors here
    PLINT pl_errcode;
    char[160] errmsg;
    plsError( &pl_errcode, errmsg.ptr );

    PLINT id1;
    plstripc( &id1, "bcnst", "bcnstv",
        tmin, tmax, tjump, ymin, ymax,
        xlab, ylab,
        autoy, acc,
        colbox, collab,
        colline, styline, legline,
        "t", "", "Strip chart demo" );

    if ( pl_errcode )
    {
        writefln( "%s", errmsg );  // TODO: to stderr
        return 1;
    }

    // Let plplot handle errors from here on
    plsError( null, null );

    autoy = 0;  // autoscale y
    acc   = 1;  // accumulate

    // This is to represent a loop over time
    // Let's try a random walk process
    PLFLT     y1 = 0.0, y2 = 0.0, y3 = 0.0, y4 = 0.0;
    PLFLT     dt = 0.1;

    const int nsteps = 1000;
    PLFLT     t, noise;
    for ( size_t n = 0; n < nsteps; n++ )
    {
        // todo: usleep?
        // todo: poll?
        t     = n * dt;
        noise = plrandd() - 0.5;
        y1   += noise;
        y2    = sin( t * PI / 18. );
        y3    = y2 * noise;
        y4    = y2 + noise / 3.;

        // There is no need for all pens to have the same number of
        // points or beeing equally time spaced.
        if ( n % 2 )
            plstripa( id1, 0, t, y1 );
        if ( n % 3 )
            plstripa( id1, 1, t, y2 );
        if ( n % 4 )
            plstripa( id1, 2, t, y3 );
        if ( n % 5 )
            plstripa( id1, 3, t, y4 );

        /* pleop(); */  /* use double buffer (-db on command line) */
    }

    // Destroy strip chart and it's memory
    plstripd( id1 );
    plend();
    return 0;
}
