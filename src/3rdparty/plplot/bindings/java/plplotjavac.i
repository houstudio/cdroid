//
//Copyright (C) 2002  Gary Bishop
//Copyright (C) 2002, 2004  Alan W. Irwin
//Copyright (C) 2004-2010  Andrew Ross
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
//A SWIG interface to PLplot for Java. This wrapper does the following:
//
//   1) it strictly provides the C-API with the usual change of not
//      requiring lengths for arrays,
//
//   2) it attempts to provide the entire API *excluding* callbacks for
//      plcont and plshade(s) (for now).
//
//   3) it works both with the single and double-precision versions of the
//      PLplot library.
//
//This is known to work with swig-1.3.21.
//
//
%module plplotjavac
%include typemaps.i

%{
#include "plplotP.h"
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

// Set jni version and cache JVM - needed for callbacks
%{
    static JavaVM *cached_jvm = NULL;

    SWIGEXPORT JNIEXPORT jint JNICALL JNI_OnLoad( JavaVM *jvm, void * PL_UNUSED( reserved ) )
    {
        cached_jvm = jvm;
        return JNI_VERSION_1_2;
    }
%}

// Simple (input) PLBOOL arguments
// Use macro style similar to INPUT_TYPEMAP defined in typemaps.i, but
// actually follow what is done in java.swg for bool C type except
// change action of typemap(in) from "? true : false;" to "? 1 : 0;" to
// be consistent with actual C type of PLBOOL which is PLINT.  If C type
// of PLBOOL ever changed to bool, none of this would be necessary, but
// such a change would demand using the c99 standard for PLplot which is
// not widely implemented yet.
//
%define PLBOOL_INPUT_TYPEMAP( TYPE, JNITYPE, JTYPE, JNIDESC )
%typemap( jni ) TYPE "JNITYPE"
%typemap( jtype ) TYPE "JTYPE"
%typemap( jstype ) TYPE "JTYPE"

%typemap( in ) TYPE
%{
    $1 = $input ? 1 : 0;
%}

%typemap( javadirectorin ) TYPE "$jniinput"
%typemap( javadirectorout ) TYPE "$javacall"

%typemap( directorin, descriptor = JNIDESC ) TYPE
%{
    $input = (JNITYPE *) $1;
%}

%typemap( javain ) TYPE "$javainput"
%enddef

PLBOOL_INPUT_TYPEMAP( PLBOOL, jboolean, boolean, "Z" );

// This renamed macro copied exactly from OUTPUT_TYPEMAP macro
// in typemaps.i which handles *OUTPUT types.

%define PLBOOL_OUTPUT_TYPEMAP( TYPE, JNITYPE, JTYPE, JAVATYPE, JNIDESC, TYPECHECKTYPE )
%typemap( jni ) TYPE * OUTPUT, TYPE & OUTPUT
%{
    JNITYPE ## Array
%}
%typemap( jtype ) TYPE * OUTPUT, TYPE & OUTPUT "JTYPE[]"
%typemap( jstype ) TYPE * OUTPUT, TYPE & OUTPUT "JTYPE[]"
%typemap( javain ) TYPE * OUTPUT, TYPE & OUTPUT "$javainput"
%typemap( javadirectorin ) TYPE * OUTPUT, TYPE & OUTPUT "$jniinput"
%typemap( javadirectorout ) TYPE * OUTPUT, TYPE & OUTPUT "$javacall"

%typemap( in ) TYPE * OUTPUT( $*1_ltype temp ), TYPE &OUTPUT( $*1_ltype temp )
{
    if ( !$input )
    {
        SWIG_JavaThrowException( jenv, SWIG_JavaNullPointerException, "array null" );
        return $null;
    }
    if ( JCALL1( GetArrayLength, jenv, $input ) == 0 )
    {
        SWIG_JavaThrowException( jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element" );
        return $null;
    }
    $1 = &temp;
}

%typemap( directorin, descriptor = JNIDESC ) TYPE & OUTPUT
%{
    *( ( $&1_ltype )$input ) = &$1;
%}

%typemap( directorin, descriptor = JNIDESC, warning = "Need to provide TYPE *OUTPUT directorin typemap, TYPE array length is unknown" ) TYPE * OUTPUT
{
}

%typemap( freearg ) TYPE * OUTPUT, TYPE & OUTPUT ""

%typemap( argout ) TYPE * OUTPUT, TYPE & OUTPUT
{
    JNITYPE jvalue = (JNITYPE) temp$argnum;
    JCALL4( Set ## JAVATYPE ## ArrayRegion, jenv, $input, 0, 1, &jvalue );
}

%typemap( typecheck ) TYPE * INOUT = TYPECHECKTYPE;
%typemap( typecheck ) TYPE &INOUT  = TYPECHECKTYPE;
%enddef

// Copy what is done for C bool type, only use PLBOOL instead.
PLBOOL_OUTPUT_TYPEMAP( PLBOOL, jboolean, boolean, Boolean, "[Ljava/lang/Boolean;", jbooleanArray );

//**************************
//        A trick for docstrings
//***************************

%define DOC( func, string )
%wrapper
%{
    # define _doc_    ## func string
%}
%enddef

// Infrastructure for handling swig compatible plplot API definitions.

#ifdef PL_DOUBLE_INTERFACE
#define setup_array_1d_PLFLT         setup_array_1d_d
#define setup_array_2d_PLFLT         setup_array_2d_d
#define jPLFLTArray                  "jdoubleArray"
#define jPLFLTbracket                "double[]"
#define jPLFLTbracket2               "double[][]"
#define GetPLFLTArrayElements        GetDoubleArrayElements
#define ReleasePLFLTArrayElements    ReleaseDoubleArrayElements
#define jPLFLT                       jdouble
#else
#define setup_array_1d_PLFLT         setup_array_1d_f
#define setup_array_2d_PLFLT         setup_array_2d_f
#define jPLFLTArray                  "jfloatArray"
#define jPLFLTbracket                "float[]"
#define jPLFLTbracket2               "float[][]"
#define GetPLFLTArrayElements        GetFloatArrayElements
#define ReleasePLFLTArrayElements    ReleaseFloatArrayElements
#define jPLFLT                       jfloat
#endif

%{
//--------------------------------------------------------------------------
// Array allocation & copy helper routines.  Note because of swig limitations
// it is necessary to release the java array memory right after calling these
// routines.  Thus it is necessary to allocate and copy the arrays  even if
// the java and plplot arrays are of the same type.  Note, because of this
// change to Geoffrey's original versions, caller must always free memory
// afterwards.  Thus, the must_free_buffers logic is gone as well.
//--------------------------------------------------------------------------

// 1d array of jbooleans

    static void
    setup_array_1d_b( PLBOOL **pa, jboolean *adat, int n )
    {
        int i;
        *pa = (PLBOOL *) malloc( (size_t) n * sizeof ( PLBOOL ) );
        for ( i = 0; i < n; i++ )
        {
            ( *pa )[i] = adat[i] ? 1 : 0;
        }
    }

// 1d array of jints

    static void
    setup_array_1d_i( PLINT **pa, jint *adat, int n )
    {
        int i;
        *pa = (PLINT *) malloc( (size_t) n * sizeof ( PLINT ) );
        for ( i = 0; i < n; i++ )
        {
            ( *pa )[i] = adat[i];
        }
    }

// 1d array of jfloats

    static void
    setup_array_1d_f( PLFLT **pa, jfloat *adat, int n )
    {
        int i;
        *pa = (PLFLT *) malloc( (size_t) n * sizeof ( PLFLT ) );
        for ( i = 0; i < n; i++ )
        {
            ( *pa )[i] = adat[i];
        }
    }

// 1d array of jdoubles

    static void
    setup_array_1d_d( PLFLT **pa, jdouble *adat, int n )
    {
        int i;
        *pa = (PLFLT *) malloc( (size_t) n * sizeof ( PLFLT ) );
        for ( i = 0; i < n; i++ )
        {
            ( *pa )[i] = adat[i];
        }
    }

// 2d array of floats
// Here caller must free(a[0]) and free(a) (in that order) afterward

    static void
    setup_array_2d_f( PLFLT ***pa, jfloat **adat, int nx, int ny )
    {
        int i, j;

        *pa        = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        ( *pa )[0] = (PLFLT *) malloc( (size_t) ( nx * ny ) * sizeof ( PLFLT ) );

        for ( i = 0; i < nx; i++ )
        {
            ( *pa )[i] = ( *pa )[0] + i * ny;
            for ( j = 0; j < ny; j++ )
                ( *pa )[i][j] = adat[i][j];
        }
    }

// 2d array of doubles
// Here caller must free(a[0]) and free(a) (in that order) afterward

    static void
    setup_array_2d_d( PLFLT ***pa, jdouble **adat, int nx, int ny )
    {
        int i, j;

        *pa        = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        ( *pa )[0] = (PLFLT *) malloc( (size_t) ( nx * ny ) * sizeof ( PLFLT ) );

        for ( i = 0; i < nx; i++ )
        {
            ( *pa )[i] = ( *pa )[0] + i * ny;
            for ( j = 0; j < ny; j++ )
                ( *pa )[i][j] = adat[i][j];
        }
    }

// Setup java arrays (for callback functions)

// Create a jdoubleArray and fill it from the C PLFLT array dat
    static jdoubleArray
    setup_java_array_1d_PLFLT( JNIEnv *jenv, PLFLT *dat, PLINT n )
    {
        double       *x;
        jdoubleArray jadat;
#ifdef PL_DOUBLE
        x = (double *) dat;
#else
        x = (double *) malloc( (size_t) n * sizeof ( double ) );
        for ( i = 0; i < n; i++ )
        {
            x[i] = (double) dat[i];
        }
#endif
        jadat = ( *jenv )->NewDoubleArray( jenv, n );
        ( *jenv )->SetDoubleArrayRegion( jenv, jadat, 0, n, x );
#ifndef PL_DOUBLE
        free( x );
#endif
        return jadat;
    }

// Copy back data from jdoubleArray to C PLFLT array then release java array
    static void
    release_java_array_1d_PLFLT( JNIEnv *jenv, jdoubleArray jadat, PLFLT *dat, PLINT n )
    {
        PLINT   i;
        jdouble *jdata = ( *jenv )->GetDoubleArrayElements( jenv, jadat, 0 );
        for ( i = 0; i < n; i++ )
        {
            dat[i] = (PLFLT) jdata[i];
        }
        ( *jenv )->ReleaseDoubleArrayElements( jenv, jadat, jdata, 0 );
    }

%}



// I hate global variables but this is the best way I can think of to manage consistency
//   checking among function arguments.
%{
    static PLINT    Alen = 0;
    static PLINT    Xlen = 0, Ylen = 0;
    static PLFLT    **xg;
    static PLFLT    **yg;
    static PLcGrid2 *cgrid;
%}

// The following typemaps take care of marshaling values into and out of PLplot functions. The
//Array rules are trickly because of the need for length checking. These rules manage
//some global variables (above) to handle consistency checking amoung parameters.
//
//Naming rules:
//        Array           (sets Alen to dim[0])
//        ArrayCk         (tests that dim[0] == Alen)
//        ArrayCkNull     (tests that dim[0] == Alen or array is null)
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

// with preceding count
%typemap ( in ) ( PLINT n, const PLINT * Array )
{
    jint *jxdata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
    $1   = ( *jenv )->GetArrayLength( jenv, $input );
    Alen = $1;
    setup_array_1d_i( &$2, jxdata, Alen );
    // Could find no easy way to do this as part of freearg so I modified
    // the previous function so it ALWAYS mallocs and copies so that
    // the java array can be released immediately.
    ( *jenv )->ReleaseIntArrayElements( jenv, $input, jxdata, 0 );
}
%typemap ( freearg ) ( PLINT n, const PLINT * Array )
{
    free( $2 );
}
%typemap ( jni ) ( PLINT n, const PLINT * Array ) "jintArray"
%typemap ( jtype ) ( PLINT n, const PLINT * Array ) "int[]"
%typemap ( jstype ) ( PLINT n, const PLINT * Array ) "int[]"
%typemap ( javain ) ( PLINT n, const PLINT * Array ) "$javainput"
%typemap ( javaout ) ( PLINT n, const PLINT * Array )
{
    return $jnicall;
}

// Trailing count and check consistency with previous
%typemap ( in ) ( const PLINT * ArrayCk, PLINT n )
{
    jint *jydata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) != Alen )
    {
        printf( "Vectors must be same length.\n" );
        return;
    }
    $2 = ( *jenv )->GetArrayLength( jenv, $input );
    setup_array_1d_i( &$1, jydata, Alen );
    ( *jenv )->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap ( freearg ) ( const PLINT * ArrayCk, PLINT n )
{
    free( $1 );
}
%typemap ( jni ) ( const PLINT * ArrayCk, PLINT n ) "jintArray"
%typemap ( jtype ) ( const PLINT * ArrayCk, PLINT n ) "int[]"
%typemap ( jstype ) ( const PLINT * ArrayCk, PLINT n ) "int[]"
%typemap ( javain ) ( const PLINT * ArrayCk, PLINT n ) "$javainput"
%typemap ( javaout ) ( const PLINT * ArrayCk, PLINT n )
{
    return $jnicall;
}

// no count but check consistency with previous
%typemap( in ) const PLINT * ArrayCk {
    jint *jydata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) != Alen )
    {
        printf( "Vectors must be same length.\n" );
        return;
    }
    setup_array_1d_i( &$1, jydata, Alen );
    ( *jenv )->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap( freearg ) const PLINT * ArrayCk {
    free( $1 );
}
%typemap( jni ) const PLINT * ArrayCk "jintArray"
%typemap( jtype ) const PLINT * ArrayCk "int[]"
%typemap( jstype ) const PLINT * ArrayCk "int[]"
%typemap( javain ) const PLINT * ArrayCk "$javainput"
%typemap( javaout ) const PLINT * ArrayCk {
    return $jnicall;
}

// no count but check consistency with previous or is null
%typemap( in ) const PLINT * ArrayCkNull {
    if ( $input != NULL )
    {
        jint *jydata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
        if ( ( *jenv )->GetArrayLength( jenv, $input ) != Alen )
        {
            printf( "Vectors must be same length.\n" );
            return;
        }
        setup_array_1d_i( &$1, jydata, Alen );
        ( *jenv )->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
    }
    else
    {
        $1 = NULL;
    }
}
%typemap( freearg ) const PLINT * ArrayCkNull {
    if ( $1 != NULL )
        free( $1 );
}
%typemap( jni ) const PLINT * ArrayCkNull "jintArray"
%typemap( jtype ) const PLINT * ArrayCkNull "int[]"
%typemap( jstype ) const PLINT * ArrayCkNull "int[]"
%typemap( javain ) const PLINT * ArrayCkNull "$javainput"
%typemap( javaout ) const PLINT * ArrayCkNull {
    return $jnicall;
}

// Weird case to allow argument to be one shorter than others
// This case is used both for PLBOOL and PLINT.  Define PLBOOL version
// first.  (AWI thinks this may be necessary because of the above
// typedef PLINT PLBOOL;)
// Also add version which must be one shorter than others or null.
//
%typemap( in ) const PLBOOL * ArrayCkMinus1 {
    jboolean *jydata = ( *jenv )->GetBooleanArrayElements( jenv, $input, 0 );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) < Alen - 1 )
    {
        printf( "Vector must be at least length of others minus 1.\n" );
        return;
    }
    setup_array_1d_b( &$1, jydata, Alen );
    ( *jenv )->ReleaseBooleanArrayElements( jenv, $input, jydata, 0 );
}
%typemap( freearg ) const PLBOOL * ArrayCkMinus1 {
    free( $1 );
}
%typemap( jni ) const PLBOOL * ArrayCkMinus1 "jbooleanArray"
%typemap( jtype ) const PLBOOL * ArrayCkMinus1 "boolean[]"
%typemap( jstype ) const PLBOOL * ArrayCkMinus1 "boolean[]"
%typemap( javain ) const PLBOOL * ArrayCkMinus1 "$javainput"
%typemap( javaout ) const PLBOOL * ArrayCkMinus1 {
    return $jnicall;
}

%typemap( in ) const PLBOOL * ArrayCkMinus1Null {
    if ( $input != NULL )
    {
        jboolean *jydata = ( *jenv )->GetBooleanArrayElements( jenv, $input, 0 );
        if ( ( *jenv )->GetArrayLength( jenv, $input ) < Alen - 1 )
        {
            printf( "Vector must be at least length of others minus 1.\n" );
            return;
        }
        setup_array_1d_b( &$1, jydata, Alen );
        ( *jenv )->ReleaseBooleanArrayElements( jenv, $input, jydata, 0 );
    }
    else
    {
        $1 = NULL;
    }
}
%typemap( freearg ) const PLBOOL * ArrayCkMinus1Null {
    if ( $1 != NULL )
        free( $1 );
}
%typemap( jni ) const PLBOOL * ArrayCkMinus1Null "jbooleanArray"
%typemap( jtype ) const PLBOOL * ArrayCkMinus1Null "boolean[]"
%typemap( jstype ) const PLBOOL * ArrayCkMinus1Null "boolean[]"
%typemap( javain ) const PLBOOL * ArrayCkMinus1Null "$javainput"
%typemap( javaout ) const PLBOOL * ArrayCkMinus1Null {
    return $jnicall;
}

%typemap( in ) const PLINT * ArrayCkMinus1 {
    jint *jydata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) < Alen - 1 )
    {
        printf( "Vector must be at least length of others minus 1.\n" );
        return;
    }
    setup_array_1d_i( &$1, jydata, Alen );
    ( *jenv )->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap( freearg ) const PLINT * ArrayCkMinus1 {
    free( $1 );
}
%typemap( jni ) const PLINT * ArrayCkMinus1 "jintArray"
%typemap( jtype ) const PLINT * ArrayCkMinus1 "int[]"
%typemap( jstype ) const PLINT * ArrayCkMinus1 "int[]"
%typemap( javain ) const PLINT * ArrayCkMinus1 "$javainput"
%typemap( javaout ) const PLINT * ArrayCkMinus1 {
    return $jnicall;
}

%typemap( in ) const PLINT * ArrayCkMinus1Null {
    if ( $input != NULL )
    {
        jint *jydata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
        if ( ( *jenv )->GetArrayLength( jenv, $input ) < Alen - 1 )
        {
            printf( "Vector must be at least length of others minus 1.\n" );
            return;
        }
        setup_array_1d_i( &$1, jydata, Alen );
        ( *jenv )->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
    }
    else
    {
        $1 = NULL;
    }
}
%typemap( freearg ) const PLINT * ArrayCkMinus1Null {
    if ( $1 != NULL )
        free( $1 );
}
%typemap( jni ) const PLINT * ArrayCkMinus1Null "jintArray"
%typemap( jtype ) const PLINT * ArrayCkMinus1Null "int[]"
%typemap( jstype ) const PLINT * ArrayCkMinus1Null "int[]"
%typemap( javain ) const PLINT * ArrayCkMinus1Null "$javainput"
%typemap( javaout ) const PLINT * ArrayCkMinus1Null {
    return $jnicall;
}

// No length but remember size to check others
%typemap( in ) const PLINT * Array {
    jint *jydata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
    Alen = ( *jenv )->GetArrayLength( jenv, $input );
    setup_array_1d_i( &$1, jydata, Alen );
    ( *jenv )->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap( freearg ) const PLINT * Array {
    free( $1 );
}
%typemap( jni ) const PLINT * Array "jintArray"
%typemap( jtype ) const PLINT * Array "int[]"
%typemap( jstype ) const PLINT * Array "int[]"
%typemap( javain ) const PLINT * Array "$javainput"
%typemap( javaout ) const PLINT * Array {
    return $jnicall;
}

// Set X and Y length for later consistency checking
%typemap( in ) const PLINT * ArrayN {
    int  i;
    jint *jydata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) != Alen )
    {
        printf( "Vectors must be same length.\n" );
        return;
    }
    Xlen = ( *jenv )->GetArrayLength( jenv, $input );
    Ylen = -1;
    for ( i = 0; i < Xlen; i++ )
        if ( jydata[i] > Ylen )
            Ylen = jydata[i];
    setup_array_1d_i( &$1, jydata, Alen );
    ( *jenv )->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap( freearg ) const PLINT * ArrayN {
    free( $1 );
}
%typemap( jni ) const PLINT * ArrayN "jintArray"
%typemap( jtype ) const PLINT * ArrayN "int[]"
%typemap( jstype ) const PLINT * ArrayN "int[]"
%typemap( javain ) const PLINT * ArrayN "$javainput"
%typemap( javaout ) const PLINT * ArrayN {
    return $jnicall;
}

// With trailing count and NULL array option.
%typemap ( in ) ( const PLINT * ArrayNull, PLINT n )
{
    if ( $input != NULL )
    {
        jint *jydata = ( *jenv )->GetIntArrayElements( jenv, $input, 0 );
        $2 = ( *jenv )->GetArrayLength( jenv, $input );
        setup_array_1d_i( &$1, jydata, $2 );
        ( *jenv )->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
    }
    else
    {
        $1 = NULL;
        $2 = 0;
    }
}
%typemap ( freearg ) ( const PLINT * ArrayNull, PLINT n )
{
    free( $1 );
}
%typemap ( jni ) ( const PLINT * ArrayNull, PLINT n ) "jintArray"
%typemap ( jtype ) ( const PLINT * ArrayNull, PLINT n ) "int[]"
%typemap ( jstype ) ( const PLINT * ArrayNull, PLINT n ) "int[]"
%typemap ( javain ) ( const PLINT * ArrayNull, PLINT n ) "$javainput"
%typemap ( javaout ) ( const PLINT * ArrayNull, PLINT n )
{
    return $jnicall;
}

//--------------------------------------------------------------------------
//                                 PLFLT Arrays
//--------------------------------------------------------------------------

//temporary
#if 0
#ifndef PL_DOUBLE
%wrapper
%{
// some really twisted stuff to allow calling a single precision library from python
    PyArrayObject* myArray_ContiguousFromObject( PyObject* in, int type, int mindims, int maxdims )
    {
        PyArrayObject* tmp = (PyArrayObject *) PyArray_ContiguousFromObject( in, PyArray_FLOAT,
            mindims, maxdims );
        if ( !tmp )
        {
            // could be an incoming double array which can't be "safely" converted, do it anyway
            if ( PyArray_Check( in ) )
            {
                PyErr_Clear();
                tmp = (PyArrayObject *) PyArray_Cast( (PyArrayObject *) in, PyArray_FLOAT );
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

// temporary
#endif
// with preceding count
%typemap ( in ) ( PLINT n, const PLFLT * Array )
{
    jPLFLT *jxdata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    $1   = ( *jenv )->GetArrayLength( jenv, $input );
    Alen = $1;
    setup_array_1d_PLFLT( &$2, jxdata, Alen );
    // Could find no easy way to do this as part of freearg so I modified
    // the previous function so it ALWAYS mallocs and copies so that
    // the java array can be released immediately.
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap ( freearg ) ( PLINT n, const PLFLT * Array )
{
    free( $2 );
}
%typemap ( jni ) ( PLINT n, const PLFLT * Array ) jPLFLTArray
%typemap ( jtype ) ( PLINT n, const PLFLT * Array ) jPLFLTbracket
%typemap ( jstype ) ( PLINT n, const PLFLT * Array ) jPLFLTbracket
%typemap ( javain ) ( PLINT n, const PLFLT * Array ) "$javainput"
%typemap ( javaout ) ( PLINT n, const PLFLT * Array )
{
    return $jnicall;
}

// trailing count, and check consistency with previous
%typemap ( in ) ( const PLFLT * ArrayCk, PLINT n )
{
    jPLFLT *jydata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    $2 = ( *jenv )->GetArrayLength( jenv, $input );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) != Alen )
    {
        printf( "Vectors must be same length.\n" );
        return;
    }
    setup_array_1d_PLFLT( &$1, jydata, Alen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap ( freearg ) ( const PLFLT * ArrayCk, PLINT n )
{
    free( $1 );
}
%typemap ( jni ) ( const PLFLT * ArrayCk, PLINT n ) jPLFLTArray
%typemap ( jtype ) ( const PLFLT * ArrayCk, PLINT n ) jPLFLTbracket
%typemap ( jstype ) ( const PLFLT * ArrayCk, PLINT n ) jPLFLTbracket
%typemap ( javain ) ( const PLFLT * ArrayCk, PLINT n ) "$javainput"
%typemap ( javaout ) ( const PLFLT * ArrayCk, PLINT n )
{
    return $jnicall;
}

// no count, but check consistency with previous
%typemap( in ) const PLFLT * ArrayCk {
    jPLFLT *jydata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) != Alen )
    {
        printf( "Vectors must be same length.\n" );
        return;
    }
    setup_array_1d_PLFLT( &$1, jydata, Alen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap( freearg ) const PLFLT * ArrayCk {
    free( $1 );
}
%typemap( jni ) const PLFLT * ArrayCk jPLFLTArray
%typemap( jtype ) const PLFLT * ArrayCk jPLFLTbracket
%typemap( jstype ) const PLFLT * ArrayCk jPLFLTbracket
%typemap( javain ) const PLFLT * ArrayCk "$javainput"
%typemap( javaout ) const PLFLT * ArrayCk {
    return $jnicall;
}

// trailing count, and check consistency with previous
%typemap ( in ) ( const PLFLT * ArrayCkNull, PLINT n )
{
    if ( $input != NULL )
    {
        jPLFLT *jydata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
        $2 = ( *jenv )->GetArrayLength( jenv, $input );
        if ( ( *jenv )->GetArrayLength( jenv, $input ) != Alen )
        {
            printf( "Vectors must be same length.\n" );
            return;
        }
        setup_array_1d_PLFLT( &$1, jydata, Alen );
        ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
    }
    else
    {
        $1 = NULL;
        $2 = 0;
    }
}
%typemap ( freearg ) ( const PLFLT * ArrayCkNull, PLINT n )
{
    if ( $1 != NULL )
        free( $1 );
}
%typemap ( jni ) ( const PLFLT * ArrayCkNull, PLINT n ) jPLFLTArray
%typemap ( jtype ) ( const PLFLT * ArrayCkNull, PLINT n ) jPLFLTbracket
%typemap ( jstype ) ( const PLFLT * ArrayCkNull, PLINT n ) jPLFLTbracket
%typemap ( javain ) ( const PLFLT * ArrayCkNull, PLINT n ) "$javainput"
%typemap ( javaout ) ( const PLFLT * ArrayCkNull, PLINT n )
{
    return $jnicall;
}


// no count, but check consistency with previous or NULL
%typemap( in ) const PLFLT * ArrayCkNull {
    if ( $input != NULL )
    {
        jPLFLT *jydata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
        if ( ( *jenv )->GetArrayLength( jenv, $input ) != Alen )
        {
            printf( "Vectors must be same length.\n" );
            return;
        }
        setup_array_1d_PLFLT( &$1, jydata, Alen );
        ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
    }
    else
    {
        $1 = NULL;
    }
}
%typemap( freearg ) const PLFLT * ArrayCkNull {
    if ( $1 != NULL )
        free( $1 );
}
%typemap( jni ) const PLFLT * ArrayCkNull jPLFLTArray
%typemap( jtype ) const PLFLT * ArrayCkNull jPLFLTbracket
%typemap( jstype ) const PLFLT * ArrayCkNull jPLFLTbracket
%typemap( javain ) const PLFLT * ArrayCkNull "$javainput"
%typemap( javaout ) const PLFLT * ArrayCkNull {
    return $jnicall;
}

// set X length for later consistency checking
%typemap ( in ) ( const PLFLT * ArrayX, PLINT nx )
{
    jPLFLT *jxdata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    Xlen = ( *jenv )->GetArrayLength( jenv, $input );
    $2   = Xlen;
    setup_array_1d_PLFLT( &$1, jxdata, Xlen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap ( freearg ) ( const PLFLT * ArrayX, PLINT nx )
{
    free( $1 );
}
%typemap ( jni ) ( const PLFLT * ArrayX, PLINT nx ) jPLFLTArray
%typemap ( jtype ) ( const PLFLT * ArrayX, PLINT nx ) jPLFLTbracket
%typemap ( jstype ) ( const PLFLT * ArrayX, PLINT nx ) jPLFLTbracket
%typemap ( javain ) ( const PLFLT * ArrayX, PLINT nx ) "$javainput"
%typemap ( javaout ) ( const PLFLT * ArrayX, PLINT nx )
{
    return $jnicall;
}

%typemap( in ) const PLFLT * ArrayX {
    jPLFLT *jxdata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    Xlen = ( *jenv )->GetArrayLength( jenv, $input );
    setup_array_1d_PLFLT( &$1, jxdata, Xlen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap( freearg ) const PLFLT * ArrayX {
    free( $1 );
}
%typemap( jni ) const PLFLT * ArrayX jPLFLTArray
%typemap( jtype ) const PLFLT * ArrayX jPLFLTbracket
%typemap( jstype ) const PLFLT * ArrayX jPLFLTbracket
%typemap( javain ) const PLFLT * ArrayX "$javainput"
%typemap( javaout ) const PLFLT * ArrayX {
    return $jnicall;
}

// set Y length for later consistency checking
%typemap ( in ) ( const PLFLT * ArrayY, PLINT ny )
{
    jPLFLT *jydata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    Ylen = ( *jenv )->GetArrayLength( jenv, $input );
    $2   = Ylen;
    setup_array_1d_PLFLT( &$1, jydata, Ylen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap ( freearg ) ( const PLFLT * ArrayY, PLINT ny )
{
    free( $1 );
}
%typemap ( jni ) ( const PLFLT * ArrayY, PLINT ny ) jPLFLTArray
%typemap ( jtype ) ( const PLFLT * ArrayY, PLINT ny ) jPLFLTbracket
%typemap ( jstype ) ( const PLFLT * ArrayY, PLINT ny ) jPLFLTbracket
%typemap ( javain ) ( const PLFLT * ArrayY, PLINT ny ) "$javainput"
%typemap ( javaout ) ( const PLFLT * ArrayY, PLINT ny )
{
    return $jnicall;
}

%typemap( in ) const PLFLT * ArrayY {
    jPLFLT *jydata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    Ylen = ( *jenv )->GetArrayLength( jenv, $input );
    setup_array_1d_PLFLT( &$1, jydata, Ylen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap( freearg ) const PLFLT * ArrayY {
    free( $1 );
}
%typemap( jni ) const PLFLT * ArrayY jPLFLTArray
%typemap( jtype ) const PLFLT * ArrayY jPLFLTbracket
%typemap( jstype ) const PLFLT * ArrayY jPLFLTbracket
%typemap( javain ) const PLFLT * ArrayY "$javainput"
%typemap( javaout ) const PLFLT * ArrayY {
    return $jnicall;
}

// with trailing count
%typemap ( in ) ( const PLFLT * Array, PLINT n )
{
    jPLFLT *jxdata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    $2 = ( *jenv )->GetArrayLength( jenv, $input );
    setup_array_1d_PLFLT( &$1, jxdata, $2 );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap ( freearg ) ( const PLFLT * Array, PLINT n )
{
    free( $1 );
}
%typemap ( jni ) ( const PLFLT * Array, PLINT n ) jPLFLTArray
%typemap ( jtype ) ( const PLFLT * Array, PLINT n ) jPLFLTbracket
%typemap ( jstype ) ( const PLFLT * Array, PLINT n ) jPLFLTbracket
%typemap ( javain ) ( const PLFLT * Array, PLINT n ) "$javainput"
%typemap ( javaout ) ( const PLFLT * Array, PLINT n )
{
    return $jnicall;
}

// with no trailing count
%typemap( in ) const PLFLT * Array {
    jPLFLT *jxdata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    Alen = ( *jenv )->GetArrayLength( jenv, $input );
    setup_array_1d_PLFLT( &$1, jxdata, Alen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap( freearg ) const PLFLT * Array {
    free( $1 );
}
%typemap( jni ) const PLFLT * Array jPLFLTArray
%typemap( jtype ) const PLFLT * Array jPLFLTbracket
%typemap( jstype ) const PLFLT * Array jPLFLTbracket
%typemap( javain ) const PLFLT * Array "$javainput"
%typemap( javaout ) const PLFLT * Array {
    return $jnicall;
}


// with no trailing count
%typemap( in ) const PLFLT * ArrayNull {
    if ( $input != NULL )
    {
        jPLFLT *jxdata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
        Alen = ( *jenv )->GetArrayLength( jenv, $input );
        setup_array_1d_PLFLT( &$1, jxdata, Alen );
        ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
    }
    else
    {
        $1   = NULL;
        Alen = 0;
    }
}
%typemap( freearg ) const PLFLT * ArrayNull {
    if ( $1 != NULL )
        free( $1 );
}
%typemap( jni ) const PLFLT * ArrayNull jPLFLTArray
%typemap( jtype ) const PLFLT * ArrayNull jPLFLTbracket
%typemap( jstype ) const PLFLT * ArrayNull jPLFLTbracket
%typemap( javain ) const PLFLT * ArrayNull "$javainput"
%typemap( javaout ) const PLFLT * ArrayNull {
    return $jnicall;
}

// check consistency with X dimension of previous
%typemap( in ) const PLFLT * ArrayCkX {
    jPLFLT *jxdata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) != Xlen )
    {
        printf( "Vectors must be same length.\n" );
        return;
    }
    setup_array_1d_PLFLT( &$1, jxdata, Xlen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap( freearg ) const PLFLT * ArrayCkX {
    free( $1 );
}
%typemap( jni ) const PLFLT * ArrayCkX jPLFLTArray
%typemap( jtype ) const PLFLT * ArrayCkX jPLFLTbracket
%typemap( jstype ) const PLFLT * ArrayCkX jPLFLTbracket
%typemap( javain ) const PLFLT * ArrayCkX "$javainput"
%typemap( javaout ) const PLFLT * ArrayCkX {
    return $jnicall;
}

// check consistency with Y dimension of previous
%typemap( in ) const PLFLT * ArrayCkY {
    jPLFLT *jydata = ( *jenv )->GetPLFLTArrayElements( jenv, $input, 0 );
    if ( ( *jenv )->GetArrayLength( jenv, $input ) != Ylen )
    {
        printf( "Vectors must be same length.\n" );
        return;
    }
    setup_array_1d_PLFLT( &$1, jydata, Ylen );
    ( *jenv )->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap( freearg ) const PLFLT * ArrayCkY {
    free( $1 );
}
%typemap( jni ) const PLFLT * ArrayCkY jPLFLTArray
%typemap( jtype ) const PLFLT * ArrayCkY jPLFLTbracket
%typemap( jstype ) const PLFLT * ArrayCkY jPLFLTbracket
%typemap( javain ) const PLFLT * ArrayCkY "$javainput"
%typemap( javaout ) const PLFLT * ArrayCkY {
    return $jnicall;
}

// 2D array with trailing dimensions, check consistency with previous
%typemap ( in ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny )
{
    jPLFLT  **adat;
    jobject *ai;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i, j;
    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai[i] ) )
        {
            printf( "Misshapen a array.\n" );
            for ( j = 0; j <= i; j++ )
                ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
            free( adat );
            free( ai );
            return;
        }
    }

    if ( nx != Xlen || ny != Ylen )
    {
        printf( "Vectors must match matrix.\n" );
        for ( i = 0; i < nx; i++ )
            ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        free( adat );
        free( ai );
        return;
    }
    setup_array_2d_PLFLT( &$1, adat, nx, ny );
    $2 = nx;
    $3 = ny;
    for ( i = 0; i < nx; i++ )
    {
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }


    free( adat );
    free( ai );
}
%typemap ( freearg ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny )
{
    free( $1[0] );
    free( $1 );
}
%typemap ( jni ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny ) "jobjectArray"
%typemap ( jtype ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny ) jPLFLTbracket2
%typemap ( jstype ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny ) jPLFLTbracket2
%typemap ( javain ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny ) "$javainput"
%typemap ( javaout ) ( const PLFLT **MatrixCk, PLINT nx, PLINT ny )
{
    return $jnicall;
}

// 2D array with trailing dimensions, set the X, Y size for later checking
%typemap ( in ) ( const PLFLT **Matrix, PLINT nx, PLINT ny )
{
    jPLFLT  **adat;
    jobject *ai;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i, j;
    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai[i] ) )
        {
            printf( "Misshapen a array.\n" );
            for ( j = 0; j <= i; j++ )
                ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
            free( adat );
            free( ai );
            return;
        }
    }

    Xlen = nx;
    Ylen = ny;
    setup_array_2d_PLFLT( &$1, adat, nx, ny );
    $2 = nx;
    $3 = ny;
    for ( i = 0; i < nx; i++ )
    {
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }

    free( adat );
    free( ai );
}
%typemap ( freearg ) ( const PLFLT **Matrix, PLINT nx, PLINT ny )
{
    free( $1[0] );
    free( $1 );
}
%typemap ( jni ) ( const PLFLT **Matrix, PLINT nx, PLINT ny ) "jobjectArray"
%typemap ( jtype ) ( const PLFLT **Matrix, PLINT nx, PLINT ny ) jPLFLTbracket2
%typemap ( jstype ) ( const PLFLT **Matrix, PLINT nx, PLINT ny ) jPLFLTbracket2
%typemap ( javain ) ( const PLFLT **Matrix, PLINT nx, PLINT ny ) "$javainput"
%typemap ( javaout ) ( const PLFLT **Matrix, PLINT nx, PLINT ny )
{
    return $jnicall;
}

// 2D array with no trailing dimensions, set the X, Y size for later checking
%typemap( in ) const PLFLT * *Matrix {
    jPLFLT  **adat;
    jobject *ai;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i, j;
    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai[i] ) )
        {
            printf( "Misshapen a array.\n" );
            for ( j = 0; j <= i; j++ )
                ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
            free( adat );
            free( ai );
            return;
        }
    }

    Xlen = nx;
    Ylen = ny;
    setup_array_2d_PLFLT( &$1, adat, nx, ny );
    for ( i = 0; i < nx; i++ )
    {
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }

    free( adat );
    free( ai );
}
%typemap( freearg ) const PLFLT * *Matrix {
    free( $1[0] );
    free( $1 );
}
%typemap( jni ) const PLFLT * *Matrix "jobjectArray"
%typemap( jtype ) const PLFLT * *Matrix jPLFLTbracket2
%typemap( jstype ) const PLFLT * *Matrix jPLFLTbracket2
%typemap( javain ) const PLFLT * *Matrix "$javainput"
%typemap( javaout ) const PLFLT * *Matrix {
    return $jnicall;
}

// 2D array, check for consistency
%typemap( in ) const PLFLT * *MatrixCk {
    jPLFLT  **adat;
    jobject *ai;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i, j;
    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai[i] ) )
        {
            printf( "Misshapen a array.\n" );
            for ( j = 0; j <= i; j++ )
                ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
            free( adat );
            free( ai );
            return;
        }
    }

    if ( nx != Xlen || ny != Ylen )
    {
        printf( "Vectors must match matrix.\n" );
        for ( i = 0; i < nx; i++ )
            ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        free( adat );
        free( ai );
        return;
    }
    setup_array_2d_PLFLT( &$1, adat, nx, ny );
    for ( i = 0; i < nx; i++ )
    {
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }

    free( adat );
    free( ai );
}
%typemap( freearg ) const PLFLT * *MatrixCk {
    free( $1[0] );
    free( $1 );
}
%typemap( jni ) const PLFLT * *MatrixCk "jobjectArray"
%typemap( jtype ) const PLFLT * *MatrixCk jPLFLTbracket2
%typemap( jstype ) const PLFLT * *MatrixCk jPLFLTbracket2
%typemap( javain ) const PLFLT * *MatrixCk "$javainput"
%typemap( javaout ) const PLFLT * *MatrixCk {
    return $jnicall;
}

// 2D array, check for consistency
// Version with values returned to java
%typemap( in ) PLFLT * *OutMatrixCk {
    jobject ai;
    PLFLT   **ptr;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i;

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai = ( *jenv )->GetObjectArrayElement( jenv, $input, i );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai ) )
        {
            printf( "Misshapen a array.\n" );
            return;
        }
    }

    if ( nx != Xlen || ny != Ylen )
    {
        printf( "Vectors must match matrix.\n" );
        return;
    }

    ptr    = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
    ptr[0] = (PLFLT *) malloc( (size_t) nx * ny * sizeof ( PLFLT ) );
    for ( i = 0; i < nx; i++ )
    {
        ptr[i] = ptr[0] + i * ny;
    }

    $1 = ptr;
}
%typemap( argout ) PLFLT * *OutMatrixCk {
    jPLFLT  **adat;
    jobject *ai;
    PLFLT   **ptr;
    int     i, j;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;

    ptr = $1;

    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
    }
    for ( i = 0; i < nx; i++ )
    {
        for ( j = 0; j < ny; j++ )
        {
            adat[i][j] = ptr[i][j];
        }
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }

    free( adat );
    free( ai );
}
%typemap( freearg ) PLFLT * *OutMatrixCk {
    free( $1[0] );
    free( $1 );
}
%typemap( jni ) PLFLT * *OutMatrixCk "jobjectArray"
%typemap( jtype ) PLFLT * *OutMatrixCk jPLFLTbracket2
%typemap( jstype ) PLFLT * *OutMatrixCk jPLFLTbracket2
%typemap( javain ) PLFLT * *OutMatrixCk "$javainput"
%typemap( javaout ) PLFLT * *OutMatrixCk {
    return $jnicall;
}


%{
    typedef PLINT ( *defined_func )( PLFLT, PLFLT );
    typedef void ( *fill_func )( PLINT, const PLFLT*, const PLFLT* );
    typedef void ( *pltr_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
    typedef void ( *ct_func )( PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer );
    typedef void ( *mapform_func )( PLINT, PLFLT *, PLFLT* );
    typedef PLFLT ( *f2eval_func )( PLINT, PLINT, PLPointer );
    typedef void ( *label_func )( PLINT, PLFLT, char *, PLINT, PLPointer );
%}

// First of two object arrays, where we check X and Y with previous.
// Note this is the simplified Tcl-like approach to handling the xg
// and yg arrays.  Later we would like to move to true call-back functions
// here instead like is done with the python interface.
%typemap( in ) pltr_func pltr {
    jPLFLT  **adat;
    jobject *ai;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i, j;
    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai[i] ) )
        {
            printf( "Misshapen a array.\n" );
            for ( j = 0; j <= i; j++ )
                ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
            free( adat );
            free( ai );
            return;
        }
    }

    if ( !( ( nx == Xlen && ny == Ylen ) || ( nx == Xlen && ny == 1 ) ) )
    {
        printf( "Xlen =%d, nx =%d, Ylen =%d, ny =%d\n", Xlen, nx, Ylen, ny );
        printf( "X vector or matrix must match matrix dimensions.\n" );
        for ( i = 0; i < nx; i++ )
            ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        free( adat );
        free( ai );
        return;
    }
    // Store whether second dimension is unity.
    Alen = ny;
    setup_array_2d_PLFLT( &xg, adat, nx, ny );
    for ( i = 0; i < nx; i++ )
    {
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }

    free( adat );
    free( ai );
    $1 = pltr2;
}

%typemap( freearg ) pltr_func pltr {
    free( xg[0] );
    free( xg );
}
%typemap( jni ) pltr_func pltr "jobjectArray"
%typemap( jtype ) pltr_func pltr jPLFLTbracket2
%typemap( jstype ) pltr_func pltr jPLFLTbracket2
%typemap( javain ) pltr_func pltr "$javainput"
%typemap( javaout ) pltr_func pltr {
    return $jnicall;
}

%{
    jobject   mapformClass;
    jmethodID mapformID;
    JNIEnv    *cbenvMapform;

    void mapform_java( PLINT n, PLFLT *x, PLFLT *y );

    // C mapform callback function which calls the java
    // mapform function in a PLCallbackMapform object.
    void mapform_java( PLINT n, PLFLT *x, PLFLT *y )
    {
        jdoubleArray jx = setup_java_array_1d_PLFLT( cbenvMapform, x, n );
        jdoubleArray jy = setup_java_array_1d_PLFLT( cbenvMapform, y, n );
        ( *cbenvMapform )->CallVoidMethod( cbenvMapform, mapformClass, mapformID, jx, jy );
        release_java_array_1d_PLFLT( cbenvMapform, jx, x, n );
        release_java_array_1d_PLFLT( cbenvMapform, jy, y, n );
    }
%}


// Handle function pointers to mapform function using an java class
%typemap( in ) mapform_func mapform {
    jobject obj = $input;
    if ( obj != NULL )
    {
        jclass cls = ( *jenv )->GetObjectClass( jenv, obj );
        mapformID    = ( *jenv )->GetMethodID( jenv, cls, "mapform", "([D[D)V" );
        mapformClass = obj;
        cbenvMapform = jenv;
        $1           = mapform_java;
    }
    else
    {
        $1 = NULL;
    }
}

%typemap( jni ) mapform_func "jobject"
%typemap( jtype ) mapform_func "PLCallbackMapform"
%typemap( jstype ) mapform_func "PLCallbackMapform"
%typemap( javain ) mapform_func mapform "$javainput"

%{
    jobject labelClass    = 0;
    jobject labelClassRef = 0;

    void label_java( PLINT axis, PLFLT value, char *string, PLINT len, PLPointer data );

    // C label plotting callback function which calls the java
    // label function in a PLCallbackLabel labelClassobelID
// bject.
    void label_java( PLINT axis, PLFLT value, char *string, PLINT len, PLPointer PL_UNUSED( data ) )
    {
        jstring    javaString;
        const char *nativeString;
        jint       jaxis;
        jdouble    jvalue;
        JNIEnv     *cbenv;
        jmethodID  labelID = 0;
        jclass     cls;

        jaxis  = (jint) axis;
        jvalue = (jdouble) value;

        if ( ( string == NULL ) || ( len == 0 ) )
        {
            return;
        }

        string[0] = '\0';

        if ( cached_jvm == NULL )
        {
            fprintf( stderr, "Error! NULL jvm\n" );
            return;
        }
        ( *cached_jvm )->GetEnv( cached_jvm, (void **) &cbenv, JNI_VERSION_1_2 );
        if ( cbenv == NULL )
        {
            fprintf( stderr, "Thread not attached\n" );
            if ( ( *cached_jvm )->AttachCurrentThread( cached_jvm, (void **) &cbenv, NULL ) != 0 )
            {
                fprintf( stderr, "Error attaching to JVM\n" );
                return;
            }
        }
        if ( labelClass == 0 )
        {
            fprintf( stderr, "Error - callback undefined\n" );
            return;
        }
        cls = ( *cbenv )->GetObjectClass( cbenv, labelClass );
        if ( cls == 0 )
        {
            fprintf( stderr, "Error getting callback class\n" );
            return;
        }
        labelID = ( *cbenv )->GetMethodID( cbenv, cls, "label", "(ID)Ljava/lang/String;" );
        if ( labelID != 0 )
        {
            javaString   = (jstring) ( *cbenv )->CallObjectMethod( cbenv, labelClass, labelID, jaxis, jvalue );
            nativeString = ( *cbenv )->GetStringUTFChars( cbenv, javaString, 0 );
            strncpy( string, nativeString, (size_t) len );
            ( *cbenv )->ReleaseStringUTFChars( cbenv, javaString, nativeString );
        }
        else
        {
            fprintf( stderr, "Java callback not found\n" );
            string[0] = '\0';
        }
    }
%}


// Handle function pointers to label function using an java class
%typemap( in ) label_func lf {
    jobject obj = $input;

    // Delete any old references
    if ( labelClass != 0 )
    {
        ( *jenv )->DeleteGlobalRef( jenv, labelClass );
        labelClass = 0;
    }
    // Need a reference to this object to ensure it is
    // valid when we reach the callback
    if ( obj != NULL )
    {
        labelClass = ( *jenv )->NewGlobalRef( jenv, obj );
    }
    if ( labelClass != 0 )
    {
        $1 = label_java;
    }
    else
    {
        $1 = NULL;
    }
}

%typemap( jni ) label_func lf "jobject"
%typemap( jtype ) label_func lf "PLCallbackLabel"
%typemap( jstype ) label_func lf "PLCallbackLabel"
%typemap( javain ) label_func lf "$javainput"

%{
    jobject ctClass    = 0;
    jobject ctClassRef = 0;

    void ct_java( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer data );

    // C coordinate transform callback function which calls the java
    // coordinate transform function in a PLCallbackCoordTrans object.
    void ct_java( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer data )
    {
        jdouble      jx, jy;
        jdoubleArray jxt, jyt;
        jdouble      *xtval;
        jdouble      *ytval;
        jobject      jdata;
        JNIEnv       *cbenv;
        jmethodID    ctID = 0;
        jclass       cls;

        jx    = (jdouble) x;
        jy    = (jdouble) y;
        jdata = (jobject) data;

        if ( cached_jvm == NULL )
        {
            fprintf( stderr, "Error! NULL jvm\n" );
            return;
        }
        ( *cached_jvm )->GetEnv( cached_jvm, (void **) &cbenv, JNI_VERSION_1_2 );
        if ( cbenv == NULL )
        {
            fprintf( stderr, "Thread not attached\n" );
            if ( ( *cached_jvm )->AttachCurrentThread( cached_jvm, (void **) &cbenv, NULL ) != 0 )
            {
                fprintf( stderr, "Error attaching to JVM\n" );
                return;
            }
        }
        jxt = ( *cbenv )->NewDoubleArray( cbenv, 1 );
        jyt = ( *cbenv )->NewDoubleArray( cbenv, 1 );
        if ( ctClass == 0 )
        {
            fprintf( stderr, "Error - callback undefined\n" );
            return;
        }
        cls = ( *cbenv )->GetObjectClass( cbenv, ctClass );
        if ( cls == 0 )
        {
            fprintf( stderr, "Error getting callback class\n" );
            return;
        }
        ctID = ( *cbenv )->GetMethodID( cbenv, cls, "coordTransform", "(DD[D[DLjava/lang/Object;)V" );
        if ( ctID != 0 )
        {
            ( *cbenv )->CallVoidMethod( cbenv, ctClass, ctID, jx, jy, jxt, jyt, jdata );
            xtval = ( *cbenv )->GetDoubleArrayElements( cbenv, jxt, JNI_FALSE );
            ytval = ( *cbenv )->GetDoubleArrayElements( cbenv, jyt, JNI_FALSE );
            *xt   = (PLFLT) xtval[0];
            *yt   = (PLFLT) ytval[0];
        }
        else
        {
            fprintf( stderr, "Java callback not found\n" );
        }
    }
%}


// Handle function pointers to coordinate transform function using a
// java class
%typemap( in ) ct_func ctf {
    jobject obj = $input;

    // Delete any old references
    if ( ctClass != 0 )
    {
        ( *jenv )->DeleteGlobalRef( jenv, ctClass );
        ctClass = 0;
    }
    // Need a reference to this object to ensure it is
    // valid when we reach the callback
    if ( obj != NULL )
    {
        ctClass = ( *jenv )->NewGlobalRef( jenv, obj );
    }
    if ( ctClass != 0 )
    {
        $1 = ct_java;
    }
    else
    {
        $1 = NULL;
    }
}

%typemap( jni ) ct_func ctf "jobject"
%typemap( jtype ) ct_func ctf "PLCallbackCT"
%typemap( jstype ) ct_func ctf "PLCallbackCT"
%typemap( javain ) ct_func ctf "$javainput"

%typemap( in ) PLPointer data {
    $1 = NULL;
}
%typemap( jni ) PLPointer data "jobject"
%typemap( jtype ) PLPointer data "Object"
%typemap( jstype ) PLPointer data "Object"
%typemap( javain ) PLPointer data "$javainput"

// Second of two object arrays, where we check X and Y with previous object.
%typemap( in ) PLPointer OBJECT_DATA {
    jPLFLT  **adat;
    jobject *ai;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i, j;
    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai[i] ) )
        {
            printf( "Misshapen a array.\n" );
            for ( j = 0; j <= i; j++ )
                ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
            free( adat );
            free( ai );
            return;
        }
    }

    if ( !( ( nx == Xlen && ny == Ylen ) || ( nx == Ylen && ny == 1 && ny == Alen ) ) )
    {
        printf( "Xlen =%d, nx =%d, Ylen =%d, Alen =%d, ny =%d\n",
            Xlen, nx, Ylen, Alen, ny );
        printf( "Y vector or matrix must match matrix dimensions.\n" );
        for ( i = 0; i < nx; i++ )
            ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        free( adat );
        free( ai );
        return;
    }
    setup_array_2d_PLFLT( &yg, adat, nx, ny );
    for ( i = 0; i < nx; i++ )
    {
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }

    free( adat );
    free( ai );
    cgrid     = (PLcGrid2 *) malloc( sizeof ( PLcGrid2 ) );
    cgrid->xg = xg;
    cgrid->yg = yg;
    cgrid->nx = nx;
    cgrid->ny = ny;
    $1        = cgrid;
}

%typemap( freearg ) PLPointer OBJECT_DATA {
    free( yg[0] );
    free( yg );
    free( cgrid );
}
%typemap( jni ) PLPointer OBJECT_DATA "jobjectArray"
%typemap( jtype ) PLPointer OBJECT_DATA jPLFLTbracket2
%typemap( jstype ) PLPointer OBJECT_DATA jPLFLTbracket2
%typemap( javain ) PLPointer OBJECT_DATA "$javainput"
%typemap( javaout ) PLPointer OBJECT_DATA {
    return $jnicall;
}

// First of two object arrays, where we check X and Y with previous.
// Note this is the simplified Tcl-like approach to handling the xg
// and yg arrays.  Later we would like to move to true call-back functions
// here instead like is done with the python interface.
// This is the version for plimagefr where size is 1 larger than previous
// array
%typemap( in ) pltr_func pltr_img {
    jPLFLT  **adat;
    jobject *ai;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i, j;
    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai[i] ) )
        {
            printf( "Misshapen a array.\n" );
            for ( j = 0; j <= i; j++ )
                ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
            free( adat );
            free( ai );
            return;
        }
    }

    if ( !( ( nx == Xlen + 1 && ny == Ylen + 1 ) || ( nx == Xlen + 1 && ny == 1 ) ) )
    {
        printf( "Xlen =%d, nx =%d, Ylen =%d, ny =%d\n", Xlen, nx, Ylen, ny );
        printf( "X vector or matrix must match matrix dimensions.\n" );
        for ( i = 0; i < nx; i++ )
            ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        free( adat );
        free( ai );
        return;
    }
    // Store whether second dimension is unity.
    Alen = ny;
    setup_array_2d_PLFLT( &xg, adat, nx, ny );
    for ( i = 0; i < nx; i++ )
    {
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }

    free( adat );
    free( ai );
    $1 = pltr2;
}

%typemap( freearg ) pltr_func pltr_img {
    free( xg[0] );
    free( xg );
}
%typemap( jni ) pltr_func pltr_img "jobjectArray"
%typemap( jtype ) pltr_func pltr_img jPLFLTbracket2
%typemap( jstype ) pltr_func pltr_img jPLFLTbracket2
%typemap( javain ) pltr_func pltr_img "$javainput"
%typemap( javaout ) pltr_func pltr_img {
    return $jnicall;
}

// Second of two object arrays, where we check X and Y with previous object.
// This is the version for plimagefr where size is 1 larger than previous
// array
%typemap( in ) PLPointer OBJECT_DATA_img {
    jPLFLT  **adat;
    jobject *ai;
    int     nx = ( *jenv )->GetArrayLength( jenv, $input );
    int     ny = -1;
    int     i, j;
    ai   = (jobject *) malloc( (size_t) nx * sizeof ( jobject ) );
    adat = (jPLFLT **) malloc( (size_t) nx * sizeof ( jPLFLT * ) );

    ( *jenv )->EnsureLocalCapacity( jenv, nx );

    for ( i = 0; i < nx; i++ )
    {
        ai[i]   = ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        adat[i] = ( *jenv )->GetPLFLTArrayElements( jenv, ai[i], 0 );

        if ( ny == -1 )
            ny = ( *jenv )->GetArrayLength( jenv, ai[i] );
        else if ( ny != ( *jenv )->GetArrayLength( jenv, ai[i] ) )
        {
            printf( "Misshapen a array.\n" );
            for ( j = 0; j <= i; j++ )
                ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
            free( adat );
            free( ai );
            return;
        }
    }

    if ( !( ( nx == Xlen + 1 && ny == Ylen + 1 ) || ( nx == Ylen + 1 && ny == 1 && ny == Alen ) ) )
    {
        printf( "Xlen =%d, nx =%d, Ylen =%d, Alen =%d, ny =%d\n",
            Xlen, nx, Ylen, Alen, ny );
        printf( "Y vector or matrix must match matrix dimensions.\n" );
        for ( i = 0; i < nx; i++ )
            ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        free( adat );
        free( ai );
        return;
    }
    setup_array_2d_PLFLT( &yg, adat, nx, ny );
    for ( i = 0; i < nx; i++ )
    {
        ( *jenv )->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
        ( *jenv )->DeleteLocalRef( jenv, ai[i] );
    }

    free( adat );
    free( ai );
    cgrid     = (PLcGrid2 *) malloc( sizeof ( PLcGrid2 ) );
    cgrid->xg = xg;
    cgrid->yg = yg;
    cgrid->nx = nx;
    cgrid->ny = ny;
    $1        = cgrid;
}

%typemap( freearg ) PLPointer OBJECT_DATA_img {
    free( yg[0] );
    free( yg );
    free( cgrid );
}
%typemap( jni ) PLPointer OBJECT_DATA_img "jobjectArray"
%typemap( jtype ) PLPointer OBJECT_DATA_img jPLFLTbracket2
%typemap( jstype ) PLPointer OBJECT_DATA_img jPLFLTbracket2
%typemap( javain ) PLPointer OBJECT_DATA_img "$javainput"
%typemap( javaout ) PLPointer OBJECT_DATA_img {
    return $jnicall;
}

// Do not specify defined function or fill function from java.  Instead
// specify NULL and plfill defaults in the interface C code.
%typemap( in, numinputs = 0 ) defined_func df {
    $1 = NULL;
}
%typemap( in, numinputs = 0 ) fill_func ff {
    $1 = plfill;
}

//**************************
//        String returning functions
//        Adopt method in SWIG-1.3.21/Examples/java/typemap/example.i
//***************************

// Define the types to use in the generated JNI C code and Java code
%typemap( jni ) char *OUTPUT "jobject"
%typemap( jtype ) char *OUTPUT "StringBuffer"
%typemap( jstype ) char *OUTPUT "StringBuffer"

// How to convert Java(JNI) type to requested C type
%typemap( in ) char *OUTPUT {
    $1 = NULL;
    if ( $input != NULL )
    {
        // Get the String from the StringBuffer
        jmethodID setLengthID;
        jclass    sbufClass = ( *jenv )->GetObjectClass( jenv, $input );
        // Take a copy of the C string as the typemap is for a non const C string
        jmethodID capacityID = ( *jenv )->GetMethodID( jenv, sbufClass, "capacity", "()I" );
        jint      capacity   = ( *jenv )->CallIntMethod( jenv, $input, capacityID );
        $1 = (char *) malloc( (size_t) ( capacity + 1 ) );

        // Zero the original StringBuffer, so we can replace it with the result
        setLengthID = ( *jenv )->GetMethodID( jenv, sbufClass, "setLength", "(I)V" );
        ( *jenv )->CallVoidMethod( jenv, $input, setLengthID, (jint) 0 );
    }
}


// How to convert the C type to the Java(JNI) type
%typemap( argout ) char *OUTPUT {
    if ( $1 != NULL )
    {
        // Append the result to the empty StringBuffer
        jstring   newString      = ( *jenv )->NewStringUTF( jenv, $1 );
        jclass    sbufClass      = ( *jenv )->GetObjectClass( jenv, $input );
        jmethodID appendStringID = ( *jenv )->GetMethodID( jenv, sbufClass, "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;" );
        ( *jenv )->CallObjectMethod( jenv, $input, appendStringID, newString );

        // Clean up the string object, no longer needed
        free( $1 );
        $1 = NULL;
    }
}
// Prevent the default freearg typemap from being used
%typemap( freearg ) char *OUTPUT ""

// Convert the jstype to jtype typemap type
%typemap( javain ) char *OUTPUT "$javainput"

// Character arrays:

%typemap ( jni ) ( int *p_argc, char **argv ) "jobjectArray"
%typemap ( jtype ) ( int *p_argc, char **argv ) "String[]"
%typemap ( jstype ) ( int *p_argc, char **argv ) "String[]"
%typemap ( javain ) ( int *p_argc, char **argv ) "$javainput"
%typemap ( javaout ) ( int *p_argc, char **argv )
{
    return $jnicall;
}
%typemap ( in ) ( int *p_argc, char **argv ) ( int size )
{
    int i = 0;
    size = (int) ( ( *jenv )->GetArrayLength( jenv, $input ) );
    $1   = &size;
    $2   = (char **) malloc( (size_t) ( size + 1 ) * sizeof ( char * ) );
    // make a copy of each string
    for ( i = 0; i < size; i++ )
    {
        jstring    j_string   = (jstring) ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        const char * c_string = (const char *) ( *jenv )->GetStringUTFChars( jenv, j_string, 0 );
// Commented out version straight from swig documentation, but I think
// it is wrong.
//    $2[i] = malloc(strlen((c_string)+1)*sizeof(const char *));
        $2[i] = malloc( ( strlen( c_string ) + 1 ) * sizeof ( char * ) );
        strcpy( $2[i], c_string );
        ( *jenv )->ReleaseStringUTFChars( jenv, j_string, c_string );
        ( *jenv )->DeleteLocalRef( jenv, j_string );
    }
    $2[i] = 0;
}

// This cleans up the memory we malloc'd before the function call
%typemap ( freearg ) ( int *p_argc, char **argv )
{
    int i;
// Commented out version straight from swig documentation, but I think
// it is wrong.
// for (i=0; i<size$argnum-1; i++)
    for ( i = 0; i < size$argnum; i++ )
        free( $2[i] );
    free( $2 );
}

%typemap ( jni ) ( const char *legline[4] ) "jobjectArray"
%typemap ( jtype ) ( const char *legline[4] ) "String[]"
%typemap ( jstype ) ( const char *legline[4] ) "String[]"
%typemap ( javain ) ( const char *legline[4] ) "$javainput"
%typemap ( javaout ) ( const char *legline[4] )
{
    return $jnicall;
}
%typemap ( in ) ( const char *legline[4] )
{
    int i    = 0;
    int size = ( *jenv )->GetArrayLength( jenv, $input );
    if ( size != 4 )
    {
        printf( "legline must be an array of length 4\n" );
        return;
    }
    $1 = (char **) malloc( 4 * sizeof ( char * ) );
    // make a copy of each string
    for ( i = 0; i < 4; i++ )
    {
        jstring    j_string   = (jstring) ( *jenv )->GetObjectArrayElement( jenv, $input, i );
        const char * c_string = (const char *) ( *jenv )->GetStringUTFChars( jenv, j_string, 0 );
        $1[i] = malloc( ( strlen( c_string ) + 1 ) * sizeof ( const char * ) );
        strcpy( $1[i], c_string );
        ( *jenv )->ReleaseStringUTFChars( jenv, j_string, c_string );
        ( *jenv )->DeleteLocalRef( jenv, j_string );
    }
}

// This cleans up the memory we malloc'd before the function call
%typemap ( freearg ) ( const char *legline[4] )
{
    int i;
    for ( i = 0; i < 4; i++ )
        free( $1[i] );
    free( $1 );
}

%typemap ( in ) ( const char **ArrayCk )
{
    int i = 0;
    if ( $input != NULL )
    {
        int size = ( *jenv )->GetArrayLength( jenv, $input );
        if ( size != Alen )
        {
            printf( "Arrays must be the same length\n" );
            return;
        }
        $1 = (char **) malloc( (size_t) Alen * sizeof ( char * ) );
        // make a copy of each string
        for ( i = 0; i < Alen; i++ )
        {
            jstring    j_string   = (jstring) ( *jenv )->GetObjectArrayElement( jenv, $input, i );
            const char * c_string = (const char *) ( *jenv )->GetStringUTFChars( jenv, j_string, 0 );
            $1[i] = malloc( ( strlen( c_string ) + 1 ) * sizeof ( const char * ) );
            strcpy( $1[i], c_string );
            ( *jenv )->ReleaseStringUTFChars( jenv, j_string, c_string );
            ( *jenv )->DeleteLocalRef( jenv, j_string );
        }
    }
    else
    {
        $1 = NULL;
    }
}

// This cleans up the memory we malloc'd before the function call
%typemap ( freearg ) ( const char **ArrayCk )
{
    int i;
    if ( $1 != NULL )
    {
        for ( i = 0; i < Alen; i++ )
            free( $1[i] );
        free( $1 );
    }
}
%typemap ( jni ) ( const char **ArrayCk ) "jobjectArray"
%typemap ( jtype ) ( const char **ArrayCk ) "String[]"
%typemap ( jstype ) ( const char **ArrayCk ) "String[]"
%typemap ( javain ) ( const char **ArrayCk ) "$javainput"
%typemap ( javaout ) ( const char **ArrayCk )
{
    return $jnicall;
}

%typemap ( in ) ( PLINT n, const char **Array )
{
    int i = 0;
    if ( $input != NULL )
    {
        int size = ( *jenv )->GetArrayLength( jenv, $input );
        Alen = size;
        $1   = size;
        $2   = (char **) malloc( (size_t) Alen * sizeof ( char * ) );
        // make a copy of each string
        for ( i = 0; i < Alen; i++ )
        {
            jstring    j_string   = (jstring) ( *jenv )->GetObjectArrayElement( jenv, $input, i );
            const char * c_string = (const char *) ( *jenv )->GetStringUTFChars( jenv, j_string, 0 );
            $2[i] = malloc( ( strlen( c_string ) + 1 ) * sizeof ( const char * ) );
            strcpy( $2[i], c_string );
            ( *jenv )->ReleaseStringUTFChars( jenv, j_string, c_string );
            ( *jenv )->DeleteLocalRef( jenv, j_string );
        }
    }
    else
    {
        $1 = 0;
        $2 = NULL;
    }
}

// This cleans up the memory we malloc'd before the function call
%typemap ( freearg ) ( PLINT n, const char **Array )
{
    int i;
    if ( $2 != NULL )
    {
        for ( i = 0; i < Alen; i++ )
            free( $2[i] );
        free( $2 );
    }
}
%typemap ( jni ) ( PLINT n, const char **Array ) "jobjectArray"
%typemap ( jtype ) ( PLINT n, const char **Array ) "String[]"
%typemap ( jstype ) ( PLINT n, const char **Array ) "String[]"
%typemap ( javain ) ( PLINT n, const char **Array ) "$javainput"
%typemap ( javaout ) ( PLINT n, const char **Array )
{
    return $jnicall;
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

// swig compatible PLplot API definitions from here on.
%include plplotcapi.i
