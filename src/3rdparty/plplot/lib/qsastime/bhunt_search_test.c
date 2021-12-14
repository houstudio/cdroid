//
// Copyright (C) 2009 Alan W. Irwin
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
#include "qsastimeP.h"

int gedouble( double *number1, double *number2 );

int count_gedouble;
int gedouble( double *number1, double *number2 )
{
    count_gedouble++;
    return ( *number1 >= *number2 );
}

int main()
{
    int    i, j, iswitch, ifrandom, ifhunt, ntable, offset, multiplier, ntest, index;
    double *table, test;

    // size of ordered table of data and offset and multiplier to determine
    // number and value of test values used to search the ordered table.
    scanf( "%i %i %i", &ntable, &offset, &multiplier );
    ntest = abs( multiplier ) * ( ntable - 1 ) + 1;
    printf( "ntable, offset, multiplier, ntest = %i, %i, %i, %i\n", ntable, offset, multiplier, ntest );

    table = (double *) malloc( ntable * sizeof ( double ) );
    if ( table == NULL )
    {
        printf( "Could not malloc desired memory\n" );
        return 1;
    }


    // Generate ordered table to be searched.
    for ( i = 0; i < ntable; i++ )
    {
        table[i] = (double) i;
    }

    for ( iswitch = 0; iswitch < 4; iswitch++ )
    {
        ifrandom = ( iswitch & 0x1 ) == 0x1;
        ifhunt   = ( iswitch & 0x2 ) == 0x2;
        // Generate series of test values (not necessarily ordered) to be used
        // as keys for searching the table array).
        index          = -40;
        count_gedouble = 0;
        for ( i = 0; i < ntest; i++ )
        {
            if ( ifrandom )
            {
                j = (int) ( (double) ntest * (double) rand() / ( ( (double) RAND_MAX ) + 1. ) );
            }
            else
            {
                j = i;
            }

            test = offset + (double) j / (double) multiplier;
            if ( !ifhunt )
                index = -40;
            bhunt_search( &test, table, ntable, sizeof ( double ), &index, ( int ( * )( const void *, const void * ) )gedouble );
            if ( index < -1 || index > ntable - 1 )
            {
                printf( "ERROR: test = %20.16f lead to an invalid index of %i\n", test, index );
                return 1;
            }

            if ( !( ( index == -1 && test < table[index + 1] ) || ( index > -1 && index < ntable - 1 && table[index] <= test && test < table[index + 1] ) || ( index == ntable - 1 && table[index] <= test ) ) )
            {
                if ( index == -1 )
                {
                    printf( "ERROR for index == -1,  test = %20.16f, table[index+1] = %20.16f\n", test, table[index + 1] );
                    return 1;
                }
                else if ( index > -1 && index < ntable - 1 )
                {
                    printf( "ERROR for index > -1 && index < ntable-1,  table[index] = %20.16f, test = %20.16f, table[index+1] = %20.16f\n", table[index], test, table[index + 1] );
                    return 1;
                }
                else if ( index == ntable - 1 )
                {
                    printf( "ERROR for index == ntable - 1, table[index] = %20.16f, test = %20.16f\n", table[index], test );
                    return 1;
                }
                else
                {
                    printf( "Internal logic ERROR\n" );
                    return 1;
                }
            }
        }
        printf( "Average number of gedouble calls per bhunt_search call = %f for ifhunt, ifrandom = %i,%i\n", (double) count_gedouble / (double) ntest, ifhunt, ifrandom );
    }
    printf( "Successful completion of bhunt_search test\n" );

    free( (void *) table );
    return 0;
}
