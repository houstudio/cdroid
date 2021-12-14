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

// Program for generating data structure used for containing tai-utc
// conversion information (linear transforms and leap seconds).
//
// The program assumes that argv[1] will be the input file, and
// argv[2] the output file.  This works cross-platform without
// worrying about shell redirects of stdin and stdout that are
// not accessible on Windows, apparently.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


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
                      "  This header file contains the table containing the linear transforms \n"        \
                      "  for converting between TAI and UTC.\n"                                          \
                      "  It is an automatically generated file, so please do\n"                          \
                      "  not edit it directly. Make any changes to tai-utc.dat then use\n"               \
                      "  tai-utc-gen to recreate this header file.\n"                                    \
                      "  \n"                                                                             \
                      "  tai-utc.dat contains four essential fields to represent the following\n"        \
                      "  formula for the linear transformation between TAI and UTC: \n"                  \
                      "    TAI-UTC (seconds) = offset1 + (MJD-offset2)*slope\n"                          \
                      "  There are four essential fields per line in tai-utc.dat to represent\n"         \
                      "  this formula.  They are the Julian date (UTC) where the linear\n"               \
                      "  transformation implied by the line is first applied;\n"                         \
                      "  offset1 (seconds); offset2 (days), and slope (secs/day).\n"                     \
                      "  \n"                                                                             \
                      "*/";

int main( int argc, char *argv[] )
{
    FILE   *fr, *fw;
    char   readbuffer[256];
    int    *MJDstart = NULL;
    double *offset1  = NULL;
    int    *offset2  = NULL;
    double *slope    = NULL;
    double sec, *leap_sec = NULL;
    int    jd;
    int    i = 0;
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

    if ( ( MJDstart = (int *) calloc( (size_t) number_of_lines, (size_t) sizeof ( int ) ) ) == NULL )
        MemError1( "Allocating memory to the MJDstart table" );

    if ( ( offset1 = (double *) calloc( (size_t) number_of_lines, (size_t) sizeof ( double ) ) ) == NULL )
        MemError1( "Allocating memory to the offset1 table" );

    if ( ( offset2 = (int *) calloc( (size_t) number_of_lines, (size_t) sizeof ( int ) ) ) == NULL )
        MemError1( "Allocating memory to the offset2 table" );

    if ( ( slope = (double *) calloc( (size_t) number_of_lines, (size_t) sizeof ( double ) ) ) == NULL )
        MemError1( "Allocating memory to the slope table" );

    if ( ( leap_sec = (double *) calloc( (size_t) number_of_lines, (size_t) sizeof ( double ) ) ) == NULL )
        MemError1( "Allocating memory to the leap_sec table" );

    rewind( fr ); // Go back to the start of the file

    //
    //    Read in line by line, and copy the numbers into our arrays
    //

    while ( ( fgets( readbuffer, 255, fr ) != NULL ) )
    {
        sscanf( readbuffer, "%*s %*s %*s %*s %d.5 %*s %lf %*s %*s %*s %*s %d.) X %lf S", (int *) &jd, (double *) &offset1[i], (int *) &offset2[i], (double *) &slope[i] );
        // Should be exact since all jd's in the file are integer+0.5
        MJDstart[i] = jd - 2400000;
        i++;
    }

    fclose( fr );

//
//   Write the data out to file ready to be included in our source
//


    fprintf( fw, "%s\n", header );

    fprintf( fw, "typedef struct {\n\tint base_day;\n\tdouble time_sec_tai;\n\tdouble time_sec_utc;\n\tdouble size_prev_leap_sec;\n\tdouble offset1;\n\tint offset2;\n\tdouble slope;\n} TAI_UTC;\n\n" );

    fprintf( fw, "const int number_of_entries_in_tai_utc_table=%d;\n\n", number_of_lines );

    fprintf( fw, "const TAI_UTC TAI_UTC_lookup_table[%d] = {\n", number_of_lines );
    for ( i = 0; i < number_of_lines; i++ )
    {
        sec = offset1[i] + (double) ( MJDstart[i] - offset2[i] ) * slope[i];
        if ( i == 0 )
            leap_sec[i] = 0.;
        else
            // sec is TAI-UTC in seconds calculated from UTC transformation
            // (equation 1 in README.tai-utc).  This calculation must be correct
            // for start of epoch range.  However, near end of epoch range where
            // ambiguities in UTC occur, must use equivalent TAI transformation
            // (equation 2 from same source) to calculate the UTC discontinuity
            // unambiguously.
            leap_sec[i] = sec - ( offset1[i - 1] + (double) ( MJDstart[i] + sec / 86400. - offset2[i - 1] ) * slope[i - 1] ) / ( 1. + slope[i - 1] / 86400. );
        if ( fabs( leap_sec[i] ) < 1.e-14 )
            leap_sec[i] = 0.;
        fprintf( fw, "{%d, %15.8f, 0., %20.14f, %15.8f, %d, %15.8f},\n", MJDstart[i], sec, leap_sec[i], offset1[i], offset2[i], slope[i] );
    }
    fprintf( fw, "};\n" );

    fclose( fw );
    free( MJDstart );
    free( offset1 );
    free( offset2 );
    free( slope );
    free( leap_sec );

    return ( 0 );
}

