#include <stdio.h>

#include <cairo.h>
#include <cairo-ps.h>

#include <plplot.h>

int main( int argc, char *argv[] )
{
    cairo_surface_t *cairoSurface;
    cairo_t         *cairoContext;

    cairoSurface = cairo_ps_surface_create( "ext-cairo-test.ps", 720, 540 );
    cairoContext = cairo_create( cairoSurface );

    plparseopts( &argc, argv, PL_PARSE_FULL );

    plsdev( "extcairo" );
    plinit();
    pl_cmd( PLESC_DEVINIT, cairoContext );
    plenv( 0.0, 1.0, 0.0, 1.0, 1, 0 );
    pllab( "x", "y", "title" );
    plend();

    cairo_destroy( cairoContext );
    cairo_surface_destroy( cairoSurface );
    exit( 0 );
}

