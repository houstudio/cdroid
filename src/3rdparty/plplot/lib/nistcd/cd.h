#ifndef CD_H
#define CD_H    1

// cd.h: declarations file for the cgmdraw module.
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
//

// #include <prof.h>

// stdio is needed for file I/O.
#include <stdio.h>
#include "cddll.h"
// This can not be changed to a value larger than 256, though smaller
// values can be used.
//

#define cdMaxColors    256

// If you know you will be working with large pictures, increase the values
// of the next two constants.
//

// The initial size of the element list.  When it fills up, we will just
// make it bigger.  Starting  with a larger number reduces the frequency of
// the list growing, but increases the memory needed for small pictures
//

#define CDSTARTLISTSIZE    4096

// How much the element list grows by.  When the list fills up, we allocate
// a new larger list.  This number is how much larger.  using a larger number
// decreases the frequency of the list growing, but if only a small amount
// more is needed, it could waste memory
//

#define CDGROWLISTSIZE    2048

// Image type. See functions below; you will not need to change
//      the elements directly. Use the provided macros to
//      access sx, sy, the color table, and colorsTotal for
//      read-only purposes.

#if defined ( __cplusplus ) || defined ( c_plusplus )
extern "C" {
#endif

typedef struct cdImageStruct
{
    // Don't mess with these
    unsigned char * elemlist;
    short int     state;
    int           red[cdMaxColors];
    int           green[cdMaxColors];
    int           blue[cdMaxColors];
    int           open[cdMaxColors];
    int           colorsTotal;
    // You can have multiple pictures in the file,  this keeps track
    // of which one you are on
    int           picnum;
    // these take effect only when the first picture is created.
    // subsequent changes have no effect
    unsigned char *desc;
    unsigned char *fontlist;
    short int     numfonts;
    FILE          *outfile;
    // these take effect when a new picture is opened.  Subsequent
    // changes are for the next picture
    int           linespec;
    int           edgespec;
    int           markerspec;
    int           sx;
    int           sy;
    // these take effect immediately
    // Linetype, line width, line color have a broader scope in CGM
    int           ltype;
    int           lwidth;
    int           lcolor;
    // interior style [of filled objects] (for me) can be empty, hollow,
    // solid, hatch [don't do pattern, geometric pattern, interpolated
    int           shapestyle;
    // fill color, color used on inside of closed objects, significant
    // if interior style is hollow, solid, hatch, or geometric pattern
    int           shapecolor;
    // hatch index, which hatch style to use, 1=horizontal, 2=vertical,
    // 3=pos.slope, 4=neg.slope, 5=hor/vert.crosshatch,
    // 6=pos/neg.crosshatch
    int           shapehatch;
    // The edges of filled shapes can have line styles too.  They
    // correspond to the ones for lines.  These next few set them.
    int           edgetype;
    int           edgewidth;
    int           edgecolor;
    int           edgevis; // is the edge visible or invisible
    // now for the TEXT related attributes,  Text Color, Text Height,
    // and Text font index
    int           textfont;
    int           textcolor;
    int           textheight;
    int           textpath;
    // Marker type, Marker size, marker color
    int           mtype;
    int           msize;
    int           mcolor;
    // the next three are used for maintaining the element list
    long int      bytestoend;        // number of bytes to end of the element list
    long int      listlen;           // the total length of the element list
    unsigned char * curelemlist;     // where we curently are in the list
} cdImage;

typedef cdImage * cdImagePtr;


// Point type for use in polygon drawing.

typedef struct cdPointStruct
{
    int x, y, e;
} cdPoint, *cdPointPtr;



// Functions to manipulate images.

CDDLLIMPEXP cdImagePtr cdImageCreate( int sx, int sy );
CDDLLIMPEXP int cdCgmNewPic( cdImagePtr im, int sticky );
CDDLLIMPEXP int cdImageCgm( cdImagePtr im, FILE * );
CDDLLIMPEXP int cdImageDestroy( cdImagePtr im );

// Use cdLine, not cdImageLine
CDDLLIMPEXP int cdLine( cdImagePtr im, int x1, int y1, int x2, int y2 );
// Corners specified (not width and height). Upper left first, lower right
//      second.
CDDLLIMPEXP int cdRectangle( cdImagePtr im, int x1, int y1, int x2, int y2 );
// center x, then center y, then radius of circle
CDDLLIMPEXP int cdCircle( cdImagePtr im, int cx, int cy, int r );
// start, middle and end of arc
CDDLLIMPEXP int cdArc3Pt( cdImagePtr im, int sx, int sy, int ix, int iy, int ex, int ey );
// cl is 0 for pie closure, 1 for cord closure
CDDLLIMPEXP int cdArc3PtClose( cdImagePtr im, int sx, int sy, int ix, int iy, int ex, int ey, int cl );
CDDLLIMPEXP int cdEllipse( cdImagePtr im, int cx, int cy, int d1x, int d1y, int d2x, int d2y );
CDDLLIMPEXP int cdMarker( cdImagePtr im, int x, int y );
// polyshapes
CDDLLIMPEXP int cdPolygon( cdImagePtr im, cdPointPtr p, int n );
CDDLLIMPEXP int cdPolygonSet( cdImagePtr im, cdPointPtr p, int n );
CDDLLIMPEXP int cdPolyLine( cdImagePtr im, cdPointPtr p, int n );
CDDLLIMPEXP int cdPolyMarker( cdImagePtr im, cdPointPtr p, int n );

// Functions for Compatibility with gd
CDDLLIMPEXP int cdImageLine( cdImagePtr im, int x1, int y1, int x2, int y2, int color );
CDDLLIMPEXP int cdImageRectangle( cdImagePtr im, int x1, int y1, int x2, int y2, int color );


CDDLLIMPEXP int cdImageBoundsSafe( cdImagePtr im, int x, int y );
// These put characters in the picture.  CGM can handle fonts
// (x,y) is the lower left corner of where the text goes
CDDLLIMPEXP int cdText( cdImagePtr im, int x, int y, const char * );


// Functions for allocating colors
CDDLLIMPEXP int cdImageColorAllocate( cdImagePtr im, int r, int g, int b );
CDDLLIMPEXP int cdImageColorClosest( cdImagePtr im, int r, int g, int b );
CDDLLIMPEXP int cdImageColorExact( cdImagePtr im, int r, int g, int b );
CDDLLIMPEXP int cdImageColorDeallocate( cdImagePtr /* im */, int /* color */ );
// wogl: the parameter names are commented to avoid compiler warnings
CDDLLIMPEXP int cdImageColor16( cdImagePtr im );

// gej: functions that set style attributes
CDDLLIMPEXP int cdSetLineAttrib( cdImagePtr im, int lntype, int lnwidth, int lncolor );
CDDLLIMPEXP int cdSetShapeFillAttrib( cdImagePtr im, int instyle, int incolor, int inhatch );
CDDLLIMPEXP int cdSetShapeEdgeAttrib( cdImagePtr im, int edtype, int edwidth, int edcolor, int edvis );
CDDLLIMPEXP int cdSetTextAttrib( cdImagePtr im, int font, int color, int height );
CDDLLIMPEXP int cdSetMarkerAttrib( cdImagePtr im, int mtype, int msize, int mcolor );

// gej: or if you prefer, set the attributes individually
CDDLLIMPEXP int cdSetLineType( cdImagePtr im, int lntype );
CDDLLIMPEXP int cdSetLineWidth( cdImagePtr im, int lnwidth );
CDDLLIMPEXP int cdSetLineColor( cdImagePtr im, int lncolor );
CDDLLIMPEXP int cdSetFillStyle( cdImagePtr im, int instyle );
CDDLLIMPEXP int cdSetFillColor( cdImagePtr im, int incolor );
CDDLLIMPEXP int cdSetFillHatch( cdImagePtr im, int inhatch );
CDDLLIMPEXP int cdSetEdgeType( cdImagePtr im, int edtype );
CDDLLIMPEXP int cdSetEdgeWidth( cdImagePtr im, int edwidth );
CDDLLIMPEXP int cdSetEdgeColor( cdImagePtr im, int edcolor );
CDDLLIMPEXP int cdSetEdgeVis( cdImagePtr im, int edvis );
CDDLLIMPEXP int cdSetTextFont( cdImagePtr im, int font );
CDDLLIMPEXP int cdSetTextColor( cdImagePtr im, int color );
CDDLLIMPEXP int cdSetTextHeight( cdImagePtr im, int height );
// geJ: these individual attributes can't be set with a group function
CDDLLIMPEXP int cdSetTextPath( cdImagePtr im, int tpath );
CDDLLIMPEXP int cdSetTextOrient( cdImagePtr im, int xup, int yup, int xbase, int ybase );
CDDLLIMPEXP int cdSetMarkerType( cdImagePtr im, int mtype );
CDDLLIMPEXP int cdSetMarkerSize( cdImagePtr im, int msize );
CDDLLIMPEXP int cdSetMarkerColor( cdImagePtr im, int mcolor );

// EJ: Expert Functions,  If you just need more control
CDDLLIMPEXP int cdImageSetSize( cdImagePtr im, int x, int y );
CDDLLIMPEXP int cdImageSetLineSpec( cdImagePtr im, int specmode );
CDDLLIMPEXP int cdImageSetMarkerSpec( cdImagePtr im, int specmode );
CDDLLIMPEXP int cdImageSetEdgeSpec( cdImagePtr im, int specmode );
CDDLLIMPEXP int cdImageSetOutput( cdImagePtr im, FILE *output );
CDDLLIMPEXP int cdImageAddFont( cdImagePtr im, const char *fontname );
CDDLLIMPEXP int cdImageClearFonts( cdImagePtr im );
CDDLLIMPEXP cdImagePtr cdImageStartCgm();
CDDLLIMPEXP int cdCgmHeader( cdImagePtr );
CDDLLIMPEXP int cdCgmPic( cdImagePtr, int );
CDDLLIMPEXP int cdImageSetDefaults( cdImagePtr im );
CDDLLIMPEXP int cdImageEndPic( cdImagePtr im );
CDDLLIMPEXP int cdImageEndCgm( cdImagePtr im );

// Macros to access information about images. READ ONLY. Changing
//      these values will NOT have the desired result.
#define cdImageSX( im )             ( ( im )->sx )
#define cdImageSY( im )             ( ( im )->sy )
#define cdImageColorsTotal( im )    ( ( im )->colorsTotal )
#define cdImageRed( im, c )         ( ( im )->red[( c )] )
#define cdImageGreen( im, c )       ( ( im )->green[( c )] )
#define cdImageBlue( im, c )        ( ( im )->blue[( c )] )

// Source: Independent JPEG Group
// In ANSI C, and indeed any rational implementation, size_t is also the
// type returned by sizeof().  However, it seems there are some irrational
// implementations out there, in which sizeof() returns an int even though
// size_t is defined as long or unsigned long.  To ensure consistent results
// we always use this SIZEOF() macro in place of using sizeof() directly.
//

#define SIZEOF( object )    ( (size_t) sizeof ( object ) )

// GeJ: these are helper functions I use in cd.  That means DON'T call
// them from your program.  Yes, that means you.
CDDLLIMPEXP int cdImageColorClear( cdImagePtr im );

#if defined ( __cplusplus ) || defined ( c_plusplus )
}
#endif


#endif
