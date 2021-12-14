//
// cdtext is an example program that uses the text attribute commands
// cdSetTextPath and cdSetTextOrient
//
//
// cdtext.c: test program for the cgmdraw module.
//
//      Written by G. Edward Johnson <mailto:lorax@nist.gov>
//      Date: May 1996
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


int main()
{
    // you must create a pointer to the image(s) that you will be using
    // not suprisingly, it is of type cdImagePtr
    cdImagePtr im;

    // this is a pointer to the output file you will be using
    FILE *outf;

    // these will be index's into the color palette containing
    // the corresponding colors
    int black, white, blue;


    // Create an image 800 pixels wide by 400 pixels high
    im = cdImageCreate( 800, 400 );

    // allocate some colors (isn't this fun?)
    // the first color allocated is the background color
    white = cdImageColorAllocate( im, 255, 255, 255 );
    black = cdImageColorAllocate( im, 0, 0, 0 );
    blue  = cdImageColorAllocate( im, 0, 0, 255 );


    // set the text attributes
    // font, colorindex, and size respectivily

    // font is the style the text is written in. 1 is for Times,
    // 5 is for Helvetica.
    // we will have black text for this one
    // Size is a tough one,  but larger numbers give larger text.
    //
    if ( !( cdSetTextAttrib( im, 5, black, 20 ) ) )
        return 1;

    // Set some line attributes,  lets make lines solid, width 1, and blue
    //
    if ( !( cdSetLineAttrib( im, 1, 1, blue ) ) )
        return 1;

    // Draw a couple of grid lines
    if ( !( cdLine( im, 0, 200, 799, 200 ) ) )
        return 1;
    if ( !( cdLine( im, 200, 0, 200, 399 ) ) )
        return 1;
    if ( !( cdLine( im, 600, 0, 600, 399 ) ) )
        return 1;


    // Show Text going left, up, down, and right, all starting
    // from the same point

    // Text going to the left
    if ( !( cdSetTextPath( im, 1 ) ) )
        return 1;
    if ( !( cdText( im, 200, 200, "Text Left" ) ) )
        return 1;

    // Text going UP
    if ( !( cdSetTextPath( im, 2 ) ) )
        return 1;
    if ( !( cdText( im, 200, 200, "Text Up" ) ) )
        return 1;

    // Text going DOWN
    if ( !( cdSetTextPath( im, 3 ) ) )
        return 1;
    if ( !( cdText( im, 200, 200, "Text Down" ) ) )
        return 1;

    // Text going to the RIGHT
    if ( !( cdSetTextPath( im, 0 ) ) )
        return 1;
    if ( !( cdText( im, 200, 200, "Text Right" ) ) )
        return 1;

    // Show text going at an angle of 0, 45, 90, 135, 180 Degrees
    //

    // Text at no angle
    if ( !( cdText( im, 600, 200, "CGM Draw" ) ) )
        return 1;

    // Text, 45 Degree Angle
    if ( !( cdSetTextOrient( im, -1, 1, 1, 1 ) ) )
        return 1;
    if ( !( cdText( im, 600, 200, "CGM Draw" ) ) )
        return 1;

    // Text, 90 Degree Angle
    if ( !( cdSetTextOrient( im, -1, 0, 0, 1 ) ) )
        return 1;
    if ( !( cdText( im, 600, 200, "CGM Draw" ) ) )
        return 1;

    // Text, 135 Degree Angle
    if ( !( cdSetTextOrient( im, -1, -1, -1, 1 ) ) )
        return 1;
    if ( !( cdText( im, 600, 200, "CGM Draw" ) ) )
        return 1;

    // Text, 180 Degree Angle
    if ( !( cdSetTextOrient( im, 0, -1, -1, 0 ) ) )
        return 1;
    if ( !( cdText( im, 600, 200, "CGM Draw" ) ) )
        return 1;

    // Skewed Text, No Angle
    if ( !( cdSetTextOrient( im, 1, 1, 1, 0 ) ) )
    {
        return 1;
    }
    if ( !( cdSetTextAttrib( im, -1, -1, 40 ) ) )
    {
        return 1;
    }
    if ( !( cdText( im, 300, 300, "CGM Draw" ) ) )
    {
        return 1;
    }
    // show some lines around it
    if ( !( cdLine( im, 300, 300, 500, 300 ) ) )
        return 1;
    if ( !( cdLine( im, 300, 300, 340, 340 ) ) )
        return 1;

    // reset the text to 0 angle
    if ( !( cdSetTextOrient( im, 0, 1, 1, 0 ) ) )
        return 1;


    if ( !( cdSetTextAttrib( im, 5, -1, 20 ) ) )
        return 1;
    if ( !( cdText( im, 5, 5, "G. Edward Johnson" ) ) )
        return 1;

    // now write the file out, lets call it cdtext.cgm
    outf = fopen( "cdtext.cgm", "wb" );
    if ( !outf )
        return 1;
    cdImageCgm( im, outf );
    fclose( outf );
    outf = 0;

    // Remember to destroy the image when you are done
    cdImageDestroy( im );
    im = 0;

    printf( "CGM Text Example!!!\n" );

    return 0;
}
