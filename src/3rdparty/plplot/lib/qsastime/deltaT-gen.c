//  Copyright (C) 2009-2014 Alan W. Irwin
//
//  This file is part of PLplot.
//  PLplot is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  PLplot is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public License
//  along with PLplot; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//

// Program for generating spline representation (xspline, yspline,
// and y2spline arrays) header from deltaT.dat.
//
// The program assumes that argv[1] will be the input file, and
// argv[2] the output file.  This works cross-platform without
// worrying about shell redirects of stdin and stdout that are
// not accessible on Windows, apparently.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dspline.h"


//--------------------------------------------------------------------------
//   Function-like macro definitions
//--------------------------------------------------------------------------

#define MemError1( a )    do { fprintf( stderr, "MEMORY ERROR %d\n" a "\n", __LINE__ ); exit( __LINE__ ); } while ( 0 )

const char header[] = ""                                                                                 \
                      "/*\n"                                                                             \
                      "  This file is part of PLplot.\n"                                                 \
                      "  \n"                                                                             \
                      "  PLplot is free software; you can redistribute it and/or modify\n"               \
                      "  it under the terms of the GNU Library General Public License as published\n"    \
                      "  by the Free Software Foundation; either version 2 of the License, or\n"         \
                      "  (at your option) any later version.\n"                                          \
                      "  \n"                                                                             \
                      "  PLplot is distributed in the hope that it will be useful,\n"                    \
                      "  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"               \
                      "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"                \
                      "  GNU Library General Public License for more details.\n"                         \
                      "  \n"                                                                             \
                      "  You should have received a copy of the GNU Library General Public License\n"    \
                      "  along with PLplot; if not, write to the Free Software\n"                        \
                      "  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA\n" \
                      "  \n"                                                                             \
                      "  \n"                                                                             \
                      "  This header file contains spline data (xspline, yspline, and y2spline)\n"       \
                      "  for converting between UT1 and ephemeris time.\n"                               \
                      "  It is an automatically generated file, so please do\n"                          \
                      "  not edit it directly. Make any changes to deltaT.dat then use\n"                \
                      "  deltaT-gen to recreate this header file.\n"                                     \
                      "  \n"                                                                             \
                      "*/";

int main( int argc, char *argv[] )
{
    FILE   *fr, *fw;
    char   readbuffer[256];
    double *xspline        = NULL;
    double *yspline        = NULL;
    double *y2spline       = NULL;
    int    i               = 0;
    int    number_of_lines = 0;

    if ( ( argc < 2 ) || ( fr = fopen( argv[1], "r" ) ) == NULL )
    {
        fprintf( stderr, "Cannot open first file as readable\n" );
        exit( 1 );
    }

    if ( ( argc < 3 ) || ( fw = fopen( argv[2], "w" ) ) == NULL )
    {
        fprintf( stderr, "Cannot open second file as writable\n" );
        exit( 1 );
    }

    //
    //   Work out how many lines we have all up
    //

    while ( ( fgets( readbuffer, 255, fr ) != NULL ) )
    {
        ++number_of_lines;
    }

    //
    //   Allocate memory to the arrays which will hold the data
    //

    if ( ( xspline = (double *) calloc( (size_t) number_of_lines, (size_t) sizeof ( double ) ) ) == NULL )
        MemError1( "Allocating memory to the xspline table" );

    if ( ( yspline = (double *) calloc( (size_t) number_of_lines, (size_t) sizeof ( double ) ) ) == NULL )
        MemError1( "Allocating memory to the yspline table" );

    if ( ( y2spline = (double *) calloc( (size_t) number_of_lines, (size_t) sizeof ( double ) ) ) == NULL )
        MemError1( "Allocating memory to the y2spline table" );

    rewind( fr ); // Go back to the start of the file

    //
    //    Read in line by line, and copy the numbers into our arrays
    //

    while ( ( fgets( readbuffer, 255, fr ) != NULL ) )
    {
        sscanf( readbuffer, "%lf %lf", (double *) &xspline[i], (double *) &yspline[i] );
        i++;
    }

    fclose( fr );
    // Calculate spline representation using second derivative condition
    // on end points that is consistent with overall average parabolic shape
    // of delta T curve (Morrison and Stephenson, 2004) with second
    // derivative = 6.4e-3 secs/year/year.
    dspline( xspline, yspline, number_of_lines, 2, 6.4e-3, 2, 6.4e-3, y2spline );

//
//   Write the data out to file ready to be included in our source
//


    fprintf( fw, "%s\n", header );

    fprintf( fw, "const int number_of_entries_in_spline_tables=%d;\n\n", number_of_lines );

    fprintf( fw, "const double xspline[%d] = {\n", number_of_lines );
    for ( i = 0; i < number_of_lines; i++ )
    {
        fprintf( fw, "%10.0f,\n", xspline[i] );
    }
    fprintf( fw, "};\n" );

    fprintf( fw, "const double yspline[%d] = {\n", number_of_lines );
    for ( i = 0; i < number_of_lines; i++ )
    {
        fprintf( fw, "%10.0f,\n", yspline[i] );
    }
    fprintf( fw, "};\n" );

    fprintf( fw, "const double y2spline[%d] = {\n", number_of_lines );
    for ( i = 0; i < number_of_lines; i++ )
    {
        fprintf( fw, "%25.15e,\n", y2spline[i] );
    }
    fprintf( fw, "};\n" );

    fclose( fw );
    free( xspline );
    free( yspline );
    free( y2spline );

    return ( 0 );
}

