//
// Color16 is a test program that is part of the CGM Draw Library.
// It Will write out a CGM file that allocates a 16 color pallete
// using the function cdImageColor16.  These correspond to the 16
// standard Windows colors.
//
//
// color16.c: test program for the cgmdraw module.
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
    cdImagePtr im;
    FILE       *outf;


    // Create an image 500 pixels high by 400 pixels wide
    im = cdImageCreate( 400, 500 );

    // allocate the 16 windows colors
    if ( -1 == cdImageColor16( im ) )
        return 1;

    // Set the fill attributes
    // fill=solid, colorindex=0, hatch=no change
    if ( !( cdSetShapeFillAttrib( im, 1, 0, 1 ) ) )
        return 1;

    // set the text attributes
    // font=helvetica, colorindex=1, size=25
    if ( !( cdSetTextAttrib( im, 5, 1, 25 ) ) )
        return 1;

    // The first color index is white, so lets draw a box around it
    // edge=solid, width=1, color=1 (black), visible=yes
    cdSetShapeEdgeAttrib( im, 1, 1, 1, 1 );


    // Draw a rectangle (10,450) is upper left, (35,425) is lower right
    if ( !( cdRectangle( im, 10, 450, 35, 425 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 40, 425, "Color Index: 0" ) ) )
        return 1;

    // Make the edges invisible
    if ( !( cdSetEdgeVis( im, 0 ) ) )
        return 1;


    // Set the fill color
    if ( !( cdSetFillColor( im, 1 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 10, 400, 35, 375 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 40, 375, "Color Index: 1" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 2 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 10, 350, 35, 325 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 40, 325, "Color Index: 2" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 3 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 10, 300, 35, 275 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 40, 275, "Color Index: 3" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 4 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 10, 250, 35, 225 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 40, 225, "Color Index: 4" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 5 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 10, 200, 35, 175 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 40, 175, "Color Index: 5" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 6 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 10, 150, 35, 125 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 40, 125, "Color Index: 6" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 7 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 10, 100, 35, 75 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 40, 75, "Color Index: 7" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 8 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 210, 450, 235, 425 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 240, 425, "Color Index: 8" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 9 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 210, 400, 235, 375 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 240, 375, "Color Index: 9" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 10 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 210, 350, 235, 325 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 240, 325, "Color Index: 10" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 11 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 210, 300, 235, 275 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 240, 275, "Color Index: 11" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 12 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 210, 250, 235, 225 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 240, 225, "Color Index: 12" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 13 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 210, 200, 235, 175 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 240, 175, "Color Index: 13" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 14 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 210, 150, 235, 125 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 240, 125, "Color Index: 14" ) ) )
        return 1;

    // Set the fill color
    if ( !( cdSetFillColor( im, 15 ) ) )
        return 1;
    // Draw a rectangle
    if ( !( cdRectangle( im, 210, 100, 235, 75 ) ) )
        return 1;
    // Label it
    if ( !( cdText( im, 240, 75, "Color Index: 15" ) ) )
        return 1;

    // now put a little thing at the bottom of the picture
    if ( !cdText( im, 5, 10, "Colors allocated by cdImageColor16" ) )
        return 1;

    // now write the file out, lets call it color16.cgm
    outf = fopen( "color16.cgm", "wb" );
    if ( !outf )
        return 1;
    cdImageCgm( im, outf );
    fclose( outf );
    outf = 0;

    // Remember to destroy the image when you are done
    cdImageDestroy( im );
    im = 0;

    printf( "CGM with 16 color colortable generated as color16.cgm\n" );

    return 0;
}
