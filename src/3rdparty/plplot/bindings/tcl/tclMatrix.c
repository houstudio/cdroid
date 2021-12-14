//  Copyright 1994, 1995
//  Maurice LeBrun			mjl@dino.ph.utexas.edu
//  Institute for Fusion Studies	University of Texas at Austin
//
//  Copyright (C) 2004 Joao Cardoso
//  Copyright (C) 2016 Alan W. Irwin
//
//  This file is part of PLplot.
//
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
//--------------------------------------------------------------------------
//
//  This file contains routines that implement Tcl matrices.
//  These are operators that are used to store, return, and modify
//  numeric data stored in binary array format.  The emphasis is
//  on high performance and low overhead, something that Tcl lists
//  or associative arrays aren't so good at.
//

//
//#define DEBUG
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pldll.h"
#include "tclMatrix.h"

// Cool math macros

#ifndef MAX
#define MAX( a, b )    ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif
#ifndef MIN
#define MIN( a, b )    ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

// For the truly desperate debugging task

#ifdef DEBUG_ENTER
#define dbug_enter( a ) \
    fprintf( stderr, "%s: Entered %s\n", __FILE__, a );

#else
#define dbug_enter( a )
#endif

// Internal data

static int           matTable_initted = 0; // Hash table initialization flag
static Tcl_HashTable matTable;             // Hash table for external access to data

// Function prototypes

// Handles matrix initialization lists

static int
MatrixAssign( Tcl_Interp* interp, tclMatrix* m,
              int level, int *offset, int nargs, const char** args );

// Invoked to process the "matrix" Tcl command.

static int
MatrixCmd( ClientData clientData, Tcl_Interp *interp, int argc, const char **argv );

// Causes matrix command to be deleted.

static char *
DeleteMatrixVar( ClientData clientData,
                 Tcl_Interp *interp, char *name1, char *name2, int flags );

// Releases all the resources allocated to the matrix command.

static void
DeleteMatrixCmd( ClientData clientData );

// These do the put/get operations for each supported type

static void
MatrixPut_f( ClientData clientData, Tcl_Interp* interp, int index, const char *string );

static void
MatrixGet_f( ClientData clientData, Tcl_Interp* interp, int index, char *string );

static void
MatrixPut_i( ClientData clientData, Tcl_Interp* interp, int index, const char *string );

static void
MatrixGet_i( ClientData clientData, Tcl_Interp* interp, int index, char *string );

//--------------------------------------------------------------------------
//
// Tcl_MatCmd --
//
//	Invoked to process the "matrix" Tcl command.  Creates a multiply
//	dimensioned array (matrix) of floats or ints.  The number of
//	arguments determines the dimensionality.
//
// Results:
//	Returns the name of the new matrix.
//
// Side effects:
//	A new matrix (operator) gets created.
//
//--------------------------------------------------------------------------

int
Tcl_MatrixCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
               int argc, const char **argv )
{
    register tclMatrix *matPtr;
    int i, j, new, index, persist = 0, initializer = 0;
    Tcl_HashEntry      *hPtr;
    Tcl_CmdInfo        infoPtr;
    char c;
    size_t             argv0_length;
    int offset = 0;
    size_t             concatenated_argv_len;
    char               *concatenated_argv;
    const char         *const_concatenated_argv;

    dbug_enter( "Tcl_MatrixCmd" );

    if ( argc < 3 )
    {
        Tcl_AppendResult( interp, "wrong # args: should be \"", argv[0],
            " ?-persist? var type dim1 ?dim2? ?dim3? ...\"", (char *) NULL );
        return TCL_ERROR;
    }

    // Create hash table on first call

    if ( !matTable_initted )
    {
        matTable_initted = 1;
        Tcl_InitHashTable( &matTable, TCL_STRING_KEYS );
    }

    // Check for -persist flag

    for ( i = 1; i < argc; i++ )
    {
        c            = argv[i][0];
        argv0_length = strlen( argv[i] );

        // If found, set persist variable and compress argv-list

        if ( ( c == '-' ) && ( strncmp( argv[i], "-persist", argv0_length ) == 0 ) )
        {
            persist = 1;
            argc--;
            for ( j = i; j < argc; j++ )
                argv[j] = argv[j + 1];
            break;
        }
    }

    // Create matrix data structure

    matPtr          = (tclMatrix *) malloc( sizeof ( tclMatrix ) );
    matPtr->fdata   = NULL;
    matPtr->idata   = NULL;
    matPtr->name    = NULL;
    matPtr->dim     = 0;
    matPtr->len     = 1;
    matPtr->tracing = 0;
    matPtr->indices = NULL;

    // MAX_ARRAY_DIM is #defined to be 3. Later programming logic
    // treats all lower-dimensioned matrices as 3D matrices where the
    // higher dimension size is 1.  So must initialize all sizes
    // to 1 here.
    for ( i = 0; i < MAX_ARRAY_DIM; i++ )
        matPtr->n[i] = 1;

    // Create name
    // It should be unique

    argc--; argv++;

    if ( Tcl_GetCommandInfo( interp, argv[0], &infoPtr ) )
    {
        Tcl_AppendResult( interp, "Matrix operator \"", argv[0],
            "\" already in use", (char *) NULL );
        free( (void *) matPtr );
        return TCL_ERROR;
    }

    if ( Tcl_GetVar( interp, argv[0], 0 ) != NULL )
    {
        Tcl_AppendResult( interp, "Illegal name for Matrix operator \"",
            argv[0], "\": local variable of same name is active",
            (char *) NULL );
        free( (void *) matPtr );
        return TCL_ERROR;
    }

    matPtr->name = (char *) malloc( strlen( argv[0] ) + 1 );
    strcpy( matPtr->name, argv[0] );

    // Initialize type

    argc--; argv++;
    c            = argv[0][0];
    argv0_length = strlen( argv[0] );

    if ( ( c == 'f' ) && ( strncmp( argv[0], "float", argv0_length ) == 0 ) )
    {
        matPtr->type = TYPE_FLOAT;
        matPtr->put  = MatrixPut_f;
        matPtr->get  = MatrixGet_f;
    }
    else if ( ( c == 'i' ) && ( strncmp( argv[0], "int", argv0_length ) == 0 ) )
    {
        matPtr->type = TYPE_INT;
        matPtr->put  = MatrixPut_i;
        matPtr->get  = MatrixGet_i;
    }
    else
    {
        Tcl_AppendResult( interp, "Matrix type \"", argv[0],
            "\" not supported, should be \"float\" or \"int\"",
            (char *) NULL );

        DeleteMatrixCmd( (ClientData) matPtr );
        return TCL_ERROR;
    }

    // Initialize dimensions

    argc--; argv++;
    for (; argc > 0; argc--, argv++ )
    {
        // Check for initializer

        if ( strcmp( argv[0], "=" ) == 0 )
        {
            argc--; argv++;
            initializer = 1;
            break;
        }

        // Must be a dimensional parameter.  Increment number of dimensions.

        matPtr->dim++;
        if ( matPtr->dim > MAX_ARRAY_DIM )
        {
            Tcl_AppendResult( interp,
                "too many dimensions specified for Matrix operator \"",
                matPtr->name, "\"", (char *) NULL );

            DeleteMatrixCmd( (ClientData) matPtr );
            return TCL_ERROR;
        }

        // Check to see if dimension is valid and store

        index            = matPtr->dim - 1;
        matPtr->n[index] = MAX( 0, atoi( argv[0] ) );
        matPtr->len     *= matPtr->n[index];
    }

    if ( matPtr->dim < 1 )
    {
        Tcl_AppendResult( interp,
            "insufficient dimensions given for Matrix operator \"",
            matPtr->name, "\"", (char *) NULL );
        DeleteMatrixCmd( (ClientData) matPtr );
        return TCL_ERROR;
    }

    // Allocate space for data

    switch ( matPtr->type )
    {
    case TYPE_FLOAT:
        matPtr->fdata = (Mat_float *) malloc( (size_t) ( matPtr->len ) * sizeof ( Mat_float ) );
        for ( i = 0; i < matPtr->len; i++ )
            matPtr->fdata[i] = 0.0;
        break;

    case TYPE_INT:
        matPtr->idata = (Mat_int *) malloc( (size_t) ( matPtr->len ) * sizeof ( Mat_int ) );
        for ( i = 0; i < matPtr->len; i++ )
            matPtr->idata[i] = 0;
        break;
    }

    // Process the initializer, if present

    if ( initializer )
    {
        if ( argc <= 0 )
        {
            Tcl_AppendResult( interp,
                "no initialization data given after \"=\" for Matrix operator \"",
                matPtr->name, "\"", (char *) NULL );
            DeleteMatrixCmd( (ClientData) matPtr );
            return TCL_ERROR;
        }

        // Prepare concatenated_argv string consisting of "{argv[0] argv[1] ... argv[argc-1]}"
        // so that _any_ space-separated bunch of numerical arguments will work.
        // Account for beginning and ending curly braces and trailing \0.
        concatenated_argv_len = 3;
        for ( i = 0; i < argc; i++ )
            // Account for length of string + space separator.
            concatenated_argv_len += strlen( argv[i] ) + 1;
        concatenated_argv = (char *) malloc( concatenated_argv_len * sizeof ( char ) );

        // Prepare for string concatenation using strcat
        concatenated_argv[0] = '\0';
        strcat( concatenated_argv, "{" );
        for ( i = 0; i < argc; i++ )
        {
            strcat( concatenated_argv, argv[i] );
            strcat( concatenated_argv, " " );
        }
        strcat( concatenated_argv, "}" );

        const_concatenated_argv = (const char *) concatenated_argv;

        // Use all raw indices in row-major (C) order for put in MatrixAssign
        matPtr->nindices = matPtr->len;
        matPtr->indices  = NULL;

        if ( MatrixAssign( interp, matPtr, 0, &offset, 1, &const_concatenated_argv ) != TCL_OK )
        {
            DeleteMatrixCmd( (ClientData) matPtr );
            free( (void *) concatenated_argv );
            return TCL_ERROR;
        }
        free( (void *) concatenated_argv );
    }

    // For later use in matrix assigments
    // N.B. matPtr->len could be large so this check for success might
    // be more than pro forma.
    if ( ( matPtr->indices = (int *) malloc( (size_t) ( matPtr->len ) * sizeof ( int ) ) ) == NULL )
    {
        Tcl_AppendResult( interp,
            "memory allocation failed for indices vector associated with Matrix operator \"",
            matPtr->name, "\"", (char *) NULL );
        DeleteMatrixCmd( (ClientData) matPtr );
        return TCL_ERROR;
    }
    // Delete matrix when it goes out of scope unless -persist specified
    // Use local variable of same name as matrix and trace it for unsets

    if ( !persist )
    {
        if ( Tcl_SetVar( interp, matPtr->name,
                 "old_bogus_syntax_please_upgrade", 0 ) == NULL )
        {
            Tcl_AppendResult( interp, "unable to schedule Matrix operator \"",
                matPtr->name, "\" for automatic deletion", (char *) NULL );
            DeleteMatrixCmd( (ClientData) matPtr );
            return TCL_ERROR;
        }
        matPtr->tracing = 1;
        Tcl_TraceVar( interp, matPtr->name, TCL_TRACE_UNSETS,
            (Tcl_VarTraceProc *) DeleteMatrixVar, (ClientData) matPtr );
    }

    // Create matrix operator

#ifdef DEBUG
    fprintf( stderr, "Creating Matrix operator of name %s\n", matPtr->name );
#endif
    Tcl_CreateCommand( interp, matPtr->name, (Tcl_CmdProc *) MatrixCmd,
        (ClientData) matPtr, (Tcl_CmdDeleteProc *) DeleteMatrixCmd );

    // Store pointer to interpreter to handle bizarre uses of multiple
    // interpreters (e.g. as in [incr Tcl])

    matPtr->interp = interp;

    // Create hash table entry for this matrix operator's data
    // This should never fail

    hPtr = Tcl_CreateHashEntry( &matTable, matPtr->name, &new );
    if ( !new )
    {
        Tcl_AppendResult( interp,
            "Unable to create hash table entry for Matrix operator \"",
            matPtr->name, "\"", (char *) NULL );
        return TCL_ERROR;
    }
    Tcl_SetHashValue( hPtr, matPtr );

    Tcl_SetResult( interp, matPtr->name, TCL_VOLATILE );
    return TCL_OK;
}

//--------------------------------------------------------------------------
//
// Tcl_GetMatrixPtr --
//
//	Returns a pointer to the specified matrix operator's data.
//
// Results:
//	None.
//
// Side effects:
//	None.
//
//--------------------------------------------------------------------------

tclMatrix *
Tcl_GetMatrixPtr( Tcl_Interp *interp, const char *matName )
{
    Tcl_HashEntry *hPtr;

    dbug_enter( "Tcl_GetMatrixPtr" );

    if ( !matTable_initted )
    {
        return NULL;
    }

    hPtr = Tcl_FindHashEntry( &matTable, matName );
    if ( hPtr == NULL )
    {
        Tcl_AppendResult( interp, "No matrix operator named \"",
            matName, "\"", (char *) NULL );
        return NULL;
    }
    return (tclMatrix *) Tcl_GetHashValue( hPtr );
}

//--------------------------------------------------------------------------
//
//  Tcl_MatrixInstallXtnsn --
//
//	Install a tclMatrix extension subcommand.
//
// Results:
//	Should be 1.  Have to think about error results.
//
// Side effects:
//	Enables you to install special purpose compiled code to handle
//	custom operations on a tclMatrix.
//
//--------------------------------------------------------------------------

static tclMatrixXtnsnDescr *head = (tclMatrixXtnsnDescr *) NULL;
static tclMatrixXtnsnDescr *tail = (tclMatrixXtnsnDescr *) NULL;

int
Tcl_MatrixInstallXtnsn( const char *cmd, tclMatrixXtnsnProc proc )
{
//
// My goodness how I hate primitive/pathetic C.  With C++ this
// could've been as easy as:
//     List<TclMatrixXtnsnDescr> xtnlist;
//     xtnlist.append( tclMatrixXtnsnDescr(cmd,proc) );
// grrrrr.
//

    tclMatrixXtnsnDescr *new =
        (tclMatrixXtnsnDescr *) malloc( sizeof ( tclMatrixXtnsnDescr ) );

    dbug_enter( "Tcl_MatrixInstallXtnsn" );

#ifdef DEBUG
    fprintf( stderr, "Installing a tclMatrix extension -> %s\n", cmd );
#endif

    new->cmd = malloc( strlen( cmd ) + 1 );
    strcpy( new->cmd, cmd );
    new->cmdproc = proc;
    new->next    = (tclMatrixXtnsnDescr *) NULL;

    if ( !head )
    {
        tail = head = new;
        return 1;
    }
    else
    {
        tail = tail->next = new;
        return 1;
    }
}

//--------------------------------------------------------------------------
//
// MatrixAssign --
//
// Assign values to the elements of a matrix.
//
// Returns TCL_OK on success or TC_ERROR on failure.
//
//--------------------------------------------------------------------------

static int MatrixAssign( Tcl_Interp* interp, tclMatrix* m,
                         int level, int *offset, int nargs, const char** args )
{
    static int verbose = 0;

    const char ** newargs;
    int        numnewargs;
    int        i;

    if ( verbose )
    {
        fprintf( stderr, "level %d  offset %d  nargs %d\n", level, *offset, nargs );
        for ( i = 0; i < nargs; i++ )
        {
            fprintf( stderr, "i = %d, args[i] = %s\n", i, args[i] );
        }
    }
    // Just in case of some programming error below that creates an infinite loop
    if ( level > 100 )
    {
        Tcl_AppendResult( interp, "too many list levels", (char *) NULL );
        return TCL_ERROR;
    }

    for ( i = 0; i < nargs; i++ )
    {
        if ( Tcl_SplitList( interp, args[i], &numnewargs, &newargs )
             != TCL_OK )
        {
            // Tcl_SplitList has already appended an error message
            // to the result associated with interp so no need to
            // append more.
            return TCL_ERROR;
        }

        if ( numnewargs == 1 && strlen( args[i] ) == strlen( newargs[0] ) && strcmp( args[i], newargs[0] ) == 0 )
        {
            // Tcl_SplitList has gone as deep as it can go into hierarchical lists ....
            if ( *offset >= m->nindices )
            {
                // Ignore any values in array assignment beyond what are needed.
            }
            else
            {
                if ( verbose )
                    fprintf( stderr, "\ta[%d] = %s\n", *offset, args[i] );
                if ( m->indices == NULL )
                    ( m->put )( (ClientData) m, interp, *offset, args[i] );
                else
                    ( m->put )( (ClientData) m, interp, m->indices[*offset], args[i] );
                ( *offset )++;
            }
        }
        else if ( MatrixAssign( interp, m, level + 1, offset, numnewargs, newargs )
                  != TCL_OK )
        {
            Tcl_Free( (char *) newargs );
            return TCL_ERROR;
        }
        Tcl_Free( (char *) newargs );
    }
    return TCL_OK;
}

//--------------------------------------------------------------------------
//
// MatrixCmd --
//
//	When a Tcl matrix command is invoked, this routine is called.
//
// Results:
//	A standard Tcl result value, usually TCL_OK.
//	On matrix get commands, one or a number of matrix elements are
//	printed.
//
// Side effects:
//	Depends on the matrix command.
//
//--------------------------------------------------------------------------

static int
MatrixCmd( ClientData clientData, Tcl_Interp *interp,
           int argc, const char **argv )
{
    register tclMatrix *matPtr = (tclMatrix *) clientData;
    int        put             = 0;
    char       c, tmp[200];
    const char *name = argv[0];
    // In one case (negative step and desired last actual index of 0)
    // stop[i] is -1 so it must have an int type rather than size_t.
    // To reduce casting most other slice-related types are also int
    // rather than size_t.
    int    start[MAX_ARRAY_DIM], stop[MAX_ARRAY_DIM], step[MAX_ARRAY_DIM], sign_step[MAX_ARRAY_DIM];
    int    i, j, k;
    int    char_converted, change_default_start, change_default_stop;
    size_t argv0_length;
    // Needs dimension of 2 to contain ":" and terminating NULL as a result of sscanf calls below.
    char   c1[2], c2[2];

    // Initialize

    if ( argc < 2 )
    {
        Tcl_AppendResult( interp, "wrong # args, type: \"",
            argv[0], " help\" for more info", (char *) NULL );
        return TCL_ERROR;
    }

    for ( i = 0; i < MAX_ARRAY_DIM; i++ )
    {
        start[i]     = 0;
        stop[i]      = matPtr->n[i];
        step[i]      = 1;
        sign_step[i] = 1;
    }

    // First check for a matrix command

    argc--; argv++;
    c            = argv[0][0];
    argv0_length = strlen( argv[0] );

    // dump -- send a nicely formatted listing of the array contents to stdout
    // (very helpful for debugging)

    if ( ( c == 'd' ) && ( strncmp( argv[0], "dump", argv0_length ) == 0 ) )
    {
        for ( i = start[0]; i < stop[0]; i++ )
        {
            for ( j = start[1]; j < stop[1]; j++ )
            {
                for ( k = start[2]; k < stop[2]; k++ )
                {
                    ( *matPtr->get )( (ClientData) matPtr, interp, I3D( i, j, k ), tmp );
                    printf( "%s ", tmp );
                }
                if ( matPtr->dim > 2 )
                    printf( "\n" );
            }
            if ( matPtr->dim > 1 )
                printf( "\n" );
        }
        printf( "\n" );
        return TCL_OK;
    }

    // delete -- delete the array

    else if ( ( c == 'd' ) && ( strncmp( argv[0], "delete", argv0_length ) == 0 ) )
    {
#ifdef DEBUG
        fprintf( stderr, "Deleting array %s\n", name );
#endif
        Tcl_DeleteCommand( interp, name );
        return TCL_OK;
    }

    // filter
    // Only works on 1d matrices

    else if ( ( c == 'f' ) && ( strncmp( argv[0], "filter", argv0_length ) == 0 ) )
    {
        Mat_float *tmpMat;
        int       ifilt, nfilt;

        if ( argc != 2 )
        {
            Tcl_AppendResult( interp, "wrong # args: should be \"",
                name, " ", argv[0], " num-passes\"",
                (char *) NULL );
            return TCL_ERROR;
        }

        if ( matPtr->dim != 1 || matPtr->type != TYPE_FLOAT )
        {
            Tcl_AppendResult( interp, "can only filter a 1d float matrix",
                (char *) NULL );
            return TCL_ERROR;
        }

        nfilt  = atoi( argv[1] );
        tmpMat = (Mat_float *) malloc( (size_t) ( matPtr->len + 2 ) * sizeof ( Mat_float ) );

        for ( ifilt = 0; ifilt < nfilt; ifilt++ )
        {
            // Set up temporary filtering array.  Use even boundary conditions.

            j = 0; tmpMat[j] = matPtr->fdata[0];
            for ( i = 0; i < matPtr->len; i++ )
            {
                j++; tmpMat[j] = matPtr->fdata[i];
            }
            j++; tmpMat[j] = matPtr->fdata[matPtr->len - 1];

            // Apply 3-point binomial filter

            for ( i = 0; i < matPtr->len; i++ )
            {
                j = i + 1;
                matPtr->fdata[i] = 0.25 * ( tmpMat[j - 1] + 2 * tmpMat[j] + tmpMat[j + 1] );
            }
        }

        free( (void *) tmpMat );
        return TCL_OK;
    }

    // help

    else if ( ( c == 'h' ) && ( strncmp( argv[0], "help", argv0_length ) == 0 ) )
    {
        Tcl_AppendResult( interp,
            "Available subcommands:\n\
dump   - return the values in the matrix as a string\n\
delete - delete the matrix (including the matrix command)\n\
filter - apply a three-point averaging (with a number of passes; ome-dimensional only)\n\
help   - this information\n\
info   - return the dimensions\n\
max    - return the maximum value for the entire matrix or for the first N entries\n\
min    - return the minimum value for the entire matrix or for the first N entries\n\
redim  - resize the matrix (for one-dimensional matrices only)\n\
scale  - scale the values by a given factor (for one-dimensional matrices only)\n\
\n\
Set and get values:\n\
matrix m f 3 3 3 - define matrix command \"m\", three-dimensional, floating-point data\n\
m 1 2 3          - return the value of matrix element [1,2,3]\n\
m 1 2 3 = 2.0    - set the value of matrix element [1,2,3] to 2.0 (do not return the value)\n\
m * 2 3 = 2.0    - set a slice consisting of all elements with second index 2 and third index 3 to 2.0",
            (char *) NULL );
        return TCL_OK;
    }

    // info

    else if ( ( c == 'i' ) && ( strncmp( argv[0], "info", argv0_length ) == 0 ) )
    {
        for ( i = 0; i < matPtr->dim; i++ )
        {
            sprintf( tmp, "%d", matPtr->n[i] );
            // Must avoid trailing space.
            if ( i < matPtr->dim - 1 )
                Tcl_AppendResult( interp, tmp, " ", (char *) NULL );
            else
                Tcl_AppendResult( interp, tmp, (char *) NULL );
        }
        return TCL_OK;
    }

    // max

    else if ( ( c == 'm' ) && ( strncmp( argv[0], "max", argv0_length ) == 0 ) )
    {
        int len;
        if ( argc < 1 || argc > 2 )
        {
            Tcl_AppendResult( interp, "wrong # args: should be \"",
                name, " ", argv[0], " ?length?\"",
                (char *) NULL );
            return TCL_ERROR;
        }

        if ( argc == 2 )
        {
            len = atoi( argv[1] );
            if ( len < 0 || len > matPtr->len )
            {
                Tcl_AppendResult( interp, "specified length out of valid range",
                    (char *) NULL );
                return TCL_ERROR;
            }
        }
        else
            len = matPtr->len;

        if ( len == 0 )
        {
            Tcl_AppendResult( interp, "attempt to find maximum of array with zero elements",
                (char *) NULL );
            return TCL_ERROR;
        }

        switch ( matPtr->type )
        {
        case TYPE_FLOAT: {
            Mat_float max = matPtr->fdata[0];
            for ( i = 1; i < len; i++ )
                max = MAX( max, matPtr->fdata[i] );
            //sprintf(tmp, "%.17g", max);
            Tcl_PrintDouble( interp, max, tmp );
            Tcl_AppendResult( interp, tmp, (char *) NULL );
            break;
        }
        case TYPE_INT: {
            Mat_int max = matPtr->idata[0];
            for ( i = 1; i < len; i++ )
                max = MAX( max, matPtr->idata[i] );
            sprintf( tmp, "%d", max );
            Tcl_AppendResult( interp, tmp, (char *) NULL );
            break;
        }
        }
        return TCL_OK;
    }

    // min

    else if ( ( c == 'm' ) && ( strncmp( argv[0], "min", argv0_length ) == 0 ) )
    {
        int len;
        if ( argc < 1 || argc > 2 )
        {
            Tcl_AppendResult( interp, "wrong # args: should be \"",
                name, " ", argv[0], " ?length?\"",
                (char *) NULL );
            return TCL_ERROR;
        }

        if ( argc == 2 )
        {
            len = atoi( argv[1] );
            if ( len < 0 || len > matPtr->len )
            {
                Tcl_AppendResult( interp, "specified length out of valid range",
                    (char *) NULL );
                return TCL_ERROR;
            }
        }
        else
            len = matPtr->len;

        if ( len == 0 )
        {
            Tcl_AppendResult( interp, "attempt to find minimum of array with zero elements",
                (char *) NULL );
            return TCL_ERROR;
        }

        switch ( matPtr->type )
        {
        case TYPE_FLOAT: {
            Mat_float min = matPtr->fdata[0];
            for ( i = 1; i < len; i++ )
                min = MIN( min, matPtr->fdata[i] );
            //sprintf(tmp, "%.17g", min);
            Tcl_PrintDouble( interp, min, tmp );
            Tcl_AppendResult( interp, tmp, (char *) NULL );
            break;
        }
        case TYPE_INT: {
            Mat_int min = matPtr->idata[0];
            for ( i = 1; i < len; i++ )
                min = MIN( min, matPtr->idata[i] );
            sprintf( tmp, "%d", min );
            Tcl_AppendResult( interp, tmp, (char *) NULL );
            break;
        }
        }
        return TCL_OK;
    }

    // redim
    // Only works on 1d matrices

    else if ( ( c == 'r' ) && ( strncmp( argv[0], "redim", argv0_length ) == 0 ) )
    {
        int  newlen;
        void *data;

        if ( argc != 2 )
        {
            Tcl_AppendResult( interp, "wrong # args: should be \"",
                name, " ", argv[0], " length\"",
                (char *) NULL );
            return TCL_ERROR;
        }

        if ( matPtr->dim != 1 )
        {
            Tcl_AppendResult( interp, "can only redim a 1d matrix",
                (char *) NULL );
            return TCL_ERROR;
        }

        newlen = atoi( argv[1] );
        switch ( matPtr->type )
        {
        case TYPE_FLOAT:
            data = realloc( matPtr->fdata, (size_t) newlen * sizeof ( Mat_float ) );
            if ( newlen != 0 && data == NULL )
            {
                Tcl_AppendResult( interp, "redim failed!",
                    (char *) NULL );
                return TCL_ERROR;
            }
            matPtr->fdata = (Mat_float *) data;
            for ( i = matPtr->len; i < newlen; i++ )
                matPtr->fdata[i] = 0.0;
            break;

        case TYPE_INT:
            data = realloc( matPtr->idata, (size_t) newlen * sizeof ( Mat_int ) );
            if ( newlen != 0 && data == NULL )
            {
                Tcl_AppendResult( interp, "redim failed!",
                    (char *) NULL );
                return TCL_ERROR;
            }
            matPtr->idata = (Mat_int *) data;
            for ( i = matPtr->len; i < newlen; i++ )
                matPtr->idata[i] = 0;
            break;
        }
        matPtr->n[0] = matPtr->len = newlen;
        // For later use in matrix assigments
        // N.B. matPtr->len could be large so this check for success might
        // be more than pro forma.
        data = realloc( matPtr->indices, (size_t) ( matPtr->len ) * sizeof ( int ) );
        if ( newlen != 0 && data == NULL )
        {
            Tcl_AppendResult( interp, "redim failed!", (char *) NULL );
            return TCL_ERROR;
        }
        matPtr->indices = (int *) data;
        return TCL_OK;
    }

    // scale
    // Only works on 1d matrices

    else if ( ( c == 's' ) && ( strncmp( argv[0], "scale", argv0_length ) == 0 ) )
    {
        Mat_float scale;

        if ( argc != 2 )
        {
            Tcl_AppendResult( interp, "wrong # args: should be \"",
                name, " ", argv[0], " scale-factor\"",
                (char *) NULL );
            return TCL_ERROR;
        }

        if ( matPtr->dim != 1 )
        {
            Tcl_AppendResult( interp, "can only scale a 1d matrix",
                (char *) NULL );
            return TCL_ERROR;
        }

        scale = atof( argv[1] );
        switch ( matPtr->type )
        {
        case TYPE_FLOAT:
            for ( i = 0; i < matPtr->len; i++ )
                matPtr->fdata[i] *= scale;
            break;

        case TYPE_INT:
            for ( i = 0; i < matPtr->len; i++ )
                matPtr->idata[i] = (Mat_int) ( (Mat_float) ( matPtr->idata[i] ) * scale );
            break;
        }
        return TCL_OK;
    }

    // Not a "standard" command, check the extension commands.

    {
        tclMatrixXtnsnDescr *p = head;
        for (; p; p = p->next )
        {
            if ( ( c == p->cmd[0] ) && ( strncmp( argv[0], p->cmd, argv0_length ) == 0 ) )
            {
#ifdef DEBUG
                fprintf( stderr, "found a match, invoking %s\n", p->cmd );
#endif
                return ( *( p->cmdproc ) )( matPtr, interp, --argc, ++argv );
            }
        }
    }

    // Must be a put or get of an array slice or array value.

    // Determine array index slice adopting the same rules as the Python case
    // documented at <https://docs.python.org/3/library/stdtypes.html#common-sequence-operations>
    // Also, for the case where just a _single_ ":" is used to represent the
    // complete range of indices for a dimension, the
    // notation "*" can be used as well for backwards compatibility
    // with the limited slice capability that was available before
    // this full slice capability was implemented.

    if ( argc < matPtr->dim )
    {
        Tcl_AppendResult( interp, "not enough dimensions specified for \"",
            name, "\"", (char *) NULL );
        return TCL_ERROR;
    }

    for ( i = 0; i < matPtr->dim; i++ )
    {
        // Because of argc and argv initialization and logic at end of
        // loop which decrements argc and increments argv, argv[0]
        // walks through the space-separated command-line strings that
        // have been parsed by Tcl for each iteration of this loop.
        // N.B. argv[0] should point to valid memory (i.e., one of the
        // command-line strings) because of the above initial argc
        // check and loop limits.
        argv0_length = strlen( argv[0] );
        // According to Linux man page for sscanf, a straightforward interpretation of the C standard
        // indicates that %n should not be counted as a successful conversion when calculating
        // the sscanf return value, but that man page also says should not count on that in general.
        // So in the logic below use the ">= " test to allow for both possibilities.

        // Default values if not determined below.
        start[i]             = 0;
        stop[i]              = matPtr->n[i];
        step[i]              = 1;
        change_default_start = 0;
        change_default_stop  = 0;
        // i:j:k
        if ( sscanf( argv[0], "%d%1[:]%d%1[:]%d%n", start + i, c1, stop + i, c2, step + i, &char_converted ) >= 5 )
        {
        }
        // i:j:
        else if ( sscanf( argv[0], "%d%1[:]%d%1[:]%n", start + i, c1, stop + i, c2, &char_converted ) >= 4 )
        {
        }
        // i:j
        else if ( sscanf( argv[0], "%d%1[:]%d%n", start + i, c1, stop + i, &char_converted ) >= 3 )
        {
        }
        // i::k
        else if ( sscanf( argv[0], "%d%1[:]%1[:]%d%n", start + i, c1, c2, step + i, &char_converted ) >= 4 )
        {
            if ( step[i] < 0 )
            {
                change_default_stop = 1;
            }
        }
        // i::
        else if ( sscanf( argv[0], "%d%1[:]%1[:]%n", start + i, c1, c2, &char_converted ) >= 3 )
        {
        }
        // i:
        else if ( sscanf( argv[0], "%d%1[:]%n", start + i, c1, &char_converted ) >= 2 )
        {
        }
        // :j:k
        else if ( sscanf( argv[0], "%1[:]%d%1[:]%d%n", c1, stop + i, c2, step + i, &char_converted ) >= 4 )
        {
            if ( step[i] < 0 )
            {
                change_default_start = 1;
            }
        }
        // :j:
        else if ( sscanf( argv[0], "%1[:]%d%1[:]%n", c1, stop + i, c2, &char_converted ) >= 3 )
        {
        }
        // :j
        else if ( sscanf( argv[0], "%1[:]%d%n", c1, stop + i, &char_converted ) >= 2 )
        {
        }
        // ::k
        else if ( sscanf( argv[0], "%1[:]%1[:]%d%n", c1, c2, step + i, &char_converted ) >= 3 )
        {
            if ( step[i] < 0 )
            {
                change_default_start = 1;
                change_default_stop  = 1;
            }
        }
        // ::
        else if ( strcmp( argv[0], "::" ) == 0 )
            char_converted = 2;
        // :
        else if ( strcmp( argv[0], ":" ) == 0 )
            char_converted = 1;
        // *
        else if ( strcmp( argv[0], "*" ) == 0 )
            char_converted = 1;
        // i
        else if ( sscanf( argv[0], "%d%n", start + i, &char_converted ) >= 1 )
        {
            // Special checks for the pure index case (just like in Python).
            if ( start[i] < 0 )
                start[i] += matPtr->n[i];
            if ( start[i] < 0 || start[i] > matPtr->n[i] - 1 )
            {
                sprintf( tmp, "Array index %d out of bounds: original string = \"%s\"; transformed = %d; min = 0; max = %d\n",
                    i, argv[0], start[i], matPtr->n[i] - 1 );
                Tcl_AppendResult( interp, tmp, (char *) NULL );
                return TCL_ERROR;
            }
            stop[i] = start[i] + 1;
        }
        else
        {
            sprintf( tmp, "Array slice for index %d with original string = \"%s\" could not be parsed\n",
                i, argv[0] );
            Tcl_AppendResult( interp, tmp, (char *) NULL );
            return TCL_ERROR;
        }

        // Check, convert and sanitize start[i], stop[i], and step[i] values.
        if ( step[i] == 0 )
        {
            Tcl_AppendResult( interp, "step part of slice must be non-zero",
                (char *) NULL );
            return TCL_ERROR;
        }
        sign_step[i] = ( step[i] > 0 ) ? 1 : -1;
        if ( (size_t) char_converted > argv0_length )
        {
            Tcl_AppendResult( interp, "MatrixCmd, internal logic error",
                (char *) NULL );
            return TCL_ERROR;
        }
        if ( (size_t) char_converted < argv0_length )
        {
            sprintf( tmp, "Array slice for index %d with original string = \"%s\" "
                "had trailing unparsed characters\n", i, argv[0] );
            Tcl_AppendResult( interp, tmp, (char *) NULL );
            return TCL_ERROR;
        }
        if ( start[i] < 0 )
            start[i] += matPtr->n[i];
        start[i] = MAX( 0, MIN( matPtr->n[i] - 1, start[i] ) );
        if ( change_default_start )
            start[i] = matPtr->n[i] - 1;
        if ( stop[i] < 0 )
            stop[i] += matPtr->n[i];
        if ( step[i] > 0 )
            stop[i] = MAX( 0, MIN( matPtr->n[i], stop[i] ) );
        else
            stop[i] = MAX( -1, MIN( matPtr->n[i], stop[i] ) );
        if ( change_default_stop )
            stop[i] = -1;

        // At this stage, start, stop, and step (!=0), correspond to
        // i, j, and k (!=0) in the slice documentation given at
        // <https://docs.python.org/3/library/stdtypes.html#common-sequence-operations>.
        // with all checks and conversions made.  According to note 5
        // of that documentation (translated to the present start,
        // stop and step notation and also subject to the clarifying
        // discussion in <http://bugs.python.org/issue28614>) the
        // array index should take on the values
        // index = start + n*step
        // where n 0, 1, etc., with that sequence
        // terminated just before index = stop is reached.
        // Therefore, the for loop for a typical index when step is positive should read
        // for ( i = start[0]; i < stop[0]; i += step[0] )
        // and when step is negative should read
        // for ( i = start[0]; i > stop[0]; i += step[0] )
        // So to cover both cases, we use for loops of the
        // following form below
        // for ( i = start[0]; sign_step[0]*i < stop[0]; i += step[0] )
        // where stop has been transformed as follows:
#ifdef DEBUG
        fprintf( stderr, "Array slice for index %d with original string = \"%s\" "
            "yielded start[i], stop[i], transformed stop[i], and step[i] = "
            "%d, %d, ", i, argv[0], start[i], stop[i] );
#endif
        stop[i] = sign_step[i] * stop[i];
#ifdef DEBUG
        fprintf( stderr, "%d, %d\n", stop[i], step[i] );
#endif
        argc--; argv++;
    }

    // If there is an "=" after indices, it's a put.  Do error checking.

    if ( argc > 0 )
    {
        put = 1;
        if ( strcmp( argv[0], "=" ) == 0 )
        {
            argc--; argv++;
            if ( argc == 0 )
            {
                Tcl_AppendResult( interp, "no value specified",
                    (char *) NULL );
                return TCL_ERROR;
            }
        }
        else
        {
            Tcl_AppendResult( interp, "extra characters after indices: \"",
                argv[0], "\"", (char *) NULL );
            return TCL_ERROR;
        }
    }

    // Calculate which indices will be used for the given index slices.
    matPtr->nindices = 0;

    for ( i = start[0]; sign_step[0] * i < stop[0]; i += step[0] )
    {
        for ( j = start[1]; sign_step[1] * j < stop[1]; j += step[1] )
        {
            for ( k = start[2]; sign_step[2] * k < stop[2]; k += step[2] )
            {
                matPtr->indices[matPtr->nindices++] = I3D( i, j, k );
            }
        }
    }

    // Do the get/put.
    // The loop over all elements takes care of the multi-element cases.
    if ( put )
    {
        char *endptr;
        // Check whether argv[0] could be interpreted as a raw single
        // number with no trailing characters.
        switch ( matPtr->type )
        {
        case TYPE_FLOAT:
            strtod( argv[0], &endptr );
            break;
        case TYPE_INT:
            strtol( argv[0], &endptr, 10 );
            break;
        }
        if ( argc == 1 && *argv[0] != '\0' && *endptr == '\0' )
        {
            // If _all_ characters of single RHS string can be
            // successfully read as a single number, then assign all
            // matrix elements with indices in matPtr->indices to that
            // single number.
            for ( i = 0; i < matPtr->nindices; i++ )
                ( *matPtr->put )( (ClientData) matPtr, interp, matPtr->indices[i], argv[0] );
        }
        else
        {
            // If RHS cannot be successfully read as a single number,
            // then assume it is a collection of numbers (in list form
            // or white-space separated).  Concatenate all remaining
            // elements of argv into list form, then use MatrixAssign
            // to assign all matrix elements with indices in
            // matPtr->indices using all (deep) non-list elements of
            // that list.
            int        offset = 0;
            size_t     concatenated_argv_len;
            char       *concatenated_argv;
            const char *const_concatenated_argv;

            // Prepare concatenated_argv string consisting of
            // "{argv[0] argv[1] ... argv[argc-1]}" so that _any_
            // space-separated bunch of numerical arguments or lists
            // of those will work.  Account for beginning and ending
            // curly braces and trailing \0.
            concatenated_argv_len = 3;
            for ( i = 0; i < argc; i++ )
                // Account for length of string + space separator.
                concatenated_argv_len += strlen( argv[i] ) + 1;
            concatenated_argv = (char *) malloc( concatenated_argv_len * sizeof ( char ) );

            // Prepare for string concatenation using strcat
            concatenated_argv[0] = '\0';
            strcat( concatenated_argv, "{" );
            for ( i = 0; i < argc; i++ )
            {
                strcat( concatenated_argv, argv[i] );
                strcat( concatenated_argv, " " );
            }
            strcat( concatenated_argv, "}" );

            const_concatenated_argv = (const char *) concatenated_argv;

            // Assign matrix elements using all numbers collected from
            // the potentially deep list, const_concatenated_argv.
            if ( MatrixAssign( interp, matPtr, 0, &offset, 1, &const_concatenated_argv ) != TCL_OK )
            {
                free( (void *) concatenated_argv );
                return TCL_ERROR;
            }
            free( (void *) concatenated_argv );
        }
    }
    else
    {
        // get
        for ( i = 0; i < matPtr->nindices; i++ )
        {
            ( *matPtr->get )( (ClientData) matPtr, interp, matPtr->indices[i], tmp );
            if ( i < matPtr->nindices - 1 )
                Tcl_AppendResult( interp, tmp, " ", (char *) NULL );
            else
                Tcl_AppendResult( interp, tmp, (char *) NULL );
        }
    }

    return TCL_OK;
}

//--------------------------------------------------------------------------
//
// Routines to handle Matrix get/put dependent on type:
//
// MatrixPut_f	MatrixGet_f
// MatrixPut_i	MatrixGet_i
//
// A "put" converts from string format to the intrinsic type, storing into
// the array.
//
// A "get" converts from the intrinsic type to string format, storing into
// a string buffer.
//
//--------------------------------------------------------------------------

static void
MatrixPut_f( ClientData clientData, Tcl_Interp* PL_UNUSED( interp ), int index, const char *string )
{
    tclMatrix *matPtr = (tclMatrix *) clientData;

    matPtr->fdata[index] = atof( string );
}

static void
MatrixGet_f( ClientData clientData, Tcl_Interp* interp, int index, char *string )
{
    tclMatrix *matPtr = (tclMatrix *) clientData;
    double    value   = matPtr->fdata[index];

    //sprintf(string, "%.17g", value);
    Tcl_PrintDouble( interp, value, string );
}

static void
MatrixPut_i( ClientData clientData, Tcl_Interp* PL_UNUSED( interp ), int index, const char *string )
{
    tclMatrix *matPtr = (tclMatrix *) clientData;

    if ( ( strlen( string ) > 2 ) && ( strncmp( string, "0x", 2 ) == 0 ) )
    {
        matPtr->idata[index] = (Mat_int) strtoul( &string[2], NULL, 16 );
    }
    else
        matPtr->idata[index] = atoi( string );
}

static void
MatrixGet_i( ClientData clientData, Tcl_Interp* PL_UNUSED( interp ), int index, char *string )
{
    tclMatrix *matPtr = (tclMatrix *) clientData;

    sprintf( string, "%d", matPtr->idata[index] );
}

//--------------------------------------------------------------------------
//
// DeleteMatrixVar --
//
//	Causes matrix command to be deleted.  Invoked when variable
//	associated with matrix command is unset.
//
// Results:
//	None.
//
// Side effects:
//	See DeleteMatrixCmd.
//
//--------------------------------------------------------------------------

static char *
DeleteMatrixVar( ClientData clientData,
                 Tcl_Interp * PL_UNUSED( interp ), char * PL_UNUSED( name1 ), char * PL_UNUSED( name2 ), int PL_UNUSED( flags ) )
{
    tclMatrix   *matPtr = (tclMatrix *) clientData;
    Tcl_CmdInfo infoPtr;
    char        *name;

    dbug_enter( "DeleteMatrixVar" );

    if ( matPtr->tracing != 0 )
    {
        matPtr->tracing = 0;
        name            = (char *) malloc( strlen( matPtr->name ) + 1 );
        strcpy( name, matPtr->name );

#ifdef DEBUG
        if ( Tcl_GetCommandInfo( matPtr->interp, matPtr->name, &infoPtr ) )
        {
            if ( Tcl_DeleteCommand( matPtr->interp, matPtr->name ) == TCL_OK )
                fprintf( stderr, "Deleted command %s\n", name );
            else
                fprintf( stderr, "Unable to delete command %s\n", name );
        }
#else
        if ( Tcl_GetCommandInfo( matPtr->interp, matPtr->name, &infoPtr ) )
            Tcl_DeleteCommand( matPtr->interp, matPtr->name );
#endif
        free( (void *) name );
    }
    return (char *) NULL;
}

//--------------------------------------------------------------------------
//
// DeleteMatrixCmd --
//
//	Releases all the resources allocated to the matrix command.
//	Invoked just before a matrix command is removed from an interpreter.
//
//	Note: If the matrix has tracing enabled, it means the user
//	explicitly deleted a non-persistent matrix.  Not a good idea,
//	because eventually the local variable that was being traced will
//	become unset and the matrix data will be referenced in
//	DeleteMatrixVar.  So I've massaged this so that at worst it only
//	causes a minor memory leak instead of imminent program death.
//
// Results:
//	None.
//
// Side effects:
//	All memory associated with the matrix operator is freed (usually).
//
//--------------------------------------------------------------------------

static void
DeleteMatrixCmd( ClientData clientData )
{
    tclMatrix     *matPtr = (tclMatrix *) clientData;
    Tcl_HashEntry *hPtr;

    dbug_enter( "DeleteMatrixCmd" );

#ifdef DEBUG
    fprintf( stderr, "Freeing space associated with matrix %s\n", matPtr->name );
#endif

    // Remove hash table entry

    hPtr = Tcl_FindHashEntry( &matTable, matPtr->name );
    if ( hPtr != NULL )
        Tcl_DeleteHashEntry( hPtr );

    // Free data

    if ( matPtr->fdata != NULL )
    {
        free( (void *) matPtr->fdata );
        matPtr->fdata = NULL;
    }
    if ( matPtr->idata != NULL )
    {
        free( (void *) matPtr->idata );
        matPtr->idata = NULL;
    }
    if ( matPtr->indices != NULL )
    {
        free( (void *) matPtr->indices );
        matPtr->indices = NULL;
    }

    // Attempt to turn off tracing if possible.

    if ( matPtr->tracing )
    {
        if ( Tcl_VarTraceInfo( matPtr->interp, matPtr->name, TCL_TRACE_UNSETS,
                 (Tcl_VarTraceProc *) DeleteMatrixVar, NULL ) != NULL )
        {
            matPtr->tracing = 0;
            Tcl_UntraceVar( matPtr->interp, matPtr->name, TCL_TRACE_UNSETS,
                (Tcl_VarTraceProc *) DeleteMatrixVar, (ClientData) matPtr );
            Tcl_UnsetVar( matPtr->interp, matPtr->name, 0 );
        }
    }

    // Free name.

    if ( matPtr->name != NULL )
    {
        free( (void *) matPtr->name );
        matPtr->name = NULL;
    }

    // Free tclMatrix

    if ( !matPtr->tracing )
        free( (void *) matPtr );
#ifdef DEBUG
    else
        fprintf( stderr, "OOPS!  You just lost %d bytes\n", sizeof ( tclMatrix ) );
#endif
}
