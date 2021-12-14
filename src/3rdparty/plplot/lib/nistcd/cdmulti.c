//
// cdmulti is a program to make a cgm file with multiple pictures in it.
//
//
// cdmulti.c: test program for the cgmdraw module.
//
//      Written by G. Edward Johnson <mailto:lorax@nist.gov>
//      Date: June 26, 1996
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


    // Create an image 200 pixels wide by 250 pixels high
    im = cdImageCreate( 200, 250 );

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
    // 25 is a not too large size
    if ( !( cdSetTextAttrib( im, 5, black, 25 ) ) )
        return 1;


    // Now that we have set some attributes, lets do some drawing

    // lets put some text in the picture.
    // (20,100) is the point at the lower left corner of the text
    if ( !( cdText( im, 20, 100, "Hello World" ) ) )
        return 1;


    // Here's something special, put a second picture in the file
    // we put in a second picture, and reset all defaults.  This means
    // we have to re-allocate the colors as well
    if ( !( cdCgmNewPic( im, 0 ) ) )
        return 1;

    // allocate some colors (Again!)
    // the first color allocated is the background color
    white = cdImageColorAllocate( im, 255, 255, 255 );
    black = cdImageColorAllocate( im, 0, 0, 0 );
    blue  = cdImageColorAllocate( im, 0, 0, 255 );
    // set text attributes
    if ( !( cdSetTextAttrib( im, 5, black, 25 ) ) )
        return 1;
    if ( !( cdText( im, 20, 100, "Goodbye World" ) ) )
        return 1;


    // now write the file out.
    outf = fopen( "cdmulti.cgm", "wb" );
    if ( !outf )
        return 1;
    cdImageCgm( im, outf );
    fclose( outf );
    outf = 0;

    // Remember to destroy the image when you are done
    cdImageDestroy( im );
    im = 0;

    printf( "I just created a multi picture CGM!!!\n" );

    return 0;
}
