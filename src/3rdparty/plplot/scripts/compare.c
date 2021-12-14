//      Simple, dedicated comparison program for comparing the output from examples
//      in different languages.
//
//      Usage:
//      compare.exe name-of-postscript-file
//
//      From the first argument the name of the corresponding PS file for C will be
//      constructed. (It is a limitation of the Windows batch files that you cannot
//      take a substring)
//
//      The first 211 characters are skipped, as these contain a timestamp.
//
//      Note 1:
//      To keep it simple, the first three characters of the file name are used
//      to construct the name of the C reference file, so the files are expected
//      to have the pattern: xnnf.ps or the like where nn is the number of the
//      example.
//
//      Note 2:
//      Because it turns out not to be possible to use the diff program that comes
//      with MinGW-w64/MSYS2 outside the dedicated shells, I wrote this C program.
//      I tried to run "diff" by expanding the PATH and then run the program, but
//      the console froze. Not even control-C worked. I tried a few other commands,
//      same results. Apparently the shell takes care of some initialisation that
//      cannot easily be done in any other way.
//
// Copyright (C) 2016 Arjen Markus
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//

#include <stdio.h>
#include <stdlib.h>

#define SKIPHEADER 211L

int main( int argc, char *argv[] ) {
    FILE *infile1, *infile2;
    char *contents1, *contents2;
    char refname[100];
    long size1, size2;

    // Check for the first argument
    if ( argc != 2 ) {
        printf( "Usage: compare.exe name-of-PS-file\n" );
        exit(1);
    }

    // Check if the file exists
    infile1 = fopen( argv[1], "r" );
    if ( infile1 == NULL ) {
        printf( "File does not exist - %s - no comparison\n", argv[1] );
        exit(1);
    }

    // Construct the name of the reference file and open it
    sprintf( refname, "..\\c\\%3.3sc.ps", argv[1] );
    infile2 = fopen( refname, "r" );
    if ( infile2 == NULL ) {
        printf( "Reference file does not exist - %s - no comparison\n", refname );
        exit(1);
    }

    // Determine the sizes of both files
    fseek( infile1, 0L, SEEK_END );
    fseek( infile2, 0L, SEEK_END );
    size1 = ftell( infile1 );
    size2 = ftell( infile2 );

    if ( size1 != size2 ) {
        printf( "Files differ in size:\nNew file:     %s -- %ld\nReference file: %s -- %ld\n",
            argv[1], size1, refname, size2 );
        exit(1);
    }

    // Apparently they have the same size, so compare the contents
    fseek( infile1, SKIPHEADER, SEEK_END );
    fseek( infile2, SKIPHEADER, SEEK_END );

    contents1 = (char *) malloc( size1 * sizeof(char) );
    contents2 = (char *) malloc( size2 * sizeof(char) );

    fread( contents1, sizeof(char), (size_t)(size1-SKIPHEADER), infile1 );
    fread( contents2, sizeof(char), (size_t)(size2-SKIPHEADER), infile2 );

    if ( strncmp( contents1, contents2, (size_t)(size1-SKIPHEADER) ) != 0 ) {
        printf( "Files differ in contents:\nNew file:       %s\nReference file: %s\n",
            argv[1], refname );
        exit(1);
    } else {
        printf( "Files are identical -- %s\n", argv[1] );
    }
}
