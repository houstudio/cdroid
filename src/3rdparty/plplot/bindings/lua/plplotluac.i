//
//Copyright (C) 2009  Werner Smekal
//This file is part of PLplot.
//
//PLplot is free software; you can redistribute it and/or modify
//it under the terms of the GNU Library General Public License as published by
//the Free Software Foundation; version 2 of the License.
//
//PLplot is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Library General Public License for more details.
//
//You should have received a copy of the GNU Library General Public License
//along with the file PLplot; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
//

//
//A SWIG interface to PLplot for LUA. This wrapper does the following:
//
//   1) it strictly provides the C-API with the usual change of not
//      requiring lengths for arrays,
//
//   2) it attempts to provide the entire API *excluding* callbacks for
//      plcont and plshade(s) (for now).
//
//   3) it works only with the double-precision version of the
//      PLplot library.
//
//This is known to work with swig-1.3.36.
//
//
%module plplotluac

%{
#include "plplotP.h"
%}


// macro definitions
%{
#define LUA_ALLOC_ARRAY( TYPE, LEN )    (TYPE *) malloc( LEN * sizeof ( TYPE ) )
#define LUA_FREE_ARRAY( PTR )           if ( PTR ) { free( PTR ); PTR = NULL;}

// super macro to declare array typemap helper fns
    SWIGINTERN int SWIG_itable_size( lua_State* L, int index );
#define LUA_DECLARE_TYPEMAP_ARR_FN( NAME, TYPE )                                                        \
    SWIGINTERN int LUA_read_ ## NAME ## _num_array( lua_State * L, int index, TYPE * array, int size ){ \
        int i;                                                                                          \
        for ( i = 0; i < size; i++ ) {                                                                  \
            lua_rawgeti( L, index, i + 1 );                                                             \
            if ( lua_isnumber( L, -1 ) ) {                                                              \
                array[i] = (TYPE) lua_tonumber( L, -1 );                                                \
            } else {                                                                                    \
                lua_pop( L, 1 );                                                                        \
                return 0;                                                                               \
            }                                                                                           \
            lua_pop( L, 1 );                                                                            \
        }                                                                                               \
        return 1;                                                                                       \
    }                                                                                                   \
    SWIGINTERN TYPE* LUA_get_ ## NAME ## _num_array_var( lua_State * L, int index, int* size )          \
    {                                                                                                   \
        TYPE *array;                                                                                    \
        if ( !lua_istable( L, index ) ) {                                                               \
            lua_pushstring( L, "expected a table" );                                                    \
            return 0;                                                                                   \
        }                                                                                               \
        *size = SWIG_itable_size( L, index );                                                           \
        if ( *size < 1 ) {                                                                              \
            array    = LUA_ALLOC_ARRAY( TYPE, 1 );                                                      \
            array[0] = (TYPE) 0;                                                                        \
            return array;                                                                               \
        }                                                                                               \
        array = LUA_ALLOC_ARRAY( TYPE, *size );                                                         \
        if ( !LUA_read_ ## NAME ## _num_array( L, index, array, *size ) ) {                             \
            lua_pushstring( L, "table must contain numbers" );                                          \
            LUA_FREE_ARRAY( array );                                                                    \
            return 0;                                                                                   \
        }                                                                                               \
        return array;                                                                                   \
    }

    LUA_DECLARE_TYPEMAP_ARR_FN( double, double );
    LUA_DECLARE_TYPEMAP_ARR_FN( int, int );
%}

// type definitions
typedef double         PLFLT;
typedef int            PLINT;
typedef unsigned int   PLUNICODE;
typedef PLINT          PLBOOL;

%include lua_namespace.i

// I hate global variables but this is the best way I can think of to manage consistency
//   checking among function arguments.
%{
    static PLINT Alen = 0;
    static PLINT Xlen = 0, Ylen = 0;
%}

// The following typemaps take care of marshaling values into and out of PLplot functions. The
//Array rules are trickly because of the need for length checking. These rules manage
//some global variables (above) to handle consistency checking amoung parameters.
//
//Naming rules:
//        Array           (sets Alen to dim[0])
//        ArrayCk         (tests that dim[0] == Alen)
//        ArrayX          (sets Xlen to dim[0]
//        ArrayCkX        (tests dim[0] == Xlen)
//        ArrayY          (sets Ylen to dim[1])
//        ArrayCkY        (tests dim[1] == Ylen)
//        Matrix          (sets Xlen to dim[0], Ylen to dim[1])
//        MatrixCk        (test Xlen == dim[0] && Ylen == dim[1])
//

// typemaps
%include <typemaps.i>

//--------------------------------------------------------------------------
//                         PLINT arrays
//--------------------------------------------------------------------------

// With preceding count
%typemap ( in ) ( PLINT n, const PLINT * Array )
{
    $2 = (PLINT *) LUA_get_int_num_array_var( L, $input, &$1 );
    if ( !$2 )
        SWIG_fail;
    Alen = $1;
}
%typemap ( freearg ) ( PLINT n, const PLINT * Array )
{
    LUA_FREE_ARRAY( $2 );
}


// Trailing count and check consistency with previous
%typemap ( in ) ( const PLINT * ArrayCk, PLINT n ) ( int temp )
{
    $1 = (PLINT *) LUA_get_int_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    if ( temp != Alen )
    {
        lua_pushfstring( L, "Tables must be of same length." );
        SWIG_fail;
    }
    $2 = temp;
}
%typemap ( freearg ) ( const PLINT * ArrayCk, PLINT n )
{
    LUA_FREE_ARRAY( $1 );
}


// No count but check consistency with previous
%typemap( in ) const PLINT * ArrayCk( int temp )
{
    $1 = (PLINT *) LUA_get_int_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    if ( temp != Alen )
    {
        lua_pushfstring( L, "Tables must be of same length." );
        SWIG_fail;
    }
}
%typemap( freearg ) const PLINT * ArrayCk { LUA_FREE_ARRAY( $1 ); }

// No count but check consistency with previous, or NULL
%typemap( in ) const PLINT * ArrayCkNull( int temp )
{
    if ( lua_isnil( L, $input ) )
    {
        $1 = NULL;
    }
    else
    {
        $1 = (PLINT *) LUA_get_int_num_array_var( L, $input, &temp );
        if ( !$1 )
            SWIG_fail;
        if ( temp != Alen )
        {
            lua_pushfstring( L, "Tables must be of same length." );
            SWIG_fail;
        }
    }
}
%typemap( freearg ) const PLINT * ArrayCkNull { LUA_FREE_ARRAY( $1 ); }
%typemap( default ) const PLINT * ArrayCkNull { $1 = NULL; }

// Weird case to allow argument to be one shorter than others
%typemap( in ) const PLINT * ArrayCkMinus1( int temp )
{
    $1 = (PLINT *) LUA_get_int_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    if ( temp < Alen - 1 )
    {
        lua_pushfstring( L, "Tables must be at least length of others minus 1." );
        SWIG_fail;
    }
}
%typemap( freearg ) const PLINT * ArrayCkMinus1 { LUA_FREE_ARRAY( $1 ); }

%typemap( in ) const PLINT * ArrayCkMinus1Null( int temp )
{
    if ( lua_isnil( L, $input ) )
    {
        $1 = NULL;
    }
    else
    {
        $1 = (PLINT *) LUA_get_int_num_array_var( L, $input, &temp );
        if ( !$1 )
            SWIG_fail;
        if ( temp < Alen - 1 )
        {
            lua_pushfstring( L, "Tables must be at least length of others minus 1." );
            SWIG_fail;
        }
    }
}
%typemap( freearg ) const PLINT * ArrayCkMinus1Null { LUA_FREE_ARRAY( $1 ); }
%typemap( default ) const PLINT * ArrayCkMinus1Null { $1 = NULL; }


// No length but remember size to check others
%typemap( in ) const PLINT * Array( int temp )
{
    $1 = (PLINT *) LUA_get_int_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    Alen = temp;
}
%typemap ( freearg ) ( const PLINT * Array )
{
    LUA_FREE_ARRAY( $1 );
}

// No length but remember size to check others
%typemap( in ) const PLINT * ArrayN( int temp )
{
    int i;

    $1 = (PLINT *) LUA_get_int_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    if ( temp != Alen )
    {
        lua_pushfstring( L, "Tables must be of same length." );
        SWIG_fail;
    }

    Xlen = temp;
    Ylen = -1;
    for ( i = 0; i < Xlen; i++ )
        if ( $1[i] > Ylen )
            Ylen = $1[i];
}
%typemap ( freearg ) ( const PLINT * ArrayN )
{
    LUA_FREE_ARRAY( $1 );
}

// With trailing count and NULL array option.
%typemap ( in ) ( const PLINT * ArrayNull, PLINT n ) ( int temp )
{
    if ( lua_isnil( L, $input ) )
    {
        $1 = NULL;
        $2 = 0;
    }
    else
    {
        $1 = (PLINT *) LUA_get_int_num_array_var( L, $input, &temp );
        if ( !$1 )
            SWIG_fail;
        $2 = temp;
    }
}
%typemap ( freearg ) ( const PLINT * ArrayNull, PLINT n )
{
    LUA_FREE_ARRAY( $1 );
}

//--------------------------------------------------------------------------
//                                 PLFLT Arrays
//--------------------------------------------------------------------------

// with preceding count
%typemap ( in ) ( PLINT n, const PLFLT * Array )
{
    int temp;
    $2 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$2 )
        SWIG_fail;
    $1 = Alen = temp;
}
%typemap ( freearg ) ( PLINT n, const PLFLT * Array )
{
    LUA_FREE_ARRAY( $2 );
}


// Trailing count and check consistency with previous
%typemap ( in ) ( const PLFLT * ArrayCk, PLINT n )
{
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    if ( temp != Alen )
    {
        lua_pushfstring( L, "Tables must be of same length." );
        SWIG_fail;
    }
    $2 = temp;
}
%typemap ( freearg ) ( const PLFLT * ArrayCk, PLINT n )
{
    LUA_FREE_ARRAY( $1 );
}


// Trailing count and check consistency with previous
%typemap ( in ) ( const PLFLT * ArrayCkNull, PLINT n )
{
    int temp = 0;
    if ( lua_isnil( L, $input ) )
    {
        $1 = NULL;
    }
    else
    {
        $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
        if ( !$1 )
            SWIG_fail;
        if ( temp != Alen )
        {
            lua_pushfstring( L, "Tables must be of same length." );
            SWIG_fail;
        }
    }
    $2 = temp;
}
%typemap ( freearg ) ( const PLFLT * ArrayCkNull, PLINT n )
{
    LUA_FREE_ARRAY( $1 );
}
%typemap ( default ) ( const PLFLT * ArrayCkNull, PLINT n )
{
    $1 = NULL; $2 = 0;
}


// no count, but check consistency with previous
%typemap( in ) const PLFLT * ArrayCk( int temp )
{
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    if ( temp != Alen )
    {
        lua_pushfstring( L, "Tables must be of same length." );
        SWIG_fail;
    }
}
%typemap( freearg ) const PLFLT * ArrayCk { LUA_FREE_ARRAY( $1 ); }


// no count, but check consistency with previous, or NULL
%typemap( in ) const PLFLT * ArrayCkNull( int temp )
{
    if ( lua_isnil( L, $input ) )
    {
        $1 = NULL;
    }
    else
    {
        $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
        if ( !$1 )
            SWIG_fail;
        if ( temp != Alen )
        {
            lua_pushfstring( L, "Tables must be of same length." );
            SWIG_fail;
        }
    }
}
%typemap( freearg ) const PLFLT * ArrayCkNull { LUA_FREE_ARRAY( $1 ); }
%typemap( default ) const PLFLT * ArrayCkNull { $1 = NULL; }


// No length but remember size to check others
%typemap( in ) const PLFLT * Array {
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    Alen = temp;
}
%typemap ( freearg ) ( const PLFLT * Array )
{
    LUA_FREE_ARRAY( $1 );
}

// No length but remember size to check others
%typemap( in ) const PLFLT * ArrayNull {
    int temp;
    if ( lua_isnil( L, $input ) )
    {
        $1   = NULL;
        Alen = 0;
    }
    else
    {
        $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
        if ( !$1 )
            SWIG_fail;
        Alen = temp;
    }
}
%typemap ( freearg ) ( const PLFLT * Array )
{
    LUA_FREE_ARRAY( $1 );
}

// with trailing count
%typemap ( in ) ( const PLFLT * Array, PLINT n )
{
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    $2 = Alen = temp;
}
%typemap ( freearg ) ( const PLFLT * Array, PLINT n )
{
    LUA_FREE_ARRAY( $1 );
}


// check consistency with X dimension of previous
%typemap( in ) const PLFLT * ArrayCkX {
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    if ( temp != Xlen )
    {
        lua_pushfstring( L, "Tables must be of same length." );
        SWIG_fail;
    }
}
%typemap( freearg ) const PLFLT * ArrayCkX { LUA_FREE_ARRAY( $1 ); }


// check consistency with Y dimension of previous
%typemap( in ) const PLFLT * ArrayCkY {
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    if ( temp != Ylen )
    {
        lua_pushfstring( L, "Tables must be of same length." );
        SWIG_fail;
    }
}
%typemap( freearg ) const PLFLT * ArrayCkY { LUA_FREE_ARRAY( $1 ); }


// set X length for later consistency checking, with trailing count
%typemap ( in ) ( const PLFLT * ArrayX, PLINT nx )
{
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    $2 = Xlen = temp;
}
%typemap ( freearg ) ( const PLFLT * ArrayX, PLINT nx )
{
    LUA_FREE_ARRAY( $1 );
}


// set X length for later consistency checking
%typemap( in ) const PLFLT * ArrayX {
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    Xlen = temp;
}
%typemap( freearg ) const PLFLT * ArrayX { LUA_FREE_ARRAY( $1 ); }


// Set Y length for later consistency checking, with trailing count
%typemap ( in ) ( const PLFLT * ArrayY, PLINT ny )
{
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    $2 = Ylen = temp;
}
%typemap ( freearg ) ( const PLFLT * ArrayY, PLINT ny )
{
    LUA_FREE_ARRAY( $1 );
}


// set Y length for later consistency checking
%typemap( in ) const PLFLT * ArrayY {
    int temp;
    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    Ylen = temp;
}
%typemap ( freearg ) ( const PLFLT * ArrayY )
{
    LUA_FREE_ARRAY( $1 );
}


%{
    PLFLT** read_double_Matrix( lua_State* L, int index, int* nx, int *ny );

    PLFLT** read_double_Matrix( lua_State* L, int index, int* nx, int *ny )
    {
        int  i, j;
        PLFLT** matrix;

        *nx = 0;
        *ny = 0;

        if ( !lua_istable( L, index ) )
        {
            lua_pushstring( L, "expected a table" );
            return NULL;
        }
        *nx = SWIG_itable_size( L, index );
        if ( *nx < 1 )
        {
            lua_pushstring( L, "table appears to be empty" );
            return NULL;
        }
        matrix = LUA_ALLOC_ARRAY( PLFLT *, *nx );
        for ( i = 0; i < *nx; i++ )
            matrix[i] = NULL;

        lua_rawgeti( L, index, 1 );
        if ( !lua_istable( L, -1 ) )
        {
            lua_pop( L, 1 );
            lua_pushstring( L, "expected a table" );
            LUA_FREE_ARRAY( matrix );
            return NULL;
        }
        *ny = SWIG_itable_size( L, -1 );
        if ( *ny < 1 )
        {
            lua_pushstring( L, "table appears to be empty" );
            LUA_FREE_ARRAY( matrix );
            return NULL;
        }
        lua_pop( L, 1 );

        for ( i = 0; i < *nx; i++ )
        {
            lua_rawgeti( L, index, i + 1 );
            if ( !lua_istable( L, -1 ) )
            {
                lua_pop( L, 1 );
                lua_pushstring( L, "expected a table" );
                for ( j = 0; j < *ny; j++ )
                    LUA_FREE_ARRAY( matrix[j] );
                LUA_FREE_ARRAY( matrix );
                return NULL;
            }
            if ( *ny != SWIG_itable_size( L, -1 ) )
            {
                lua_pop( L, 1 );
                lua_pushstring( L, "inconsistent table sizes" );
                for ( j = 0; j < i; j++ )
                    LUA_FREE_ARRAY( matrix[j] );
                LUA_FREE_ARRAY( matrix );
                return NULL;
            }
            matrix[i] = LUA_ALLOC_ARRAY( PLFLT, *ny );
            for ( j = 0; j < *ny; j++ )
            {
                lua_rawgeti( L, -1, j + 1 );
                if ( lua_isnumber( L, -1 ) )
                {
                    matrix[i][j] = (PLFLT) lua_tonumber( L, -1 );
                }
                else
                {
                    lua_pop( L, 1 );
                    lua_pushstring( L, "table must contain numbers" );
                    for ( j = 0; j < i + 1; j++ )
                        LUA_FREE_ARRAY( matrix[j] );
                    LUA_FREE_ARRAY( matrix );
                    return NULL;
                }
                lua_pop( L, 1 );
            }
            lua_pop( L, 1 );
        }

        return matrix;
    }
%}


// 2D array with trailing dimensions, check consistency with previous
%typemap ( in ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny ) ( int ii )
{
    int jj;

    $1 = read_double_Matrix( L, $input, &ii, &jj );
    if ( !$1 )
        SWIG_fail;
    $2 = ii;
    $3 = jj;
    if ( ( $2 != Xlen ) || ( $3 != Ylen ) )
    {
        lua_pushfstring( L, "Vectors must match matrix." );
        SWIG_fail;
    }
}
%typemap ( freearg ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny )
{
    int i;

    if ( $1 )
    {
        for ( i = 0; i < ii$argnum; i++ )
            LUA_FREE_ARRAY( $1[i] );
        LUA_FREE_ARRAY( $1 );
    }
}


// 2D array with trailing dimensions, set the X, Y size for later checking
%typemap ( in ) ( const PLFLT **Matrix, PLINT nx, PLINT ny ) ( int ii )
{
    int jj;

    $1 = read_double_Matrix( L, $input, &ii, &jj );
    if ( !$1 )
        SWIG_fail;
    Xlen = $2 = ii;
    Ylen = $3 = jj;
}
%typemap ( freearg ) ( const PLFLT **Matrix, PLINT nx, PLINT ny )
{
    int i;

    if ( $1 )
    {
        for ( i = 0; i < ii$argnum; i++ )
            LUA_FREE_ARRAY( $1[i] );
        LUA_FREE_ARRAY( $1 );
    }
}


// 2D array with no dimensions, set the X, Y size for later checking
%typemap( in ) const PLFLT * *Matrix( int ii )
{
    int jj;

    $1 = read_double_Matrix( L, $input, &ii, &jj );
    if ( !$1 )
        SWIG_fail;
    Xlen = ii;
    Ylen = jj;
}
%typemap( freearg ) const PLFLT * *Matrix {
    int i;

    if ( $1 )
    {
        for ( i = 0; i < ii$argnum; i++ )
            LUA_FREE_ARRAY( $1[i] );
        LUA_FREE_ARRAY( $1 );
    }
}


// 2D array, check for consistency
%typemap( in ) const PLFLT * *MatrixCk( int ii )
{
    int jj;

    $1 = read_double_Matrix( L, $input, &ii, &jj );
    if ( !$1 )
        SWIG_fail;
    if ( ( ii != Xlen ) || ( jj != Ylen ) )
    {
        lua_pushfstring( L, "Vectors must match matrix." );
        SWIG_fail;
    }
}
%typemap( freearg ) const PLFLT * *MatrixCk {
    int i;

    if ( $1 )
    {
        for ( i = 0; i < ii$argnum; i++ )
            LUA_FREE_ARRAY( $1[i] );
        LUA_FREE_ARRAY( $1 );
    }
}


// Set Y length for later consistency checking, with trailing count
// and 2D array, check for consistency input / output version
%typemap ( in ) ( const PLFLT * ArrayY, PLINT ny, PLFLT **OutMatrixCk )
{
    int temp, i;

    $1 = (PLFLT *) LUA_get_double_num_array_var( L, $input, &temp );
    if ( !$1 )
        SWIG_fail;
    $2 = Ylen = temp;

    $3 = LUA_ALLOC_ARRAY( PLFLT *, Xlen );
    if ( !$3 )
        SWIG_fail;
    for ( i = 0; i < Xlen; i++ )
        $3[i] = NULL;

    for ( i = 0; i < Xlen; i++ )
    {
        $3[i] = LUA_ALLOC_ARRAY( PLFLT, Ylen );
        if ( !$3[i] )
            SWIG_fail;
    }
}
%typemap ( argout ) ( const PLFLT * ArrayY, PLINT ny, PLFLT **OutMatrixCk )
{
    int i;

    if ( $3 )
    {
        lua_newtable( L );
        for ( i = 0; i < Xlen; i++ )
        {
            SWIG_write_double_num_array( L, $3[i], Ylen );
            lua_rawseti( L, -2, i + 1 ); // -1 is the inner table, -2 is the outer table
        }
        SWIG_arg++;
    }
}
%typemap ( freearg ) ( const PLFLT * ArrayY, PLINT ny, PLFLT **OutMatrixCk )
{
    int i;

    LUA_FREE_ARRAY( $1 );

    if ( $3 )
    {
        for ( i = 0; i < Xlen; i++ )
            LUA_FREE_ARRAY( $3[i] );
        LUA_FREE_ARRAY( $3 );
    }
}

//--------------------------------------------------------------------------
//                                 String returning functions
//--------------------------------------------------------------------------

// This currently just used for plgdev, plgfnam, and plgver which apparently
// have a limit of 80 bytes.  But to (hopefully) be safe for any future use
// have a 1000 byte limit here.
%typemap( in, numinputs = 0 ) char *OUTPUT( char buff[1000] )
{
    $1 = buff;
}
%typemap( argout ) char *OUTPUT {
    lua_pushstring( L, $1 );
    SWIG_arg++;
}

%typemap( in, checkfn = "lua_isstring" ) const char *message {
    $1 = (char *) lua_tostring( L, $input );
}


//--------------------------------------------------------------------------
//                                 Function calls
//--------------------------------------------------------------------------

%{
    void mapform( PLINT n, PLFLT* x, PLFLT* y );

    static lua_State* myL = NULL;
    static char     mapform_funcstr[255];

    void mapform( PLINT n, PLFLT* x, PLFLT* y )
    {
        PLFLT *xtemp, *ytemp;
        int   len, i;

        // check Lua state
        if ( myL == NULL )
        {
            fprintf( stderr, "Lua state is not set!" );
            return;
        }

        // push functions and arguments
        lua_getglobal( myL, mapform_funcstr );    // function to be called
        lua_pushnumber( myL, n );                 // push 1st argument
        SWIG_write_double_num_array( myL, x, n ); // push 2nd argument
        SWIG_write_double_num_array( myL, y, n ); // push 3rd argument

        // do the call (3 arguments, 2 result)
        if ( lua_pcall( myL, 3, 2, 0 ) != 0 )
            fprintf( stderr, "error running function `%s':%s",
                mapform_funcstr, lua_tostring( myL, -1 ) );

        // retrieve results
        if ( !lua_istable( myL, -2 ) )
        {
            fprintf( stderr, "function `%s' must return a table as 1st result", mapform_funcstr );
            return;
        }
        if ( !lua_istable( myL, -1 ) )
        {
            fprintf( stderr, "function `%s' must return a table as 2nd result", mapform_funcstr );
            return;
        }
        xtemp = (PLFLT *) LUA_get_double_num_array_var( myL, -2, &len );
        if ( !xtemp || len != n )
        {
            fprintf( stderr, "function `%s' must return a table of length%d", mapform_funcstr, n );
            return;
        }
        for ( i = 0; i < n; i++ )
            x[i] = xtemp[i];
        LUA_FREE_ARRAY( xtemp );

        ytemp = (PLFLT *) LUA_get_double_num_array_var( myL, -1, &len );
        if ( !ytemp || len != n )
        {
            fprintf( stderr, "function `%s' must return a table of length%d", mapform_funcstr, n );
            return;
        }
        for ( i = 0; i < n; i++ )
            y[i] = ytemp[i];
        LUA_FREE_ARRAY( ytemp );

        lua_pop( myL, 2 ); // pop returned values

        return;
    }
%}

typedef PLINT ( *defined_func )( PLFLT, PLFLT );
typedef void ( *fill_func )( PLINT, const PLFLT*, const PLFLT* );
typedef void ( *pltr_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
typedef void ( *ct_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
typedef void ( *mapform_func )( PLINT, PLFLT*, PLFLT* );
typedef PLFLT ( *f2eval_func )( PLINT, PLINT, PLPointer );
typedef void ( *label_func )( PLINT, PLFLT, char*, PLINT, PLPointer );

%{
    typedef PLINT ( *defined_func )( PLFLT, PLFLT );
    typedef void ( *fill_func )( PLINT, const PLFLT*, const PLFLT* );
    typedef void ( *pltr_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
    typedef void ( *ct_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
    typedef void ( *mapform_func )( PLINT, PLFLT *, PLFLT* );
    typedef PLFLT ( *f2eval_func )( PLINT, PLINT, PLPointer );
    typedef void ( *label_func )( PLINT, PLFLT, char*, PLINT, PLPointer );

// Function prototypes
    void mypltr( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void * pltr_data );
    void myct( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void * pltr_data );
    void mylabel( PLINT axis, PLFLT value, char* label, PLINT length, PLPointer data );

    static char mypltr_funcstr[255];

// This is the callback that gets handed to the C code.
//   It, in turn, calls the Lua callback
    void mypltr( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void * PL_UNUSED( pltr_data ) )
    {
        *tx = 0;
        *ty = 0;

        // check Lua state
        if ( myL == NULL )
        {
            fprintf( stderr, "Lua state is not set!" );
            return;
        }

        // push functions and arguments
        lua_getglobal( myL, mypltr_funcstr ); // function to be called
        lua_pushnumber( myL, x );             // push 1st argument
        lua_pushnumber( myL, y );             // push 2nd argument

        // do the call (2 arguments, 2 result)
        if ( lua_pcall( myL, 2, 2, 0 ) != 0 )
            fprintf( stderr, "error running function `%s':%s",
                mypltr_funcstr, lua_tostring( myL, -1 ) );

        // retrieve results
        if ( !lua_isnumber( myL, -2 ) )
        {
            fprintf( stderr, "function `%s' must return a number as 1st result", mypltr_funcstr );
            return;
        }
        if ( !lua_isnumber( myL, -1 ) )
        {
            fprintf( stderr, "function `%s' must return a number as 2nd result", mypltr_funcstr );
            return;
        }
        *tx = lua_tonumber( myL, -2 );
        *ty = lua_tonumber( myL, -1 );
        lua_pop( myL, 2 ); // pop returned values

        return;
    }

    static char myct_funcstr[255];

// This is the callback that gets handed to the C code.
//   It, in turn, calls the Lua callback
    void myct( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void * PL_UNUSED( pltr_data ) )
    {
        *tx = 0;
        *ty = 0;

        // check Lua state
        if ( myL == NULL )
        {
            fprintf( stderr, "Lua state is not set!" );
            return;
        }

        // push functions and arguments
        lua_getglobal( myL, myct_funcstr ); // function to be called
        lua_pushnumber( myL, x );           // push 1st argument
        lua_pushnumber( myL, y );           // push 2nd argument

        // do the call (2 arguments, 2 result)
        if ( lua_pcall( myL, 2, 2, 0 ) != 0 )
            fprintf( stderr, "error running function `%s':%s",
                myct_funcstr, lua_tostring( myL, -1 ) );

        // retrieve results
        if ( !lua_isnumber( myL, -2 ) )
        {
            fprintf( stderr, "function `%s' must return a number as 1st result", myct_funcstr );
            return;
        }
        if ( !lua_isnumber( myL, -1 ) )
        {
            fprintf( stderr, "function `%s' must return a number as 2nd result", myct_funcstr );
            return;
        }
        *tx = lua_tonumber( myL, -2 );
        *ty = lua_tonumber( myL, -1 );
        lua_pop( myL, 2 ); // pop returned values

        return;
    }

    static char mylabel_funcstr[255];

    void mylabel( PLINT axis, PLFLT value, char* label, PLINT length, PLPointer PL_UNUSED( data ) )
    {
        // check Lua state
        if ( myL == NULL )
        {
            fprintf( stderr, "Lua state is not set!" );
            return;
        }

        // push functions and arguments
        lua_getglobal( myL, mylabel_funcstr ); // function to be called
        lua_pushnumber( myL, axis );           // push 1st argument
        lua_pushnumber( myL, value );          // push 1st argument

        // do the call (2 arguments, 1 result)
        if ( lua_pcall( myL, 2, 1, 0 ) != 0 )
            fprintf( stderr, "error running function `%s':%s",
                mylabel_funcstr, lua_tostring( myL, -1 ) );

        // retrieve results
        if ( !lua_isstring( myL, -1 ) )
        {
            fprintf( stderr, "function `%s' must return a string as result", mylabel_funcstr );
            return;
        }
        strncpy( label, lua_tostring( myL, -1 ), length );

        lua_pop( myL, 1 ); // pop returned values

        return;
    }
%}

%typemap( in, numinputs = 0 ) defined_func df {
    $1 = NULL;
}

%typemap( in, numinputs = 0 ) fill_func ff {
    $1 = plfill;
}

%typemap( in ) mapform_func mapform {
    $1 = NULL;
    mapform_funcstr[0] = '\0';

    if ( lua_isnil( L, $input ) )
    {
        $1 = NULL;
    }
    else if ( lua_isstring( L, $input ) )
    {
        $1 = mapform;
        strncpy( mapform_funcstr, lua_tostring( L, $input ), 255 );
        myL = L;
    }
    else
        SWIG_fail_arg( "$symname", $argnum, "$1_type" );
}
%typemap( freearg ) mapform_func mapform {
    mapform_funcstr[0] = '\0';
}

// you can omit PLPointer data
%typemap( in, numinputs = 0 ) PLPointer data {
    $1 = NULL;
}

%typemap( in ) label_func lf {
    $1 = NULL;
    mylabel_funcstr[0] = '\0';

    if ( lua_isnil( L, $input ) )
    {
        $1 = NULL;
    }
    else if ( lua_isstring( L, $input ) )
    {
        $1 = mylabel;
        strncpy( mylabel_funcstr, lua_tostring( L, $input ), 255 );
        myL = L;
    }
    else
        SWIG_fail_arg( "$symname", $argnum, "$1_type" );
}
%typemap( freearg ) label_func lf {
}

%typemap( in ) pltr_func pltr {
    $1 = NULL;
    mypltr_funcstr[0] = '\0';

    if ( lua_isstring( L, $input ) )
    {
        const char* funcstr = lua_tostring( L, $input );
        if ( strcmp( "pltr0", funcstr ) == 0 )
        {
            $1 = pltr0;
        }
        else if ( strcmp( "pltr1", funcstr ) == 0 )
        {
            $1 = pltr1;
        }
        else if ( strcmp( "pltr2", funcstr ) == 0 )
        {
            $1 = pltr2;
        }
        else
        {
            $1 = mypltr;
            strncpy( mypltr_funcstr, funcstr, 255 );
            myL = L;
        }
    }
    else
        SWIG_fail_arg( "$symname", $argnum, "$1_type" );
}
%typemap( freearg ) pltr_func pltr {
    mypltr_funcstr[0] = '\0';
}
// you can omit the pltr func
%typemap( default ) pltr_func pltr {
    $1 = NULL;
}
%apply pltr_func pltr { pltr_func pltr_img };

%typemap( in ) ct_func ctf {
    $1 = NULL;
    myct_funcstr[0] = '\0';

    if ( lua_isstring( L, $input ) )
    {
        const char* funcstr = lua_tostring( L, $input );
        $1 = myct;
        strncpy( myct_funcstr, funcstr, 255 );
        myL = L;
    }
    else
        SWIG_fail_arg( "$symname", $argnum, "$1_type" );
}
// you can omit the ct func
%typemap( default ) ct_func ctf {
    $1 = NULL;
    myct_funcstr[0] = '\0';
}

%typemap( arginit ) PLPointer OBJECT_DATA {
    cgrid1$argnum.xg = cgrid1$argnum.yg = cgrid1$argnum.zg = NULL;
    cgrid1$argnum.nx = cgrid1$argnum.ny = cgrid1$argnum.nz = 0;
    cgrid2$argnum.xg = cgrid2$argnum.yg = cgrid2$argnum.zg = NULL;
    cgrid2$argnum.nx = cgrid2$argnum.ny = 0;
}
%typemap( in ) PLPointer OBJECT_DATA( PLcGrid cgrid1, PLcGrid2 cgrid2 )
{
    int nx, ny;
    int gridmode = 0;

    lua_pushstring( L, "xg" );
    lua_gettable( L, $input );
    if ( !lua_istable( L, -1 ) )
    {
        lua_pop( L, 1 ); // pop "xg"
        lua_pushstring( L, "expected a table xg" );
        SWIG_fail;
    }
    lua_rawgeti( L, -1, 1 );
    if ( lua_istable( L, -1 ) )
        gridmode = 2; // two dimensional array
    else if ( lua_isnumber( L, -1 ) )
        gridmode = 1; // one dimensional array
    else
    {
        lua_pop( L, 1 ); // pop "1"
        lua_pop( L, 1 ); // pop "xg"
        lua_pushstring( L, "expected a one or two dimensional array/table in xg" );
        SWIG_fail;
    }
    lua_pop( L, 1 ); // pop test element
    if ( gridmode == 1 )
    {
        cgrid1.xg = (PLFLT *) LUA_get_double_num_array_var( L, -1, &nx );
        if ( !cgrid1.xg )
        {
            lua_pop( L, 1 ); // pop "xg"
            SWIG_fail;
        }
        if ( nx != Xlen )
        {
            lua_pushfstring( L, "Table xg must be of length%d.", Xlen );
            SWIG_fail;
        }
        cgrid1.nx = nx;
    }
    else
    {
        cgrid2.xg = read_double_Matrix( L, -1, &nx, &ny );
        if ( !cgrid2.xg )
        {
            lua_pop( L, 1 ); // pop "xg"
            SWIG_fail;
        }
        if ( ( nx != Xlen ) || ( ny != Ylen ) )
        {
            lua_pop( L, 1 ); // pop "xg"
            lua_pushfstring( L, "Vectors must match matrix." );
            SWIG_fail;
        }
        cgrid2.nx = nx;
        cgrid2.ny = ny;
    }
    lua_pop( L, 1 ); // pop "xg"

    lua_pushstring( L, "yg" );
    lua_gettable( L, $input );
    if ( !lua_istable( L, -1 ) )
    {
        lua_pop( L, 1 );
        lua_pushstring( L, "expected a table yg" );
        SWIG_fail;
    }
    lua_rawgeti( L, -1, 1 );
    if ( gridmode == 2 )
    {
        if ( !lua_istable( L, -1 ) )
        {
            lua_pop( L, 1 ); // pop "1"
            lua_pop( L, 1 ); // pop "yg"
            lua_pushstring( L, "expected a two dimensional array/table in yg" );
            SWIG_fail;
        }
    }
    else
    {
        if ( !lua_isnumber( L, -1 ) )
        {
            lua_pop( L, 1 ); // pop "1"
            lua_pop( L, 1 ); // pop "yg"
            lua_pushstring( L, "expected a one dimensional array/table in yg" );
            SWIG_fail;
        }
    }
    lua_pop( L, 1 ); // pop "1"
    if ( gridmode == 1 )
    {
        cgrid1.yg = (PLFLT *) LUA_get_double_num_array_var( L, -1, &ny );
        if ( !cgrid1.yg )
        {
            lua_pop( L, 1 ); // pop "yg"
            SWIG_fail;
        }
        if ( ny != Ylen )
        {
            lua_pushfstring( L, "Table yg must be of length%d.", Ylen );
            SWIG_fail;
        }
        cgrid1.ny = ny;
    }
    else
    {
        cgrid2.yg = read_double_Matrix( L, -1, &nx, &ny );
        if ( !cgrid2.yg )
        {
            lua_pop( L, 1 ); // pop "xg"
            SWIG_fail;
        }
        if ( ( nx != Xlen ) || ( ny != Ylen ) )
        {
            lua_pop( L, 1 ); // pop "xg"
            lua_pushfstring( L, "Vectors must match matrix." );
            SWIG_fail;
        }
        // cgrid2.nx/ny already set
    }
    lua_pop( L, 1 ); // pop "yg"

    if ( gridmode == 1 )
        $1 = &cgrid1;
    else if ( gridmode == 2 )
        $1 = &cgrid2;
}
%typemap( freearg ) PLPointer OBJECT_DATA {
    int i;

    LUA_FREE_ARRAY( cgrid1$argnum.xg );
    LUA_FREE_ARRAY( cgrid1$argnum.yg );

    if ( cgrid2$argnum.xg )
    {
        for ( i = 0; i < Xlen; i++ )
            LUA_FREE_ARRAY( cgrid2$argnum.xg[i] );
        LUA_FREE_ARRAY( cgrid2$argnum.xg );
    }
    if ( cgrid2$argnum.yg )
    {
        for ( i = 0; i < Xlen; i++ )
            LUA_FREE_ARRAY( cgrid2$argnum.yg[i] );
        LUA_FREE_ARRAY( cgrid2$argnum.yg );
    }
}
// you can omit the data too
%typemap( default ) PLPointer OBJECT_DATA {
    $1 = NULL;
}
%apply PLPointer OBJECT_DATA { PLPointer OBJECT_DATA_img };


// this typemap takes a sequence of strings and converts them for plstripc
//   also checks that previous Arrays were of length 4
//
%typemap( in, checkfn = "lua_istable" ) const char *legline[4] {
    int i;
    $1 = NULL;

    if ( SWIG_table_size( L, $input ) != 4 )
    {
        lua_pushfstring( L, "Requires a sequence of 4 strings." );
        SWIG_fail;
    }
    if ( Alen != 4 )
    {
        lua_pushfstring( L, "colline and styline args must be length 4." );
        SWIG_fail;
    }

    $1 = malloc( sizeof ( char* ) * 4 );
    for ( i = 1; i <= 4; i++ )
    {
        lua_rawgeti( L, $input, i );
        if ( lua_isstring( L, -1 ) )
        {
            $1[i - 1] = (char *) lua_tostring( L, -1 );
        }
        else
        {
            lua_pop( L, 1 );
            lua_pushfstring( L, "Requires a sequence of 4 strings." );
            SWIG_fail;
            // $1 array is freed after 'fail:'
        }
        lua_pop( L, 1 );
    }
}
%typemap( freearg ) const char *legline[4] {
    if ( $1 )
    {
        free( $1 ); $1 = NULL;
    }
}
%typemap( default ) const char *legline[4] { $1 = NULL; }

// No count but check consistency with previous
%typemap( in, checkfn = "lua_istable" ) const char **ArrayCk {
    int i;
    $1 = NULL;

    if ( SWIG_table_size( L, $input ) != Alen )
    {
        lua_pushfstring( L, "Tables must be of same length." );
        SWIG_fail;
    }
    $1 = malloc( sizeof ( char* ) * Alen );
    for ( i = 1; i <= Alen; i++ )
    {
        lua_rawgeti( L, $input, i );
        if ( lua_isstring( L, -1 ) )
        {
            $1[i - 1] = (char *) lua_tostring( L, -1 );
        }
        else
        {
            lua_pop( L, 1 );
            lua_pushfstring( L, "Requires a sequence of strings." );
            SWIG_fail;
            // $1 array is freed after 'fail:'
        }
        lua_pop( L, 1 );
    }
}
%typemap( freearg ) const char **ArrayCk {
    if ( $1 )
    {
        free( $1 ); $1 = NULL;
    }
}

%typemap ( in ) ( PLINT n, const char **Array )
{
    int i;
    $1   = SWIG_table_size( L, $input );
    Alen = $1;

    $2 = malloc( sizeof ( char* ) * Alen );
    for ( i = 1; i <= Alen; i++ )
    {
        lua_rawgeti( L, $input, i );
        if ( lua_isstring( L, -1 ) )
        {
            $2[i - 1] = (char *) lua_tostring( L, -1 );
        }
        else
        {
            lua_pop( L, 1 );
            lua_pushfstring( L, "Requires a sequence of strings." );
            SWIG_fail;
        }
        lua_pop( L, 1 );
    }
}
%typemap ( freearg ) ( PLINT n, const char **Array )
{
    if ( $2 )
    {
        free( $2 ); $2 = NULL;
    }
}


// Process options list using current options info.
%typemap( in, checkfn = "lua_istable" ) ( int *p_argc, char **argv )
{
    int i, n;

    // from lua 5.1 on there is no element "n" anymore,
//       so we need to find out the number of command line
//       options manually
    for ( i = 1;; i++ )
    {
        lua_rawgeti( L, $input, i );
        if ( lua_isnil( L, -1 ) )
        {
            // ok, this index doesn't exist anymore, we have i-1
//               command line options
            lua_pop( L, 1 );
            break;
        }
    }
    n  = i;
    $1 = &n;

    $2 = LUA_ALLOC_ARRAY( char*, ( n + 1 ) );

    for ( i = 0; i < n; i++ )
    {
        lua_rawgeti( L, $input, i );
        if ( lua_isstring( L, -1 ) )
        {
            $2[i] = (char *) lua_tostring( L, -1 );
        }
        else
        {
            lua_pop( L, 1 );
            lua_pushfstring( L, "List items must be strings" );
            SWIG_fail;
            // $2 array is freed after 'fail:'
        }
        lua_pop( L, 1 );
    }
    $2[n] = NULL;
}
%typemap ( freearg ) ( int *p_argc, char **argv )
{
    LUA_FREE_ARRAY( $2 );
}

%typemap ( default ) ( PLBOOL deffalse )
{
    $1 = 0;
}


//--------------------------------------------------------------------------
//                                 Renames
//--------------------------------------------------------------------------
%rename( parseopts ) plparseopts;
%rename( setcontlabelformat ) pl_setcontlabelformat;
%rename( setcontlabelparam ) pl_setcontlabelparam;
%rename( adv ) pladv;
%rename( arc ) plarc;
%rename( axes ) plaxes;
%rename( bin ) plbin;
%rename( bop ) plbop;
%rename( box ) plbox;
%rename( box3 ) plbox3;
%rename( btime ) plbtime;
%rename( calc_world ) plcalc_world;
%rename( clear ) plclear;
%rename( col0 ) plcol0;
%rename( col1 ) plcol1;
%rename( colorbar ) plcolorbar;
%rename( configtime ) plconfigtime;
%rename( cont ) plcont;
%rename( cpstrm ) plcpstrm;
%rename( ctime ) plctime;
%rename( plend ) plend;
%rename( plend1 ) plend1;
%rename( env ) plenv;
%rename( env0 ) plenv0;
%rename( eop ) pleop;
%rename( errx ) plerrx;
%rename( erry ) plerry;
%rename( famadv ) plfamadv;
%rename( fill ) plfill;
%rename( fill3 ) plfill3;
%rename( flush ) plflush;
%rename( font ) plfont;
%rename( fontld ) plfontld;
%rename( gchr ) plgchr;
%rename( gcmap1_range ) plgcmap1_range;
%rename( gcol0 ) plgcol0;
%rename( gcolbg ) plgcolbg;
%rename( gcol0a ) plgcol0a;
%rename( gcolbga ) plgcolbga;
%rename( gcompression ) plgcompression;
%rename( gdev ) plgdev;
%rename( gdidev ) plgdidev;
%rename( gdiori ) plgdiori;
%rename( gdiplt ) plgdiplt;
%rename( gfam ) plgfam;
%rename( gfci ) plgfci;
%rename( gfnam ) plgfnam;
%rename( gfont ) plgfont;
%rename( glevel ) plglevel;
%rename( gpage ) plgpage;
%rename( gra ) plgra;
%rename( gradient ) plgradient;
%rename( griddata ) plgriddata;
%rename( gspa ) plgspa;
%rename( gstrm ) plgstrm;
%rename( gver ) plgver;
%rename( gvpd ) plgvpd;
%rename( gvpw ) plgvpw;
%rename( gxax ) plgxax;
%rename( gyax ) plgyax;
%rename( gzax ) plgzax;
%rename( hist ) plhist;
%rename( hlsrgb ) plhlsrgb;
%rename( image ) plimage;
%rename( imagefr ) plimagefr;
%rename( init ) plinit;
%rename( join ) pljoin;
%rename( lab ) pllab;
%rename( legend ) pllegend;
%rename( lightsource ) pllightsource;
%rename( line ) plline;
%rename( line3 ) plline3;
%rename( lsty ) pllsty;
%rename( map ) plmap;
%rename( mapline ) plmapline;
%rename( mapstring ) plmapstring;
%rename( maptex ) plmaptex;
%rename( mapfill ) plmapfill;
%rename( meridians ) plmeridians;
%rename( mesh ) plmesh;
%rename( meshc ) plmeshc;
%rename( mkstrm ) plmkstrm;
%rename( mtex ) plmtex;
%rename( mtex3 ) plmtex3;
%rename( plot3d ) plot3d;
%rename( plot3dc ) plot3dc;
%rename( plot3dcl ) plot3dcl;
%rename( parseopts ) plparseopts;
%rename( pat ) plpat;
%rename( path ) plpath;
%rename( poin ) plpoin;
%rename( poin3 ) plpoin3;
%rename( poly3 ) plpoly3;
%rename( prec ) plprec;
%rename( psty ) plpsty;
%rename( ptex ) plptex;
%rename( ptex3 ) plptex3;
%rename( randd ) plrandd;
%rename( replot ) plreplot;
%rename( rgbhls ) plrgbhls;
%rename( schr ) plschr;
%rename( scmap0 ) plscmap0;
%rename( scmap0a ) plscmap0a;
%rename( scmap0n ) plscmap0n;
%rename( scmap1 ) plscmap1;
%rename( scmap1a ) plscmap1a;
%rename( scmap1l ) plscmap1l;
%rename( scmap1la ) plscmap1la;
%rename( scmap1n ) plscmap1n;
%rename( scmap1_range ) plscmap1_range;
%rename( scol0 ) plscol0;
%rename( scol0a ) plscol0a;
%rename( scolbg ) plscolbg;
%rename( scolbga ) plscolbga;
%rename( scolor ) plscolor;
%rename( scompression ) plscompression;
%rename( sdev ) plsdev;
%rename( sdidev ) plsdidev;
%rename( sdimap ) plsdimap;
%rename( sdiori ) plsdiori;
%rename( sdiplt ) plsdiplt;
%rename( sdiplz ) plsdiplz;
%rename( seed ) plseed;
%rename( sesc ) plsesc;
%rename( setopt ) plsetopt;
%rename( sfam ) plsfam;
%rename( sfci ) plsfci;
%rename( sfnam ) plsfnam;
%rename( sfont ) plsfont;
%rename( shade ) plshade;
%rename( shades ) plshades;
%rename( slabelfunc ) plslabelfunc;
%rename( smaj ) plsmaj;
%rename( smem ) plsmem;
%rename( smin ) plsmin;
%rename( sori ) plsori;
%rename( spage ) plspage;
%rename( spal0 ) plspal0;
%rename( spal1 ) plspal1;
%rename( spause ) plspause;
%rename( sstrm ) plsstrm;
%rename( ssub ) plssub;
%rename( ssym ) plssym;
%rename( star ) plstar;
%rename( start ) plstart;
%rename( stransform ) plstransform;
%rename( string ) plstring;
%rename( string3 ) plstring3;
%rename( stripa ) plstripa;
%rename( stripc ) plstripc;
%rename( stripd ) plstripd;
%rename( styl ) plstyl;
%rename( surf3d ) plsurf3d;
%rename( surf3dl ) plsurf3dl;
%rename( svect ) plsvect;
%rename( svpa ) plsvpa;
%rename( sxax ) plsxax;
%rename( syax ) plsyax;
%rename( sym ) plsym;
%rename( szax ) plszax;
%rename( text ) pltext;
%rename( timefmt ) pltimefmt;
%rename( vasp ) plvasp;
%rename( vect ) plvect;
%rename( vpas ) plvpas;
%rename( vpor ) plvpor;
%rename( vsta ) plvsta;
%rename( w3d ) plw3d;
%rename( width ) plwidth;
%rename( wind ) plwind;
%rename( xormod ) plxormod;
%rename( warn ) plwarn;
%rename( abort ) plabort;
%rename( MinMax2dGrid ) plMinMax2dGrid;

// swig compatible PLplot API definitions from here on.
%include plplotcapi.i

// A handy way to issue warnings, if need be.
void
plwarn( const char *message );

// Much the same as plwarn(), but appends ", aborting operation" to the
//   error message.
void
plabort( const char *message );
