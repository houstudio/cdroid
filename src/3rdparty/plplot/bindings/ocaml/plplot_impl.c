//
// Copyright 2007, 2008, 2009, 2010, 2011 Hezekiah M. Carty
// Copyright (C) 2016-2018 Alan W. Irwin
//
// This file is part of PLplot.
//
// PLplot is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with PLplot.  If not, see <http://www.gnu.org/licenses/>.
//

// The "usual" OCaml includes
#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/misc.h>
#include <caml/mlvalues.h>
#include <caml/bigarray.h>

#include <plplotP.h>
#include <plplot.h>

#undef snprintf

#include <stdio.h>

#define MAX_EXCEPTION_MESSAGE_LENGTH       1000
#define CAML_PLPLOT_PLOTTER_FUNC_NAME      "caml_plplot_plotter"
#define CAML_PLPLOT_MAPFORM_FUNC_NAME      "caml_plplot_mapform"
#define CAML_PLPLOT_DEFINED_FUNC_NAME      "caml_plplot_defined"
#define CAML_PLPLOT_LABEL_FUNC_NAME        "caml_plplot_customlabel"
#define CAML_PLPLOT_ABORT_FUNC_NAME        "caml_plplot_abort"
#define CAML_PLPLOT_EXIT_FUNC_NAME         "caml_plplot_exit"
#define CAML_PLPLOT_TRANSFORM_FUNC_NAME    "caml_plplot_transform"

typedef void ( *ML_PLOTTER_FUNC )( PLFLT, PLFLT, PLFLT*, PLFLT*, PLPointer );
typedef PLINT ( *ML_DEFINED_FUNC )( PLFLT, PLFLT );
typedef void ( *ML_MAPFORM_FUNC )( PLINT, PLFLT*, PLFLT* );
typedef void ( *ML_LABEL_FUNC )( PLINT, PLFLT, char*, PLINT, PLPointer );
typedef PLINT ( *ML_VARIANT_FUNC )( PLINT );

//
//
// CALLBACK WRAPPERS
//
//

// A simple routine to wrap a properly registered OCaml callback in a form
// usable by PLPlot routines.  If an appropriate callback is not registered
// then the PLPlot built-in pltr0 function is used instead.
void ml_plotter( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, PLPointer pltr_data )
{
    CAMLparam0();
    CAMLlocal1( result );

    // Get the OCaml callback function (if there is one)
    static value * pltr = NULL;
    if ( pltr == NULL )
        pltr = caml_named_value( CAML_PLPLOT_PLOTTER_FUNC_NAME );

    // No check to see if a callback function has been designated yet,
    // because that is checked before we get to this point.
    result =
        caml_callback2( *pltr, caml_copy_double( x ), caml_copy_double( y ) );
    double new_x, new_y;
    new_x = Double_val( Field( result, 0 ) );
    new_y = Double_val( Field( result, 1 ) );

    *tx = new_x;
    *ty = new_y;

    CAMLreturn0;
}

// A simple routine to wrap a properly registered OCaml callback in a form
// usable by PLPlot routines.  If an appropriate callback is not registered
// then the result is always 1 (the data point is defined).
// This function is used in the plshade* functions to determine if a given data
// point is valid/defined or not.
PLINT ml_defined( PLFLT x, PLFLT y )
{
    CAMLparam0();
    CAMLlocal1( result );

    // The result which will be returned to the user.
    PLINT is_it_defined;

    // Get the OCaml callback function (if there is one)
    static value * defined = NULL;
    if ( defined == NULL )
        defined = caml_named_value( CAML_PLPLOT_DEFINED_FUNC_NAME );

    // No check to see if a callback function has been designated yet,
    // because that is checked before we get to this point.
    result =
        caml_callback2( *defined, caml_copy_double( x ), caml_copy_double( y ) );
    is_it_defined = Int_val( result );

    CAMLreturn( is_it_defined );
}

// A simple routine to wrap a properly registered OCaml callback in a form
// usable by PLPlot routines.  If an appropriate callback is not registered
// then nothing is done.
void ml_mapform( PLINT n, PLFLT *x, PLFLT *y )
{
    CAMLparam0();
    CAMLlocal1( result );

    // Get the OCaml callback function (if there is one)
    static value * mapform = NULL;
    if ( mapform == NULL )
        mapform = caml_named_value( CAML_PLPLOT_MAPFORM_FUNC_NAME );

    // No check to see if a callback function has been designated yet,
    // because that is checked before we get to this point.
    int i;
    for ( i = 0; i < n; i++ )
    {
        result =
            caml_callback2( *mapform,
                caml_copy_double( x[i] ), caml_copy_double( y[i] ) );

        double new_x, new_y;
        new_x = Double_val( Field( result, 0 ) );
        new_y = Double_val( Field( result, 1 ) );

        x[i] = new_x;
        y[i] = new_y;
    }

    CAMLreturn0;
}

// A simple routine to wrap a properly registered OCaml callback in a form
// usable by PLPlot routines.
void ml_labelfunc( PLINT axis, PLFLT n, char *label, PLINT length, PLPointer d )
{
    CAMLparam0();
    CAMLlocal1( result );

    // Get the OCaml callback function (if there is one)
    static value * callback = NULL;
    if ( callback == NULL )
        callback = caml_named_value( CAML_PLPLOT_LABEL_FUNC_NAME );

    // No check to see if a callback function has been designated yet,
    // because that is checked before we get to this point.
    result =
        caml_callback2( *callback, Val_int( axis - 1 ), caml_copy_double( n ) );

    // Copy the OCaml callback output to the proper location.
    snprintf( label, length, "%s", String_val( result ) );

    CAMLreturn0;
}

// OCaml callback for plsabort
void ml_abort( const char* message )
{
    CAMLparam0();
    CAMLlocal1( result );

    // Get the OCaml callback function (if there is one)
    static value * handler = NULL;
    if ( handler == NULL )
        handler = caml_named_value( CAML_PLPLOT_ABORT_FUNC_NAME );

    // No check to see if a callback function has been designated yet,
    // because that is checked before we get to this point.
    result =
        caml_callback( *handler, caml_copy_string( message ) );

    CAMLreturn0;
}

// OCaml callback for plsexit
int ml_exit( const char* message )
{
    CAMLparam0();
    CAMLlocal1( result );

    // Get the OCaml callback function (if there is one)
    static value * handler = NULL;
    if ( handler == NULL )
        handler = caml_named_value( CAML_PLPLOT_EXIT_FUNC_NAME );

    // No check to see if a callback function has been designated yet,
    // because that is checked before we get to this point.
    result =
        caml_callback( *handler, caml_copy_string( message ) );

    CAMLreturn( Int_val( result ) );
}

// A simple routine to wrap a properly registered OCaml callback in a form
// usable by PLPlot routines.  If an appropriate callback is not registered
// then nothing is done.
void ml_transform( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer data )
{
    CAMLparam0();
    CAMLlocal1( result );

    // Get the OCaml callback function (if there is one)
    static value * transform = NULL;
    if ( transform == NULL )
        transform = caml_named_value( CAML_PLPLOT_TRANSFORM_FUNC_NAME );

    // No check to see if a callback function has been designated yet,
    // because that is checked before we get to this point.
    result =
        caml_callback2( *transform, caml_copy_double( x ), caml_copy_double( y ) );

    *xt = Double_val( Field( result, 0 ) );
    *yt = Double_val( Field( result, 1 ) );

    CAMLreturn0;
}

// Check if the matching OCaml callback is defined.  Return NULL if it is not,
// and the proper function pointer if it is.
ML_PLOTTER_FUNC get_ml_plotter_func()
{
    static value * pltr = NULL;
    if ( pltr == NULL )
        pltr = caml_named_value( CAML_PLPLOT_PLOTTER_FUNC_NAME );

    if ( pltr == NULL || Val_int( 0 ) == *pltr )
    {
        // No plotter defined
        return NULL;
    }
    else
    {
        // Plotter is defined
        return ml_plotter;
    }
}
ML_DEFINED_FUNC get_ml_defined_func()
{
    static value * defined = NULL;
    if ( defined == NULL )
        defined = caml_named_value( CAML_PLPLOT_DEFINED_FUNC_NAME );

    if ( defined == NULL || Val_int( 0 ) == *defined )
    {
        // No plotter defined
        return NULL;
    }
    else
    {
        // Plotter is defined
        return ml_defined;
    }
}
ML_MAPFORM_FUNC get_ml_mapform_func()
{
    static value * mapform = NULL;
    if ( mapform == NULL )
        mapform = caml_named_value( CAML_PLPLOT_MAPFORM_FUNC_NAME );

    if ( mapform == NULL || Val_int( 0 ) == *mapform )
    {
        // No plotter defined
        return NULL;
    }
    else
    {
        // Plotter is defined
        return ml_mapform;
    }
}

// Custom wrapper for plslabelfunc
value ml_plslabelfunc( value unit )
{
    CAMLparam1( unit );
    static value * label = NULL;
    if ( label == NULL )
        label = caml_named_value( CAML_PLPLOT_LABEL_FUNC_NAME );

    if ( label == NULL || Val_int( 0 ) == *label )
    {
        // No plotter defined
        plslabelfunc( NULL, NULL );
    }
    else
    {
        // Plotter is defined
        plslabelfunc( ml_labelfunc, NULL );
    }

    CAMLreturn( Val_unit );
}

// Custom wrappers for plsabort and plsexit
value ml_plsabort( value unit )
{
    CAMLparam1( unit );
    static value * handler = NULL;
    if ( handler == NULL )
        handler = caml_named_value( CAML_PLPLOT_ABORT_FUNC_NAME );

    if ( handler == NULL || Val_int( 0 ) == *handler )
    {
        // No handler defined
        plsabort( NULL );
    }
    else
    {
        // Handler is defined
        plsabort( ml_abort );
    }
    CAMLreturn( Val_unit );
}
value ml_plsexit( value unit )
{
    CAMLparam1( unit );
    static value * handler = NULL;
    if ( handler == NULL )
        handler = caml_named_value( CAML_PLPLOT_EXIT_FUNC_NAME );

    if ( handler == NULL || Val_int( 0 ) == *handler )
    {
        // No handler defined
        plsexit( NULL );
    }
    else
    {
        // Handler is defined
        plsexit( ml_exit );
    }
    CAMLreturn( Val_unit );
}

// Set a global coordinate transform
value ml_plstransform( value unit )
{
    CAMLparam1( unit );
    static value * handler = NULL;
    if ( handler == NULL )
        handler = caml_named_value( CAML_PLPLOT_TRANSFORM_FUNC_NAME );

    if ( handler == NULL || Val_int( 0 ) == *handler )
    {
        // No handler defined
        plstransform( NULL, NULL );
    }
    else
    {
        // Handler is defined
        plstransform( ml_transform, NULL );
    }
    CAMLreturn( Val_unit );
}

//
//
// CONTOURING, SHADING and IMAGE FUNCTIONS
//
//

//
// void
// c_plcont(PLFLT **f, PLINT nx, PLINT ny, PLINT kx, PLINT lx,
// PLINT ky, PLINT ly, PLFLT *clevel, PLINT nlevel,
// void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
// PLPointer pltr_data);
//
void ml_plcont( const PLFLT **f, PLINT nx, PLINT ny,
                PLINT kx, PLINT lx, PLINT ky, PLINT ly,
                PLFLT *clevel, PLINT nlevel )
{
    if ( get_ml_plotter_func() == NULL )
    {
        // This is handled in PLplot, but the error is raised here to clarify
        // what the user needs to do since the custom plotter is defined
        // separately from the call to plcont.
        caml_invalid_argument( "A custom plotter must be defined \
                               before calling plcont" );
    }
    else
    {
        c_plcont( f, nx, ny, kx, lx, ky, ly, clevel, nlevel,
            get_ml_plotter_func(), (void *) 1 );
    }
}

//
// void
// c_plshade(PLFLT **a, PLINT nx, PLINT ny, PLINT (*defined) (PLFLT, PLFLT),
// PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
// PLFLT shade_min, PLFLT shade_max,
// PLINT sh_cmap, PLFLT sh_color, PLINT sh_width,
// PLINT min_color, PLINT min_width,
// PLINT max_color, PLINT max_width,
// void (*fill) (PLINT, PLFLT *, PLFLT *), PLBOOL rectangular,
// void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
// PLPointer pltr_data);
//
void ml_plshade( const PLFLT **a, PLINT nx, PLINT ny,
                 PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
                 PLFLT shade_min, PLFLT shade_max,
                 PLINT sh_cmap, PLFLT sh_color, PLFLT sh_width,
                 PLINT min_color, PLFLT min_width,
                 PLINT max_color, PLFLT max_width,
                 PLBOOL rectangular )
{
    c_plshade( a, nx, ny,
        get_ml_defined_func(),
        left, right, bottom, top,
        shade_min, shade_max,
        sh_cmap, sh_color, sh_width, min_color, min_width,
        max_color, max_width, plfill, rectangular,
        get_ml_plotter_func(), (void *) 1 );
}

//
// void
// c_plshades( PLFLT **a, PLINT nx, PLINT ny, PLINT (*defined) (PLFLT, PLFLT),
// PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
// PLFLT *clevel, PLINT nlevel, PLINT fill_width,
// PLINT cont_color, PLINT cont_width,
// void (*fill) (PLINT, PLFLT *, PLFLT *), PLBOOL rectangular,
// void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
// PLPointer pltr_data);
//
void ml_plshades( const PLFLT **a, PLINT nx, PLINT ny,
                  PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
                  PLFLT *clevel, PLINT nlevel, PLFLT fill_width,
                  PLINT cont_color, PLFLT cont_width,
                  PLBOOL rectangular )
{
    c_plshades( a, nx, ny,
        get_ml_defined_func(),
        xmin, xmax, ymin, ymax,
        clevel, nlevel, fill_width,
        cont_color, cont_width,
        plfill, rectangular,
        get_ml_plotter_func(),
        (void *) 1 );
}

//
// void
// c_plimagefr(PLFLT **idata, PLINT nx, PLINT ny,
//      PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax, PLFLT zmin, PLFLT zmax,
//      PLFLT valuemin, PLFLT valuemax,
//      void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
//      PLPointer pltr_data);
//
void ml_plimagefr( const PLFLT **idata, PLINT nx, PLINT ny,
                   PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
                   PLFLT zmin, PLFLT zmax,
                   PLFLT valuemin, PLFLT valuemax )
{
    c_plimagefr( idata, nx, ny,
        xmin, xmax, ymin, ymax,
        zmin, zmax,
        valuemin, valuemax,
        get_ml_plotter_func(),
        (void *) 1 );
}

//
// void
// c_plvect(PLFLT **u, PLFLT **v, PLINT nx, PLINT ny, PLFLT scale,
// void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
//      PLPointer pltr_data);
//
void ml_plvect( const PLFLT **u, const PLFLT **v, PLINT nx, PLINT ny, PLFLT scale )
{
    c_plvect( u, v, nx, ny, scale,
        get_ml_plotter_func(),
        (void *) 1 );
}

//
// Wrapper to reset vector rendering
//
void ml_plsvect_reset()
{
    c_plsvect( NULL, NULL, 0, 0 );
}

// Plot continental outline in world coordinates

// c_plmap( PLMAPFORM_callback mapform, PLCHAR_VECTOR name,
//         PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy );

void ml_plmap( PLCHAR_VECTOR name,
               PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy )
{
    c_plmap( get_ml_mapform_func(),
        name, minx, maxx, miny, maxy );
}

// Plot map outlines

// void
// c_plmapline( PLMAPFORM_callback mapform, PLCHAR_VECTOR name,
//              PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
//              PLINT_VECTOR plotentries, PLINT nplotentries );

void ml_plmapline( PLCHAR_VECTOR name,
                   PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                   PLINT_VECTOR plotentries, PLINT nplotentries )
{
    if ( nplotentries > 0 )
        c_plmapline( get_ml_mapform_func(), name,
            minx, maxx, miny, maxy,
            plotentries, nplotentries );
    else
        c_plmapline( get_ml_mapform_func(), name,
            minx, maxx, miny, maxy,
            NULL, nplotentries );
}

// Plot map points

// void
// c_plmapstring( PLMAPFORM_callback mapform,
//                PLCHAR_VECTOR name, PLCHAR_VECTOR string,
//                PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
//                PLINT_VECTOR plotentries, PLINT nplotentries );

void ml_plmapstring( PLCHAR_VECTOR name, PLCHAR_VECTOR string,
                     PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                     PLINT_VECTOR plotentries, PLINT nplotentries )
{
    if ( nplotentries > 0 )
        c_plmapstring( get_ml_mapform_func(), name, string,
            minx, maxx, miny, maxy,
            plotentries, nplotentries );
    else
        c_plmapstring( get_ml_mapform_func(), name, string,
            minx, maxx, miny, maxy,
            NULL, nplotentries );
}

// Plot map text

// void
// c_plmaptex( PLMAPFORM_callback mapform,
//             PLCHAR_VECTOR name, PLFLT dx, PLFLT dy, PLFLT just, PLCHAR_VECTOR text,
//             PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
//             PLINT plotentry );

void ml_plmaptex( PLCHAR_VECTOR name, PLFLT dx, PLFLT dy, PLFLT just, PLCHAR_VECTOR text,
                  PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                  PLINT plotentry )
{
    c_plmaptex( get_ml_mapform_func(), name, dx, dy, just, text,
        minx, maxx, miny, maxy,
        plotentry );
}

// Plot map fills

// void
// c_plmapfill( PLMAPFORM_callback mapform,
//              PLCHAR_VECTOR name, PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
//              PLINT_VECTOR plotentries, PLINT nplotentries );

void ml_plmapfill( PLCHAR_VECTOR name,
                   PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                   PLINT_VECTOR plotentries, PLINT nplotentries )
{
    if ( nplotentries > 0 )
        c_plmapfill( get_ml_mapform_func(), name,
            minx, maxx, miny, maxy,
            plotentries, nplotentries );
    else
        c_plmapfill( get_ml_mapform_func(), name,
            minx, maxx, miny, maxy,
            NULL, nplotentries );
}

//
// void
// c_plmeridians( void (*mapform)(PLINT, PLFLT *, PLFLT *),
//             PLFLT dlong, PLFLT dlat,
//             PLFLT minlong, PLFLT maxlong, PLFLT minlat, PLFLT maxlat );
//
void ml_plmeridians( PLFLT dlong, PLFLT dlat,
                     PLFLT minlong, PLFLT maxlong, PLFLT minlat, PLFLT maxlat )
{
    c_plmeridians( get_ml_mapform_func(),
        dlong, dlat, minlong, maxlong, minlat, maxlat );
}

//
// void
// c_plgriddata(PLFLT *x, PLFLT *y, PLFLT *z, PLINT npts,
//  PLFLT *xg, PLINT nptsx, PLFLT *yg, PLINT nptsy,
//  PLFLT **zg, PLINT type, PLFLT data);
//
// This one is currently wrapped by hand, as I am not sure how to get camlidl
// to allocate zg in a way that makes plgriddata happy and doesn't require the
// user to pre-allocate the space.
value ml_plgriddata( value x, value y, value z,
                     value xg, value yg,
                     value type, value data )
{
    CAMLparam5( x, y, z, xg, yg );
    CAMLxparam2( type, data );

    // zg holds the OCaml float array array.
    // y_ml_array is a temporary structure which will be used to form each
    // float array making up zg.
    CAMLlocal2( zg, y_ml_array );

    PLFLT **zg_local;

    int   npts, nptsx, nptsy;
    int   i, j;

    // Check to make sure x, y and z are all the same length.
    npts = Wosize_val( x ) / Double_wosize;
    if ( ( Wosize_val( y ) / Double_wosize != Wosize_val( z ) / Double_wosize ) ||
         ( Wosize_val( y ) / Double_wosize != npts ) ||
         ( Wosize_val( z ) / Double_wosize != npts )
         )
    {
        caml_failwith( "ml_plgriddata: x, y, z must all have the same dimensions" );
    }

    nptsx = Wosize_val( xg ) / Double_wosize;
    nptsy = Wosize_val( yg ) / Double_wosize;

    // Allocate the 2D grid in a way that will make PLplot happy
    plAlloc2dGrid( &zg_local, nptsx, nptsy );

    // Using "type + 1" because "type" is passed in as a variant type, so
    // the indexing starts from 0 rather than 1.
    c_plgriddata( (double *) x, (double *) y, (double *) z, npts, (double *) xg, nptsx,
        (double *) yg, nptsy, zg_local, Int_val( type ) + 1,
        Double_val( data ) );

    // Allocate the X-dimension of the to-be-returned OCaml array
    zg = caml_alloc( nptsx, 0 );

    for ( i = 0; i < nptsx; i++ )
    {
        // Allocate each Y-dimension array of the OCaml array
        y_ml_array = caml_alloc( nptsy * Double_wosize, Double_array_tag );
        for ( j = 0; j < nptsy; j++ )
        {
            Store_double_field( y_ml_array, j, zg_local[i][j] );
        }
        caml_modify( &Field( zg, i ), y_ml_array );
    }

    // Free the memory used by the C array
    plFree2dGrid( zg_local, nptsx, nptsy );

    CAMLreturn( zg );
}

value ml_plgriddata_bytecode( value* argv, int argn )
{
    return ml_plgriddata( argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6] );
}

//
// void
// c_plpoly3(PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, PLBOOL *draw, PLBOOL ifcc);
//
// plpoly3 is wrapped by hand because draw has a length of (n - 1) and camlidl
// does not have a way to indicate this automatically.
void ml_plpoly3( PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, PLINT ndraw, PLBOOL *draw, PLBOOL ifcc )
{
    plpoly3( n, x, y, z, draw, ifcc );
}

// Raise Invalid_argument if the given value is <> 0
void plplot_check_nonzero_result( int result )
{
    if ( result != 0 )
    {
        char exception_message[MAX_EXCEPTION_MESSAGE_LENGTH];
        sprintf( exception_message, "Error, return code %d", result );
        caml_invalid_argument( exception_message );
    }
    return;
}

// Translate the integer version of the OCaml variant to the appropriate
// PLplot constant.
int translate_parse_option( int parse_option )
{
    int translated_option;
    switch ( parse_option )
    {
    case 0: translated_option  = PL_PARSE_PARTIAL; break;
    case 1: translated_option  = PL_PARSE_FULL; break;
    case 2: translated_option  = PL_PARSE_QUIET; break;
    case 3: translated_option  = PL_PARSE_NODELETE; break;
    case 4: translated_option  = PL_PARSE_SHOWALL; break;
    case 5: translated_option  = PL_PARSE_OVERRIDE; break;
    case 6: translated_option  = PL_PARSE_NOPROGRAM; break;
    case 7: translated_option  = PL_PARSE_NODASH; break;
    case 8: translated_option  = PL_PARSE_SKIP; break;
    default: translated_option = -1;
    }
    return translated_option;
}

// Copy to a const string array
#define INIT_STRING_ARRAY( o )         \
    int o ## _length;                  \
    o ## _length = Wosize_val( o );    \
    const char *c_ ## o[o ## _length]; \
    for ( i = 0; i < o ## _length; i++ ) { c_ ## o[i] = String_val( Field( o, i ) ); }

// Copy to a non-const string array
#define INIT_NC_STRING_ARRAY( o )   \
    int o ## _length;               \
    o ## _length = Wosize_val( o ); \
    char *c_ ## o[o ## _length];    \
    for ( i = 0; i < o ## _length; i++ ) { c_ ## o[i] = String_val( Field( o, i ) ); }

// Copy an int array, o, of n element to the C array c
#define INIT_INT_ARRAY( o )         \
    int o ## _length;               \
    o ## _length = Wosize_val( o ); \
    int c_ ## o[o ## _length];      \
    for ( i = 0; i < ( o ## _length ); i++ ) { ( c_ ## o )[i] = Int_val( Field( ( o ), i ) ); }

// Copy an int array, o, of n element to the C array c
#define INIT_INT_ARRAYS( o )                   \
    int o ## _length, o ## _inner;             \
    o ## _length = Wosize_val( o );            \
    int *c_ ## o[o ## _length];                \
    for ( i = 0; i < ( o ## _length ); i++ ) { \
        INIT_INT_ARRAY( o ## _subarray );      \
        ( c_ ## o )[i] = c_ ## o ## _subarray; \
    }

int lor_ml_list( value list, ML_VARIANT_FUNC variant_f )
{
    CAMLparam1( list );
    int result;

    result = 0;
    while ( list != Val_emptylist )
    {
        // Accumulate the elements of the list
        result = result | variant_f( Int_val( Field( list, 0 ) ) );
        // Point to the tail of the list for the next loop
        list = Field( list, 1 );
    }

    CAMLreturn( result );
}

value ml_plparseopts( value argv, value parse_method )
{
    CAMLparam2( argv, parse_method );
    int   i;
    PLINT result;
    int   combined_parse_method;
    // Make a copy of the command line argument strings
    INIT_NC_STRING_ARRAY( argv )

    // OR the elements of the parse_method list together
    combined_parse_method = lor_ml_list( parse_method, translate_parse_option );

    result = plparseopts( &argv_length, c_argv, combined_parse_method );
    if ( result != 0 )
    {
        char exception_message[MAX_EXCEPTION_MESSAGE_LENGTH];
        sprintf( exception_message, "Invalid arguments in plparseopts, error %d", result );
        caml_invalid_argument( exception_message );
    }
    CAMLreturn( Val_unit );
}

value ml_plstripc( value xspec, value yspec, value xmin, value xmax, value xjump,
                   value ymin, value ymax, value xlpos, value ylpos, value y_ascl,
                   value acc, value colbox, value collab, value colline, value styline,
                   value legline, value labx, value laby, value labtop )
{
    // Function parameters
    CAMLparam5( xspec, yspec, xmin, xmax, xjump );
    CAMLxparam5( ymin, ymax, xlpos, ylpos, y_ascl );
    CAMLxparam5( acc, colbox, collab, colline, styline );
    CAMLxparam4( legline, labx, laby, labtop );
    // Line attribute array copies
    int       colline_copy[4];
    int       styline_copy[4];
    const char* legend_copy[4];
    int       i;
    for ( i = 0; i < 4; i++ )
    {
        colline_copy[i] = Int_val( Field( colline, i ) );
        styline_copy[i] = Int_val( Field( styline, i ) );
        legend_copy[i]  = String_val( Field( legline, i ) );
    }
    // The returned value
    int id;
    plstripc( &id, String_val( xspec ), String_val( yspec ),
        Double_val( xmin ), Double_val( xmax ),
        Double_val( xjump ), Double_val( ymin ), Double_val( ymax ),
        Double_val( xlpos ), Double_val( ylpos ), Bool_val( y_ascl ),
        Bool_val( acc ), Int_val( colbox ), Int_val( collab ),
        colline_copy, styline_copy, legend_copy,
        String_val( labx ), String_val( laby ), String_val( labtop ) );
    // Make me do something!
    CAMLreturn( Val_int( id ) );
}

value ml_plstripc_byte( value* argv, int argn )
{
    return ml_plstripc( argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9],
        argv[10], argv[11], argv[12], argv[13], argv[14],
        argv[15], argv[16], argv[17], argv[18] );
}

int translate_legend_option( int legend_option )
{
    int translated_option;
    switch ( legend_option )
    {
    case 0: translated_option  = PL_LEGEND_NULL; break;
    case 1: translated_option  = PL_LEGEND_NONE; break;
    case 2: translated_option  = PL_LEGEND_COLOR_BOX; break;
    case 3: translated_option  = PL_LEGEND_LINE; break;
    case 4: translated_option  = PL_LEGEND_SYMBOL; break;
    case 5: translated_option  = PL_LEGEND_TEXT_LEFT; break;
    case 6: translated_option  = PL_LEGEND_BACKGROUND; break;
    case 7: translated_option  = PL_LEGEND_BOUNDING_BOX; break;
    case 8: translated_option  = PL_LEGEND_ROW_MAJOR; break;
    default: translated_option = -1;
    }
    return translated_option;
}

int translate_colorbar_option( int colorbar_option )
{
    int translated_option;
    switch ( colorbar_option )
    {
    case 0: translated_option  = PL_COLORBAR_NULL; break;
    case 1: translated_option  = PL_COLORBAR_LABEL_LEFT; break;
    case 2: translated_option  = PL_COLORBAR_LABEL_RIGHT; break;
    case 3: translated_option  = PL_COLORBAR_LABEL_TOP; break;
    case 4: translated_option  = PL_COLORBAR_LABEL_BOTTOM; break;
    case 5: translated_option  = PL_COLORBAR_IMAGE; break;
    case 6: translated_option  = PL_COLORBAR_SHADE; break;
    case 7: translated_option  = PL_COLORBAR_GRADIENT; break;
    case 8: translated_option  = PL_COLORBAR_CAP_NONE; break;
    case 9: translated_option  = PL_COLORBAR_CAP_LOW; break;
    case 10: translated_option = PL_COLORBAR_CAP_HIGH; break;
    case 11: translated_option = PL_COLORBAR_SHADE_LABEL; break;
    case 12: translated_option = PL_COLORBAR_ORIENT_RIGHT; break;
    case 13: translated_option = PL_COLORBAR_ORIENT_TOP; break;
    case 14: translated_option = PL_COLORBAR_ORIENT_LEFT; break;
    case 15: translated_option = PL_COLORBAR_ORIENT_BOTTOM; break;
    case 16: translated_option = PL_COLORBAR_BACKGROUND; break;
    case 17: translated_option = PL_COLORBAR_BOUNDING_BOX; break;
    default: translated_option = -1;
    }
    return translated_option;
}

int translate_position_option( int position_option )
{
    int translated_option;
    switch ( position_option )
    {
    case 0: translated_option  = PL_POSITION_NULL; break;
    case 1: translated_option  = PL_POSITION_LEFT; break;
    case 2: translated_option  = PL_POSITION_RIGHT; break;
    case 3: translated_option  = PL_POSITION_TOP; break;
    case 4: translated_option  = PL_POSITION_BOTTOM; break;
    case 5: translated_option  = PL_POSITION_INSIDE; break;
    case 6: translated_option  = PL_POSITION_OUTSIDE; break;
    case 7: translated_option  = PL_POSITION_VIEWPORT; break;
    case 8: translated_option  = PL_POSITION_SUBPAGE; break;
    default: translated_option = -1;
    }
    return translated_option;
}

value ml_pllegend( value opt, value position, value x, value y, value plot_width,
                   value bg_color,
                   value bb_color, value bb_style,
                   value nrow, value ncolumn,
                   value opt_array,
                   value text_offset, value text_scale, value text_spacing,
                   value text_justification, value text_colors, value text,
                   value box_colors, value box_patterns, value box_scales,
                   value box_line_widths,
                   value line_colors, value line_styles, value line_widths,
                   value symbol_colors, value symbol_scales,
                   value symbol_numbers, value symbols )
{
    CAMLparam5( position, opt, x, y, plot_width );
    CAMLxparam5( bg_color, bb_color, bb_style, nrow, ncolumn );
    CAMLxparam5( opt_array, text_offset, text_scale, text_spacing, text_justification );
    CAMLxparam5( text_colors, text, box_colors, box_patterns, box_scales );
    CAMLxparam5( box_line_widths, line_colors, line_styles, line_widths, symbol_colors );
    CAMLxparam3( symbol_scales, symbol_numbers, symbols );
    CAMLlocal1( result );
    result = caml_alloc( 2, 0 );

    // Counter
    int i;
    // General legend options
    int c_position, c_opt;
    // Number of legend entries
    int n_legend;
    n_legend = Wosize_val( opt_array );
    // Options for each legend entry
    int c_opt_array[n_legend];

    // Assume that the dimensions all line up on the OCaml side, so we don't
    // need to do any further dimension checks.

    // Define and initialize all of the C arrays to pass in to pllegend
    INIT_STRING_ARRAY( text )
    INIT_INT_ARRAY( text_colors )
    INIT_INT_ARRAY( box_colors )
    INIT_INT_ARRAY( box_patterns )
    INIT_INT_ARRAY( line_colors )
    INIT_INT_ARRAY( line_styles )
    INIT_INT_ARRAY( symbol_colors )
    INIT_INT_ARRAY( symbol_numbers )
    INIT_STRING_ARRAY( symbols )

    // Translate the legend configuration options
    c_opt      = lor_ml_list( opt, translate_legend_option );
    c_position = lor_ml_list( position, translate_position_option );

    for ( i = 0; i < n_legend; i++ )
    {
        c_opt_array[i] =
            lor_ml_list( Field( opt_array, i ), translate_legend_option );
    }

    // The returned width and height of the legend
    PLFLT width, height;

    pllegend( &width, &height, c_opt, c_position, Double_val( x ), Double_val( y ),
        Double_val( plot_width ), Int_val( bg_color ),
        Int_val( bb_color ), Int_val( bb_style ),
        Int_val( nrow ), Int_val( ncolumn ),
        n_legend, c_opt_array,
        Double_val( text_offset ), Double_val( text_scale ),
        Double_val( text_spacing ),
        Double_val( text_justification ),
        c_text_colors, c_text,
        c_box_colors, c_box_patterns, (double *) box_scales,
        (double *) box_line_widths,
        c_line_colors, c_line_styles, (double *) line_widths,
        c_symbol_colors, (double *) symbol_scales, c_symbol_numbers,
        c_symbols );

    // Return a tuple with the legend's size
    Store_field( result, 0, caml_copy_double( width ) );
    Store_field( result, 1, caml_copy_double( height ) );

    CAMLreturn( result );
}

value ml_pllegend_byte( value* argv, int argn )
{
    return ml_pllegend( argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9],
        argv[10], argv[11], argv[12], argv[13], argv[14],
        argv[15], argv[16], argv[17], argv[18], argv[19],
        argv[20], argv[21], argv[22], argv[23], argv[24],
        argv[25], argv[26], argv[27] );
}

value ml_plcolorbar( value opt, value position, value x, value y,
                     value x_length, value y_length,
                     value bg_color, value bb_color, value bb_style,
                     value low_cap_color, value high_cap_color,
                     value cont_color, value cont_width,
                     value label_opts, value label,
                     value axis_opts,
                     value ticks, value sub_ticks,
                     value values )
{
    CAMLparam5( opt, position, x, y, x_length );
    CAMLxparam5( y_length, bg_color, bb_color, bb_style, low_cap_color );
    CAMLxparam5( high_cap_color, cont_color, cont_width, label_opts, label );
    CAMLxparam4( axis_opts, ticks, sub_ticks, values );
    CAMLlocal1( result );
    result = caml_alloc( 2, 0 );

    // Counter
    int i;
    // General colorbar options
    int c_opt, c_position;
    // Number of labels
    int n_labels;
    n_labels = Wosize_val( label_opts );
    // Number of axes and value ranges
    int n_axes;
    n_axes = Wosize_val( axis_opts );

    // Translate configuration options
    c_opt      = lor_ml_list( opt, translate_colorbar_option );
    c_position = lor_ml_list( position, translate_position_option );

    // Assume that the dimensions all line up on the OCaml side, so we don't
    // need to do any further dimension checks.

    // Define and initialize all of the C arrays to pass into plcolorbar
    INIT_STRING_ARRAY( label )
    INIT_STRING_ARRAY( axis_opts )
    INIT_INT_ARRAY( sub_ticks );

    // Label options
    int c_label_opts[ n_labels ];
    for ( i = 0; i < n_labels; i++ )
    {
        c_label_opts[i] = lor_ml_list( Field( label_opts, i ), translate_colorbar_option );
    }

    // Copy the axis/range values
    double **c_values;
    int    n_values[ n_axes ];
    c_values = malloc( n_axes * sizeof ( double * ) );
    // TODO: Add allocation failure check
    for ( i = 0; i < n_axes; i++ )
    {
        c_values[i] = (double *) Field( values, i );
        n_values[i] = Wosize_val( Field( values, i ) ) / Double_wosize;
    }

    // Return values
    PLFLT width, height;

    plcolorbar( &width, &height,
        c_opt, c_position, Double_val( x ), Double_val( y ),
        Double_val( x_length ), Double_val( y_length ),
        Int_val( bg_color ), Int_val( bb_color ), Int_val( bb_style ),
        Double_val( low_cap_color ), Double_val( high_cap_color ),
        Int_val( cont_color ), Double_val( cont_width ),
        n_labels, c_label_opts, c_label,
        n_axes, c_axis_opts,
        (double *) ticks, c_sub_ticks,
        n_values, (const PLFLT * const *) c_values );

    // Return a tuple with the colorbar's size
    Store_field( result, 0, caml_copy_double( width ) );
    Store_field( result, 1, caml_copy_double( height ) );

    CAMLreturn( result );
}

value ml_plcolorbar_byte( value *argv, int argn )
{
    return ml_plcolorbar( argv[0], argv[1], argv[2], argv[3], argv[4],
        argv[5], argv[6], argv[7], argv[8], argv[9],
        argv[10], argv[11], argv[12], argv[13], argv[14],
        argv[15], argv[16], argv[17], argv[18] );
}

// pltr* function implementations
void ml_pltr0( double x, double y, double* tx, double* ty )
{
    pltr0( x, y, tx, ty, NULL );
}

value ml_pltr1( value xg, value yg, value x, value y )
{
    CAMLparam4( xg, yg, x, y );
    CAMLlocal1( tx_ty );
    tx_ty = caml_alloc( 2, 0 );
    double  tx;
    double  ty;
    PLcGrid grid;
    grid.xg = (double *) xg;
    grid.yg = (double *) yg;
    grid.nx = Wosize_val( xg ) / Double_wosize;
    grid.ny = Wosize_val( yg ) / Double_wosize;
    pltr1( Double_val( x ), Double_val( y ), &tx, &ty, ( PLPointer ) & grid );

    // Allocate a tuple and return it with the results
    Store_field( tx_ty, 0, caml_copy_double( tx ) );
    Store_field( tx_ty, 1, caml_copy_double( ty ) );
    CAMLreturn( tx_ty );
}

value ml_pltr2( value xg, value yg, value x, value y )
{
    CAMLparam4( xg, yg, x, y );
    CAMLlocal1( tx_ty );
    tx_ty = caml_alloc( 2, 0 );
    double   ** c_xg;
    double   ** c_yg;
    int      i;
    int      length1;
    int      length2;
    PLcGrid2 grid;
    double   tx;
    double   ty;

    // TODO: As of now, you will probably get a segfault of the xg and yg
    // dimensions don't match up properly.
    // Build the grid.
    // Length of "outer" array
    length1 = Wosize_val( xg );
    // Length of the "inner" arrays
    length2 = Wosize_val( Field( xg, 0 ) ) / Double_wosize;
    c_xg    = malloc( length1 * sizeof ( double* ) );
    for ( i = 0; i < length1; i++ )
    {
        c_xg[i] = (double *) Field( xg, i );
    }
    c_yg = malloc( length1 * sizeof ( double* ) );
    for ( i = 0; i < length1; i++ )
    {
        c_yg[i] = (double *) Field( yg, i );
    }
    grid.xg = c_xg;
    grid.yg = c_yg;
    grid.nx = length1;
    grid.ny = length2;

    pltr2( Double_val( x ), Double_val( y ), &tx, &ty, ( PLPointer ) & grid );

    // Clean up
    free( c_xg );
    free( c_yg );

    // Allocate a tuple and return it with the results
    Store_field( tx_ty, 0, caml_copy_double( tx ) );
    Store_field( tx_ty, 1, caml_copy_double( ty ) );
    CAMLreturn( tx_ty );
}

// XXX Non-core functions follow XXX
//*
// The following functions are here for (my?) convenience.  As far as I can
// tell, they are not defined in the core PLplot library.
//

// Get the current color map 0 color index
int plg_current_col0( void )
{
    return plsc->icol0;
}

// Get the current color map 1 color index
PLFLT plg_current_col1( void )
{
    return plsc->icol1;
}

// Get the current pen width. TODO: Remove this, as I think this information
// can be retrieved from another proper PLplot function.
PLFLT plgwidth( void )
{
    return plsc->width;
}

// Get the current character (text) height in mm.  TODO: Remove this, as I
// think this information can be retrieved from another proper PLplot
// function
PLFLT plgchrht( void )
{
    return plsc->chrht;
}
