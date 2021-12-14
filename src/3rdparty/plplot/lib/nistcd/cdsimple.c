//
// cdsimple is a very simple program that uses the cd library.
// it will walk you through the basics.
//
//
// cdsimple.c: test program for the cgmdraw module.
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


#ifndef NOMALLOCH
#include <malloc.h>
#endif
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


    // Create an image 400 pixels wide by 500 pixels high
    im = cdImageCreate( 400, 500 );

    // allocate some colors (isn't this fun?)
    // the first color allocated is the background color
    white = cdImageColorAllocate( im, 255, 255, 255 );
    black = cdImageColorAllocate( im, 0, 0, 0 );
    blue  = cdImageColorAllocate( im, 0, 0, 255 );

    // Set the fill attributes
    // fill, colorindex, and hatch respectivily
    // see the cd documentation for a complete description

    // fill is the color that will be on the inside of filled objects
    // such as rectangles and polygons.
    // It can be 1 for solid color, 2 for hatch pattern, 4 for empty
    // let's use blue for the fill color
    // we are going to set it to solid, so the hatch pattern doesn't
    // matter.  we will signify no change by putting in -1
    if ( !( cdSetShapeFillAttrib( im, 1, blue, -1 ) ) )
        return 1;

    // notice that we also checked to make sure the command actually
    // worked.

    // we don't want the edges of our shapes to be a different color
    // so make them invisible.  0 means invisible, 1 means visible.
    if ( !( cdSetEdgeVis( im, 0 ) ) )
        return 1;


    // set the text attributes
    // font, colorindex, and size respectivily

    // font is the style the text is written in. 1 is for Times,
    // 5 is for Helvetica.
    // we will have black text for this one
    // Size is a tough one,  but larger numbers give larger text.
    // 25 is a not too large size
    if ( !( cdSetTextAttrib( im, 5, black, 25 ) ) )
        return 1;



    // Now that we have set some attributes, lets do some drawing

    // Draw a rectangle (10,450) is upper left, (350,350) is lower right
    if ( !( cdRectangle( im, 10, 450, 350, 350 ) ) )
        return 1;

    // lets put some text in the picture too.
    // (100,100) is the point at the lower left corner of the text
    if ( !( cdText( im, 100, 100, "Hello World" ) ) )
        return 1;



    // now write the file out, lets call it cdsimple.cgm
    outf = fopen( "cdsimple.cgm", "wb" );
    if ( !outf )
        return 1;
    cdImageCgm( im, outf );
    fclose( outf );
    outf = 0;

    // Remember to destroy the image when you are done
    cdImageDestroy( im );
    im = 0;

    printf( "I just created a simple CGM!!!\n" );

    return 0;
}
