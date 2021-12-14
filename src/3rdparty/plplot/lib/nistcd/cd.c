
// cd.c main file for cgmdraw module.
//
//      Written by G. Edward Johnson <mailto:lorax@nist.gov>
//      Date: April 1996
//      Copyright: cd software produced by NIST, an agency of the
//      U.S. government, is by statute not subject to copyright
//      in the United States. Recipients of this software assume all
//      responsibilities associated with its operation, modification
//      and maintenance.
//
//      Portions of this package are from the gd package written by
//      Thomas Boutell and are copyright 1994, 1995, Quest Protein
//      Database Center, Cold Spring Harbor Labs.  They are marked in the
//      source code.
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


static int cdImageAddColor( cdImagePtr im, int si, int ei );

cdImagePtr cdImageCreate( int sx, int sy )
// Creates a new image of size (sx,sy).  Most people should always
// start by calling this function
{
    cdImagePtr im;

    im = cdImageStartCgm();
    if ( !im )
        return 0;          // memory allocation failed
    if ( !cdImageSetSize( im, sx, sy ) )
    {
        free( im ); return 0;
    }

    if ( !cdCgmHeader( im ) )
    {
        free( im ); return 0;
    }

    if ( cdCgmPic( im, 0 ) )
    {
        return im;
    }
    else
    {
        free( im );
        return 0;
    }
}

static int cdAppNull( unsigned char *es, int x )
{
// put x  nulls in the string.
// return value is number of octets added (1)
    int y;

    for ( y = 0; y < x; y++ )
    {
        *es = '\0';
        es++;
    }
    return x;
}

static int cdAppByte( unsigned char *es, short int addme )
{
// Append an octet to the end of es
// Return value is number of octets added
// for internal cd functions only, do not call
//
    *es = (unsigned char) addme & 0377;
    return 1;
}

static int cdAppShort( unsigned char *es, short int addme )
{
// Append a short to the end of es
// return value is number of octets added
// For internal cd functions only, do not call!
//
    short int temp;

    temp = addme >> 8;
    *es  = (unsigned char) temp & 0377;
    es++;
    *es = (unsigned char) addme & 0377;
    return 2;
}

// static int cdAppWord(unsigned char *es, int addme){
// Append an word to es
// Return value is number of octets added
// For internal cd functions only, do not call!
//
//
//      int temp;
//      temp = addme >> 24;
//es = (unsigned char) temp & 0377;
//      es++;
//      temp = addme >> 16;
//es = (unsigned char) temp & 0377;
//      es++;
//      temp = addme >> 8;
//es = (unsigned char) temp & 0377;
//      es++;
//es = (unsigned char) addme & 0377;
//      es++;
//      return 4;
// }
//

static int cdcomhead( unsigned char *es, int elemclass, int id, int len )
{
// sets the command header in the first two bytes of string es
// element class is in bits 15-12
// element id is in bits 11-5
// parameter list length is in bits 4-0
//
    int temp;

    if ( !es )
        return 0;          // the string must be allocated first

    // set the element class
    *es = (unsigned char) elemclass << 4;
    // set the element id
    temp = 0177 & id;
    temp = temp >> 3;
    *es  = *es | temp;
    es++;
    id  = id << 5;
    *es = (unsigned char) id;
    *es = *es | (unsigned char) ( 037 & len );

    return 1;
}

static int cdcomheadlong( unsigned char *es, int elemclass, int id, int len )
{
// sets the command header for the long form.
// first 16 bits:
//  element class is in bits 15-12
//  element id is in bits 11-5
//  parameter list length is in bits 4-0 = 31
// second 16 bits:
//  bit 15 = 0  (for last partition)
//  bit 14-0 param list len
//

    // I'm lazy, call cdcomhead to set the first two bytes
    if ( !cdcomhead( es, elemclass, id, 31 ) )
        return 0;
    es += 2;

    // now set the second two bytes
    cdAppShort( es, (short int) len );
    *es = *es & 0177;     // make bit 15 = 0
    es += 2;

    return 1;
}

static int cdAddElem( cdImagePtr im, unsigned char *es, int octet_count )
// adds a string, which is a CGM element to the elemlist.
// This function is called by other functions in this library and
// should NOT be called by users of the library
// For internal cd functions only, do not call!
//
{
    unsigned char *newlist;     // in case memory allocation fails
    int           x;            // counter

    while ( ( octet_count + 1 ) >= im->bytestoend )
    {
        // not enough space, must grow elemlist
        im->listlen = im->listlen + CDGROWLISTSIZE;
        newlist     = (unsigned char *) realloc( im->elemlist, SIZEOF( unsigned char ) * im->listlen );
        if ( newlist )
        {
            // successfully allocated memory
            im->elemlist    = newlist;
            im->bytestoend  = im->bytestoend + CDGROWLISTSIZE;
            im->curelemlist = im->elemlist + ( im->listlen - im->bytestoend );
        }
        else
        {
            // memory allocation failed, save yurself
            im->listlen = im->listlen - CDGROWLISTSIZE;
            return 0;
        }
    }

    // ok, if we get to here, there is enough space, so add it.
    for ( x = 0; x < octet_count; x++ )
    {
        *im->curelemlist = (unsigned char) *es;
        im->curelemlist++;
        es++;
    }
    im->bytestoend = im->bytestoend - octet_count;
    return 1;
}

int cdCgmHeader( cdImagePtr im )
{
// add the cgm header to the imagepointer's  element list
// do it all in a string than call cdAddElem on it
// For internal cd functions only, do not call!
//
    unsigned char       *headerp;
    unsigned char       *head;
    const unsigned char *buf, *buf2;
    int octet_count = 0;
    int blen;                 // length of buf
    int curly;
    int fontlistlen;          // each font in the font list is stored as a string,
    // with a single octet in front of the string
    // giving its length, fontlistlen is the sum of
    // the lengths of all the font strings + the
    // length octets.

    if ( im->state != 0 )
        return 0;
    headerp = (unsigned char *) calloc( 1024, SIZEOF( unsigned char ) );
    if ( !headerp )
        return 0;               // memory allocation failed
    head = headerp;

    //** Attribute: BegMF; Elem Class 0; Elem ID 1
    buf  = (const unsigned char *) "cd: CgmDraw Library";
    blen = strlen( (const char *) buf );
    cdcomhead( head, 0, 1, blen + 1 );
    head += 2;
    head += cdAppByte( head, (short int) blen );
    buf2  = buf;
    while ( *buf2 )
    {
        *head++ = *buf2++;
    }
    octet_count += ( blen + 3 );
    curly        = 4 - ( octet_count % 4 );
    if ( curly % 4 )
    {
        octet_count += curly;
        head        += cdAppNull( head, curly );
    }

    //** Attribute: MFVersion; Elem Class 1; Elem ID 1
    cdcomhead( head, 1, 1, 2 );
    head        += 2;
    head        += cdAppShort( head, (short int) 1 );
    octet_count += 4;

    //** Attribute: MFDesc; Elem Class 1; Elem ID 2
    blen = strlen( (char *) im->desc );
    cdcomheadlong( head, 1, 2, blen + 1 );
    head += 4;
    head += cdAppByte( head, (short int) blen );
    buf2  = im->desc;
    while ( *buf2 )
    {
        *head++ = *buf2++;
    }
    octet_count += ( blen + 5 );
    curly        = 4 - ( octet_count % 4 );
    if ( curly % 4 )
    {
        octet_count += curly;
        head        += cdAppNull( head, curly );
    }

    //** Attribute: ColrPrec; Elem Class 1; Elem ID 7
    cdcomhead( head, 1, 7, 2 );
    head        += 2;
    head        += cdAppShort( head, (short int) 8 );
    octet_count += 4;

    //** Attribute: ColrIndexPrec; Elem Class 1; Elem ID 8
    cdcomhead( head, 1, 8, 2 );
    head        += 2;
    head        += cdAppShort( head, (short int) 8 );
    octet_count += 4;

    //** Attribute: MaxColrIndex; Elem Class 1; Elem ID 9
    cdcomhead( head, 1, 9, 1 );
    head        += 2;
    head        += cdAppByte( head, (short int) 255 );
    octet_count += 4; head++;

    //** Attribute: MFElemList; Elem Class 1; Elem ID 11
    // shorthand here.  1 means 1 element specified, (-1,1)
    // means drawing-plus-control set
    cdcomhead( head, 1, 11, 6 );
    head        += 2;
    head        += cdAppShort( head, (short int) 1 );
    head        += cdAppShort( head, (short int) -1 );
    head        += cdAppShort( head, (short int) 1 );
    octet_count += 8;

    //** Attribute: FontList; Elem Class 1; Elem ID 13
    // im->fontlist contains a comma separated list of font names
    // since we don't need the commas, and every font except one has
    // a comma, and we do need a length octet, that means that
    // taking the string length will give us one less than the
    // correct length.
    buf = im->fontlist;
    if ( buf )     // don't do this if there aren't any fonts
    {
        fontlistlen = strlen( (const char *) buf ) + 1;
        cdcomheadlong( head, 1, 13, fontlistlen );
        head += 4;

        while ( *buf )
        {
            blen = 0;
            buf2 = buf;
            while ( ( *buf ) && ( *buf != ',' ) )
            {
                buf++;
                blen++;
            }
            head += cdAppByte( head, (short int) blen );
            while ( buf2 < buf )
            {
                *head++ = *buf2++;
            }
            if ( *buf )
            {
                buf++;
            }
        }
        octet_count += ( 4 + fontlistlen );
        curly        = 4 - ( octet_count % 4 );
        if ( curly % 4 )
        {
            octet_count += curly;
            head        += cdAppNull( head, curly );
        }
    }     // end of check to see if any fonts

    if ( cdAddElem( im, headerp, octet_count ) )
    {
        free( headerp );
        headerp = 0;
        return 1;
    }
    else
    {
        free( headerp );
        return 0;
    }
}


int cdCgmPic( cdImagePtr im, int sticky )
{
// Start the picture.  if the sticky bit is set, set and use the defaults
// of the previous picture.  Otherwise, reset all defaults.
// Gej: sticky = 0 reset defaults, 1 dont reset anything, 2 only
// reset the color table
//
    unsigned char *headerp;
    unsigned char *head;
    unsigned char *buf, *buf2;
    char          *tb;
    int           octet_count = 0;
    int           blen;           // length of buf
    int           x1, x2, x3, x4; // needed for setting defaults

    if ( ( im->state != 0 ) && ( im->state != 2 ) )
        return 0;
    if ( ( sticky > 2 ) || ( sticky < 0 ) )
        return 0;                                     // invalid sticky bit
    // increment the picture number
    im->picnum++;
    tb = (char *) calloc( 4 * 4, SIZEOF( char ) );
    if ( !tb )
        return 0;         // memory allocation failed
    headerp = (unsigned char *) calloc( 1024, SIZEOF( unsigned char ) );
    if ( !headerp )
    {   // memory allocation failed
        free( tb );
        return 0;
    }
    head = headerp;

    //** Attribute: BegPic; Elem Class 0; Elem ID 3
    sprintf( tb, "picture %d", im->picnum );
    buf = (unsigned char *) tb;
    // buf = (unsigned char *) "picture 1";
    blen = strlen( (char *) buf );
    cdcomhead( head, 0, 3, blen + 1 );
    head += 2;
    head += cdAppByte( head, (short int) blen );
    buf2  = buf;
    while ( *buf2 )
    {
        *head++ = *buf2++;
    }
    free( tb );
    octet_count += ( blen + 3 );
    if ( !( blen % 2 ) )
    {
        octet_count++;
        head += cdAppNull( head, 1 );
    }
    if ( octet_count % 4 )
    {
        octet_count += 2;
        head        += cdAppNull( head, 2 );
    }

    //** Attribute: ColrMode; Elem Class 2; Elem ID 2
    cdcomhead( head, 2, 2, 2 );
    head        += 2;
    head        += cdAppShort( head, (short int) 0 );
    octet_count += 4;
    // Picture Descriptor: Line Width Specification Mode;
    // Elem Class 2; Elem ID 3
    if ( sticky && ( im->linespec != CDLINESPEC ) )
    {
        cdcomhead( head, 2, 3, 2 );
        head        += 2;
        head        += cdAppShort( head, (short int) im->linespec );
        octet_count += 4;
    }
    // Picture Descriptor: Marker Size Specification Mode;
    // Elem Class 2; Elem ID 4
    if ( sticky && ( im->markerspec != CDMARKERSPEC ) )
    {
        cdcomhead( head, 2, 4, 2 );
        head        += 2;
        head        += cdAppShort( head, (short int) im->markerspec );
        octet_count += 4;
    }
    // Picture Descriptor: Edge Width Specification Mode;
    // Elem Class 2; Elem ID 5
    if ( sticky && ( im->edgespec != CDEDGESPEC ) )
    {
        cdcomhead( head, 2, 5, 2 );
        head        += 2;
        head        += cdAppShort( head, (short int) im->edgespec );
        octet_count += 4;
    }

    //** Attribute: VDCExt; Elem Class 2; Elem ID 6
    cdcomhead( head, 2, 6, 8 );
    head        += 2;
    head        += cdAppShort( head, (short int) 0 );
    head        += cdAppShort( head, (short int) 0 );
    head        += cdAppShort( head, (short int) im->sx );
    head        += cdAppShort( head, (short int) im->sy );
    octet_count += 10;

    //** Attribute: Begin Picture Body; Elem Class 0; Elem ID 4
    cdcomhead( head, 0, 4, 0 );
    head        += 2;
    octet_count += 2;

    if ( cdAddElem( im, headerp, octet_count ) )
    {
        free( headerp );
    }
    else
    {
        free( headerp );
        return 0;
    }

    if ( sticky )
    {
        // keep defaults the way they are
        if ( sticky == 1 )
        {
            // keep the color table
            if ( cdImageAddColor( im, 0, im->colorsTotal - 1 ) == -1 )
            {
                // no colortable
                return 1;
            }
        }
        else
        {
            // Nuke the color table if there is one
            cdImageColorClear( im );
        }
        im->state = 1;
        x1        = im->ltype; x2 = im->lwidth; x3 = im->lcolor;
        im->ltype = CDLTYPE; im->lwidth = CDLWIDTH; im->lcolor = CDLCOLOR;
        if ( !cdSetLineAttrib( im, x1, x2, x3 ) )
            return 0;

        x1             = im->shapestyle; x2 = im->shapecolor; x3 = im->shapehatch;
        im->shapestyle = CDSHAPESTYLE; im->shapecolor = CDSHAPECOLOR;
        im->shapehatch = CDSHAPEHATCH;
        if ( !cdSetShapeFillAttrib( im, x1, x2, x3 ) )
            return 0;

        x1            = im->edgetype; x2 = im->edgewidth;
        x3            = im->edgecolor; x4 = im->edgevis;
        im->edgetype  = CDEDGETYPE; im->edgewidth = CDEDGEWIDTH;
        im->edgecolor = CDEDGECOLOR; im->edgevis = CDEDGEVIS;
        if ( !cdSetShapeEdgeAttrib( im, x1, x2, x3, x4 ) )
            return 0;

        x1             = im->textfont; x2 = im->textcolor; x3 = im->textheight;
        im->textfont   = CDTEXTFONT; im->textcolor = CDTEXTCOLOR;
        im->textheight = CDTEXTHEIGHT;
        if ( !cdSetTextAttrib( im, x1, x2, x3 ) )
            return 0;

        x1 = im->textpath; im->textpath = CDTEXTPATH;
        if ( !cdSetTextPath( im, x1 ) )
            return 0;

        x1        = im->mtype; x2 = im->msize; x3 = im->mcolor;
        im->ltype = CDMTYPE; im->lwidth = CDMSIZE; im->lcolor = CDMCOLOR;
        if ( !cdSetMarkerAttrib( im, x1, x2, x3 ) )
            return 0;
    }
    else
    {
        // reset all the defaults
        cdImageSetDefaults( im );
        // Nuke the color table if there is one
        cdImageColorClear( im );
        im->state = 1;         // now we are officially in the picture
    }
    return 1;
}

int cdCgmNewPic( cdImagePtr im, int sticky )
// The CGM standard allows multiple images in a single file.  This function
// will close the current picture, then open a new one.
// if sticky is 0 then all attributes will be reset to the defaults
// if sticky is 1 then all attributes will be inherited from the prevous
// picture.
// if sticky is 2 all attributes except the color table will be inherited
// from the previous picture
//
{
    // close the current picture
    if ( !cdImageEndPic( im ) )
        return 0;

    // now start the new picture
    return ( cdCgmPic( im, sticky ) );
}

int cdImageCgm( cdImagePtr im, FILE *out )
// Gej: Write the image to  file *out, which must be open already
// does not close the file
{
//
//      if (out) {
//              im->outfile = out;
//      }
//
    cdImageSetOutput( im, out );
    return cdImageEndCgm( im );
}


int cdSetLineType( cdImagePtr im, int lntype )
{
// Attribute: Line Type; Elem Class 5; Elem ID 2
// Set the line type.  Possible values are:
// 1=solid, 2=dash, 3=dot, 4=dash-dot, 5=dash-dot-dot
// Even though new ones can be defined, I am limiting lntype to these values
// If you really need more, you can make the proper changes.
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( lntype == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( lntype == im->ltype )
        return 1;

    // Make sure that lntype is between 1 and 5
    if ( ( lntype < 1 ) || ( lntype > 5 ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 5, 2, 2 ) )
    {
        free( esp ); return 0;
    }
    es += 2;
    // set Param_List_Len to 2 (signed int at index precision)

    // add in the value of lntype
    es += cdAppShort( es, (short int) lntype );

    octet_count = 4;     // we just know this

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->ltype = (short int) lntype;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetLineWidth( cdImagePtr im, int lnwidth )
{
// Attribute: Line Width; Elem Class 5; Elem ID 3
// sets the line width.  with an image of height X with line width 1
// the displayed width will be 1/X%.  as an example, if you image is
// x=5, y=10, and you set line width = 1, and draw a vertical line, the
// resulting line will  cover 20% of horizontal area.
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( lnwidth == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( lnwidth == im->lwidth )
        return 1;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;


    //gej: line width is 32 bit floating point number, 16 bits before the
    // decimal, 16 bits after if Line Spec is default (1, scaled)
    // if Line Spec is 0 (0, absolute) then it is 16 bit SI
    if ( im->linespec )
    {
        if ( !cdcomhead( es, 5, 3, 4 ) )
        {
            free( esp ); return 0;
        }
        es          += 2;
        octet_count  = 2;
        es          += cdAppShort( es, (short int) lnwidth );
        octet_count += 2;
        // the next two (after decimal point) will always be zero
        es          += cdAppNull( es, 2 );
        octet_count += 2;
    }
    else
    {
        if ( !cdcomhead( es, 5, 3, 2 ) )
        {
            free( esp ); return 0;
        }
        octet_count  = 2;
        es          += 2;
        es          += cdAppShort( es, (short int) lnwidth );
        octet_count += 2;
    }


    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->lwidth = lnwidth;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetLineColor( cdImagePtr im, int lncolor )
{
// Attribute: Line Colour; Elem Class 5; Elem ID 4
// Sets the line color.  lncolor should be an index into the color
// table that you have previously allocated.
//
    unsigned char *es, *esp;
    int           octet_count;
    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( lncolor == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( lncolor == im->lcolor )
        return 1;

    // Make sure the color they want to use has been allocated.
    // also, that color must be non-negative
    if ( ( lncolor >= im->colorsTotal ) || ( lncolor < 0 ) )
        return 0;          // you must allocate a color before you use it

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;


    if ( !cdcomhead( es, 5, 4, 1 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    *es = 0377 & lncolor;      // mask off last 8 bits and put in es
    es++;

    es += cdAppNull( es, 1 );

    octet_count = 4;     // we just know this; 2 octets of header,
                         // 1 octet of data, 1 octet of null data

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->lcolor = (short int) lncolor;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetFillStyle( cdImagePtr im, int instyle )
{
// set the style of the interior of filled area elements.
// Attribute: Interior Style; Elem Class 5; Elem ID 22
// These attributes stay in effect until changed, so you don't have to output
// them every time.
//     Interior Style: (integers 0-6, corresponding to: hollow, solid,
//                      [not pattern], hatch, empty, [not geometric pattern],
//                      interpolated.)
// attribute is 16 bit signed int
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( instyle == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( instyle == im->shapestyle )
        return 1;

    // Make sure that lnhatch is between 0 and 6, but not
    // 2, 5, or 6
    if ( ( instyle < 0 ) || ( instyle > 4 ) || ( instyle == 2 ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    // set the header to Class 5, ID 22, Length 2
    if ( !cdcomhead( es, 5, 22, 2 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    // add in the value of inhatch
    es += cdAppShort( es, (short int) instyle );

    octet_count = 4;     // we just know this

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->shapestyle = (short int) instyle;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetFillColor( cdImagePtr im, int incolor )
{
// set the color of the interior of filled area elements
// Attribute: Fill Colour; Elem Class 5; Elem ID 23
// These attributes stay in effect until changed, so you don't have to output
// them every time.
//     Fill Colour: (index into the color table)
//
    unsigned char *es, *esp;
    int           octet_count;
    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( incolor == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( incolor == im->shapecolor )
        return 1;

    // Make sure the color they want to use has been allocated.
    // also, that color must be non-negative
    if ( ( incolor >= im->colorsTotal ) || ( incolor < 0 ) )
        return 0;          // you must allocate a color before you use it

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 5, 23, 1 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    *es = 0377 & incolor;      // mask off last 8 bits and put in es
    es++;
    es += cdAppNull( es, 1 );

    octet_count = 4;     // we just know this; 2 octets of header,
                         // 1 octet of data, 1 octet of null data

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->shapecolor = (short int) incolor;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetFillHatch( cdImagePtr im, int inhatch )
{
// Set the hatch pattern for the interior of filled-area elements
// the fill style must be set to hatch for this to have an effect.
// Attribute: Hatch Index; Elem Class 5; Elem ID 24
// These attributes stay in effect until changed, so you don't have to output
// them every time.
//     Hatch Index: (integers 1-6, corresponding to: horizontal lines,
//                   vertical lines, pos. slope parallel lines,
//                   neg. slope parallel lines, horizontal/vertical
//                   crosshatch, positive/negative slope crosshatch)
//

    unsigned char *es, *esp;
    int           octet_count, temp;


    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( inhatch == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( inhatch == im->shapehatch )
        return 1;

    // Make sure that lnhatch is between 1 and 6
    if ( ( inhatch < 1 ) || ( inhatch > 6 ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    // set the command header to class 5, id 24, length 2
    if ( !cdcomhead( es, 5, 24, 2 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    // add in the value of inhatch
    temp = inhatch >> 8;
    *es  = *es | ( temp & 0377 );
    es++;
    *es = *es | ( inhatch & 0377 );
    es++;

    octet_count = 4;     // we just know this

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->shapehatch = (short int) inhatch;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetEdgeType( cdImagePtr im, int edtype )
{
// set the type of the edge of filled-area elements.
// Attribute: Edge Type; Elem Class 5; Elem ID 27
// These attributes stay in effect until changed, so you don't have to output
// them every time.
//     Edge Type (integers 1-5, corresponding to: solid, dash, dot,
//                dash-dot, dash-dot-dot. These are the same as those used
//                for line type.)
// In Part 3 of the standard (Binary Encoding) on page 47 it says that
// edge type is integer.  This is incorrect.  Edge type is Index, just
// like line type.
// Even though new ones can be defined, I am limiting lntype to these values
// If you really need more, you can make the proper changes.
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( edtype == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( edtype == im->edgetype )
        return 1;

    // Make sure that lntype is between 1 and 5
    if ( ( edtype < 1 ) || ( edtype > 5 ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 5, 27, 2 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    // add in the value of edtype
    es += cdAppShort( es, (short int) edtype );

    octet_count = 4;     // we just know this

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->edgetype = (short int) edtype;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetEdgeWidth( cdImagePtr im, int edwidth )
{
// Set the width of the edge of filled-area elements.
// Attribute: Edge Width; Elem Class 5; Elem ID 28
// These attributes stay in effect until changed, so you don't have to output
// them every time.
//     Edge Width (should be the same as line width)
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( edwidth == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( edwidth == im->edgewidth )
        return 1;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    //gej: edge width is 32 bit floating point number, 16 bits before the
    // decimal, 16 bits after for default edge spec (1, scaled) if
    // edge spec is absolute (0) then just 16 bit SI
    if ( im->edgespec )
    {
        if ( !cdcomhead( es, 5, 28, 4 ) )
        {
            free( esp ); return 0;
        }
        es          += 2;
        octet_count  = 2;
        es          += cdAppShort( es, edwidth );
        octet_count += 2;
        // the next two (after decimal point) will always be zero
        es          += cdAppNull( es, 2 );
        octet_count += 2;
    }
    else
    {
        if ( !cdcomhead( es, 5, 28, 2 ) )
        {
            free( esp ); return 0;
        }
        es          += 2;
        octet_count  = 2;
        es          += cdAppShort( es, edwidth );
        octet_count += 2;
    }

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->edgewidth = edwidth;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetEdgeColor( cdImagePtr im, int edcolor )
{
// Set the color of the edge of filled-area elements.
// Attribute: Edge Color; Elem Class 5; Elem ID 29
// These attributes stay in effect until changed, so you don't have to output
// them every time.
//     Edge Colour (index into the color table)
//
    unsigned char *es, *esp;
    int           octet_count;
    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( edcolor == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( edcolor == im->edgecolor )
        return 1;

    // Make sure the color they want to use has been allocated.
    // also, that color must be non-negative
    if ( ( edcolor >= im->colorsTotal ) || ( edcolor < 0 ) )
        return 0;          // you must allocate a color before you use it

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;
    if ( !cdcomhead( es, 5, 29, 1 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    *es = 0377 & edcolor;      // mask off last 8 bits and put in es
    es++;
    es += cdAppNull( es, 1 );

    octet_count = 4;     // we just know this; 2 octets of header,
                         // 1 octet of data, 1 octet of null data

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->edgecolor = (short int) edcolor;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetEdgeVis( cdImagePtr im, int edvis )
{
// Set the visibility of the edge of filled-area elements.
// Attribute: Edge Visibility; Elem Class 5; Elem ID 30
// These attributes stay in effect until changed, so you don't have to output
// them every time.
//     Edge Visibility (integer 0 or 1, corresponding to: Off, On)
// Attribute is 16 bit signed int.
//
    unsigned char *es, *esp;
    int           octet_count, temp;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( edvis == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( edvis == im->edgevis )
        return 1;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 5, 30, 2 ) )
    {
        free( esp ); return 0;
    }
    es  += 2; octet_count = 2;
    temp = edvis >> 8;
    *es  = *es | ( temp & 0377 );
    es++;
    *es = *es | ( edvis & 0377 );
    es++;
    octet_count += 2;


    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->edgevis = (short int) edvis;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetTextFont( cdImagePtr im, int font )
{
// Attribute: Text Font Index; Elem Class 5; Elem ID 10
// font is an index into the font table.  it can have one of the following
// values:
// 1 Times Roman
// 2 Times Bold
// 3 Times Italic
// 4 Times Bold Italic
// 5 Helvetica
// 6 Helvetica Bold
// 7 Helvetica Italic
// 8 Helvetica Bold Italic
// 9 Courier
// 10 Courier Bold
// 11 Courier Italic
// 12 Courier Bold Italic
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( font == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( font == im->textfont )
        return 1;

    // Make sure that font is between 1 and the number of fonts
    if ( ( font < 1 ) || ( font > im->numfonts ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 5, 10, 2 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    es += cdAppShort( es, (short int) font );

    octet_count = 4;     // we just know this

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->textfont = (short int) font;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetTextColor( cdImagePtr im, int color )
{
// Attribute: Text Colour ; Elem Class 5; Elem ID 14
// set the forground color of text
//
    unsigned char *es, *esp;
    int           octet_count;
    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( color == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( color == im->textcolor )
        return 1;

    // Make sure the color they want to use has been allocated.
    // also, that color must be non-negative
    if ( ( color >= im->colorsTotal ) || ( color < 0 ) )
        return 0;          // you must allocate a color before you use it

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 5, 14, 1 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    *es = 0377 & color;      // mask off last 8 bits and put in es
    es++;

    octet_count = 4;     // we just know this; 2 octets of header,
                         // 1 octet of data, 1 octet of null data

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->textcolor = (short int) color;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetTextHeight( cdImagePtr im, int height )
{
// Attribute: Character Height; Elem Class 5; Elem ID 15
// the height is in the same units as line width
//
    unsigned char *es, *esp;
    int           octet_count;
    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( height == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( height == im->textheight )
        return 1;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 5, 15, 2 ) )
    {
        free( esp ); return 0;
    }
    octet_count = 2; es += 2;

    es          += cdAppShort( es, height );
    octet_count += 2;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->textheight = height;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetTextPath( cdImagePtr im, int tpath )
{
// Attribute: Text Path; Elem Class 5; Elem ID 17
// Is one of:
//   0 right -- Means the direction of the character base vector
//   1 left  -- means 180 degrees from the character base vector
//   2 up    -- means the direction of the character up vector
//   3 down  -- means 180 degrees from the character up vector
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( tpath == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( tpath == im->textpath )
        return 1;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp         = es;
    octet_count = 0;

    if ( !cdcomhead( es, 5, 17, 2 ) )
    {
        free( esp ); return 0;
    }
    es += 2; octet_count = 2;

    es          += cdAppShort( es, (short int) tpath );
    octet_count += 2;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->textpath = (short int) tpath;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetTextOrient( cdImagePtr im, int xup, int yup, int xbase, int ybase )
{
// Attribute: Character Orientation; Elem Class 5; Elem ID 16
// (xbase,ybase) is the run and the rise of the line that the text is
// written along.  For regular text at an angle, set xup = -ybase
// and yup = xbase.  Setting it to something different will result in
// skewed text (which may be what you want.) Text written from bottom to
// top at a 90 degree angle would have the following parameters
// xup=-1, yup=0, xbase=0, ybase=1
//
// This function adds the Orientation to the metafile every time.
// It does not follow the normal -1 for no change, although if you
// put in the same numbers it won't re-add it to the meta file.
//
    unsigned char *es, *esp;
    int           octet_count;


    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp         = es;
    octet_count = 0;

    if ( !cdcomhead( es, 5, 16, 8 ) )
    {
        free( esp ); return 0;
    }
    es += 2; octet_count += 2;

    // In the metafile it is a 16 bit signed integer
    // add xup
    es          += cdAppShort( es, (short int) xup );
    octet_count += 2;
    // add the rest
    es          += cdAppShort( es, (short int) yup );
    octet_count += 2;
    es          += cdAppShort( es, (short int) xbase );
    octet_count += 2;
    es          += cdAppShort( es, (short int) ybase );
    octet_count += 2;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetMarkerType( cdImagePtr im, int mtype )
{
// Attribute: Marker Type; Elem Class 5; Elem ID 6
// Set the Marker type.  Possible values are:
// 1=dot, 2=plus, 3=asterisk, 4=circle, 5=cross
// Even though new ones can be defined, I am limiting lntype to these values
// If you really need more, you can make the proper changes.
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( mtype == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( mtype == im->mtype )
        return 1;

    // Make sure that mtype is between 1 and 5
    if ( ( mtype < 1 ) || ( mtype > 5 ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 5, 6, 2 ) )
    {
        free( esp ); return 0;
    }
    es += 2;
    // set Param_List_Len to 2 (signed int at index precision)

    // add in the value of mtype
    es += cdAppShort( es, (short int) mtype );

    octet_count = 4;     // we just know this

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->mtype = (short int) mtype;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetMarkerSize( cdImagePtr im, int msize )
{
// Attribute: Marker Size; Elem Class 5; Elem ID 7
// sets the marker size.  with an image of height X with marker size 1
// the displayed size will be 1/X%.  as an example, if you image is
// x=5, y=10, and you set marker size = 1, and draw a marker, the
// resulting marker will  cover 20% of horizontal area.
//
    unsigned char *es, *esp;
    int           octet_count;

    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( msize == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( msize == im->msize )
        return 1;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;


    //gej: marker size is 32 bit floating point number, 16 bits before the
    // decimal, 16 bits after if marker spec is default (1, scaled)
    // for absolute mode (0, absolute) it is 16 bit SI
    if ( im->markerspec )
    {
        if ( !cdcomhead( es, 5, 7, 4 ) )
        {
            free( esp ); return 0;
        }
        octet_count  = 2;
        es          += 2;
        es          += cdAppShort( es, (short int) msize );
        octet_count += 2;
        // the next two (after decimal point) will always be zero
        es          += cdAppNull( es, 2 );
        octet_count += 2;
    }
    else
    {
        if ( !cdcomhead( es, 5, 7, 4 ) )
        {
            free( esp ); return 0;
        }
        octet_count  = 2;
        es          += 2;
        es          += cdAppShort( es, (short int) msize );
        octet_count += 2;
    }


    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->msize = msize;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetMarkerColor( cdImagePtr im, int mcolor )
{
// Attribute: Marker Colour; Elem Class 5; Elem ID 8
// Sets the marker color.  mcolor should be an index into the color
// table that you have previously allocated.
//
    unsigned char *es, *esp;
    int           octet_count;
    // First check and see if the user doesn't want any changes,
    // if so, just return success
    if ( mcolor == -1 )
        return 1;

    // Check and see if the value it is being set to is the current
    // value, if so, don't make any changes, just return 1
    if ( mcolor == im->mcolor )
        return 1;

    // Make sure the color they want to use has been allocated.
    // also, that color must be non-negative
    if ( ( mcolor >= im->colorsTotal ) || ( mcolor < 0 ) )
        return 0;          // you must allocate a color before you use it

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;


    if ( !cdcomhead( es, 5, 8, 1 ) )
    {
        free( esp ); return 0;
    }
    es += 2;

    *es = 0377 & mcolor;      // mask off last 8 bits and put in es
    es++;

    es += cdAppNull( es, 1 );

    octet_count = 4;     // we just know this; 2 octets of header,
                         // 1 octet of data, 1 octet of null data

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        im->mcolor = (short int) mcolor;
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdSetLineAttrib( cdImagePtr im, int lntype, int lnwidth, int lncolor )
{
// Spits out the attributes of lines.  These attributes stay in effect
// until changed, so you don't have to output them every time.
//

    if ( !cdSetLineType( im, lntype ) )
        return 0;
    if ( !cdSetLineWidth( im, lnwidth ) )
        return 0;
    if ( !cdSetLineColor( im, lncolor ) )
        return 0;

    return 1;
}

int cdSetShapeFillAttrib( cdImagePtr im, int instyle, int incolor, int inhatch )
{
// Spits out the attributes for the interior of filled-area elements.
// These attributes stay in effect until changed, so you don't have to output
// them every time.
// Set the following attributes:
//     Interior Style: (integers 0-6, corresponding to: hollow, solid,
//                      [not pattern], hatch, empty, [not geometric pattern],
//                      interpolated.)
//     Fill Colour: (index into the color table)
//     Hatch Index: (integers 1-6, corresponding to: horizontal lines,
//                   vertical lines, pos. slope parallel lines,
//                   neg. slope parallel lines, horizontal/vertical
//                   crosshatch, positive/negative slope crosshatch)
//
    if ( !cdSetFillStyle( im, instyle ) )
        return 0;
    if ( !cdSetFillColor( im, incolor ) )
        return 0;
    if ( !cdSetFillHatch( im, inhatch ) )
        return 0;

    return 1;
}

int cdSetShapeEdgeAttrib( cdImagePtr im, int edtype, int edwidth, int edcolor, int edvis )
{
// Spits out the attributes for the edges of filled-area elements.  It may
// seem logical that these would be the same as the corresponding line
// attributes, but this is not the case.
// These attributes stay in effect until changed, so you don't have to output
// them every time.
// Set the following attributes:
//     Edge Type (integers 1-5, corresponding to: solid, dash, dot,
//                dash-dot, dash-dot-dot. These are the same as those used
//                for line type.)
//     Edge Width (should be the same as line width)
//     Edge Colour (index into the color table)
//     Edge Visibility (integer 0 or 1, corresponding to: Off, On)
//
    if ( !cdSetEdgeType( im, edtype ) )
        return 0;
    if ( !cdSetEdgeWidth( im, edwidth ) )
        return 0;
    if ( !cdSetEdgeColor( im, edcolor ) )
        return 0;
    if ( !cdSetEdgeVis( im, edvis ) )
        return 0;

    return 1;
}

int cdSetTextAttrib( cdImagePtr im, int font, int color, int height )
{
// Set the attributes of text.  the font is an integer pointer into the
// font list where:
// 1 Times
// 2 Times Bold
// 3 Times Italic
// 4 Times Bold Italic
// 5 Helvetica
// 6 Helvetica Bold
// 7 Helvetica Italic
// 8 Helvetica Bold Italic
// 9 Courier
// 10 Courier Bold
// 11 Courier Italic
// 12 Courier Bold Italic
// color is an index into the colortable which is the color of the text
// size is the approximate size you want the text written in.
//

    if ( !cdSetTextFont( im, font ) )
        return 0;
    if ( !cdSetTextColor( im, color ) )
        return 0;
    if ( !cdSetTextHeight( im, height ) )
        return 0;

    return 1;
}

int cdSetMarkerAttrib( cdImagePtr im, int mtype, int msize, int mcolor )
{
// Spits out the attributes of Markers.  These attributes stay in effect
// until changed, so you don't have to output them every time.
//

    if ( !cdSetMarkerType( im, mtype ) )
        return 0;
    if ( !cdSetMarkerSize( im, msize ) )
        return 0;
    if ( !cdSetMarkerColor( im, mcolor ) )
        return 0;

    return 1;
}

int cdImageDestroy( cdImagePtr im )
// gej: should work, unless I make changes to cdImage Struct
{
    if ( im->elemlist )
    {
        free( im->elemlist );
    }
    if ( im->desc )
    {
        free( im->desc );
    }
    if ( im->fontlist )
    {
        free( im->fontlist );
    }
    free( im );

    return 1;
}

int cdImageColorClosest( cdImagePtr im, int r, int g, int b )
// From gd library, see README file for copyright information
// gej: should work unchanged
// gej: 5/96, changed the colors to use short int
{
    short int i;
    long      rd, gd, bd;
    int       ct      = ( -1 );
    long      mindist = 0;
    for ( i = 0; ( i < ( im->colorsTotal ) ); i++ )
    {
        long dist;
        if ( im->open[i] )
        {
            continue;
        }
        rd   = ( im->red[i] - r );
        gd   = ( im->green[i] - g );
        bd   = ( im->blue[i] - b );
        dist = rd * rd + gd * gd + bd * bd;
        if ( ( i == 0 ) || ( dist < mindist ) )
        {
            mindist = dist;
            ct      = i;
        }
    }
    return ct;
}

int cdImageColorClear( cdImagePtr im )
{
// mark all entries in the color table as open
    short int i;
    for ( i = 0; ( i < ( cdMaxColors ) ); i++ )
    {
        im->open[i] = 1;
    }
    return 1;
}

int cdImageColorExact( cdImagePtr im, int r, int g, int b )
// From gd library, see README file for copyright information
// gej: should work unchanged
// gej: 5/96, changed colors to work with short ints
{
    short int i;
    for ( i = 0; ( i < ( im->colorsTotal ) ); i++ )
    {
        if ( im->open[i] )
        {
            continue;
        }
        if ( ( im->red[i] == r ) &&
             ( im->green[i] == g ) &&
             ( im->blue[i] == b ) )
        {
            return i;
        }
    }
    return -1;
}

static int cdImageAddColorIndex( cdImagePtr im, int r, int g, int b )
// adds the specified color to the colortable in the cdImagePtr.
// does not add it to the cgm file, cdImageAddColor does.
// do not use either of these two functions, use cdImageColorAllocate.
//
{
    short int i;
    short int ct = ( -1 );
    for ( i = 0; ( i < ( im->colorsTotal ) ); i++ )
    {
        if ( im->open[i] )
        {
            ct = i;
            break;
        }
    }
    if ( ct == ( -1 ) )
    {
        ct = im->colorsTotal;
        if ( ct == cdMaxColors )
        {
            return -1;
        }
        im->colorsTotal++;
    }
    im->red[ct]   = (short int) r;
    im->green[ct] = (short int) g;
    im->blue[ct]  = (short int) b;
    im->open[ct]  = (short int) 0;

    return ct;
}

static int cdImageAddColor( cdImagePtr im, int si, int ei )
// adds colors to the cgm file, gets values from the color table.
// adds all colors from si to ei inclusive.
// Use cdImageColorAllocate, not this one.
//
{
    unsigned char *cts, *ctsp;     // GEJ: color table attribute
    int           octet_count;     // GEJ: octet count
    int           numco, curly;
    octet_count = 0;
    //
    // Attribute: Colour Table; Elem Class 5; Elem ID 34
    // two parameters P1: Starting colour table index (1 octet, UI)
    // P2: list of direct colour values 3-tuples (3 one-octet values)
    //
    // G E J: find out how many values are being added
    if ( ei < 0 )
        return -1;             // no colors being added
    numco = ei - si + 1;

    if ( ( numco > 0 ) && ( numco < 10 ) )
    {
        // we can use the short form of the command
        // allocate sufficent space. Should be 32 bits * 10 to be safe
        cts = (unsigned char *) calloc( 4 * 10, SIZEOF( unsigned char ) );
        if ( !cts )
            return -1;               // memory allocation failed
        ctsp = cts;
        if ( !cdcomhead( ctsp, 5, 34, ( numco * 3 ) + 1 ) )
        {
            free( cts ); return -1;
        }
        ctsp += 2; octet_count += 2;
    }
    else if ( ( numco > 9 ) && ( numco < 256 ) )
    {
        // we must use the long form of the command
        // allocate sufficent space. Should be 32 bits*256 to be safe
        cts = (unsigned char *) calloc( 256 * 4, SIZEOF( unsigned char ) );
        if ( !cts )
            return -1;               // memory allocation failed
        ctsp = cts;
        if ( !cdcomheadlong( ctsp, 5, 34, ( numco * 3 ) + 1 ) )
        {
            free( cts ); return -1;
        }
        ctsp += 4; octet_count += 4;
    }
    else
    {
        return -1;
    }

    //ctsp += cdAppByte(ctsp, (short int) si);
    cdAppByte( ctsp, (short int) si );
    ctsp++;
    octet_count++;
    for ( numco = si; numco <= ei; numco++ )
    {
        ctsp        += cdAppByte( ctsp, im->red[numco] );
        ctsp        += cdAppByte( ctsp, im->green[numco] );
        ctsp        += cdAppByte( ctsp, im->blue[numco] );
        octet_count += 3;
    }


    curly = 4 - ( octet_count % 4 );
    if ( curly % 4 )
    {
        octet_count += curly;
        ctsp        += cdAppNull( ctsp, curly );
    }
    // add it to the buffer
    if ( cdAddElem( im, cts, octet_count ) )
    {
        free( cts );
        return 1;
    }
    else
    {
        free( cts );
        return -1;
    }
}

int cdImageColorAllocate( cdImagePtr im, int r, int g, int b )
// From gd library, see README file for copyright information
// gej: modified to allocate the color in the CGM buffer as well
// as the color table
// gej: 5/96, modified to use short ints for colors
{
    short int ct;
    ct = cdImageAddColorIndex( im, r, g, b );
    if ( ct == -1 )
        return -1;
    // GEJ: w we have successfully alocated it in the color table
    // so let's put it in the CGM as well.
    //
    if ( cdImageAddColor( im, ct, ct ) == -1 )
    {
        return -1;
    }
    else
    {
        return ct;
    }
}

int cdImageColor16( cdImagePtr im )
{
// allocate the 16 basic colors in the windows pallete
//
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//      cdImageColorAllocate(im, 0, 0, 0);
//
    int si, ei, li;
    si = cdImageAddColorIndex( im, 255, 255, 255 );
    if ( si == -1 )
        return 0;
    li = -1; ei = si;
    ei = cdImageAddColorIndex( im, 0, 0, 0 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 128, 0, 0 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 0, 128, 0 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 128, 128, 0 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 0, 0, 128 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 128, 0, 128 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 0, 128, 128 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 128, 128, 128 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 192, 192, 192 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 255, 0, 0 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 0, 255, 0 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 255, 255, 0 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 0, 0, 255 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 255, 0, 255 );
    if ( ei != -1 )
    {
        li = ei;
    }
    ei = cdImageAddColorIndex( im, 0, 255, 255 );
    if ( ei != -1 )
    {
        li = ei;
    }

    if ( ei == -1 )
    {
        ei = li;
    }
    if ( cdImageAddColor( im, si, ei ) == -1 )
    {
        return -1;
    }
    else
    {
        return ei;
    }
}
int cdImageColorDeallocate( cdImagePtr im, int color )
// wogl: the parameter names are commented to avoid compiler warnings
// From gd library, see README file for copyright information
// gej: should work unchanged
{
    // Mark it open.
    //im->open[color] = 1;
    // gej: really can't work, we are not allowing redefinition
    // of color table entries
    return 0;
}

int cdLine( cdImagePtr im, int x1, int y1, int x2, int y2 )
// Graphic Primitive: Polyline; Elem Class 4; Elem ID 1
// Actually generate the line, if you are writing a program to use this
// library, use this function, not cdImageLine or cdImageDashedLine,
// those are just in for compatiblilty with gd
//
// This function will draw a line using the current line type, width, and color
//
{
    unsigned char *es, *esp;
    int           octet_count;
    short int     sweet;
    short int     sour;

    // check to make sure the line is withing the scope of the picture
    // ie. the values you give for drawing the line are within
    // the values you created the picture with
    if ( !( cdImageBoundsSafe( im, x1, y1 ) ) || !( cdImageBoundsSafe( im, x2, y2 ) ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 4, 1, 8 ) )
    {
        free( esp ); return 0;
    }
    es         += 2;
    octet_count = 2;

    // now we are ready for the parameter data
    sweet = (short int) x1;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) y1;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) x2;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) y2;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    octet_count++;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdMarker( cdImagePtr im, int x, int y )
// Graphic Primitive: PolyMarker; Elem Class 4; Elem ID 3
// puts a marker in the file, it will have characteristics set by
// cdSetMarkerAttrib
//
{
    unsigned char *es, *esp;
    int           octet_count;

    // check to make sure the line is withing the scope of the picture
    // ie. the values you give for drawing the line are within
    // the values you created the picture with
    if ( !cdImageBoundsSafe( im, x, y ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomhead( es, 4, 3, 4 ) )
    {
        free( esp ); return 0;
    }
    es         += 2;
    octet_count = 2;

    octet_count += cdAppShort( es, (short int) x );
    es          += 2;
    octet_count += cdAppShort( es, (short int) y );
    es          += 2;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}


int cdRectangle( cdImagePtr im, int x1, int y1, int x2, int y2 )
{
// Graphic Primitive: rectangle; Elem Class 4; Elem ID 11
// Actually generate the rectangle, if you are writing a program to use this
// library, use this function, not cdImageRectangle,
// those are just in for compatiblilty with gd
//
// This function will draw a Rectangle using the current
// edge type, width, color, and visibility, and the current
// fill style, color, and hatch
//
    unsigned char *es, *esp;
    int           octet_count;
    short int     sweet;
    short int     sour;

    // check to make sure the line is withing the scope of the picture
    // ie. the values you give for drawing the line are within
    // the values you created the picture with
    if ( !( cdImageBoundsSafe( im, x1, y1 ) ) || !( cdImageBoundsSafe( im, x2, y2 ) ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    // their are four 16 bit signed integers as attributes
    if ( !cdcomhead( es, 4, 11, 8 ) )
    {
        free( esp ); return 0;
    }
    es += 2; octet_count = 2;

    // now we are ready for the parameter data
    sweet = (short int) x1;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) y1;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) x2;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) y2;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    octet_count++;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdCircle( cdImagePtr im, int cx, int cy, int r )
{
// Graphic Primitive: circle; Elem Class 4; Elem ID 12
// cx,cy is the center of the circle, r is the radius
//
// This function will draw a Circle using the current
// edge type, width, color, and visibility, and the current
// fill style, color, and hatch
//
    unsigned char *es, *esp;
    int           octet_count;
    short int     sweet;
    short int     sour;

    // check to make sure the circle is withing the scope of the picture
    // ie. the values you give for drawing the circle are within
    // the values you created the picture with
    if ( !( cdImageBoundsSafe( im, cx, cy ) ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    // their are three 16 bit signed integers as attributes
    if ( !cdcomhead( es, 4, 12, 6 ) )
    {
        free( esp ); return 0;
    }
    es += 2; octet_count = 2;

    // now we are ready for the parameter data
    sweet = (short int) cx;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) cy;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) r;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    octet_count++;


    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdArc3Pt( cdImagePtr im, int sx, int sy, int ix, int iy, int ex, int ey )
{
// Graphic Primitive: Cicular Arc 3 Point; Elem Class 4; Elem ID 13
//
// This function will draw a Circular Arc using the current
// Line type, width, and color,
//
    unsigned char *es, *esp;
    int           octet_count;
    short int     sweet;
    short int     sour;

    // check to make sure the line is withing the scope of the picture
    // ie. the values you give for drawing the line are within
    // the values you created the picture with
    if ( !( cdImageBoundsSafe( im, sx, sy ) ) || !( cdImageBoundsSafe( im, ix, iy ) ) || !( cdImageBoundsSafe( im, ex, ey ) ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    // their are six 16 bit signed integers as attributes
    if ( !cdcomhead( es, 4, 13, 12 ) )
    {
        free( esp ); return 0;
    }
    es += 2; octet_count = 2;

    // now we are ready for the parameter data
    sweet = (short int) sx;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) sy;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) ix;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) iy;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) ex;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    es++; octet_count++;
    sweet = (short int) ey;
    sour  = sweet >> 8;
    *es   = *es | ( sour & 0377 );
    es++; octet_count++;
    *es = (unsigned char) sweet;
    octet_count++;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdArc3PtClose( cdImagePtr im, int sx, int sy, int ix, int iy, int ex, int ey, int cl )
{
// Graphic Primitive: Cicular Arc 3 Point Close; Elem Class 4; Elem ID 14
//
// This function will draw a Circle using the current
// edge type, width, color, and visibility, and the current
// fill style, color, and hatch
//
// cd is the closure type.  It can be either 0 for pie closure or
// 1 for chord closure.
//
    unsigned char *es, *esp;
    int           octet_count;

    // check to make sure the line is withing the scope of the picture
    // ie. the values you give for drawing the line are within
    // the values you created the picture with
    if ( !( cdImageBoundsSafe( im, sx, sy ) ) || !( cdImageBoundsSafe( im, ix, iy ) ) || !( cdImageBoundsSafe( im, ex, ey ) ) )
        return 0;

    // make sure that they close the arc either with pie (0) or chord (1)
    if ( ( cl != 0 ) && ( cl != 1 ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 6 to be safe
    es = (unsigned char *) calloc( 4 * 6, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    // their are seven 16 bit signed integers as attributes
    if ( !cdcomhead( es, 4, 14, 14 ) )
    {
        free( esp ); return 0;
    }
    es += 2; octet_count = 2;

    // now we are ready for the parameter data
    octet_count += cdAppShort( es, (short int) sx );
    es          += 2;
    octet_count += cdAppShort( es, (short int) sy );
    es          += 2;
    octet_count += cdAppShort( es, (short int) ix );
    es          += 2;
    octet_count += cdAppShort( es, (short int) iy );
    es          += 2;
    octet_count += cdAppShort( es, (short int) ex );
    es          += 2;
    octet_count += cdAppShort( es, (short int) ey );
    es          += 2;
    octet_count += cdAppShort( es, (short int) cl );
    es          += 2;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdEllipse( cdImagePtr im, int cx, int cy, int d1x, int d1y, int d2x, int d2y )
{
// Graphic Primitive: Ellipse; Elem Class 4; Elem ID 17
//
// This function will draw an Ellipse using the current
// edge type, width, color, and visibility, and the current
// fill style, color, and hatch
//
    unsigned char *es, *esp;
    int           octet_count;

    // check to make sure the line is withing the scope of the picture
    // ie. the values you give for drawing the line are within
    // the values you created the picture with
    if ( !( cdImageBoundsSafe( im, cx, cy ) ) || !( cdImageBoundsSafe( im, d1x, d1y ) ) || !( cdImageBoundsSafe( im, d2x, d2y ) ) )
        return 0;

    // allocate sufficent space.  should be 32 bits * 4 to be safe
    es = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    // their are six 16 bit signed integers as attributes
    if ( !cdcomhead( es, 4, 17, 12 ) )
    {
        free( esp ); return 0;
    }
    es += 2; octet_count = 2;

    // now we are ready for the parameter data
    octet_count += cdAppShort( es, (short int) cx );
    es          += 2;
    octet_count += cdAppShort( es, (short int) cy );
    es          += 2;
    octet_count += cdAppShort( es, (short int) d1x );
    es          += 2;
    octet_count += cdAppShort( es, (short int) d1y );
    es          += 2;
    octet_count += cdAppShort( es, (short int) d2x );
    es          += 2;
    octet_count += cdAppShort( es, (short int) d2y );
    es          += 2;

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdPolygon( cdImagePtr im, cdPointPtr p, int n )
{
// Graphic Primitive: Polygon; Elem Class 4; Elem ID 7
//
// cdPointPtr is defined in cd.h, basically, it is two arrays of integers
// p[m].x and p[m].y  containing the x and y values respectively.  n
// is the number of points in this array (not the index of the last point,
// which is n-1).  n must be at least 3 (otherwise
// you really don't have much of a polygon, it is closer to a line.)
//
// This function will draw a Polygon using the current
// edge type, width, color, and visibility, and the current
// fill style, color, and hatch
//
    unsigned char *es, *esp;
    int           octet_count;
    int           x;         // counter

    if ( n < 3 )
        return 0;            // it is either a point or a line

    if ( n < 8 )
    {
        // It fits in the short form of the command, lets us
        // add it right now, shall we?
        // allocate sufficent space. Should be 32 bits*10 to be safe
        es = (unsigned char *) calloc( 4 * 10, SIZEOF( unsigned char ) );
        if ( !es )
            return 0;              // memory allocation failed
        esp = es;


        // their are n*2 16 bit signed integers as attributes
        if ( !cdcomhead( es, 4, 7, ( n * 4 ) ) )
        {
            free( esp ); return 0;
        }
        es += 2; octet_count = 2;
    }
    else if ( n < 8191 )
    {
        // there are more than 7 points in it, that sucks
        // gej, so basically, for this one, I set the header
        // to cdcomhead(es, 4, 7, 31) then write a function for the long
        // form that takes the first 15 bits of n and tags a 0 in front
        // of it and puts it in es, than I do the for loop all over again
        // that doesn't seem too hard.  But I will leave that for another
        // day.
        //
        // keep in mind that if CDGROWLISTSIZE is smaller than n*4
        // (at most 32769) then things could fail in a most unsavory fashion.
        //
        // allocate sufficent space.  32 bits*(n+1) to be safe
        es = (unsigned char *) calloc( 4 * ( n + 1 ), SIZEOF( unsigned char ) );
        if ( !es )
            return 0;              // memory allocation failed
        esp = es;

        if ( !cdcomheadlong( es, 4, 7, ( n * 4 ) ) )
        {
            free( esp ); return 0;
        }
        es += 4; octet_count = 4;
    }
    else
    {
        // there are more than 8191 points in it, I am not going to implement
        // that, if you want it that bad, do it yourself.
        return 0;
    }


    for ( x = 0; x < n; x++ )
    {
        // now we are ready for the parameter data
        es          += cdAppShort( es, (short int) p->x );
        es          += cdAppShort( es, (short int) p->y );
        octet_count += 4;
        p++;
    }

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdPolygonSet( cdImagePtr im, cdPointPtr p, int n )
{
// Graphic Primitive: Polygon; Elem Class 4; Elem ID 8
//
// cdPointPtr is defined in cd.h, basically, it is three arrays of integers
// p[m].x and p[m].y  containing the x and y values respectively and p[m].e
// the characteristics of the line leaving point n (0=invisible,1=visible,
// 2=close,invisible, 3=close,visible).  n is the number of points in this
// array (not the index of the last point, which is n-1).
// n must be at least 3 (otherwise you really don't have much of a polygon,
// it is closer to a line.)
//
// This function will draw a set of Polygons using the current
// edge type, width, color, and the current
// fill style, color, and hatch
//
    unsigned char *es, *esp;
    int           octet_count;
    int           x;         // counter

    if ( n < 3 )
        return 0;            // it is either a point or a line

    if ( n < 6 )
    {
        // It fits in the short form of the command, lets us
        // add it right now, shall we?
        // allocate sufficent space. Should be 48 bits*10 to be safe
        es = (unsigned char *) calloc( 6 * 10, SIZEOF( unsigned char ) );
        if ( !es )
            return 0;              // memory allocation failed
        esp = es;


        // their are n*2 16 bit signed integers as attributes
        if ( !cdcomhead( es, 4, 8, ( n * 6 ) ) )
        {
            free( esp ); return 0;
        }
        es += 2; octet_count = 2;
    }
    else if ( n < 5462 )
    {
        // there are more than 5 points in it, that sucks
        // gej, so basically, for this one, I set the header
        // to cdcomhead(es, 4, 7, 31) then write a function for the long
        // form that takes the first 15 bits of n and tags a 0 in front
        // of it and puts it in es, than I do the for loop all over again
        // that doesn't seem too hard.  But I will leave that for another
        // day.
        //
        // keep in mind that if CDGROWLISTSIZE is smaller than n*6
        // (at most 32769) then things could fail in a most unsavory fashion.
        //
        // allocate sufficent space.  48 bits*(n+1) to be safe
        es = (unsigned char *) calloc( 6 * ( n + 1 ), SIZEOF( unsigned char ) );
        if ( !es )
            return 0;              // memory allocation failed
        esp = es;

        if ( !cdcomheadlong( es, 4, 8, ( n * 6 ) ) )
        {
            free( esp ); return 0;
        }
        es += 4; octet_count = 4;
    }
    else
    {
        // there are more than 5462 points in it, I am not going to implement
        // that, if you want it that bad, do it yourself.
        return 0;
    }


    for ( x = 0; x < n; x++ )
    {
        // now we are ready for the parameter data
        es          += cdAppShort( es, (short int) p->x );
        es          += cdAppShort( es, (short int) p->y );
        es          += cdAppShort( es, (short int) p->e );
        octet_count += 6;
        p++;
    }

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdPolyLine( cdImagePtr im, cdPointPtr p, int n )
{
// Graphic Primitive: Polyline; Elem Class 4; Elem ID 1
//
// cdPointPtr is defined in cd.h, basically, it is two arrays of integers
// p[m].x and p[m].y  containing the x and y values respectively.  n
// is the number of points in this array (not the index of the last point,
// which is n-1).  if n is 2, it is a regular line, like cdline
//
// This function will draw a Polyline using the current
// line type, width, color, and visibility,
//
    unsigned char *es, *esp;
    int           octet_count;
    int           x;         // counter

    if ( n < 2 )
        return 0;            // it is a point

    if ( n < 8 )
    {
        // It fits in the short form of the command, lets us
        // add it right now, shall we?
        // allocate sufficent space. Should be 32 bits*10 to be safe
        es = (unsigned char *) calloc( 4 * 10, SIZEOF( unsigned char ) );
        if ( !es )
            return 0;              // memory allocation failed
        esp = es;


        // their are n*2 16 bit signed integers as attributes
        if ( !cdcomhead( es, 4, 1, ( n * 4 ) ) )
        {
            free( esp ); return 0;
        }
        es += 2; octet_count = 2;
    }
    else if ( n < 8191 )
    {
        // there are more than 7 points in it, that sucks
        // gej, so basically, for this one, I set the header
        // using the long version cdcomheadlong(es, 4, 1, n*4)
        //
        // keep in mind that if CDGROWLISTSIZE is smaller than n*4
        // (at most 32769) then the list may have to grow several times
        //
        // allocate sufficent space.  32 bits*(n+1) to be safe
        es = (unsigned char *) calloc( 4 * ( n + 1 ), SIZEOF( unsigned char ) );
        if ( !es )
            return 0;              // memory allocation failed
        esp = es;

        if ( !cdcomheadlong( es, 4, 1, ( n * 4 ) ) )
        {
            free( esp ); return 0;
        }
        es += 4; octet_count = 4;
    }
    else
    {
        // there are more than 8191 points in it, I am not going to implement
        // that, if you want it that bad, do it yourself.
        return 0;
    }

    for ( x = 0; x < n; x++ )
    {
        // now we are ready for the parameter data
        es          += cdAppShort( es, (short int) p->x );
        es          += cdAppShort( es, (short int) p->y );
        octet_count += 4;
        p++;
    }

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdPolyMarker( cdImagePtr im, cdPointPtr p, int n )
{
// Graphic Primitive: PolyMarker; Elem Class 4; Elem ID 3
//
// cdPointPtr is defined in cd.h, basically, it is two arrays of integers
// p[m].x and p[m].y  containing the x and y values respectively.  n
// is the number of points in this array (not the index of the last point,
// which is n-1).  if n is 2, it is a regular line, like cdline
//
// This function will insert n markers using the current
// Marker type, width, color, and visibility,
//
    unsigned char *es, *esp;
    int           octet_count;
    int           x;         // counter

    if ( n < 1 )
        return 0;            // it is nothing
    if ( n < 8 )
    {
        // It fits in the short form of the command, lets us
        // add it right now, shall we?
        // allocate sufficent space. Should be 32 bits*10 to be safe
        es = (unsigned char *) calloc( 4 * 10, SIZEOF( unsigned char ) );
        if ( !es )
            return 0;              // memory allocation failed
        esp = es;


        // their are n*2 16 bit signed integers as attributes
        if ( !cdcomhead( es, 4, 3, ( n * 4 ) ) )
        {
            free( esp ); return 0;
        }
        es += 2; octet_count = 2;
    }
    else if ( n < 8191 )
    {
        // there are more than 7 points in it, that sucks
        // gej, so basically, for this one, I set the header
        // using the long version cdcomheadlong(es, 4, 1, n*4)
        //
        // keep in mind that if CDGROWLISTSIZE is smaller than n*4
        // (at most 32769) then the list may have to grow several times
        //
        // allocate sufficent space.  32 bits*(n+1) to be safe
        es = (unsigned char *) calloc( 4 * ( n + 1 ), SIZEOF( unsigned char ) );
        if ( !es )
            return 0;              // memory allocation failed
        esp = es;

        if ( !cdcomheadlong( es, 4, 3, ( n * 4 ) ) )
        {
            free( esp ); return 0;
        }
        es += 4; octet_count = 4;
    }
    else
    {
        // there are more than 8191 points in it, I am not going to implement
        // that, if you want it that bad, do it yourself.
        return 0;
    }

    for ( x = 0; x < n; x++ )
    {
        // now we are ready for the parameter data
        es          += cdAppShort( es, (short int) p->x );
        es          += cdAppShort( es, (short int) p->y );
        octet_count += 4;
        p++;
    }

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdText( cdImagePtr im, int x, int y, const char *ts )
{
// Graphic Primitive: Text; Elem Class 4; Elem ID 4
// add text to the picture.  Start it at the point (x,y)
// this should be the lower left corner of where the text is
// the parameters are point, enumerated(set to 1), string
//
// String encoding in CGM is a little strange.  After you have the other
// parameter info, the first octet for the string is either 0..254 which
// is the number of octets of string data, or 255 which signifies a long
// string.  if it is 255 then the next 16 bits indicate the length of the
// string.  the first bit (bit15) is 0 if this is the last part of the
// string and 1 if another part follows it.  the next 15 bits  are in the
// range 0..32767 and are the number of octets of string info following.
// so the length stored in the command header is the whole enchelada.
//
    int           tslen, curly;
    unsigned char *es, *esp;
    int           octet_count;

    // check to make sure the Text is within the scope of the picture
    // actually, I am only checking the start of it
    //
    if ( !( cdImageBoundsSafe( im, x, y ) ) )
        return 0;

    // allocate sufficent space.  should be tslen+ 32 bits * 4 to be safe
    tslen = strlen( ts );

    // if there are more than 32700 characters fail
    // gej: this could go as high as 32767 I think, but lets
    // cut it off at 32700
    if ( ( tslen > 32700 ) || ( tslen < 0 ) )
        return 0;

    es = (unsigned char *) calloc( ( ( 4 * 4 ) + tslen ), SIZEOF( unsigned char ) );
    if ( !es )
        return 0;          // memory allocation failed
    esp = es;

    if ( !cdcomheadlong( es, 4, 4, 9 + tslen ) )
    {
        free( esp ); return 0;
    }
    es += 4; octet_count = 4;

    // add the x position, the y position, then 1, which signifies
    // that this is all the text, there is none appended after it
    es          += cdAppShort( es, (short int) x );
    es          += cdAppShort( es, (short int) y );
    es          += cdAppShort( es, (short int) 1 );
    octet_count += 6;

    // now take care of the string information, for strings 254 bytes
    // or less, I could use a short one, but why bother, use the long
    // form for everything
    es          += cdAppByte( es, (short int) 255 );
    es          += cdAppShort( es, (short int) tslen );
    octet_count += 3;
    // gej: I should set bit 15 to 0 because it is the final part of a
    // string but I am not going to since I already checked that it was
    // a 16 number that was non-negative

    while ( *ts )
    {
        *es++ = (unsigned char) *ts++;
    }
    octet_count += tslen;
    // now if the octet_count is not divisible by 4 add null padding
    curly = 4 - ( octet_count % 4 );
    if ( curly % 4 )
    {
        octet_count += curly;
        es          += cdAppNull( es, curly );
    }

    // add it to the buffer
    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdImageLine( cdImagePtr im, int x1, int y1, int x2, int y2, int color )
// gej: this should be so much easier to do as a cgm
// This is in for compatibility with gd, if you don't need that, use
// cdLine instead
{
    int ltstate;


    // save the linetype state
    ltstate = im->ltype;
    // set the attributes of the line
    if ( !cdSetLineAttrib( im, 1, -1, color ) )
        return 0;
    if ( !cdLine( im, x1, y1, x2, y2 ) )
        return 0;                                  // draw the line
    // restore the state If it fails, don't return an error, because
    // the line was still drawn
    cdSetLineType( im, ltstate );

    return 1;
}

int cdImageDashedLine( cdImagePtr im, int x1, int y1, int x2, int y2, int color )
// gej: this should be so much easier to do as a cgm
// in order to really get a dashed line you must call cdSetLineType first
// This is in for compatibility with gd, if you don't need that, use
// cdLine instead
{
    // set the attributes of the line
    if ( !cdSetLineAttrib( im, -1, -1, color ) )
        return 0;
    // generate the line
    if ( !cdLine( im, x1, y1, x2, y2 ) )
        return 0;

    // everthing is A-OK
    return 1;
}

int cdImageBoundsSafe( cdImagePtr im, int x, int y )
// From gd library, see README file for copyright information
// gej: this should work unchanged
{
    return ( !( ( ( y < 0 ) || ( y >= im->sy ) ) ||
                ( ( x < 0 ) || ( x >= im->sx ) ) ) );
}


int cdImageRectangle( cdImagePtr im, int x1, int y1, int x2, int y2, int color )
// Graphic Primitive: rectangle; Elem Class 4; Elem ID 11
//

// gej: but I think I will use the cgm rectangle
{
    if ( !cdImageLine( im, x1, y1, x2, y1, color ) )
        return 0;
    if ( !cdImageLine( im, x1, y2, x2, y2, color ) )
        return 0;
    if ( !cdImageLine( im, x1, y1, x1, y2, color ) )
        return 0;
    if ( !cdImageLine( im, x2, y1, x2, y2, color ) )
        return 0;

    return 1;
}


// Expert functions.  If you need more control, you can use these
// functions, but you probably won't need to.

int cdImageSetSize( cdImagePtr im, int x, int y )
// sets the width and height of subsequent pictures.
{
    im->sx = x;
    im->sy = y;
    return 1;
}

int cdImageSetLineSpec( cdImagePtr im, int specmode )
// Picture Descriptor: Line Width Specification Mode; Elem Class 2; Elem ID 3
// sets the Line Width Specification mode of subsequent pictures.
// 1 is scaled (default), 2 is absolute
{
    if ( ( specmode < 0 ) || ( specmode > 2 ) )
        return 0;
    im->linespec = specmode;
    return 1;
}

int cdImageSetMarkerSpec( cdImagePtr im, int specmode )
// Picture Descriptor: Marker Size Specification Mode; Elem Class 2; Elem ID 4
// sets the Marker Width Specification mode of subsequent pictures.
// 1 is scaled (default), 2 is absolute
{
    if ( ( specmode < 0 ) || ( specmode > 2 ) )
        return 0;
    im->linespec = specmode;
    return 1;
}

int cdImageSetEdgeSpec( cdImagePtr im, int specmode )
// Picture Descriptor: Edge Width Specification Mode; Elem Class 2; Elem ID 5
// sets the Edge Width Specification mode of subsequent pictures.
// 1 is scaled (default), 2 is absolute
{
    if ( ( specmode < 0 ) || ( specmode > 2 ) )
        return 0;
    im->edgespec = specmode;
    return 1;
}

int cdImageSetOutput( cdImagePtr im, FILE *output )
// sets the output file to *output.  which must already be open.
// does not close the file
// Useful if you want to write the file as you go along, or if you
// want to write it to a stream
//
{
    if ( output )
    {
        im->outfile = output;
        return 1;
    }
    else
    {
        return 0;
    }
}

int cdImageAddFont( cdImagePtr im, const char *fontname )
// adds a font to the list of fonts.  This only has an effect
// if you are using the expert functions for starting pictures, and
// have not yet opened the first picture.  Returns 0 for failure,
// and the font index on success
{
    unsigned char *oldfonts;
    int           listsize;
    oldfonts = im->fontlist;
    if ( oldfonts )
    {
        listsize = strlen( (char *) oldfonts ) + 1 + strlen( fontname ) + 1;
    }
    else
    {
        listsize = strlen( fontname ) + 1;
    }
    im->fontlist = (unsigned char *) calloc( listsize, SIZEOF( unsigned char ) );
    if ( !im->fontlist )
        return 0;                    // memory allocation failed
    if ( oldfonts )
    {
        sprintf( (char *) im->fontlist, "%s%s%s", (char *) oldfonts, ",", fontname );
    }
    else
    {
        sprintf( (char *) im->fontlist, "%s", fontname );
    }
    im->numfonts++;
    if ( oldfonts )
        free( oldfonts );
    oldfonts = NULL;
    return im->numfonts;
}

int cdImageClearFonts( cdImagePtr im )
// clears out ALL fonts from the font list, including the ones the
// package has be default.  Useful if you want totally different fonts.
//
{
    free( im->fontlist );
    im->fontlist = NULL;
    im->numfonts = 0;
    return 1;
}

int cdImageSetDefaults( cdImagePtr im )
// resets the defaults to what is in defines.h
{
    // you must be either before any picture has been created,
    // or after a picture has closed to call this
    if ( ( im->state != 0 ) && ( im->state != 2 ) )
        return 0;
    // set line_width, line_height, line_color to the defaults
    im->ltype  = CDLTYPE;
    im->lwidth = CDLWIDTH;
    im->lcolor = CDLCOLOR;
    // interior_style, fill_color, hatch_index
    im->shapestyle = CDSHAPESTYLE;
    im->shapecolor = CDSHAPECOLOR;
    im->shapehatch = CDSHAPEHATCH;
    // edge_type, edge_width, edge_color, edge_visibility
    im->edgetype  = CDEDGETYPE;
    im->edgecolor = CDEDGECOLOR;
    im->edgewidth = CDEDGEWIDTH;
    im->edgevis   = CDEDGEVIS;
    // text_color, text_height, text_font
    im->textcolor  = CDTEXTCOLOR;
    im->textheight = CDTEXTHEIGHT;
    im->textfont   = CDTEXTFONT;
    im->textpath   = CDTEXTPATH;
    // set marker_width, marker_size, marker_color to the defaults
    im->ltype  = CDMTYPE;
    im->lwidth = CDMSIZE;
    im->lcolor = CDMCOLOR;
    // this is set by the expert functions.  the defaults should be ok
    im->linespec   = CDLINESPEC;
    im->edgespec   = CDEDGESPEC;
    im->markerspec = CDMARKERSPEC;

    return 1;
}

cdImagePtr cdImageStartCgm()
// initializes the CGM and sets up the defaults.  If you are using
// the "expert" functions, you should call this first. _ge
{
    const char *tmps;
    int        tmpsl;
    cdImagePtr im;
    im = (cdImage *) calloc( SIZEOF( cdImage ), 1 );
    if ( !im )
        return 0;          // memory allocation failed
    // elemlist is set to some number, when it is full, make it bigger
    im->elemlist = (unsigned char *) calloc( CDSTARTLISTSIZE, SIZEOF( unsigned char ) );
    if ( !im->elemlist )
    {
        free( im ); return 0;
    }                                           // memory allocation failed
    im->colorsTotal = 0;
    // you can have multiple pictures in a file,  keep track of
    // which one you are on
    im->picnum  = 0;
    im->outfile = NULL;
    // the next three are used for maintaining the element list
    // don't change these ever
    im->bytestoend  = CDSTARTLISTSIZE;
    im->listlen     = CDSTARTLISTSIZE;
    im->curelemlist = im->elemlist;

    // don't make this longer than 250 characters
    tmps  = "'ProfileId: Model-Profile''ProfileEd:1''ColourClass:colour''Source:NIST CGMDraw 1.3''Date: 1996-12-16'";
    tmpsl = strlen( tmps );
    if ( tmpsl > 250 )
        tmpsl = 250;
    im->desc = (unsigned char *) calloc( tmpsl + 1, SIZEOF( unsigned char ) );
    strncpy( (char *) im->desc, tmps, tmpsl );
    // The font list can be quite long, but individual font names can
    // can only be 250 chars
    tmps         = "TIMES_ROMAN,TIMES_BOLD,TIMES_ITALIC,TIMES_BOLD_ITALIC,HELVETICA,HELVETICA_BOLD,HELVETICA_ITALIC,HELVETICA_BOLD_ITALIC,COURIER,COURIER_BOLD,COURIER_ITALIC,COURIER_BOLD_ITALIC";
    im->numfonts = 12;
    tmpsl        = strlen( tmps );
    im->fontlist = (unsigned char *) calloc( tmpsl + 1, SIZEOF( unsigned char ) );
    strcpy( (char *) im->fontlist, tmps );
    im->outfile = NULL;

    if ( !cdImageSetDefaults( im ) )
    {
        cdImageDestroy( im );
    }
    // set the state
    im->state = 0;     // 0 no pictures started, 1 in a picture,
                       // 2 after a picture

    return im;
}

int cdImageEndPic( cdImagePtr im )
// close the current picture
{
    unsigned char *es, *esp;
    int           octet_count = 0;

    // make sure we are really in a picture before ending it
    if ( im->state != 1 )
    {
        return 0;
    }

    esp = (unsigned char *) calloc( 1024, SIZEOF( unsigned char ) );
    if ( !esp )
        return 0;           // memory allocation failed
    es = esp;

    // Attribute: End Picture; Elem Class 0; Elem ID 5; Length 0
    if ( !cdcomhead( es, 0, 5, 0 ) )
    {
        free( esp ); return 0;
    }
    octet_count += 2;

    if ( cdAddElem( im, esp, octet_count ) )
    {
        free( esp );
        im->state = 2;
        return 1;
    }
    else
    {
        free( esp );
        return 0;
    }
}

int cdImageEndCgm( cdImagePtr im )
// close the current CGM file.  If an output stream is
// defined, write the CGM to it
{
    int           x;                   // counter
    int           used;                // number of bytes used in the list
    unsigned char *efile, *efilep;     // end of file information

    cdImageEndPic( im );
    if ( im->state == 2 )     // We have closed the pic, but not the CGM
    {
        efile = (unsigned char *) calloc( 4 * 4, SIZEOF( unsigned char ) );
        if ( !efile )
            return 0;                 // memory allocation failed
        efilep = efile;
        // Attribute: End Metafile; Elem Class 0; Elem ID 2
        cdcomhead( efilep, 0, 2, 0 );


        if ( cdAddElem( im, efile, 2 ) )
        {
            free( efile );
            efile  = 0;
            efilep = 0;
        }
        else
        {
            free( efile );
            return 0;
        }
    }

    if ( im->outfile )
    {
        // now output the CGM, one byte at a time
        used = im->listlen - im->bytestoend;
        for ( x = 0; x < used; x++ )
        {
            putc( (unsigned char) im->elemlist[x], im->outfile );
        }
    }     // else do nothing

    return 1;
}
