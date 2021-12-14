//
// cdexpert highlights the expert functions in CD.  You probably
// don't want to use these unless you have to.
//
//
// cdexpert.c: test program for the cgmdraw module.
//
//      Written by G. Edward Johnson <mailto:lorax@nist.gov>
//      Date: January 1997
//      Copyright: cd software produced by NIST, an agency of the
//      U.S. government, is by statute not subject to copyright
//      in the United States. Recipients of this software assume all
//      responsibilities associated with its operation, modification
//      and maintenance.
//
//


#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "defines.h"
#include "cd.h"

#define CD_CHECK_RETURN( x )  \
    if ( !( x ) )             \
    {                         \
        cdImageDestroy( im ); \
        fclose( outf );       \
        return 1;             \
    }


int main()
{
    // you must create a pointer to the image(s) that you will be using
    // not suprisingly, it is of type cdImagePtr
    cdImagePtr im;
    cdPoint    points[2];

    // this is a pointer to the output file you will be using
    FILE *outf;

    // these will be index's into the color palette containing
    // the corresponding colors
    int black, white, blue;

    // Create an image
    im = cdImageStartCgm();
    // now open the file lets call it cdexpert1.cgm
    outf = fopen( "cdexp1.cgm", "wb" );
    if ( !outf )
    {
        cdImageDestroy( im );
        return 1;
    }
    // set its size to 500x500
    CD_CHECK_RETURN( cdImageSetSize( im, 500, 500 ) );
    // set Line, Marker, and Edge specification modes to absolute (0)
    // the default is scaled (1)
    CD_CHECK_RETURN( cdImageSetLineSpec( im, 0 ) );
    CD_CHECK_RETURN( cdImageSetMarkerSpec( im, 0 ) );
    CD_CHECK_RETURN( cdImageSetEdgeSpec( im, 0 ) );
    // Clear the font list, then set it to just contain 1 font
    CD_CHECK_RETURN( cdImageClearFonts( im ) );
    CD_CHECK_RETURN( cdImageAddFont( im, "TIMES_ROMAN" ) );
    // start the picture
    CD_CHECK_RETURN( cdCgmHeader( im ) );
    CD_CHECK_RETURN( cdCgmPic( im, 2 ) );


    // allocate some colors (isn't this fun?)
    // the first color allocated is the background color
    white = cdImageColorAllocate( im, 255, 255, 255 );
    black = cdImageColorAllocate( im, 0, 0, 0 );
    blue  = cdImageColorAllocate( im, 0, 0, 255 );

    // fill attributes: Empty
    CD_CHECK_RETURN( cdSetShapeFillAttrib( im, 4, -1, -1 ) );

    // Edge attributes: dots, width 3, blue, visible edges.
    CD_CHECK_RETURN( cdSetShapeEdgeAttrib( im, 2, 3, blue, 1 ) );

    // Text attributes: Times, black, size 25
    CD_CHECK_RETURN( cdSetTextAttrib( im, 1, black, 25 ) );

    // Line attributes: Solid Black Line of Width 5
    CD_CHECK_RETURN( cdSetLineAttrib( im, 1, 5, black ) );

    // Marker attributes: style pluses, size 3, black
    CD_CHECK_RETURN( cdSetMarkerAttrib( im, 2, 3, black ) );

    // Now that we have set some attributes, lets do some drawing

    // Draw a rectangle (10,450) is upper left, (350,350) is lower right
    CD_CHECK_RETURN( cdRectangle( im, 10, 450, 350, 350 ) );
    // Draw a line (300,100) to (400,100)
    CD_CHECK_RETURN( cdLine( im, 300, 100, 400, 100 ) );

    // Add Two markers
    CD_CHECK_RETURN( cdMarker( im, 325, 150 ) );
    CD_CHECK_RETURN( cdMarker( im, 375, 150 ) );

    // lets put some text in the picture too.
    // (100,100) is the point at the lower left corner of the text
    CD_CHECK_RETURN( cdText( im, 100, 100, "Hello World" ) );

    // we could just finish off the CGM here with a
    // cdImageCgm(im, outf), but lets put another picture in.

    // close the picture
    CD_CHECK_RETURN( cdImageEndPic( im ) );
    // set the specifications modes back to the default
    CD_CHECK_RETURN( cdImageSetLineSpec( im, 1 ) );
    CD_CHECK_RETURN( cdImageSetMarkerSpec( im, 1 ) );
    CD_CHECK_RETURN( cdImageSetEdgeSpec( im, 1 ) );
    // start a new picture, keeping all the changes we made, including
    // the color table
    CD_CHECK_RETURN( cdCgmPic( im, 1 ) );

    // draw the same image again, notice the Specification modes are
    // different
    // Draw a rectangle (10,450) is upper left, (350,350) is lower right
    CD_CHECK_RETURN( cdRectangle( im, 10, 450, 350, 350 ) );

    // Draw a line (300,100) to (400,100)
    CD_CHECK_RETURN( cdLine( im, 300, 100, 400, 100 ) );

    // Add Two markers
    // we are doing the markers a little bit differently this time
    points[0].x = 325;
    points[0].y = 150;
    points[1].x = 375;
    points[1].y = 150;
    CD_CHECK_RETURN( cdPolyMarker( im, points, 2 ) );

    // lets put some text in the picture too.
    // (100,100) is the point at the lower left corner of the text
    CD_CHECK_RETURN( cdText( im, 100, 100, "Hello World" ) );

    cdImageCgm( im, outf );
    fclose( outf );
    outf = 0;

    // Remember to destroy the image when you are done
    cdImageDestroy( im );
    im = 0;

    printf( "I am a CGM expert!!!\n" );

    return 0;
}
