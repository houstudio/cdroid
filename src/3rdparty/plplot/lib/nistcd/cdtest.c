
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "defines.h"
#include "cd.h"

// cdtest.c: test program for the cgmdraw module.
//
//      Written by G. Edward Johnson <mailto:lorax@nist.gov>
//      Date: April 1996
//      Copyright: cd software produced by NIST, an agency of the
//      U.S. government, is by statute not subject to copyright
//      in the United States. Recipients of this software assume all
//      responsibilities associated with its operation, modification
//      and maintenance.
//
//


int main()
{
    cdImagePtr myimage;

    // Color indexes
    int     white;
    int     blue;
    int     red;
    int     green;
    int     black;
    int     lavender;
    // points for polygons and polylines
    cdPoint points[15];
    // output file
    FILE    *outf;


// create an image, 1000 by 1000 pixels
    myimage = cdImageCreate( 1000, 1000 );

// Allocate the background color
    white = cdImageColorAllocate( myimage, 255, 255, 255 );
    red   = cdImageColorAllocate( myimage, 255, 0, 0 );
    green = cdImageColorAllocate( myimage, 0, 255, 0 );
    blue  = cdImageColorAllocate( myimage, 0, 0, 255 );
    black = cdImageColorAllocate( myimage, 0, 0, 0 );

    if ( cdSetLineColor( myimage, blue ) )
    {
        fprintf( stderr, "cdSetLineColor Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineColor Unsuccessful\n" );
    }

    if ( cdSetLineColor( myimage, blue ) )
    {
        fprintf( stderr, "cdSetLineColor Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineColor Unsuccessful\n" );
    }


    if ( cdImageLine( myimage, 400, 8, 520, 8, green ) )
    {
        fprintf( stderr, "cdImageLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdImageLine Unsuccessful\n" );
    }

    if ( cdImageLine( myimage, 50, 50, 500, 500, red ) )
    {
        fprintf( stderr, "cdImageLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdImageLine Unsuccessful\n" );
    }

    if ( cdSetLineAttrib( myimage, 2, 3, red ) )
    {
        fprintf( stderr, "cdSetLineAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineAttrib Unsuccessful\n" );
    }

    if ( cdLine( myimage, 500, 50, 50, 500 ) )
    {
        fprintf( stderr, "cdLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdLine Unsuccessful\n" );
    }

// Now, build a rectangle
    if ( cdSetShapeFillAttrib( myimage, 3, blue, 6 ) )
    {
        fprintf( stderr, "cdSetShapeFillAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetShapeFillAttrib Unsuccessful\n" );
    }

    if ( cdSetShapeEdgeAttrib( myimage, 1, 2, green, 1 ) )
    {
        fprintf( stderr, "cdSetShapeEdgeAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetShapeEdgeAttrib Unsuccessful\n" );
    }

    if ( cdRectangle( myimage, 100, 100, 250, 250 ) )
    {
        fprintf( stderr, "cdRectangle Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdRectangle Unsuccessful\n" );
    }



// now check out each of the line styles in order

    if ( cdSetLineAttrib( myimage, 5, 4, green ) )
    {
        fprintf( stderr, "cdSetLineAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineAttrib Unsuccessful\n" );
    }

    if ( cdLine( myimage, 100, 600, 500, 600 ) )
    {
        fprintf( stderr, "cdLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdLine Unsuccessful\n" );
    }

    if ( cdSetLineAttrib( myimage, 4, -1, -1 ) )
    {
        fprintf( stderr, "cdSetLineAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineAttrib Unsuccessful\n" );
    }
    if ( cdLine( myimage, 100, 625, 500, 625 ) )
    {
        fprintf( stderr, "cdLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdLine Unsuccessful\n" );
    }

    if ( cdSetLineAttrib( myimage, 3, -1, -1 ) )
    {
        fprintf( stderr, "cdSetLineAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineAttrib Unsuccessful\n" );
    }
    if ( cdLine( myimage, 100, 650, 500, 650 ) )
    {
        fprintf( stderr, "cdLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdLine Unsuccessful\n" );
    }


    if ( cdSetLineAttrib( myimage, 2, -1, -1 ) )
    {
        fprintf( stderr, "cdSetLineAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineAttrib Unsuccessful\n" );
    }
    if ( cdLine( myimage, 100, 675, 500, 675 ) )
    {
        fprintf( stderr, "cdLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdLine Unsuccessful\n" );
    }

    if ( cdSetLineAttrib( myimage, 1, -1, -1 ) )
    {
        fprintf( stderr, "cdSetLineAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineAttrib Unsuccessful\n" );
    }
    if ( cdLine( myimage, 100, 700, 500, 700 ) )
    {
        fprintf( stderr, "cdLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdLine Unsuccessful\n" );
    }

// now make a circle
    if ( cdSetShapeFillAttrib( myimage, 1, -1, 6 ) )
    {
        fprintf( stderr, "cdSetShapeFillAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetShapeFillAttrib Unsuccessful\n" );
    }

    if ( cdSetShapeEdgeAttrib( myimage, 1, 2, green, 0 ) )
    {
        fprintf( stderr, "cdSetShapeEdgeAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetShapeEdgeAttrib Unsuccessful\n" );
    }

    if ( cdCircle( myimage, 500, 500, 25 ) )
    {
        fprintf( stderr, "cdCircle Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdCircle Unsuccessful\n" );
    }

// how about a Circular Arc now
    if ( cdArc3Pt( myimage, 550, 500, 600, 600, 650, 550 ) )
    {
        fprintf( stderr, "cdArc3Pt Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdArc3Pt Unsuccessful\n" );
    }

// now draw a closed Circular Arc
    if ( cdArc3PtClose( myimage, 550, 200, 600, 300, 650, 250, 0 ) )
    {
        fprintf( stderr, "cdArc3PtClose Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdArc3PtClose Unsuccessful\n" );
    }

// and now for an ellipse
    if ( cdSetShapeEdgeAttrib( myimage, 1, 2, green, 1 ) )
    {
        fprintf( stderr, "cdSetShapeEdgeAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetShapeEdgeAttrib Unsuccessful\n" );
    }

    if ( cdEllipse( myimage, 750, 200, 800, 250, 750, 300 ) )
    {
        fprintf( stderr, "cdEllipse Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdEllipse Unsuccessful\n" );
    }


// Now, the harder ones.  First lets try the polygon stuff
// a polygon with 7 or less points

    points[0].x = 700;
    points[0].y = 700;
    points[1].x = 725;
    points[1].y = 750;
    points[2].x = 775;
    points[2].y = 650;

    if ( cdPolygon( myimage, points, 3 ) )
    {
        fprintf( stderr, "cdPolygon Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdPolygon Unsuccessful\n" );
    }

// Here's a tough one, a polygon with more than seven points

    points[0].x  = 800;
    points[0].y  = 700;
    points[1].x  = 825;
    points[1].y  = 725;
    points[2].x  = 850;
    points[2].y  = 700;
    points[3].x  = 875;
    points[3].y  = 750;
    points[4].x  = 900;
    points[4].y  = 775;
    points[5].x  = 925;
    points[5].y  = 750;
    points[6].x  = 950;
    points[6].y  = 850;
    points[7].x  = 925;
    points[7].y  = 850;
    points[8].x  = 875;
    points[8].y  = 800;
    points[9].x  = 850;
    points[9].y  = 825;
    points[10].x = 825;
    points[10].y = 900;

    if ( cdPolygon( myimage, points, 11 ) )
    {
        fprintf( stderr, "cdPolygon Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdPolygon Unsuccessful\n" );
    }

// Now for a polygon set, two polygons (both triangles) the first one
// with an invisible edge closing it.

    points[0].x = 500;  // start of first polygon
    points[0].y = 350;
    points[0].e = 1;    // edge out is visible
    points[1].x = 575;
    points[1].y = 350;
    points[1].e = 1;
    points[2].x = 575;
    points[2].y = 430;
    points[2].e = 3;    // close polygon with visible edge
    points[3].x = 600;  // start of second polygon
    points[3].y = 350;
    points[3].e = 0;    // edge out is invisible
    points[4].x = 700;
    points[4].y = 350;
    points[4].e = 1;
    points[5].x = 650;
    points[5].y = 475;
    points[5].e = 2;    // close polygone with an invisible edge

// make the edges wider so it is easier to see if they are there
    if ( cdSetEdgeWidth( myimage, 3 ) )
    {
        fprintf( stderr, "cdSetEdgeWidth Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetEdgeWidth Unsuccessful\n" );
    }
    if ( cdPolygonSet( myimage, points, 6 ) ) // draw the polygonset
    {
        fprintf( stderr, "cdPolygonSet Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdPolygonSet Unsuccessful\n" );
    }

// now for poly lines, just like polygons (except they're lines)

    if ( cdSetLineAttrib( myimage, 1, 1, red ) )
    {
        fprintf( stderr, "cdSetLineAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetLineAttrib Unsuccessful\n" );
    }


    points[0].x = 400;
    points[0].y = 16;
    points[1].x = 520;
    points[1].y = 16;
    if ( cdPolyLine( myimage, points, 2 ) )
    {
        fprintf( stderr, "cdPolyLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdPolyLine Unsuccessful\n" );
    }

    points[0].x = 800;
    points[0].y = 650;
    points[1].x = 825;
    points[1].y = 675;
    points[2].x = 850;
    points[2].y = 650;
    points[3].x = 875;
    points[3].y = 700;
    if ( cdPolyLine( myimage, points, 4 ) )
    {
        fprintf( stderr, "cdPolyLine Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdPolyLine Unsuccessful\n" );
    }


// Markers
// set the attributes
    if ( cdSetMarkerAttrib( myimage, 1, 1, green ) )
    {
        fprintf( stderr, "cdSetMarkerAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetMarkerAttrib Unsuccessful\n" );
    }

// now plot the marker

    if ( cdMarker( myimage, 900, 500 ) )
    {
        fprintf( stderr, "cdMarker Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdMarker Unsuccessful\n" );
    }


    if ( cdSetMarkerAttrib( myimage, 5, 3, black ) )
    {
// set the marker type to cross
        fprintf( stderr, "cdSetMarkerAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetMarkerAttrib Unsuccessful\n" );
    }

// a Poly Marker,  two markers in different places
    points[0].x = 715;
    points[0].y = 785;
    points[1].x = 735;
    points[1].y = 815;

    if ( cdPolyMarker( myimage, points, 2 ) )
    {
        fprintf( stderr, "cdPolyMarker Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdMarker Unsuccessful\n" );
    }


// Hey, things are going so well, lets do some text
    lavender = cdImageColorAllocate( myimage, 204, 102, 255 );

    if ( cdSetTextAttrib( myimage, 1, lavender, 50 ) )
    {
        fprintf( stderr, "cdSetTextAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetTextAttrib Unsuccessful\n" );
    }

    if ( cdText( myimage, 50, 800, "CGM Draw Version 1.3" ) )
    {
        fprintf( stderr, "cdText Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdText Unsuccessful\n" );
    }

// More text.  This time test TextPath and TextOrient
    if ( cdSetTextPath( myimage, 3 ) )
    {
        fprintf( stderr, "cdSetTextPath Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetTextPath Unsuccessful\n" );
    }

    if ( cdText( myimage, 50, 800, "CGM Draw" ) )
    {
        fprintf( stderr, "cdText Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdText Unsuccessful\n" );
    }

    if ( cdSetTextOrient( myimage, 1, 0, 0, -1 ) )
    {
        fprintf( stderr, "cdSetTextOrient Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetTextOrient Unsuccessful\n" );
    }

    if ( cdSetTextPath( myimage, 0 ) )
    {
        fprintf( stderr, "cdSetTextPath Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetTextPath Unsuccessful\n" );
    }

    if ( cdText( myimage, 950, 400, "CGM Draw" ) )
    {
        fprintf( stderr, "cdText Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdText Unsuccessful\n" );
    }


    if ( cdSetTextOrient( myimage, 0, 1, 1, 0 ) )
    {
        fprintf( stderr, "cdSetTextOrient Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetTextOrient Unsuccessful\n" );
    }


    if ( cdSetTextAttrib( myimage, 5, -1, 25 ) )
    {
        fprintf( stderr, "cdSetTextAttrib Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdSetTextAttrib Unsuccessful\n" );
    }

    if ( cdText( myimage, 5, 5, "G. Edward Johnson" ) )
    {
        fprintf( stderr, "cdText Successful\n" );
    }
    else
    {
        fprintf( stderr, "cdText Unsuccessful\n" );
    }




    outf = fopen( "cdout.cgm", "wb" );
    if ( !outf )
        return 1;
    cdImageCgm( myimage, outf );
    fclose( outf );
    outf = 0;

    cdImageDestroy( myimage );


    return 0;
}
