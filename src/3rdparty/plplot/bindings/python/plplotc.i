%module plplotc

//
//Copyright (C) 2002 Gary Bishop
//Copyright (C) 2004-2019 Alan W. Irwin
//Copyright (C) 2004 Andrew Ross
//
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
//A SWIG interface to PLplot for Python. This wrapper is different from
//the one supplied in plmodule.c (now stored in the "old" subdirectory of
//the top-level build tree for reference purposes) in that:
//
//   1) it strictly provides the C-API with the usual change of not
//      requiring lengths for arrays,
//
//   2) it attempts to provide the entire API including callbacks for
//      plcont and plshade.
//
//   3) it works both with the single and double-precision versions of the
//      PLplot library.
//
//This is known to work with swig-1.3.17 (versions prior to 1.3.14 *known* not
//to work, 1.3.14-1.3.16 might work but you are on your own).  The resulting
//interface works for both python-1.5.2 (RedHat 7.3) and python-2.1.3 (Debian
//woody).
//
//

%include typemaps.i

%{
#define NPY_NO_DEPRECATED_API    NPY_1_7_API_VERSION
#include <arrayobject.h>
#include "plplot.h"
#include "plplotP.h"

#define  NPY_PLINT    NPY_INT32

#ifdef PL_DOUBLE
#define  NPY_PLFLT    NPY_FLOAT64
#else
#define  NPY_PLFLT    NPY_FLOAT32
#endif

// python-1.5 compatibility mode?
#if !defined ( PySequence_Fast_GET_ITEM )
  #define PySequence_Fast_GET_ITEM    PySequence_GetItem
#endif
#define PySequence_Size               PySequence_Length
%}

#ifdef PL_DOUBLE_INTERFACE
typedef double         PLFLT;
#else
typedef float          PLFLT;
#endif

// This assumes that C int is 32-bit - swig doesn't know about int32_t
// Ideally we should have a typemap for it
typedef int            PLINT;
typedef unsigned int   PLUNICODE;
typedef PLINT          PLBOOL;

// We have to get import_array called in our extension before we can use Numeric
%init
%{
    import_array();
%}

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

//--------------------------------------------------------------------------
//                         PLINT arrays
//--------------------------------------------------------------------------

// If Python integers are not 32-bit then we need to do some casting
#if SIZEOF_LONG != 4
%wrapper
%{
    PyArrayObject* myIntArray_ContiguousFromObject( PyObject* in, int type, int mindims, int maxdims );

// some really twisted stuff to allow calling a single precision library from python
    PyArrayObject* myIntArray_ContiguousFromObject( PyObject* in, int PL_UNUSED( type ), int mindims, int maxdims )
    {
        PyArrayObject* tmp = (PyArrayObject *) PyArray_ContiguousFromObject( in, NPY_PLINT,
            mindims, maxdims );
        if ( !tmp )
        {
            // could be an incoming long array which can't be "safely" converted, do it anyway
            if ( PyArray_Check( in ) )
            {
                PyErr_Clear();
                tmp = (PyArrayObject *) PyArray_Cast( (PyArrayObject *) in, NPY_PLINT );
            }
        }
        return tmp;
    }
%}
#else
%wrapper
%{
#define myIntArray_ContiguousFromObject    PyArray_ContiguousFromObject
%}
#endif

// With preceding count
%typemap ( in ) ( PLINT n, const PLINT * Array ) ( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    $1 = Alen = PyArray_DIMS( tmp )[0];
    $2 = (PLINT *) PyArray_DATA( tmp );
}
%typemap ( freearg ) ( PLINT n, const PLINT * Array )
{
    Py_CLEAR( tmp$argnum );
}

// Trailing count and check consistency with previous
%typemap ( in ) ( const PLINT * ArrayCk, PLINT n ) ( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Alen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    $2 = PyArray_DIMS( tmp )[0];
    $1 = (PLINT *) PyArray_DATA( tmp );
}
%typemap ( freearg ) ( const PLINT * ArrayCk, PLINT n )
{
    Py_CLEAR( tmp$argnum );
}

// No count but check consistency with previous
%typemap( in ) const PLINT * ArrayCk( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Alen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    $1 = (PLINT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLINT * ArrayCk { Py_CLEAR( tmp$argnum );}

// No count but check consistency with previous or NULL
%typemap( in ) const PLINT * ArrayCkNull( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Alen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    $1 = (PLINT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLINT * ArrayCkNull { Py_CLEAR( tmp$argnum );}

// Weird case to allow argument to be one shorter than others
%typemap( in ) const PLINT * ArrayCkMinus1( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] < Alen - 1 )
    {
        PyErr_SetString( PyExc_ValueError, "Vector must be at least length of others minus 1." );
        return NULL;
    }
    $1 = (PLINT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLINT * ArrayCkMinus1 { Py_CLEAR( tmp$argnum );}

%typemap( in ) const PLINT * ArrayCkMinus1Null( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] < Alen - 1 )
    {
        PyErr_SetString( PyExc_ValueError, "Vector must be at least length of others minus 1." );
        return NULL;
    }
    $1 = (PLINT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLINT * ArrayCkMinus1Null { Py_CLEAR( tmp$argnum );}

// No length but remember size to check others
%typemap( in ) const PLINT * Array( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    Alen = PyArray_DIMS( tmp )[0];
    $1   = (PLINT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLINT * Array { Py_CLEAR( tmp$argnum );}

// set X and Y length for later consistency checking
%typemap( in ) const PLINT * ArrayN( PyArrayObject * tmp = NULL )
{
    int i;
    tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Alen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    Xlen = PyArray_DIMS( tmp )[0];
    $1   = (PLINT *) PyArray_DATA( tmp );
    Ylen = -1;
    for ( i = 0; i < Xlen; i++ )
        if ( $1[i] > Ylen )
            Ylen = $1[i];
}
%typemap( freearg ) const PLINT * ArrayN { Py_CLEAR( tmp$argnum );}

// With trailing count and NULL array option.
%typemap ( in ) ( const PLINT * ArrayNull, PLINT n ) ( PyArrayObject * tmp = NULL )
{
    if ( $input != Py_None )
    {
        tmp = (PyArrayObject *) myIntArray_ContiguousFromObject( $input, NPY_PLINT, 1, 1 );
        if ( tmp == NULL )
            return NULL;
        $1 = (PLINT *) PyArray_DATA( tmp );
        $2 = PyArray_DIMS( tmp )[0];
    }
    else
    {
        $1 = NULL;
        $2 = 0;
    }
}
%typemap ( freearg ) ( const PLINT * ArrayNull, PLINT n )
{
    Py_CLEAR( tmp$argnum );
}

//--------------------------------------------------------------------------
//                                 PLFLT Arrays
//--------------------------------------------------------------------------

#ifndef PL_DOUBLE_INTERFACE
%wrapper
%{
// some really twisted stuff to allow calling a single precision library from python
    PyArrayObject* myArray_ContiguousFromObject( PyObject* in, int type, int mindims, int maxdims )
    {
        PyArrayObject* tmp = (PyArrayObject *) PyArray_ContiguousFromObject( in, NPY_FLOAT32,
            mindims, maxdims );
        if ( !tmp )
        {
            // could be an incoming double array which can't be "safely" converted, do it anyway
            if ( PyArray_Check( in ) )
            {
                PyErr_Clear();
                tmp = (PyArrayObject *) PyArray_Cast( (PyArrayObject *) in, NPY_FLOAT32 );
            }
        }
        return tmp;
    }
%}
#else
%wrapper
%{
#define myArray_ContiguousFromObject    PyArray_ContiguousFromObject
%}
#endif

// with preceding count
%typemap ( in ) ( PLINT n, const PLFLT * Array ) ( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    $1 = Alen = PyArray_DIMS( tmp )[0];
    $2 = (PLFLT *) PyArray_DATA( tmp );
}
%typemap ( freearg ) ( PLINT n, const PLFLT * Array )
{
    Py_CLEAR( tmp$argnum );
}

// trailing count and check consistency with previous
%typemap ( in ) ( const PLFLT * ArrayCk, PLINT n ) ( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Alen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    $1 = (PLFLT *) PyArray_DATA( tmp );
    $2 = PyArray_DIMS( tmp )[0];
}
%typemap ( freearg ) ( const PLFLT * ArrayCk, PLINT n )
{
    Py_CLEAR( tmp$argnum );
}

// trailing count and check consistency with previous
%typemap ( in ) ( const PLFLT * ArrayCkNull, PLINT n ) ( PyArrayObject * tmp = NULL )
{
    if ( $input != Py_None )
    {
        tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
        if ( tmp == NULL )
            return NULL;
        if ( PyArray_DIMS( tmp )[0] != Alen )
        {
            PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
            return NULL;
        }
        $1 = (PLFLT *) PyArray_DATA( tmp );
        $2 = PyArray_DIMS( tmp )[0];
    }
    else
    {
        $1 = NULL;
        $2 = 0;
    }
}
%typemap ( freearg ) ( const PLFLT * ArrayCkNull, PLINT n )
{
    Py_CLEAR( tmp$argnum );
}

// no count, but check consistency with previous
%typemap( in ) const PLFLT * ArrayCk( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Alen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    $1 = (PLFLT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLFLT * ArrayCk { Py_CLEAR( tmp$argnum );}

// no count, but check consistency with previous, or NULL
%typemap( in ) const PLFLT * ArrayCkNull( PyArrayObject * tmp = NULL )
{
    if ( $input != Py_None )
    {
        tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
        if ( tmp == NULL )
            return NULL;
        if ( PyArray_DIMS( tmp )[0] != Alen )
        {
            PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
            return NULL;
        }
        $1 = (PLFLT *) PyArray_DATA( tmp );
    }
    else
    {
        $1 = NULL;
    }
}
%typemap( freearg ) const PLFLT * ArrayCkNull { Py_CLEAR( tmp$argnum );}

// check consistency with X dimension of previous
%typemap( in ) const PLFLT * ArrayCkX( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Xlen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    $1 = (PLFLT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLFLT * ArrayCkX { Py_CLEAR( tmp$argnum );}

// check consistency with Y dimension of previous
%typemap( in ) const PLFLT * ArrayCkY( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Ylen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    $1 = (PLFLT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLFLT * ArrayCkY { Py_CLEAR( tmp$argnum );}

// set X length for later consistency checking, with trailing count
%typemap ( in ) ( const PLFLT * ArrayX, PLINT nx ) ( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    Xlen = PyArray_DIMS( tmp )[0];
    $2   = Xlen;
    $1   = (PLFLT *) PyArray_DATA( tmp );
}
%typemap ( freearg ) ( const PLFLT * ArrayX, PLINT nx )
{
    Py_CLEAR( tmp$argnum );
}

// set X length for later consistency checking
%typemap( in ) const PLFLT * ArrayX( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    Xlen = PyArray_DIMS( tmp )[0];
    $1   = (PLFLT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLFLT * ArrayX { Py_CLEAR( tmp$argnum );}

// Set Y length for later consistency checking, with trailing count
%typemap ( in ) ( const PLFLT * ArrayY, PLINT ny ) ( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    Ylen = PyArray_DIMS( tmp )[0];
    $2   = Ylen;
    $1   = (PLFLT *) PyArray_DATA( tmp );
}
%typemap ( freearg ) ( const PLFLT * ArrayY, PLINT ny )
{
    Py_CLEAR( tmp$argnum );
}

// set Y length for later consistency checking
%typemap( in ) const PLFLT * ArrayY( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    Ylen = PyArray_DIMS( tmp )[0];
    $1   = (PLFLT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLFLT * ArrayY { Py_CLEAR( tmp$argnum );}


// with trailing count
%typemap ( in ) ( const PLFLT * Array, PLINT n ) ( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    $2 = PyArray_DIMS( tmp )[0];
    $1 = (PLFLT *) PyArray_DATA( tmp );
}
%typemap ( freearg ) ( const PLFLT * Array, PLINT n )
{
    Py_CLEAR( tmp$argnum );
}

// with no count
%typemap( in ) const PLFLT * Array( PyArrayObject * tmp = NULL )
{
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    Alen = PyArray_DIMS( tmp )[0];
    $1   = (PLFLT *) PyArray_DATA( tmp );
}
%typemap( freearg ) const PLFLT * Array { Py_CLEAR( tmp$argnum );}


// with no count
%typemap( in ) const PLFLT * ArrayNull( PyArrayObject * tmp = NULL )
{
    if ( $input != Py_None )
    {
        tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
        if ( tmp == NULL )
            return NULL;
        Alen = PyArray_DIMS( tmp )[0];
        $1   = (PLFLT *) PyArray_DATA( tmp );
    }
    else
    {
        $1   = NULL;
        Alen = 0;
    }
}
%typemap( freearg ) const PLFLT * ArrayNull { Py_CLEAR( tmp$argnum );}

// 2D array with trailing dimensions, check consistency with previous
%typemap ( in ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny ) ( PyArrayObject * tmp = NULL )
{
    int i, size;
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 2, 2 );
    if ( tmp == NULL )
        return NULL;
    if ( Xlen != PyArray_DIMS( tmp )[0] || Ylen != PyArray_DIMS( tmp )[1] )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must match matrix." );
        return NULL;
    }
    $2   = PyArray_DIMS( tmp )[0];
    $3   = PyArray_DIMS( tmp )[1];
    size = $3;
    $1   = (PLFLT **) malloc( sizeof ( PLFLT* ) * (size_t) $2 );
    for ( i = 0; i < $2; i++ )
        $1[i] = ( (PLFLT *) PyArray_DATA( tmp ) + i * size );
}
%typemap ( freearg ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny )
{
    Py_CLEAR( tmp$argnum );
    free( $1 );
}

// 2D array with trailing dimensions, set the X, Y size for later checking
%typemap ( in ) ( const PLFLT **Matrix, PLINT nx, PLINT ny ) ( PyArrayObject * tmp = NULL )
{
    int i, size;
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 2, 2 );
    if ( tmp == NULL )
        return NULL;
    Xlen = $2 = PyArray_DIMS( tmp )[0];
    Ylen = $3 = PyArray_DIMS( tmp )[1];
    size = $3;
    $1   = (PLFLT **) malloc( sizeof ( PLFLT* ) * (size_t) $2 );
    for ( i = 0; i < $2; i++ )
        $1[i] = ( (PLFLT *) PyArray_DATA( tmp ) + i * size );
}
%typemap ( freearg ) ( const PLFLT **Matrix, PLINT nx, PLINT ny )
{
    Py_CLEAR( tmp$argnum );
    free( $1 );
}

// 2D array with no dimensions, set the X, Y size for later checking
%typemap( in ) const PLFLT * *Matrix( PyArrayObject * tmp = NULL )
{
    int i, size;
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 2, 2 );
    if ( tmp == NULL )
        return NULL;
    Xlen = PyArray_DIMS( tmp )[0];
    Ylen = PyArray_DIMS( tmp )[1];
    size = Ylen;
    $1   = (PLFLT **) malloc( sizeof ( PLFLT* ) * (size_t) Xlen );
    for ( i = 0; i < Xlen; i++ )
        $1[i] = ( (PLFLT *) PyArray_DATA( tmp ) + i * size );
}
%typemap( freearg ) const PLFLT * *Matrix {
    Py_CLEAR( tmp$argnum );
    free( $1 );
}

// 2D array, check for consistency
%typemap( in ) const PLFLT * *MatrixCk( PyArrayObject * tmp = NULL )
{
    int i, size;
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 2, 2 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Xlen || PyArray_DIMS( tmp )[1] != Ylen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must match matrix." );
        return NULL;
    }
    size = Ylen;
    $1   = (PLFLT **) malloc( sizeof ( PLFLT* ) * (size_t) Xlen );
    for ( i = 0; i < Xlen; i++ )
        $1[i] = ( (PLFLT *) PyArray_DATA( tmp ) + i * size );
}
%typemap( freearg ) const PLFLT * *MatrixCk {
    Py_CLEAR( tmp$argnum );
    free( $1 );
}

// 2D array, check for consistency and modify in place version (no longer used
// in favor of correct output version in the combined typemap below).
%typemap( in ) PLFLT * *OutMatrixCk( PyArrayObject * tmp = NULL )
{
    int i, size;
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 2, 2 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Xlen || PyArray_DIMS( tmp )[1] != Ylen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must match matrix." );
        return NULL;
    }
    size = Ylen;
    $1   = (PLFLT **) malloc( sizeof ( PLFLT* ) * (size_t) Xlen );
    for ( i = 0; i < Xlen; i++ )
        $1[i] = ( (PLFLT *) PyArray_DATA( tmp ) + i * size );
}
%typemap( freearg ) PLFLT * *OutMatrixCk {
    Py_CLEAR( tmp$argnum );
    free( $1 );
}

// Combined typemap useful for specialized case of plgriddata.
// Set Y length for consistency checking, with trailing count
// combined with 2D output array, check for consistency
%typemap ( in ) ( const PLFLT * ArrayY, PLINT ny, PLFLT **OutMatrixCk ) ( PyArrayObject * tmp = NULL, PyObject * array = NULL )
{
    int      i, size;
    npy_intp dims[2];
    tmp = (PyArrayObject *) myArray_ContiguousFromObject( $input, NPY_PLFLT, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    Ylen = PyArray_DIMS( tmp )[0];
    $2   = Ylen;
    $1   = (PLFLT *) PyArray_DATA( tmp );
    // Make created 2D array have dimensions from prior ArrayX in the argument
    // list and this ArrayY.
    dims[0] = Xlen;
    dims[1] = Ylen;
    array   = PyArray_SimpleNew( 2, dims, NPY_PLFLT );
    if ( !array )
        return NULL;
    size = Ylen;
    $3   = (PLFLT **) malloc( sizeof ( double * ) * (size_t) Xlen );
    for ( i = 0; i < Xlen; i++ )
        $3[i] = ( (PLFLT *) PyArray_DATA( (PyArrayObject *) array ) + i * size );
}
%typemap ( freearg ) ( const PLFLT * ArrayY, PLINT ny, PLFLT **OutMatrixCk )
{
    Py_CLEAR( tmp$argnum );
    free( $3 );
}
%typemap ( argout ) ( const PLFLT * ArrayY, PLINT ny, PLFLT **OutMatrixCk )
{
    $result = SWIG_Python_AppendOutput( $result, array$argnum );
}

//**************************
//        special for pllegend / plcolorbar, char ** ArrayCk
//***************************
// no count, but check consistency with previous. Always allow NULL strings.
%typemap( in ) const char **ArrayCk( PyArrayObject * tmp = NULL )
{
    int i;
    tmp = (PyArrayObject *) PyArray_ContiguousFromObject( $input, NPY_STRING, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    if ( PyArray_DIMS( tmp )[0] != Alen )
    {
        PyErr_SetString( PyExc_ValueError, "Vectors must be same length." );
        return NULL;
    }
    $1 = (char **) malloc( sizeof ( char* ) * (size_t) Alen );
    for ( i = 0; i < Alen; i++ )
    {
        $1[i] = (char *) PyArray_DATA( tmp ) + i * PyArray_STRIDES( tmp )[0];
        if ( $1[i] == NULL )
        {
            free( $1 );
            return NULL;
        }
    }
}
%typemap( freearg ) const char **ArrayCk { Py_CLEAR( tmp$argnum ); free( $1 );}

// With count. Always allow NULL strings.
%typemap ( in ) ( PLINT n, const char **Array ) ( PyArrayObject * tmp = NULL )
{
    int i;
    tmp = (PyArrayObject *) PyArray_ContiguousFromObject( $input, NPY_STRING, 1, 1 );
    if ( tmp == NULL )
        return NULL;
    Alen = PyArray_DIMS( tmp )[0];
    $1   = Alen;
    $2   = (char **) malloc( sizeof ( char* ) * (size_t) Alen );
    for ( i = 0; i < Alen; i++ )
    {
        $2[i] = (char *) PyArray_DATA( tmp ) + i * PyArray_STRIDES( tmp )[0];
        if ( $2[i] == NULL )
        {
            free( $2 );
            return NULL;
        }
    }
}
%typemap ( freearg ) ( PLINT n, const char **Array )
{
    Py_CLEAR( tmp$argnum ); free( $2 );
}

//**************************
//        String returning functions
//***************************

// This currently just used for plgdev, plgfnam, and plgver which apparently
// have a limit of 80 bytes.  But to (hopefully) be safe for any future use
// have a 1000 byte limit here.
%typemap( in, numinputs = 0 ) char *OUTPUT( char buff[1000] )
{
    $1 = buff;
}
%typemap( argout, fragment = "t_output_helper" ) char *OUTPUT {
    PyObject *o = PyString_FromString( $1 );
    $result = t_output_helper( $result, o );
}

//**************************
//        A trick for docstrings
//***************************

%define DOC( func, string )
%wrapper
%{
    # define _doc_    ## func string
%}
%enddef

// All code associated with special call-back functions.
// Identity transformation.

%typemap( in, numinputs = 0 ) PLPointer IGNORE {
    $1 = NULL;
}
void
pltr0( PLFLT x, PLFLT y, PLFLT *OUTPUT, PLFLT *OUTPUT, PLPointer IGNORE );

// This typemap takes care of converting a Python 2-tuple of 1D Arrays to the
//   PLcGrid structure that pltr1 expects
%wrapper
%{
    PyArrayObject   *pltr_xg, *pltr_yg;
    static PLcGrid  tmpGrid1;
    static PLcGrid2 tmpGrid2;

    PLcGrid* marshal_PLcGrid1( PyObject* input, int isimg );
    void cleanup_PLcGrid1( void );
    PLcGrid2* marshal_PLcGrid2( PyObject* input, int isimg );
    void cleanup_PLcGrid2( void );

    PLcGrid* marshal_PLcGrid1( PyObject* input, int isimg )
    {
        // fprintf(stderr, "marshal PLcGrid1\n");
        if ( !PySequence_Check( input ) || PySequence_Size( input ) != 2 )
        {
            PyErr_SetString( PyExc_ValueError, "Expected a sequence of two arrays." );
            return NULL;
        }
        pltr_xg = (PyArrayObject *) myArray_ContiguousFromObject( PySequence_Fast_GET_ITEM( input, 0 ),
            NPY_PLFLT, 1, 1 );
        pltr_yg = (PyArrayObject *) myArray_ContiguousFromObject( PySequence_Fast_GET_ITEM( input, 1 ),
            NPY_PLFLT, 1, 1 );
        if ( pltr_xg == 0 || pltr_yg == 0 )
        {
            PyErr_SetString( PyExc_ValueError, "Expected a sequence to two 1D arrays." );
            return NULL;
        }
        tmpGrid1.nx = (PLINT) PyArray_DIMS( pltr_xg )[0];
        tmpGrid1.ny = (PLINT) PyArray_DIMS( pltr_yg )[0];
        if ( isimg == 0 )
        {
            if ( Xlen != tmpGrid1.nx || Ylen != tmpGrid1.ny )
            {
                PyErr_SetString( PyExc_ValueError, "pltr arguments must have X and Y dimensions of first arg." );
                return NULL;
            }
        }
        else
        {
            if ( Xlen != tmpGrid1.nx - 1 || Ylen != tmpGrid1.ny - 1 )
            {
                PyErr_SetString( PyExc_ValueError, "pltr arguments must have X and Y dimensions of first arg + 1." );
                return NULL;
            }
        }
        tmpGrid1.xg = (PLFLT *) PyArray_DATA( pltr_xg );
        tmpGrid1.yg = (PLFLT *) PyArray_DATA( pltr_yg );
        return &tmpGrid1;
    }

    void cleanup_PLcGrid1( void )
    {
        // fprintf(stderr, "cleanup PLcGrid1\n");
        Py_CLEAR( pltr_xg );
        Py_CLEAR( pltr_yg );
    }

    PLcGrid2* marshal_PLcGrid2( PyObject* input, int isimg )
    {
        int i, size;
        // fprintf(stderr, "marshal PLcGrid2\n");
        if ( !PySequence_Check( input ) || PySequence_Size( input ) != 2 )
        {
            PyErr_SetString( PyExc_ValueError, "Expected a sequence of two arrays." );
            return NULL;
        }
        pltr_xg = (PyArrayObject *) myArray_ContiguousFromObject( PySequence_Fast_GET_ITEM( input, 0 ),
            NPY_PLFLT, 2, 2 );
        pltr_yg = (PyArrayObject *) myArray_ContiguousFromObject( PySequence_Fast_GET_ITEM( input, 1 ),
            NPY_PLFLT, 2, 2 );
        if ( pltr_xg == 0 || pltr_yg == 0 )
        {
            PyErr_SetString( PyExc_ValueError, "Expected a sequence of two 2D arrays." );
            return NULL;
        }
        if ( PyArray_DIMS( pltr_xg )[0] != PyArray_DIMS( pltr_yg )[0] ||
             PyArray_DIMS( pltr_xg )[1] != PyArray_DIMS( pltr_yg )[1] )
        {
            PyErr_SetString( PyExc_ValueError, "Arrays must be same size." );
            return NULL;
        }
        tmpGrid2.nx = (PLINT) PyArray_DIMS( pltr_xg )[0];
        tmpGrid2.ny = (PLINT) PyArray_DIMS( pltr_xg )[1];
        if ( isimg == 0 )
        {
            if ( Xlen != tmpGrid2.nx || Ylen != tmpGrid2.ny )
            {
                PyErr_SetString( PyExc_ValueError, "pltr arguments must have X and Y dimensions of first arg." );
                return NULL;
            }
        }
        else
        {
            if ( Xlen != tmpGrid2.nx - 1 || Ylen != tmpGrid2.ny - 1 )
            {
                PyErr_SetString( PyExc_ValueError, "pltr arguments must have X and Y dimensions of first arg + 1." );
                return NULL;
            }
        }
        size        = tmpGrid2.ny;
        tmpGrid2.xg = (PLFLT **) malloc( sizeof ( PLFLT* ) * (size_t) tmpGrid2.nx );
        for ( i = 0; i < tmpGrid2.nx; i++ )
            tmpGrid2.xg[i] = ( (PLFLT *) PyArray_DATA( pltr_xg ) + i * size );
        tmpGrid2.yg = (PLFLT **) malloc( sizeof ( PLFLT* ) * (size_t) tmpGrid2.nx );
        for ( i = 0; i < tmpGrid2.nx; i++ )
            tmpGrid2.yg[i] = ( (PLFLT *) PyArray_DATA( pltr_yg ) + i * size );
        return &tmpGrid2;
    }

    void cleanup_PLcGrid2( void )
    {
        // fprintf(stderr, "cleanup PLcGrid2\n");
        free( tmpGrid2.xg );
        free( tmpGrid2.yg );
        Py_CLEAR( pltr_xg );
        Py_CLEAR( pltr_yg );
    }
%}

%typemap( in ) PLcGrid * cgrid {
    $1 = marshal_PLcGrid1( $input, 0 );
    if ( !$1 )
        return NULL;
}
%typemap( freearg ) PLcGrid * cgrid {
    cleanup_PLcGrid1();
}

// Does linear interpolation from singly dimensioned coord arrays.
void
pltr1( PLFLT x, PLFLT y, PLFLT *OUTPUT, PLFLT *OUTPUT, PLcGrid* cgrid );

// This typemap marshals a Python 2-tuple of 2D arrays into the PLcGrid2
//   structure that pltr2 expects

%typemap( in ) PLcGrid2 * cgrid {
    $1 = marshal_PLcGrid2( $input, 0 );
    if ( !$1 )
        return NULL;
}
%typemap( freearg ) PLcGrid2 * cgrid {
    cleanup_PLcGrid2();
}

// Does linear interpolation from doubly dimensioned coord arrays
// (column dominant, as per normal C 2d arrays).
void
pltr2( PLFLT x, PLFLT y, PLFLT *OUTPUT, PLFLT *OUTPUT, PLcGrid2* cgrid );

typedef PLINT ( *defined_func )( PLFLT, PLFLT );
typedef void ( *fill_func )( PLINT, const PLFLT*, const PLFLT* );
typedef void ( *pltr_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
typedef void ( *ct_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
typedef void ( *mapform_func )( PLINT, PLFLT *, PLFLT* );
typedef PLFLT ( *f2eval_func )( PLINT, PLINT, PLPointer );
typedef void ( *label_func )( PLINT, PLFLT, char *, PLINT, PLPointer );

%{
    typedef PLINT ( *defined_func )( PLFLT, PLFLT );
    typedef void ( *fill_func )( PLINT, const PLFLT*, const PLFLT* );
    typedef void ( *pltr_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
    typedef void ( *ct_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
    typedef void ( *mapform_func )( PLINT, PLFLT *, PLFLT* );
    typedef PLFLT ( *f2eval_func )( PLINT, PLINT, PLPointer );
    typedef void ( *label_func )( PLINT, PLFLT, char *, PLINT, PLPointer );
%}

#if 0
// We need to get this PyThreadState structure initialized to use it for controlling
//   threads on the callback. Probably not *really* necessary since I'm not allowing
//   threads through the call

%init
%{
    save_interp = PyThreadState_Get()->interp;
%}

#endif

%wrapper
%{
    // helper code for handling the callback
#if 0
    static PyInterpreterState *save_interp = NULL;
#endif
    enum callback_type { CB_0, CB_1, CB_2, CB_Python } pltr_type;
    PyObject* python_pltr    = NULL;
    PyObject* python_f2eval  = NULL;
    PyObject* python_ct      = NULL;
    PyObject* python_mapform = NULL;
    PyObject* python_label   = NULL;

#if 0
#define MY_BLOCK_THREADS    {                             \
        PyThreadState *prev_state, *new_state;            \
        /* need to have started a thread at some stage */ \
        /* for the following to work */                   \
        PyEval_AcquireLock();                             \
        new_state  = PyThreadState_New( save_interp );    \
        prev_state = PyThreadState_Swap( new_state );
#define MY_UNBLOCK_THREADS                        \
    new_state = PyThreadState_Swap( prev_state ); \
    PyThreadState_Clear( new_state );             \
    PyEval_ReleaseLock();                         \
    PyThreadState_Delete( new_state );            \
    }
#else
#define MY_BLOCK_THREADS
#define MY_UNBLOCK_THREADS
#endif

// Function prototypes
    void do_pltr_callback( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, PLPointer data );
    PLFLT do_f2eval_callback( PLINT x, PLINT y, PLPointer data );
    void do_label_callback( PLINT axis, PLFLT value, char *string, PLINT len, PLPointer data );
    void do_ct_callback( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer data );
    void do_mapform_callback( PLINT n, PLFLT *x, PLFLT *y );
    pltr_func marshal_pltr( PyObject* input );
    void cleanup_pltr( void );
    ct_func marshal_ct( PyObject* input );
    void cleanup_ct( void );
    mapform_func marshal_mapform( PyObject* input );
    void cleanup_mapform( void );
    PLPointer marshal_PLPointer( PyObject* input, int isimg );
    void cleanup_PLPointer( void );


// This is the callback that gets handed to the C code. It, in turn, calls the Python callback

    void do_pltr_callback( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, PLPointer data )
    {
        PyObject      *pdata, *arglist, *result;
        PyArrayObject *tmp;

        // the data argument is acutally a pointer to a python object
        pdata = (PyObject *) data;
        if ( data == NULL )
        {
            pdata = Py_None;
        }
        if ( python_pltr ) // if not something is terribly wrong
        {                  // hold a reference to the data object
            Py_XINCREF( pdata );
            // grab the Global Interpreter Lock to be sure threads don't mess us up
            MY_BLOCK_THREADS
            // build the argument list
#ifdef PL_DOUBLE
            arglist = Py_BuildValue( "(ddO)", x, y, pdata );
#else
            arglist = Py_BuildValue( "(ffO)", x, y, pdata );
#endif
            if ( arglist == NULL )
            {
                fprintf( stderr, "Py_BuildValue failed to make argument list.\n" );
                *tx = *ty = 0;
                return;
            }
            // call the python function
            result = PyEval_CallObject( python_pltr, arglist );
            // release the argument list
            Py_CLEAR( arglist );
            // check and unpack the result
            if ( result == NULL )
            {
                fprintf( stderr, "call to python pltr function with 3 arguments failed\n" );
                PyErr_SetString( PyExc_RuntimeError, "pltr callback must take 3 arguments." );
                *tx = *ty = 0;
            }
            else
            {
                tmp = (PyArrayObject *) myArray_ContiguousFromObject( result, NPY_PLFLT, 1, 1 );
                if ( tmp == 0 || PyArray_DIMS( tmp )[0] != 2 )
                {
                    fprintf( stderr, "pltr callback must return a 2 element array or sequence\n" );
                    PyErr_SetString( PyExc_RuntimeError, "pltr callback must return a 2-sequence." );
                    *tx = *ty = 0;
                }
                else
                {
                    PLFLT* t = (PLFLT *) PyArray_DATA( tmp );
                    *tx = t[0];
                    *ty = t[1];
                    Py_CLEAR( tmp );
                }
            }
            // release the result
            Py_CLEAR( result );
            // release the global interpreter lock
            MY_UNBLOCK_THREADS
        }
    }

    PLFLT do_f2eval_callback( PLINT x, PLINT y, PLPointer data )
    {
        PyObject *pdata, *arglist, *result;
        PLFLT    fresult = 0.0;

        // the data argument is acutally a pointer to a python object
        pdata = (PyObject *) data;
        if ( python_f2eval ) // if not something is terribly wrong
        {                    // hold a reference to the data object
            Py_XINCREF( pdata );
            // grab the Global Interpreter Lock to be sure threads don't mess us up
            MY_BLOCK_THREADS
            // build the argument list
                arglist = Py_BuildValue( "(iiO)", x, y, pdata );
            // call the python function
            result = PyEval_CallObject( python_f2eval, arglist );
            // release the argument list
            Py_CLEAR( arglist );
            // check and unpack the result
            if ( !PyFloat_Check( result ) )
            {
                fprintf( stderr, "f2eval callback must return a float\n" );
                PyErr_SetString( PyExc_RuntimeError, "f2eval callback must return a float." );
            }
            else
            {
                // should I test the type here?
                fresult = (PLFLT) PyFloat_AsDouble( result );
            }
            // release the result
            Py_CLEAR( result );
            // release the global interpreter lock
            MY_UNBLOCK_THREADS
        }
        return fresult;
    }

    void do_label_callback( PLINT axis, PLFLT value, char *string, PLINT len, PLPointer data )
    {
        PyObject *pdata, *arglist, *result, *unicode_string;
        char     *pystring;

        // the data argument is acutally a pointer to a python object
        if ( data )
            pdata = (PyObject *) data;
        else
            pdata = Py_None;
        if ( python_label ) // if not something is terribly wrong
        {                   // hold a reference to the data object
            Py_XINCREF( pdata );
            // grab the Global Interpreter Lock to be sure threads don't mess us up
            MY_BLOCK_THREADS
            // build the argument list
#ifdef PL_DOUBLE
            arglist = Py_BuildValue( "(ldO)", axis, value, pdata );
#else
            arglist = Py_BuildValue( "(lfO)", axis, value, pdata );
#endif
            // call the python function
            result = PyEval_CallObject( python_label, arglist );
            // release the argument list
            //Py_CLEAR(arglist);
            // check and unpack the result
            if ( result == NULL )
            {
                fprintf( stderr, "label callback failed with 3 arguments\n" );
                PyErr_SetString( PyExc_RuntimeError, "label callback must take 3 arguments." );
            }
            else if ( PyString_Check( result ) )
            {
                // should I test the type here?
                pystring = PyString_AsString( result );
                strncpy( string, pystring, len );
            }
            else if ( PyUnicode_Check( result ) )
            {
                // unicode_string is never freed? memory leak here?
                unicode_string = PyUnicode_AsEncodedString( result, "utf-8", "Error ~" );
                pystring       = PyBytes_AS_STRING( unicode_string );
                // len may be different then the byte string length w/ unicode?
                strncpy( string, pystring, len );
            }
            else
            {
                fprintf( stderr, "label callback must return a string\n" );
                PyErr_SetString( PyExc_RuntimeError, "label callback must return a string." );
            }
            // release the result
            Py_CLEAR( result );
            // release the global interpreter lock
            MY_UNBLOCK_THREADS
        }
    }

    void do_ct_callback( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer data )
    {
        PyObject *px, *py, *pdata, *arglist, *result;
        npy_intp n;
        n = 1;

        // the data argument is acutally a pointer to a python object
        pdata = (PyObject *) data;
        if ( data == NULL )
        {
            pdata = Py_None;
        }
        if ( python_ct ) // if not something is terribly wrong
        {                // hold a reference to the data object
            Py_XINCREF( pdata );
            // grab the Global Interpreter Lock to be sure threads don't mess us up
            MY_BLOCK_THREADS
            // build the argument list
                px = PyArray_SimpleNewFromData( 1, &n, NPY_PLFLT, (void *) xt );
            py      = PyArray_SimpleNewFromData( 1, &n, NPY_PLFLT, (void *) yt );
            arglist = Py_BuildValue( "(ddOOO)", x, y, px, py, pdata );
            // call the python function
            result = PyEval_CallObject( python_ct, arglist );
            // release the argument list
            Py_CLEAR( arglist );
            Py_CLEAR( px );
            Py_CLEAR( py );
            Py_CLEAR( pdata );
            // check and unpack the result
            if ( result == NULL )
            {
                fprintf( stderr, "call to python coordinate transform function with 5 arguments failed\n" );
                PyErr_SetString( PyExc_RuntimeError, "coordinate transform callback must take 5 arguments." );
            }
            // release the result
            Py_CLEAR( result );
            // release the global interpreter lock
            MY_UNBLOCK_THREADS
        }
    }

    void do_mapform_callback( PLINT n, PLFLT *x, PLFLT *y )
    {
        PyObject *px, *py, *arglist, *result;
        // PyArrayObject *tmpx, *tmpy;
//        PLFLT *xx, *yy;
//        PLINT i;
        npy_intp nn;
        nn = n;

        if ( python_mapform ) // if not something is terribly wrong
        {                     // grab the Global Interpreter Lock to be sure threads don't mess us up
            MY_BLOCK_THREADS
            // build the argument list
#ifdef PL_HAVE_PTHREAD
            px = PyArray_SimpleNewFromData( 1, &nn, NPY_PLFLT, (void *) x );
            py = PyArray_SimpleNewFromData( 1, &nn, NPY_PLFLT, (void *) y );
#else
            px = PyArray_FromDimsAndData( 1, &n, NPY_PLFLT, (char *) x );
            py = PyArray_FromDimsAndData( 1, &n, NPY_PLFLT, (char *) y );
#endif
            arglist = Py_BuildValue( "(iOO)", n, px, py );
            // call the python function
            result = PyEval_CallObject( python_mapform, arglist );
            // release the argument list
            Py_CLEAR( arglist );
            Py_CLEAR( px );
            Py_CLEAR( py );
            // check and unpack the result
            if ( result == NULL )
            {
                fprintf( stderr, "call to python mapform function with 3 arguments failed\n" );
                PyErr_SetString( PyExc_RuntimeError, "mapform callback must take 3 arguments." );
            }
            // release the result
            Py_CLEAR( result );
            // release the global interpreter lock
            MY_UNBLOCK_THREADS
        }
    }

// marshal the pltr function pointer argument
    pltr_func marshal_pltr( PyObject* input )
    {
        pltr_func result = do_pltr_callback;
        PyObject  * rep  = PyObject_Repr( input );
        if ( rep )
        {
            // Memory leaks here? str and uni_str are not freed?
            char* str;
            if ( PyUnicode_Check( rep ) )
            {
                PyObject *uni_str = PyUnicode_AsEncodedString( rep, "utf-8", "Error ~" );
                str = PyBytes_AS_STRING( uni_str );
            }
            else
            {
                str = PyString_AsString( rep );
            }
            if ( strstr( str, "function pltr0" ) != 0 )
            {
                result      = pltr0;
                pltr_type   = CB_0;
                python_pltr = NULL;
            }
            else if ( strstr( str, "function pltr1" ) != 0 )
            {
                result      = pltr1;
                pltr_type   = CB_1;
                python_pltr = NULL;
            }
            else if ( strstr( str, "function pltr2" ) != 0 )
            {
                result      = pltr2;
                pltr_type   = CB_2;
                python_pltr = NULL;
            }
            else
            {
                python_pltr = input;
                pltr_type   = CB_Python;
                Py_XINCREF( input );
            }
            Py_CLEAR( rep );
        }
        else
        {
            python_pltr = input;
            pltr_type   = CB_Python;
            Py_XINCREF( input );
        }
        return result;
    }

    void cleanup_pltr( void )
    {
        Py_CLEAR( python_pltr );
        python_pltr = 0;
    }

// marshal the ct function pointer argument
    ct_func marshal_ct( PyObject* input )
    {
        ct_func result = do_ct_callback;
        python_ct = input;
        Py_XINCREF( input );
        return result;
    }

    void cleanup_ct( void )
    {
        Py_CLEAR( python_ct );
        python_ct = 0;
    }

// marshal the mapform function pointer argument
    mapform_func marshal_mapform( PyObject* input )
    {
        mapform_func result = do_mapform_callback;
        python_mapform = input;
        Py_XINCREF( input );
        return result;
    }

    void cleanup_mapform( void )
    {
        Py_CLEAR( python_mapform );
        python_mapform = 0;
    }

    PLPointer marshal_PLPointer( PyObject* input, int isimg )
    {
        PLPointer result = NULL;
        switch ( pltr_type )
        {
        case CB_0:
            break;
        case CB_1:
            if ( input != Py_None )
                result = marshal_PLcGrid1( input, isimg );
            break;
        case CB_2:
            if ( input != Py_None )
                result = marshal_PLcGrid2( input, isimg );
            break;
        case CB_Python:
            Py_XINCREF( input );
            result = (PLPointer *) input;
            break;
        default:
            fprintf( stderr, "pltr_type is invalid\n" );
        }
        return result;
    }

    void cleanup_PLPointer( void )
    {
        switch ( pltr_type )
        {
        case CB_0:
            break;
        case CB_1:
            cleanup_PLcGrid1();
            break;
        case CB_2:
            cleanup_PLcGrid2();
            break;
        case CB_Python:
            Py_CLEAR( python_pltr );
            break;
        default:
            fprintf( stderr, "pltr_type is invalid\n" );
        }
        python_pltr = 0;
        pltr_type   = CB_0;
    }


%}

%typemap( in ) pltr_func pltr {
    // it must be a callable or None
    if ( $input == Py_None )
    {
        $1 = NULL;
    }
    else
    {
        if ( !PyCallable_Check( (PyObject *) $input ) )
        {
            PyErr_SetString( PyExc_ValueError, "pltr argument must be callable" );
            return NULL;
        }
        $1 = marshal_pltr( $input );
    }
}
%typemap( freearg ) pltr_func pltr {
    cleanup_pltr();
}

// you can omit the pltr func
%typemap( default ) pltr_func pltr {
    python_pltr = 0;
    $1          = NULL;
}

%typemap( in ) ct_func ctf {
    if ( python_ct )
        cleanup_ct();
    // it must be a callable or none
    if ( $input == Py_None )
    {
        $1 = NULL;
    }
    else
    {
        if ( !PyCallable_Check( (PyObject *) $input ) )
        {
            PyErr_SetString( PyExc_ValueError, "coordinate transform argument must be callable" );
            return NULL;
        }
        $1 = marshal_ct( $input );
    }
}

// you can omit the ct func
%typemap( default ) ct_func ctf {
    python_ct = 0;
    $1        = NULL;
}

%typemap( in ) mapform_func mapform {
    // it must be a callable or none
    if ( $input == Py_None )
    {
        $1 = NULL;
    }
    else
    {
        if ( !PyCallable_Check( (PyObject *) $input ) )
        {
            PyErr_SetString( PyExc_ValueError, "mapform argument must be callable" );
            return NULL;
        }
        $1 = marshal_mapform( $input );
    }
}
%typemap( freearg ) mapform_func mapform {
    cleanup_mapform();
}


// you cannot omit the mapform func since it appears at the beginning of API calls, before compulsory arguments
//%typemap( default ) mapform_func mapform {
//    python_mapform = 0;
//    $1             = NULL;
//}

// convert an arbitrary Python object into the void* pointer they want
%typemap( in ) PLPointer PYOBJECT_DATA {
    if ( $input == Py_None )
        $1 = NULL;
    else
    {
        $1 = marshal_PLPointer( $input, 0 );
    }
}
%typemap( freearg ) PLPointer PYOBJECT_DATA {
    cleanup_PLPointer();
}

// you can omit the data too
%typemap( default ) PLPointer PYOBJECT_DATA {
    $1 = NULL;
}

// convert an arbitrary Python object into the void* pointer they want
%typemap( in ) PLPointer PYOBJECT_DATA_img {
    if ( $input == Py_None )
        $1 = NULL;
    else
    {
        $1 = marshal_PLPointer( $input, 1 );
    }
}
%typemap( freearg ) PLPointer PYOBJECT_DATA_img {
    cleanup_PLPointer();
}

// you can omit the data too
%typemap( default ) PLPointer PYOBJECT_DATA_img {
    $1 = NULL;
}

// marshall the f2eval function pointer argument
%typemap( in ) f2eval_func f2eval {
    // it must be a callable or None
    if ( $input == Py_None )
    {
        $1 = NULL;
    }
    else
    {
        if ( !PyCallable_Check( (PyObject *) $input ) )
        {
            PyErr_SetString( PyExc_ValueError, "pltr argument must be callable" );
            return NULL;
        }
        // hold a reference to it
        Py_XINCREF( (PyObject *) $input );
        python_f2eval = (PyObject *) $input;
        // this function handles calling the python function
        $1 = do_f2eval_callback;
    }
}
%typemap( freearg ) f2eval_func f2eval {
    Py_CLEAR( python_f2eval );
    python_f2eval = 0;
}
// marshall the label function pointer argument
%typemap( in ) label_func lf {
    // Release reference to previous function if applicable
    if ( python_label )
    {
        Py_CLEAR( python_label );
        python_label = 0;
    }
    // it must be a callable or None
    if ( $input == Py_None )
    {
        $1 = NULL;
    }
    else
    {
        if ( !PyCallable_Check( (PyObject *) $input ) )
        {
            PyErr_SetString( PyExc_ValueError, "label_func argument must be callable" );
            return NULL;
        }
        // hold a reference to it
        Py_XINCREF( (PyObject *) $input );
        python_label = (PyObject *) $input;
        // this function handles calling the python function
        $1 = do_label_callback;
    }
}
%typemap( in, numinputs = 0 ) defined_func df {
    $1 = NULL;
}
%typemap( in, numinputs = 0 ) fill_func ff {
    $1 = plfill;
}
// this typemap takes a sequence of strings and converts them for plstripc
//   also checks that previous Arrays were of length 4
//
%typemap( in ) const char *legline[4] ( char** tmp = NULL )
{
    int      i;
    PyObject *elt, *unicode_string;

    if ( !PySequence_Check( $input ) || PySequence_Size( $input ) != 4 )
    {
        PyErr_SetString( PyExc_ValueError, "Requires a sequence of 4 strings." );
        return NULL;
    }
    if ( Alen != 4 )
    {
        PyErr_SetString( PyExc_ValueError, "colline and styline args must be length 4." );
        return NULL;
    }
    tmp = (char **) malloc( sizeof ( char* ) * 4 );
    if ( tmp == NULL )
        return NULL;
    $1 = tmp;
    for ( i = 0; i < 4; i++ )
    {
        $1[i] = NULL;
        elt   = PySequence_Fast_GET_ITEM( $input, i );
        if ( PyString_Check( elt ) )
        {
            $1[i] = PyString_AsString( elt );
        }
        else if ( PyUnicode_Check( elt ) )
        {
            unicode_string = PyUnicode_AsEncodedString( elt, "utf-8", "Error ~" );
            $1[i]          = PyBytes_AS_STRING( unicode_string );
        }
        if ( $1[i] == NULL )
        {
            free( tmp );
            return NULL;
        }
    }
}
%typemap( freearg ) const char *legline[4] {
    free( tmp$argnum );
}

// End of all code associated with special call-back functions.

// Process options list using current options info.
%typemap ( in ) ( int *p_argc, char **argv ) ( int tmp )
{
    int      i;
    PyObject *unicode_string;

    if ( !PyList_Check( $input ) )
    {
        PyErr_SetString( PyExc_ValueError, "Expecting a list" );
        return NULL;
    }
    tmp = PyList_Size( $input );
    $1  = &tmp;
    $2  = (char **) malloc( (size_t) ( tmp + 1 ) * sizeof ( char * ) );
    for ( i = 0; i < tmp; i++ )
    {
        PyObject *s = PyList_GetItem( $input, i );
        if ( PyString_Check( s ) )
        {
            $2[i] = PyString_AsString( s );
        }
        else if ( PyUnicode_Check( s ) )
        {
            // unicode_string is never freed? memory leak here?
            unicode_string = PyUnicode_AsEncodedString( s, "utf-8", "Error ~" );
            $2[i]          = PyBytes_AS_STRING( unicode_string );
        }
        else
        {
            free( $2 );
            PyErr_SetString( PyExc_ValueError, "List items must be strings" );
            return NULL;
        }
    }
    $2[i] = 0;
}

%typemap ( freearg ) ( int *p_argc, char **argv )
{
    if ( $2 )
        free( $2 );
}


#if 0
%typemap( in ) PLGraphicsIn * gin( PLGraphicsIn tmp )
{
    if ( !PySequence_Check( $input ) || PySequence_Size( $input ) != 2 )
    {
        PyErr_SetString( PyExc_ValueError, "Expecting a sequence of 2 numbers." );
        return NULL;
    }
    $1     = &tmp;
    $1->dX = PyFloat_AsDouble( PySequence_Fast_GET_ITEM( $input, 0 ) );
    $1->dY = PyFloat_AsDouble( PySequence_Fast_GET_ITEM( $input, 1 ) );
}
%typemap( argout ) PLGraphicsIn * gin {
    PyObject *o;
    o         = PyFloat_FromDouble( $1->wX );
    resultobj = t_output_helper( resultobj, o );
    o         = PyFloat_FromDouble( $1->wY );
    resultobj = t_output_helper( resultobj, o );
}
#endif

#ifdef PYTHON_HAVE_PYBUFFER
%include <pybuffer.i>
%pybuffer_mutable_string( void * plotmem )
#endif

//%feature commands supporting swig-generated documentation for the bindings.
%include swig_documentation.i
// swig-compatible PLplot API definitions from here on.
%include plplotcapi.i
