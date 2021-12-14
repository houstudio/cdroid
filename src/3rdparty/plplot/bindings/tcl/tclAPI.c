//  Copyright 1994, 1995
//  Maurice LeBrun			mjl@dino.ph.utexas.edu
//  Institute for Fusion Studies	University of Texas at Austin
//
//  Copyright (C) 2004 Joao Cardoso
//  Copyright (C) 2004 Andrew Ross
//  Copyright (C) 2006-2016 Arjen Markus
//  Copyright (C) 2000-2016 Alan W. Irwin
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
//  This module implements a Tcl command set for interpretively calling
//  PLplot functions.  Each Tcl command is responsible for calling the
//  appropriate underlying function in the C API.  Can be used with any
//  driver, in principle.
//

#include "plplotP.h"
#include "pltcl.h"
#include "plplot_parameters.h"
#ifndef _WIN32
#ifdef PL_HAVE_UNISTD_H
#include <unistd.h>
#endif
#else
#ifdef _MSC_VER
#define getcwd    _getcwd
#include <direct.h>
#endif
#endif

#include "tclgen.h"

// Include non-redacted API?
//#define PLPLOTTCLTK_NON_REDACTED_API
// Exclude non-redacted API?
#undef PLPLOTTCLTK_NON_REDACTED_API

// Standardize error checking of Tcl_GetMatrixPtr calls with a macro
#define CHECK_Tcl_GetMatrixPtr( result, interp, matName ) \
    result = Tcl_GetMatrixPtr( interp, matName );         \
    if ( result == NULL ) return TCL_ERROR;

// PLplot/Tcl API handlers.  Prototypes must come before Cmds struct

static int loopbackCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plcolorbarCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plcontCmd( ClientData, Tcl_Interp *, int, const char ** );
static int pllegendCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plmeshCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plmeshcCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plot3dCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plot3dcCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plsurf3dCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plsurf3dlCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plsetoptCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plshadeCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plshadesCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plmapCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plmapfillCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plmaplineCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plmapstringCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plmaptexCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plmeridiansCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plstransformCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plsvectCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plvectCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plranddCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plgriddataCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plimageCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plimagefrCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plstripcCmd( ClientData, Tcl_Interp *, int, const char ** );
static int plslabelfuncCmd( ClientData, Tcl_Interp *, int, const char ** );
void mapform( PLINT n, PLFLT *x, PLFLT *y );
void labelform( PLINT axis, PLFLT value, char *string, PLINT string_length, PLPointer data );
PLFLT tclMatrix_feval( PLINT i, PLINT j, PLPointer p );

//
// The following structure defines all of the commands in the PLplot/Tcl
// core, and the C procedures that execute them.
//

typedef struct Command
{
    int ( *proc )( void *, struct Tcl_Interp *, int, const char ** ); // Procedure to process command.
    ClientData clientData;                                            // Arbitrary value to pass to proc.
    int        *deleteProc;                                           // Procedure to invoke when deleting
                                                                      // command.
    ClientData deleteData;                                            // Arbitrary value to pass to deleteProc
                                                                      // (usually the same as clientData).
} Command;

typedef struct
{
    const char *name;
    int ( *proc )( void *, struct Tcl_Interp *, int, const char ** );
} CmdInfo;

// Built-in commands, and the procedures associated with them

static CmdInfo Cmds[] = {
    { "loopback",     loopbackCmd     },
#include "tclgen_s.h"
    { "plcolorbar",   plcolorbarCmd   },
    { "plcont",       plcontCmd       },
    { "pllegend",     pllegendCmd     },
    { "plmap",        plmapCmd        },
    { "plmapfill",    plmapfillCmd    },
    { "plmapline",    plmaplineCmd    },
    { "plmapstring",  plmapstringCmd  },
    { "plmaptex",     plmaptexCmd     },
    { "plmeridians",  plmeridiansCmd  },
    { "plstransform", plstransformCmd },
    { "plmesh",       plmeshCmd       },
    { "plmeshc",      plmeshcCmd      },
    { "plot3d",       plot3dCmd       },
    { "plot3dc",      plot3dcCmd      },
    { "plsurf3d",     plsurf3dCmd     },
    { "plsurf3dl",    plsurf3dlCmd    },
    { "plsetopt",     plsetoptCmd     },
    { "plshade",      plshadeCmd      },
    { "plshades",     plshadesCmd     },
    { "plsvect",      plsvectCmd      },
    { "plvect",       plvectCmd       },
    { "plrandd",      plranddCmd      },
    { "plgriddata",   plgriddataCmd   },
    { "plimage",      plimageCmd      },
    { "plimagefr",    plimagefrCmd    },
    { "plstripc",     plstripcCmd     },
    { "plslabelfunc", plslabelfuncCmd },
    { NULL,           NULL            }
};

// Hash table and associated flag for directing control

static int           cmdTable_initted;
static Tcl_HashTable cmdTable;

// Variables for holding error return info from PLplot

static PLINT pl_errcode;
static char  errmsg[160];

// Library initialization

#ifndef PL_LIBRARY
#define PL_LIBRARY    ""
#endif

extern PLDLLIMPEXP char * plplotLibDir;

#if ( !defined ( MAC_TCL ) && !defined ( _WIN32 ) )
//
// Use an extended search for installations on Unix where we
// have very likely installed plplot so that plplot.tcl is
// in  /usr/local/plplot/lib/plplot5.1.0/tcl
//
#define PLPLOT_EXTENDED_SEARCH
#endif

// Static functions

// Evals the specified command, aborting on an error.

static int
tcl_cmd( Tcl_Interp *interp, const char *cmd );

//--------------------------------------------------------------------------
// Append_Cmdlist
//
// Generates command list from Cmds, storing as interps result.
//--------------------------------------------------------------------------

static void
Append_Cmdlist( Tcl_Interp *interp )
{
    static int       inited = 0;
    static const char** namelist;
    int i, j, ncmds = sizeof ( Cmds ) / sizeof ( CmdInfo );

    if ( !inited )
    {
        namelist = (const char **) malloc( (size_t) ncmds * sizeof ( char * ) );

        for ( i = 0; i < ncmds; i++ )
            namelist[i] = Cmds[i].name;

        // Sort the list, couldn't get qsort to do it for me for some reason, grrr.

        for ( i = 0; i < ncmds - 1; i++ )
            for ( j = i + 1; j < ncmds - 1; j++ )
            {
                if ( strcmp( namelist[i], namelist[j] ) > 0 )
                {
                    const char *t = namelist[i];
                    namelist[i] = namelist[j];
                    namelist[j] = t;
                }
            }

        inited = 1;
    }

    for ( i = 0; i < ncmds; i++ )
        Tcl_AppendResult( interp, " ", namelist[i], (char *) NULL );
}

//--------------------------------------------------------------------------
// plTclCmd_Init
//
// Sets up command hash table for use with plframe to PLplot Tcl API.
//
// Right now all API calls are allowed, although some of these may not
// make much sense when used with a widget.
//--------------------------------------------------------------------------

static void
plTclCmd_Init( Tcl_Interp * PL_UNUSED( interp ) )
{
    register Command *cmdPtr;
    register CmdInfo *cmdInfoPtr;

// Register our error variables with PLplot

    plsError( &pl_errcode, errmsg );

// Initialize hash table

    Tcl_InitHashTable( &cmdTable, TCL_STRING_KEYS );

// Create the hash table entry for each command

    for ( cmdInfoPtr = Cmds; cmdInfoPtr->name != NULL; cmdInfoPtr++ )
    {
        int           new;
        Tcl_HashEntry *hPtr;

        hPtr = Tcl_CreateHashEntry( &cmdTable, cmdInfoPtr->name, &new );
        if ( new )
        {
            cmdPtr             = (Command *) ckalloc( sizeof ( Command ) );
            cmdPtr->proc       = cmdInfoPtr->proc;
            cmdPtr->clientData = (ClientData) NULL;
            cmdPtr->deleteProc = NULL;
            cmdPtr->deleteData = (ClientData) NULL;
            Tcl_SetHashValue( hPtr, cmdPtr );
        }
    }
}

//--------------------------------------------------------------------------
// plTclCmd
//
// Front-end to PLplot/Tcl API for use from Tcl commands (e.g. plframe).
//
// This command is called by the plframe widget to process subcommands
// of the "cmd" plframe widget command.  This is the plframe's direct
// plotting interface to the PLplot library.  This routine can be called
// from other commands that want a similar capability.
//
// In a widget-based application, a PLplot "command" doesn't make much
// sense by itself since it isn't connected to a specific widget.
// Instead, you have widget commands.  This allows arbitrarily many
// widgets and requires a slightly different syntax than if there were
// only a single output device.  That is, the widget name (and in this
// case, the "cmd" widget command, after that comes the subcommand)
// must come first.  The plframe widget checks first for one of its
// internal subcommands, those specifically designed for use with the
// plframe widget.  If not found, control comes here.
//--------------------------------------------------------------------------

int
plTclCmd( char *cmdlist, Tcl_Interp *interp, int argc, const char **argv )
{
    register Tcl_HashEntry *hPtr;
    int result = TCL_OK;

    pl_errcode = 0; errmsg[0] = '\0';

// Create hash table on first call

    if ( !cmdTable_initted )
    {
        cmdTable_initted = 1;
        plTclCmd_Init( interp );
    }

// no option -- return list of available PLplot commands

    if ( argc == 0 )
    {
        Tcl_AppendResult( interp, cmdlist, (char *) NULL );
        Append_Cmdlist( interp );
        return TCL_OK;
    }

// Pick out the desired command

    hPtr = Tcl_FindHashEntry( &cmdTable, argv[0] );
    if ( hPtr == NULL )
    {
        Tcl_AppendResult( interp, "bad option \"", argv[0],
            "\" to \"cmd\": must be one of ",
            cmdlist, (char *) NULL );
        Append_Cmdlist( interp );
        result = TCL_ERROR;
    }
    else
    {
        register Command *cmdPtr = (Command *) Tcl_GetHashValue( hPtr );
        result = ( *cmdPtr->proc )( cmdPtr->clientData, interp, argc, argv );
        if ( result == TCL_OK )
        {
            if ( pl_errcode != 0 )
            {
                result = TCL_ERROR;
                Tcl_AppendResult( interp, errmsg, (char *) NULL );
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------
// loopbackCmd
//
// Loop-back command for Tcl interpreter.  Main purpose is to enable a
// compatible command syntax whether you are executing directly through a
// Tcl interpreter or a plframe widget.  I.e. the syntax is:
//
//	<widget> cmd <PLplot command>		(widget command)
//	loopback cmd <PLplot command>		(pltcl command)
//
// This routine is essentially the same as plTclCmd but without some of
// the window dressing required by the plframe widget.
//--------------------------------------------------------------------------

static int
loopbackCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
             int argc, const char **argv )
{
    register Tcl_HashEntry *hPtr;
    int result = TCL_OK;

    argc--; argv++;
    if ( argc == 0 || ( strcmp( argv[0], "cmd" ) != 0 ) )
    {
        Tcl_AppendResult( interp, "bad option \"", argv[0],
            "\" to \"loopback\": must be ",
            "\"cmd ?options?\" ", (char *) NULL );
        return TCL_ERROR;
    }

// Create hash table on first call

    if ( !cmdTable_initted )
    {
        cmdTable_initted = 1;
        plTclCmd_Init( interp );
    }

// no option -- return list of available PLplot commands

    argc--; argv++;
    if ( argc == 0 )
    {
        Append_Cmdlist( interp );
        return TCL_OK;
    }

// Pick out the desired command

    hPtr = Tcl_FindHashEntry( &cmdTable, argv[0] );
    if ( hPtr == NULL )
    {
        Tcl_AppendResult( interp, "bad option \"", argv[0],
            "\" to \"loopback cmd\": must be one of ",
            (char *) NULL );
        Append_Cmdlist( interp );
        result = TCL_ERROR;
    }
    else
    {
        register Command *cmdPtr = (Command *) Tcl_GetHashValue( hPtr );
        result = ( *cmdPtr->proc )( cmdPtr->clientData, interp, argc, argv );
    }

    return result;
}

//--------------------------------------------------------------------------
// PlbasicInit
//
// Used by Pltcl_Init, Pltk_Init(.c), and Plplotter_Init(.c).  Ensures we have been correctly loaded
// into a Tcl/Tk interpreter, that the plplot.tcl startup file can be
// found and sourced, and that the Matrix library can be found and used,
// and that it correctly exports a stub table.
//--------------------------------------------------------------------------

int
PlbasicInit( Tcl_Interp *interp )
{
    int         debug        = plsc->debug;
    const char  *libDir      = NULL;
    static char initScript[] =
        "tcl_findLibrary plplot " PLPLOT_VERSION " \"\" plplot.tcl PL_LIBRARY pllibrary";
#ifdef PLPLOT_EXTENDED_SEARCH
    static char initScriptExtended[] =
        "tcl_findLibrary plplot " PLPLOT_VERSION "/tcl \"\" plplot.tcl PL_LIBRARY pllibrary";
#endif

#ifdef USE_TCL_STUBS
//
// We hard-wire 8.1 here, rather than TCL_VERSION, TK_VERSION because
// we really don't mind which version of Tcl, Tk we use as long as it
// is 8.1 or newer.  Otherwise if we compiled against 8.2, we couldn't
// be loaded into 8.1
//
    Tcl_InitStubs( interp, "8.1", 0 );
#endif

#if 1
    if ( Matrix_Init( interp ) != TCL_OK )
    {
        if ( debug )
            fprintf( stderr, "error in matrix init\n" );
        return TCL_ERROR;
    }
#else

//
// This code is really designed to be used with a stubified Matrix
// extension.  It is not well tested under a non-stubs situation
// (which is in any case inferior).  The USE_MATRIX_STUBS define
// is made in pltcl.h, and should be removed only with extreme caution.
//
#ifdef USE_MATRIX_STUBS
    if ( Matrix_InitStubs( interp, "0.1", 0 ) == NULL )
    {
        if ( debug )
            fprintf( stderr, "error in matrix stubs init\n" );
        return TCL_ERROR;
    }
#else
    Tcl_PkgRequire( interp, "Matrix", "0.1", 0 );
#endif
#endif

    Tcl_SetVar( interp, "plversion", PLPLOT_VERSION, TCL_GLOBAL_ONLY );

    if ( strcmp( PLPLOT_ITCL_VERSION, "4.0.0" ) >= 0 )
        Tcl_SetVar( interp, "pl_itcl_package_name", "Itcl 4", TCL_GLOBAL_ONLY );
    else if ( strcmp( PLPLOT_ITCL_VERSION, "3.0.0" ) >= 0 )
        Tcl_SetVar( interp, "pl_itcl_package_name", "Itcl 3", TCL_GLOBAL_ONLY );
    else
        // Mark invalid package name in such a way as to cause an error
        // when, for example, itcl has been disabled by PLplot, yet one
        // of the PLplot Tcl scripts attempts to load Itcl.
        Tcl_SetVar( interp, "pl_itcl_package_name", "Itcl(because_not_found_by_PLplot)", TCL_GLOBAL_ONLY );

    if ( strcmp( PLPLOT_ITK_VERSION, "4.0.0" ) >= 0 )
        Tcl_SetVar( interp, "pl_itk_package_name", "Itk 4", TCL_GLOBAL_ONLY );
    else if ( strcmp( PLPLOT_ITK_VERSION, "3.0.0" ) >= 0 )
        Tcl_SetVar( interp, "pl_itk_package_name", "Itk 3", TCL_GLOBAL_ONLY );
    else
        // Mark invalid package name in such a way as to cause an error
        // when, for example, itk has been disabled by PLplot, yet one
        // of the PLplot Tcl scripts attempts to load Itk.
        Tcl_SetVar( interp, "pl_itk_package_name", "Itk(because_not_found_by_PLplot)", TCL_GLOBAL_ONLY );

    if ( strcmp( PLPLOT_IWIDGETS_VERSION, "4.1.0" ) >= 0 )
        Tcl_SetVar( interp, "pl_iwidgets_package_name", "Iwidgets 4", TCL_GLOBAL_ONLY );
    else if ( strcmp( PLPLOT_IWIDGETS_VERSION, "4.0.0" ) >= 0 )
        Tcl_SetVar( interp, "pl_iwidgets_package_name", "-exact Iwidgets " PLPLOT_IWIDGETS_VERSION, TCL_GLOBAL_ONLY );
    else
        // Mark invalid package name in such a way as to cause an error
        // when, for example, itk has been disabled by PLplot, yet one
        // of the PLplot Tcl scripts attempts to load Iwidgets.
        Tcl_SetVar( interp, "pl_iwidgets_package_name", "Iwidgets(because_not_found_by_PLplot)", TCL_GLOBAL_ONLY );


// Begin search for init script
// Each search begins with a test of libDir, so rearrangement is easy.
// If search is successful, both libDir (C) and pllibrary (tcl) are set

// if we are in the build tree, search there
    if ( plInBuildTree() )
    {
        if ( debug )
            fprintf( stderr, "trying BUILD_DIR\n" );
        libDir = BUILD_DIR "/bindings/tcl";
        Tcl_SetVar( interp, "pllibrary", libDir, TCL_GLOBAL_ONLY );
        if ( Tcl_Eval( interp, initScript ) != TCL_OK )
        {
            libDir = NULL;
            Tcl_UnsetVar( interp, "pllibrary", TCL_GLOBAL_ONLY );
            Tcl_ResetResult( interp );
        }
    }

// Tcl extension dir and/or PL_LIBRARY
    if ( libDir == NULL )
    {
        if ( debug )
            fprintf( stderr, "trying init script\n" );
        if ( Tcl_Eval( interp, initScript ) != TCL_OK )
        {
            // This unset is needed for Tcl < 8.4 support.
            Tcl_UnsetVar( interp, "pllibrary", TCL_GLOBAL_ONLY );
            // Clear the result to get rid of the error message
            Tcl_ResetResult( interp );
        }
        else
            libDir = Tcl_GetVar( interp, "pllibrary", TCL_GLOBAL_ONLY );
    }

#ifdef TCL_DIR
// Install directory
    if ( libDir == NULL )
    {
        if ( debug )
            fprintf( stderr, "trying TCL_DIR\n" );
        libDir = TCL_DIR;
        Tcl_SetVar( interp, "pllibrary", libDir, TCL_GLOBAL_ONLY );
        if ( Tcl_Eval( interp, initScript ) != TCL_OK )
        {
            libDir = NULL;
            Tcl_UnsetVar( interp, "pllibrary", TCL_GLOBAL_ONLY );
            Tcl_ResetResult( interp );
        }
    }
#endif

#ifdef PLPLOT_EXTENDED_SEARCH
// Unix extension directory
    if ( libDir == NULL )
    {
        if ( debug )
            fprintf( stderr, "trying extended init script\n" );
        if ( Tcl_Eval( interp, initScriptExtended ) != TCL_OK )
        {
            // This unset is needed for Tcl < 8.4 support.
            Tcl_UnsetVar( interp, "pllibrary", TCL_GLOBAL_ONLY );
            // Clear the result to get rid of the error message
            Tcl_ResetResult( interp );
        }
        else
            libDir = Tcl_GetVar( interp, "pllibrary", TCL_GLOBAL_ONLY );
    }

// Last chance, current directory
    if ( libDir == NULL )
    {
        Tcl_DString ds;
        if ( debug )
            fprintf( stderr, "trying curdir\n" );
        if ( Tcl_Access( "plplot.tcl", 0 ) != 0 )
        {
            if ( debug )
                fprintf( stderr, "couldn't find plplot.tcl in curdir\n" );
            return TCL_ERROR;
        }

        // It seems to be here.  Set pllibrary & eval plplot.tcl "by hand"
        libDir = Tcl_GetCwd( interp, &ds );
        if ( libDir == NULL )
        {
            if ( debug )
                fprintf( stderr, "couldn't get curdir\n" );
            return TCL_ERROR;
        }
        libDir = plstrdup( libDir );
        Tcl_DStringFree( &ds );
        Tcl_SetVar( interp, "pllibrary", libDir, TCL_GLOBAL_ONLY );

        if ( Tcl_EvalFile( interp, "plplot.tcl" ) != TCL_OK )
        {
            if ( debug )
                fprintf( stderr, "error evalling plplot.tcl\n" );
            return TCL_ERROR;
        }
    }
#endif

    if ( libDir == NULL )
    {
        if ( debug )
            fprintf( stderr, "libdir NULL at end of search\n" );
        return TCL_ERROR;
    }

// Used by init code in plctrl.c
    plplotLibDir = plstrdup( libDir );

// wait_until -- waits for a specific condition to arise
// Can be used with either Tcl-DP or TK

    Tcl_CreateCommand( interp, "wait_until", (Tcl_CmdProc *) plWait_Until,
        (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL );

// Define the flags as variables in the PLPLOT namespace
    set_plplot_parameters( interp );

    return TCL_OK;
}

//--------------------------------------------------------------------------
// Pltcl_Init
//
// Initialization routine for extended tclsh's.
// Sets up auto_path, creates the matrix command and numerous commands for
// interfacing to PLplot.  Should not be used in a widget-based system.
//--------------------------------------------------------------------------

int
Pltcl_Init( Tcl_Interp *interp )
{
    register CmdInfo *cmdInfoPtr;
// This must be before any other Tcl related calls
    if ( PlbasicInit( interp ) != TCL_OK )
    {
        Tcl_AppendResult( interp, "Could not find plplot.tcl - please set \
environment variable PL_LIBRARY to the directory containing that file",
            (char *) NULL );

        return TCL_ERROR;
    }

// Register our error variables with PLplot

    plsError( &pl_errcode, errmsg );

// PLplot API commands

    for ( cmdInfoPtr = Cmds; cmdInfoPtr->name != NULL; cmdInfoPtr++ )
    {
        Tcl_CreateCommand( interp, cmdInfoPtr->name, cmdInfoPtr->proc,
            (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL );
    }

// We really need this so the TEA based 'make install' can
// properly determine the package we have installed

    Tcl_PkgProvide( interp, "Pltcl", PLPLOT_VERSION );
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plWait_Until
//
// Tcl command -- wait until the specified condition is satisfied.
// Processes all events while waiting.
//
// This command is more capable than tkwait, and has the added benefit
// of working with Tcl-DP as well.  Example usage:
//
//  wait_until {[info exists foobar]}
//
// Note the [info ...] command must be protected by braces so that it
// isn't actually evaluated until passed into this routine.
//--------------------------------------------------------------------------

int
plWait_Until( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp, int PL_UNUSED( argc ), const char **argv )
{
    int result = 0;

    dbug_enter( "plWait_Until" );

    for (;; )
    {
        if ( Tcl_ExprBoolean( interp, argv[1], &result ) )
        {
            fprintf( stderr, "wait_until command \"%s\" failed:\n\t %s\n",
                argv[1], Tcl_GetStringResult( interp ) );
            break;
        }
        if ( result )
            break;

        Tcl_DoOneEvent( 0 );
    }
    return TCL_OK;
}

//--------------------------------------------------------------------------
// pls_auto_path
//
// Sets up auto_path variable.
// Directories are added to the FRONT of autopath.  Therefore, they are
// searched in reverse order of how they are listed below.
//
// Note: there is no harm in adding extra directories, even if they don't
// actually exist (aside from a slight increase in processing time when
// the autoloaded proc is first found).
//--------------------------------------------------------------------------

int
pls_auto_path( Tcl_Interp *interp )
{
    int  debug = plsc->debug;
    char *buf, *ptr = NULL, *dn;
    int  return_code = TCL_OK;
#ifdef DEBUG
    char *path;
#endif

    buf = (char *) malloc( 256 * sizeof ( char ) );

// Add TCL_DIR

#ifdef TCL_DIR
    if ( debug )
        fprintf( stderr, "adding %s to auto_path\n", TCL_DIR );
    Tcl_SetVar( interp, "dir", TCL_DIR, TCL_GLOBAL_ONLY );
    if ( tcl_cmd( interp, "set auto_path [linsert $auto_path 0 $dir]" ) == TCL_ERROR )
    {
        return_code = TCL_ERROR;
        goto finish;
    }
#ifdef DEBUG
    path = Tcl_GetVar( interp, "auto_path", 0 );
    fprintf( stderr, "auto_path is %s\n", path );
#endif
#endif

// Add $HOME/tcl

    if ( ( dn = getenv( "HOME" ) ) != NULL )
    {
        plGetName( dn, "tcl", "", &ptr );
        Tcl_SetVar( interp, "dir", ptr, 0 );
        if ( tcl_cmd( interp, "set auto_path [linsert $auto_path 0 $dir]" ) == TCL_ERROR )
        {
            return_code = TCL_ERROR;
            goto finish;
        }
#ifdef DEBUG
        fprintf( stderr, "adding %s to auto_path\n", ptr );
        path = Tcl_GetVar( interp, "auto_path", 0 );
        fprintf( stderr, "auto_path is %s\n", path );
#endif
    }

// Add PL_TCL_ENV = $(PL_TCL)

#if defined ( PL_TCL_ENV )
    if ( ( dn = getenv( PL_TCL_ENV ) ) != NULL )
    {
        plGetName( dn, "", "", &ptr );
        Tcl_SetVar( interp, "dir", ptr, 0 );
        if ( tcl_cmd( interp, "set auto_path [linsert $auto_path 0 $dir]" ) == TCL_ERROR )
        {
            return_code = TCL_ERROR;
            goto finish;
        }
#ifdef DEBUG
        fprintf( stderr, "adding %s to auto_path\n", ptr );
        path = Tcl_GetVar( interp, "auto_path", 0 );
        fprintf( stderr, "auto_path is %s\n", path );
#endif
    }
#endif  // PL_TCL_ENV

// Add PL_HOME_ENV/tcl = $(PL_HOME_ENV)/tcl

#if defined ( PL_HOME_ENV )
    if ( ( dn = getenv( PL_HOME_ENV ) ) != NULL )
    {
        plGetName( dn, "tcl", "", &ptr );
        Tcl_SetVar( interp, "dir", ptr, 0 );
        if ( tcl_cmd( interp, "set auto_path [linsert $auto_path 0 $dir]" ) == TCL_ERROR )
        {
            return_code = TCL_ERROR;
            goto finish;
        }
#ifdef DEBUG
        fprintf( stderr, "adding %s to auto_path\n", ptr );
        path = Tcl_GetVar( interp, "auto_path", 0 );
        fprintf( stderr, "auto_path is %s\n", path );
#endif
    }
#endif  // PL_HOME_ENV

// Add cwd

    if ( getcwd( buf, 256 ) == 0 )
    {
        Tcl_SetResult( interp, "Problems with getcwd in pls_auto_path", TCL_STATIC );
        {
            return_code = TCL_ERROR;
            goto finish;
        }
    }
    Tcl_SetVar( interp, "dir", buf, 0 );
    if ( tcl_cmd( interp, "set auto_path [linsert $auto_path 0 $dir]" ) == TCL_ERROR )
    {
        return_code = TCL_ERROR;
        goto finish;
    }
    //** see if plserver was invoked in the build tree **
    if ( plInBuildTree() )
    {
        Tcl_SetVar( interp, "dir", BUILD_DIR "/bindings/tk", TCL_GLOBAL_ONLY );
        if ( tcl_cmd( interp, "set auto_path [linsert $auto_path 0 $dir]" ) == TCL_ERROR )
        {
            return_code = TCL_ERROR;
            goto finish;
        }
    }

#ifdef DEBUG
    fprintf( stderr, "adding %s to auto_path\n", buf );
    path = Tcl_GetVar( interp, "auto_path", 0 );
    fprintf( stderr, "auto_path is %s\n", path );
#endif

finish:    free_mem( buf );
    free_mem( ptr );

    return return_code;
}

//--------------------------------------------------------------------------
// tcl_cmd
//
// Evals the specified command, aborting on an error.
//--------------------------------------------------------------------------

static int
tcl_cmd( Tcl_Interp *interp, const char *cmd )
{
    int result;

    result = Tcl_VarEval( interp, cmd, (char **) NULL );
    if ( result != TCL_OK )
    {
        fprintf( stderr, "TCL command \"%s\" failed:\n\t %s\n",
            cmd, Tcl_GetStringResult( interp ) );
    }
    return result;
}

//--------------------------------------------------------------------------
// PLplot API Calls
//
// Any call that results in something actually being plotted must be
// followed by by a call to plflush(), to make sure all output from
// that command is finished.  Devices that have text/graphics screens
// (e.g. Tek4xxx and emulators) implicitly switch to the graphics screen
// before graphics commands, so a plgra() is not necessary in this case.
// Although if you switch to the text screen via user control (instead of
// using pltext()), the device will get confused.
//--------------------------------------------------------------------------

static char buf[200];

#include "tclgen.c"

//--------------------------------------------------------------------------
// plcontCmd
//
// Processes plcont Tcl command.
//
// The C function is:
// void
// c_plcont(PLFLT **f, PLINT nx, PLINT ny, PLINT kx, PLINT lx,
//       PLINT ky, PLINT ly, PLFLT *clevel, PLINT nlevel,
//       void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
//       PLPointer pltr_data);
//
// Since f will be specified by a Tcl Matrix, nx and ny are redundant, and
// are automatically eliminated.  Same for nlevel, since clevel will be a 1-d
// Tcl Matrix.  Since most people plot the whole data set, we will allow kx,
// lx and ky, ly to be defaulted--either you specify all four, or none of
// them.  We allow three ways of specifying the coordinate transforms: 1)
// Nothing, in which case we will use the identity mapper pltr0 2) pltr1, in
// which case the next two args must be 1-d Tcl Matricies 3) pltr2, in which
// case the next two args must be 2-d Tcl Matricies.  Finally, a new
// paramater is allowed at the end to specify which, if either, of the
// coordinates wrap on themselves.  Can be 1 or x, or 2 or y.  Nothing or 0
// specifies that neither coordinate wraps.
//
// So, the new call from Tcl is:
//      plcont f [kx lx ky ly] clev [pltr x y] [wrap]
//
//--------------------------------------------------------------------------

static int tclmateval_modx, tclmateval_mody;

PLFLT tclMatrix_feval( PLINT i, PLINT j, PLPointer p )
{
    tclMatrix *matPtr = (tclMatrix *) p;

    i = i % tclmateval_modx;
    j = j % tclmateval_mody;

//    printf( "tclMatrix_feval: i=%d j=%d f=%f\n", i, j,
//    matPtr->fdata[I2D(i,j)] );
//
    return matPtr->fdata[I2D( i, j )];
}

static int
plcontCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
           int argc, const char *argv[] )
{
    tclMatrix  *matPtr, *matf, *matclev;
    PLINT      nx, ny, kx = 0, lx = 0, ky = 0, ly = 0, nclev;
    const char *pltrname = "pltr0";
    tclMatrix  *mattrx   = NULL, *mattry = NULL;
    PLFLT      **z, **zused, **zwrapped;

    int        arg3_is_kx = 1, i, j;
    void       ( *pltr )( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer );
    PLPointer  pltr_data = NULL;
    PLcGrid    cgrid1;
    PLcGrid2   cgrid2;

    int        wrap = 0;

    if ( argc < 3 )
    {
        Tcl_AppendResult( interp, "wrong # args: see documentation for ",
            argv[0], (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( matf, interp, argv[1] );

    if ( matf->dim != 2 )
    {
        Tcl_SetResult( interp, "Must use 2-d data.", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        nx = matf->n[0];
        ny = matf->n[1];
        tclmateval_modx = nx;
        tclmateval_mody = ny;

        // convert matf to 2d-array so can use standard wrap approach
        // from now on in this code.
        plAlloc2dGrid( &z, nx, ny );
        for ( i = 0; i < nx; i++ )
        {
            for ( j = 0; j < ny; j++ )
            {
                z[i][j] = tclMatrix_feval( i, j, matf );
            }
        }
    }

// Now check the next argument.  If it is all digits, then it must be kx,
// otherwise it is the name of clev.

    for ( i = 0; i < (int) strlen( argv[2] ) && arg3_is_kx; i++ )
        if ( !isdigit( argv[2][i] ) )
            arg3_is_kx = 0;

    if ( arg3_is_kx )
    {
        // Check that there are enough args
        if ( argc < 7 )
        {
            Tcl_SetResult( interp, "plcont, bogus syntax", TCL_STATIC );
            return TCL_ERROR;
        }

        // Peel off the ones we need
        kx = atoi( argv[3] );
        lx = atoi( argv[4] );
        ky = atoi( argv[5] );
        ly = atoi( argv[6] );

        // adjust argc, argv to reflect our consumption
        argc -= 6, argv += 6;
    }
    else
    {
        argc -= 2, argv += 2;
    }

// The next argument has to be clev

    if ( argc < 1 )
    {
        Tcl_SetResult( interp, "plcont, bogus syntax", TCL_STATIC );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( matclev, interp, argv[0] );
    nclev = matclev->n[0];

    if ( matclev->dim != 1 )
    {
        Tcl_SetResult( interp, "clev must be 1-d matrix.", TCL_STATIC );
        return TCL_ERROR;
    }

    argc--, argv++;

// Now handle trailing optional parameters, if any

    if ( argc >= 3 )
    {
        // There is a pltr spec, parse it.
        pltrname = argv[0];
        CHECK_Tcl_GetMatrixPtr( mattrx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( mattry, interp, argv[2] );

        argc -= 3, argv += 3;
    }

    if ( argc )
    {
        // There is a wrap spec, get it.
        wrap = atoi( argv[0] );

        // Hmm, I said the the doc they could also say x or y, have to come back
        // to this...

        argc--, argv++;
    }

// There had better not be anything else on the command line by this point.

    if ( argc )
    {
        Tcl_SetResult( interp, "plcont, bogus syntax, too many args.", TCL_STATIC );
        return TCL_ERROR;
    }

// Now we need to set up the data for contouring.

    if ( !strcmp( pltrname, "pltr0" ) )
    {
        pltr  = pltr0;
        zused = z;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }
    }
    else if ( !strcmp( pltrname, "pltr1" ) )
    {
        pltr      = pltr1;
        cgrid1.xg = mattrx->fdata;
        cgrid1.nx = nx;
        cgrid1.yg = mattry->fdata;
        cgrid1.ny = ny;
        zused     = z;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( mattrx->dim != 1 || mattry->dim != 1 )
        {
            Tcl_SetResult( interp, "Must use 1-d coord arrays with pltr1.", TCL_STATIC );
            return TCL_ERROR;
        }

        pltr_data = &cgrid1;
    }
    else if ( !strcmp( pltrname, "pltr2" ) )
    {
        // printf( "plcont, setting up for pltr2\n" );
        if ( !wrap )
        {
            // printf( "plcont, no wrapping is needed.\n" );
            plAlloc2dGrid( &cgrid2.xg, nx, ny );
            plAlloc2dGrid( &cgrid2.yg, nx, ny );
            cgrid2.nx = nx;
            cgrid2.ny = ny;
            zused     = z;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
        }
        else if ( wrap == 1 )
        {
            plAlloc2dGrid( &cgrid2.xg, nx + 1, ny );
            plAlloc2dGrid( &cgrid2.yg, nx + 1, ny );
            plAlloc2dGrid( &zwrapped, nx + 1, ny );
            cgrid2.nx = nx + 1;
            cgrid2.ny = ny;
            zused     = zwrapped;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                    zwrapped[i][j]  = z[i][j];
                }
            }

            for ( j = 0; j < ny; j++ )
            {
                cgrid2.xg[nx][j] = cgrid2.xg[0][j];
                cgrid2.yg[nx][j] = cgrid2.yg[0][j];
                zwrapped[nx][j]  = zwrapped[0][j];
            }

            // z not used in executable path after this so free it before
            // nx value is changed.
            plFree2dGrid( z, nx, ny );

            nx++;
        }
        else if ( wrap == 2 )
        {
            plAlloc2dGrid( &cgrid2.xg, nx, ny + 1 );
            plAlloc2dGrid( &cgrid2.yg, nx, ny + 1 );
            plAlloc2dGrid( &zwrapped, nx, ny + 1 );
            cgrid2.nx = nx;
            cgrid2.ny = ny + 1;
            zused     = zwrapped;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                    zwrapped[i][j]  = z[i][j];
                }
            }

            for ( i = 0; i < nx; i++ )
            {
                cgrid2.xg[i][ny] = cgrid2.xg[i][0];
                cgrid2.yg[i][ny] = cgrid2.yg[i][0];
                zwrapped[i][ny]  = zwrapped[i][0];
            }

            // z not used in executable path after this so free it before
            // ny value is changed.
            plFree2dGrid( z, nx, ny );

            ny++;
        }
        else
        {
            Tcl_SetResult( interp, "Invalid wrap specifier, must be <empty>, 0, 1, or 2.", TCL_STATIC );
            return TCL_ERROR;
        }

        pltr      = pltr2;
        pltr_data = &cgrid2;
    }
    else
    {
        Tcl_AppendResult( interp,
            "Unrecognized coordinate transformation spec:",
            pltrname, ", must be pltr0 pltr1 or pltr2.",
            (char *) NULL );
        return TCL_ERROR;
    }
    if ( !arg3_is_kx )
    {
        // default values must be set here since nx, ny can change with wrap.
        kx = 1; lx = nx;
        ky = 1; ly = ny;
    }

//    printf( "plcont: nx=%d ny=%d kx=%d lx=%d ky=%d ly=%d\n",
//          nx, ny, kx, lx, ky, ly );
//  printf( "plcont: nclev=%d\n", nclev );
//

// contour the data.

    plcont( (const PLFLT * const *) zused, nx, ny,
        kx, lx, ky, ly,
        matclev->fdata, nclev,
        pltr, pltr_data );

// Now free up any space which got allocated for our coordinate trickery.

// zused points to either z or zwrapped.  In both cases the allocated size
// was nx by ny.  Now free the allocated space, and note in the case
// where zused points to zwrapped, the separate z space has been freed by
// previous wrap logic.
    plFree2dGrid( zused, nx, ny );

    if ( pltr == pltr1 )
    {
        // Hmm, actually, nothing to do here currently, since we just used the
        // Tcl Matrix data directly, rather than allocating private space.
    }
    else if ( pltr == pltr2 )
    {
        // printf( "plcont, freeing space for grids used in pltr2\n" );
        plFree2dGrid( cgrid2.xg, nx, ny );
        plFree2dGrid( cgrid2.yg, nx, ny );
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plsvect
//
// Implement Tcl-side setting of arrow style.
//--------------------------------------------------------------------------

static int
plsvectCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
            int argc, const char *argv[] )
{
    tclMatrix *matx, *maty;
    PLINT     npts;
    PLBOOL    fill;

    if ( argc == 1
         || ( strcmp( argv[1], "NULL" ) == 0 ) && ( strcmp( argv[2], "NULL" ) == 0 ) )
    {
        // The user has requested to clear the transform setting.
        plsvect( NULL, NULL, 0, 0 );
        return TCL_OK;
    }
    else if ( argc != 4 )
    {
        Tcl_AppendResult( interp, "wrong # args: see documentation for ",
            argv[0], (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );

    if ( matx->dim != 1 )
    {
        Tcl_SetResult( interp, "plsvect: Must use 1-d data.", TCL_STATIC );
        return TCL_ERROR;
    }
    npts = matx->n[0];

    CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );

    if ( maty->dim != 1 )
    {
        Tcl_SetResult( interp, "plsvect: Must use 1-d data.", TCL_STATIC );
        return TCL_ERROR;
    }

    if ( maty->n[0] != npts )
    {
        Tcl_SetResult( interp, "plsvect: Arrays must be of equal length", TCL_STATIC );
        return TCL_ERROR;
    }

    fill = (PLBOOL) atoi( argv[3] );

    plsvect( matx->fdata, maty->fdata, npts, fill );

    return TCL_OK;
}


//--------------------------------------------------------------------------
// plvect implementation (based on plcont above)
//--------------------------------------------------------------------------
static int
plvectCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
           int argc, const char *argv[] )
{
    tclMatrix  *matPtr, *matu, *matv;
    PLINT      nx, ny;
    const char *pltrname = "pltr0";
    tclMatrix  *mattrx   = NULL, *mattry = NULL;
    PLFLT      **u, **v, **uused, **vused, **uwrapped, **vwrapped;
    PLFLT      scaling;

    int        i, j;
    void       ( *pltr )( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer );
    PLPointer  pltr_data = NULL;
    PLcGrid    cgrid1;
    PLcGrid2   cgrid2;

    int        wrap = 0;

    if ( argc < 3 )
    {
        Tcl_AppendResult( interp, "wrong # args: see documentation for ",
            argv[0], (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( matu, interp, argv[1] );

    if ( matu->dim != 2 )
    {
        Tcl_SetResult( interp, "Must use 2-d data.", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        nx = matu->n[0];
        ny = matu->n[1];
        tclmateval_modx = nx;
        tclmateval_mody = ny;

        // convert matu to 2d-array so can use standard wrap approach
        // from now on in this code.
        plAlloc2dGrid( &u, nx, ny );
        for ( i = 0; i < nx; i++ )
        {
            for ( j = 0; j < ny; j++ )
            {
                u[i][j] = tclMatrix_feval( i, j, matu );
            }
        }
    }

    CHECK_Tcl_GetMatrixPtr( matv, interp, argv[2] );

    if ( matv->dim != 2 )
    {
        Tcl_SetResult( interp, "Must use 2-d data.", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        nx = matv->n[0];
        ny = matv->n[1];
        tclmateval_modx = nx;
        tclmateval_mody = ny;

        // convert matv to 2d-array so can use standard wrap approach
        // from now on in this code.
        plAlloc2dGrid( &v, nx, ny );
        for ( i = 0; i < nx; i++ )
        {
            for ( j = 0; j < ny; j++ )
            {
                v[i][j] = tclMatrix_feval( i, j, matv );
            }
        }
    }

    argc -= 3, argv += 3;

// The next argument has to be scaling

    if ( argc < 1 )
    {
        Tcl_SetResult( interp, "plvect, bogus syntax", TCL_STATIC );
        return TCL_ERROR;
    }

    scaling = atof( argv[0] );
    argc--, argv++;

// Now handle trailing optional parameters, if any

    if ( argc >= 3 )
    {
        // There is a pltr spec, parse it.
        pltrname = argv[0];
        CHECK_Tcl_GetMatrixPtr( mattrx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( mattry, interp, argv[2] );

        argc -= 3, argv += 3;
    }

    if ( argc )
    {
        // There is a wrap spec, get it.
        wrap = atoi( argv[0] );

        // Hmm, I said the the doc they could also say x or y, have to come back
        // to this...

        argc--, argv++;
    }

// There had better not be anything else on the command line by this point.

    if ( argc )
    {
        Tcl_SetResult( interp, "plvect, bogus syntax, too many args.", TCL_STATIC );
        return TCL_ERROR;
    }

// Now we need to set up the data for contouring.

    if ( !strcmp( pltrname, "pltr0" ) )
    {
        pltr  = pltr0;
        uused = u;
        vused = v;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }
    }
    else if ( !strcmp( pltrname, "pltr1" ) )
    {
        pltr      = pltr1;
        cgrid1.xg = mattrx->fdata;
        cgrid1.nx = nx;
        cgrid1.yg = mattry->fdata;
        cgrid1.ny = ny;
        uused     = u;
        vused     = v;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( mattrx->dim != 1 || mattry->dim != 1 )
        {
            Tcl_SetResult( interp, "Must use 1-d coord arrays with pltr1.", TCL_STATIC );
            return TCL_ERROR;
        }

        pltr_data = &cgrid1;
    }
    else if ( !strcmp( pltrname, "pltr2" ) )
    {
        // printf( "plvect, setting up for pltr2\n" );
        if ( !wrap )
        {
            // printf( "plvect, no wrapping is needed.\n" );
            plAlloc2dGrid( &cgrid2.xg, nx, ny );
            plAlloc2dGrid( &cgrid2.yg, nx, ny );
            cgrid2.nx = nx;
            cgrid2.ny = ny;
            uused     = u;
            vused     = v;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];
            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                }
            }
        }
        else if ( wrap == 1 )
        {
            plAlloc2dGrid( &cgrid2.xg, nx + 1, ny );
            plAlloc2dGrid( &cgrid2.yg, nx + 1, ny );
            plAlloc2dGrid( &uwrapped, nx + 1, ny );
            plAlloc2dGrid( &vwrapped, nx + 1, ny );
            cgrid2.nx = nx + 1;
            cgrid2.ny = ny;
            uused     = uwrapped;
            vused     = vwrapped;


            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                    uwrapped[i][j]  = u[i][j];
                    vwrapped[i][j]  = v[i][j];
                }
            }

            for ( j = 0; j < ny; j++ )
            {
                cgrid2.xg[nx][j] = cgrid2.xg[0][j];
                cgrid2.yg[nx][j] = cgrid2.yg[0][j];
                uwrapped[nx][j]  = uwrapped[0][j];
                vwrapped[nx][j]  = vwrapped[0][j];
            }

            // u and v not used in executable path after this so free it
            // before nx value is changed.
            plFree2dGrid( u, nx, ny );
            plFree2dGrid( v, nx, ny );
            nx++;
        }
        else if ( wrap == 2 )
        {
            plAlloc2dGrid( &cgrid2.xg, nx, ny + 1 );
            plAlloc2dGrid( &cgrid2.yg, nx, ny + 1 );
            plAlloc2dGrid( &uwrapped, nx, ny + 1 );
            plAlloc2dGrid( &vwrapped, nx, ny + 1 );
            cgrid2.nx = nx;
            cgrid2.ny = ny + 1;
            uused     = uwrapped;
            vused     = vwrapped;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                    uwrapped[i][j]  = u[i][j];
                    vwrapped[i][j]  = v[i][j];
                }
            }

            for ( i = 0; i < nx; i++ )
            {
                cgrid2.xg[i][ny] = cgrid2.xg[i][0];
                cgrid2.yg[i][ny] = cgrid2.yg[i][0];
                uwrapped[i][ny]  = uwrapped[i][0];
                vwrapped[i][ny]  = vwrapped[i][0];
            }

            // u and v not used in executable path after this so free it
            // before ny value is changed.
            plFree2dGrid( u, nx, ny );
            plFree2dGrid( v, nx, ny );

            ny++;
        }
        else
        {
            Tcl_SetResult( interp, "Invalid wrap specifier, must be <empty>, 0, 1, or 2.", TCL_STATIC );
            return TCL_ERROR;
        }

        pltr      = pltr2;
        pltr_data = &cgrid2;
    }
    else
    {
        Tcl_AppendResult( interp,
            "Unrecognized coordinate transformation spec:",
            pltrname, ", must be pltr0 pltr1 or pltr2.",
            (char *) NULL );
        return TCL_ERROR;
    }


// plot the vector data.

    plvect( (const PLFLT * const *) uused, (const PLFLT * const *) vused, nx, ny,
        scaling, pltr, pltr_data );
// Now free up any space which got allocated for our coordinate trickery.

// uused points to either u or uwrapped.  In both cases the allocated size
// was nx by ny.  Now free the allocated space, and note in the case
// where uused points to uwrapped, the separate u space has been freed by
// previous wrap logic.
    plFree2dGrid( uused, nx, ny );
    plFree2dGrid( vused, nx, ny );

    if ( pltr == pltr1 )
    {
        // Hmm, actually, nothing to do here currently, since we just used the
        // Tcl Matrix data directly, rather than allocating private space.
    }
    else if ( pltr == pltr2 )
    {
        // printf( "plvect, freeing space for grids used in pltr2\n" );
        plFree2dGrid( cgrid2.xg, nx, ny );
        plFree2dGrid( cgrid2.yg, nx, ny );
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
//
// plmeshCmd
//
// Processes plmesh Tcl command.
//
// We support 3 different invocation forms:
// 1)	plmesh x y z nx ny opt (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 2)	plmesh x y z opt
// 3)	plmesh z opt
//
// Form 1) is an exact mirror of the usual C API.  In form 2) we infer nx and
// ny from the input data, and in form 3 we inver nx and ny, and also take
// the x and y arrays to just be integral spacing.
//--------------------------------------------------------------------------

static int
plmeshCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
           int argc, const char *argv[] )
{
    PLINT     nx, ny, opt;
    PLFLT     *x, *y, **z;
    tclMatrix *matx, *maty, *matz, *matPtr;
    int       i;

#ifdef PLPLOTTCLTK_NON_REDACTED_API
    if ( argc == 7 )
    {
        nx  = atoi( argv[4] );
        ny  = atoi( argv[5] );
        opt = atoi( argv[6] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }
    else if ( argc == 5 )
#else
    if ( argc == 5 )
#endif
    {
        opt = atoi( argv[4] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        nx = matx->n[0]; ny = maty->n[0];

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }
    else if ( argc == 3 )
    {
        Tcl_SetResult( interp, "unimplemented", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        Tcl_AppendResult( interp, "wrong # args: should be \"plmesh ",
            "x y z nx ny opt\", or a valid contraction ",
            "thereof.", (char *) NULL );
        return TCL_ERROR;
    }

    plmesh( x, y, (const PLFLT * const *) z, nx, ny, opt );

    if ( argc == 7 )
    {
        free( z );
    }
    else if ( argc == 5 )
    {
        free( z );
    }
    else                        // argc == 3
    {
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plmeshcCmd
//
// Processes plmeshc Tcl command.
//
// We support 6 different invocation forms:
// 1)	plmeshc x y z nx ny opt clevel nlevel (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 2)	plmeshc x y z nx ny opt clevel (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 3)	plmeshc x y z nx ny opt (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 4)	plmeshc x y z opt clevel
// 5)	plmeshc x y z opt
// 6)	plmeshc z opt
//
// Form 1) is an exact mirror of the usual C API.  In form 2) we infer nlevel.
// In form 3, 5 and 6 clevel is set to NULL. In form 4 we infer nx, ny, and nlevel
// from the input data, in form 5 we infer nx and ny, and in form 6 we take
// the x and y arrays to just be integral spacing.
//--------------------------------------------------------------------------

static int
plmeshcCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
            int argc, const char *argv[] )
{
    PLINT     nx, ny, opt, nlev = 10;
    PLFLT     *x, *y, **z;
    PLFLT     *clev;

    tclMatrix *matx, *maty, *matz, *matPtr, *matlev;
    int       i;

#ifdef PLPLOTTCLTK_NON_REDACTED_API
    if ( argc == 9 )
    {
        nlev = atoi( argv[8] );
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[7] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "popo Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 8 )
    {
        nx  = atoi( argv[4] );
        ny  = atoi( argv[5] );
        opt = atoi( argv[6] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.
        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[7] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;
        nlev = matlev->n[0];

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 7 )
    {
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );
        clev = NULL;

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 6 )
#else
    if ( argc == 6 )
#endif
    {
        opt = atoi( argv[4] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.
        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[5] );

        nx   = matx->n[0];
        ny   = maty->n[0];
        nlev = matlev->n[0];

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 5 )
    {
        opt  = atoi( argv[4] );
        clev = NULL;

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        nx = matx->n[0]; ny = maty->n[0];

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }
    else if ( argc == 3 )
    {
        Tcl_SetResult( interp, "unimplemented", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        Tcl_AppendResult( interp, "wrong # args: should be \"plmeshc ",
            "x y z nx ny opt clevel nlevel\", or a valid contraction ",
            "thereof.", (char *) NULL );
        return TCL_ERROR;
    }

    plmeshc( x, y, (const PLFLT * const *) z, nx, ny, opt, clev, nlev );

    if ( argc == 7 )
    {
        free( z );
    }
    else if ( argc == 5 || argc == 6 )
    {
        free( z );
    }
    else                        // argc == 3
    {
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plot3dCmd
//
// Processes plot3d Tcl command.
//
// We support 3 different invocation forms:
// 1)	plot3d x y z nx ny opt side (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 2)	plot3d x y z opt side
// 3)	plot3d z opt side
//
// Form 1) is an exact mirror of the usual C API.  In form 2) we infer nx and
// ny from the input data, and in form 3 we inver nx and ny, and also take
// the x and y arrays to just be integral spacing.
//--------------------------------------------------------------------------

static int
plot3dCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
           int argc, const char *argv[] )
{
    PLINT     nx, ny, opt, side;
    PLFLT     *x, *y, **z;
    tclMatrix *matx, *maty, *matz, *matPtr;
    int       i;

#ifdef PLPLOTTCLTK_NON_REDACTED_API
    if ( argc == 8 )
    {
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );
        side = atoi( argv[7] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }
    else if ( argc == 6 )
#else
    if ( argc == 6 )
#endif
    {
        opt  = atoi( argv[4] );
        side = atoi( argv[5] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        nx = matx->n[0]; ny = maty->n[0];

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }
    else if ( argc == 4 )
    {
        Tcl_SetResult( interp, "unimplemented", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        Tcl_AppendResult( interp, "wrong # args: should be \"plot3d ",
            "x y z nx ny opt side\", or a valid contraction ",
            "thereof.", (char *) NULL );
        return TCL_ERROR;
    }

    plot3d( x, y, (const PLFLT * const *) z, nx, ny, opt, side );

    if ( argc == 8 )
    {
        free( z );
    }
    else if ( argc == 6 )
    {
        free( z );
    }
    else                        // argc == 4
    {
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plot3dcCmd
//
// Processes plot3dc Tcl command.
//
// We support 6 different invocation forms:
// 1)	plot3dc x y z nx ny opt clevel nlevel (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 2)	plot3dc x y z nx ny opt clevel (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 3)	plot3dc x y z nx ny opt (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 4)	plot3dc x y z opt clevel
// 5)	plot3dc x y z opt
// 6)	plot3dc z opt
//
// Form 1) is an exact mirror of the usual C API.  In form 2) we infer nlevel.
// In form 3, 5 and 6 clevel is set to NULL. In form 4 we infer nx, ny, and nlevel
// from the input data, in form 5 we infer nx and ny, and in form 6 we take
// the x and y arrays to just be integral spacing.
//--------------------------------------------------------------------------

static int
plot3dcCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
            int argc, const char *argv[] )
{
    PLINT     nx, ny, opt, nlev = 10;
    PLFLT     *x, *y, **z;
    PLFLT     *clev;

    tclMatrix *matx, *maty, *matz, *matPtr, *matlev;
    int       i;

#ifdef PLPLOTTCLTK_NON_REDACTED_API
    if ( argc == 9 )
    {
        nlev = atoi( argv[8] );
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[7] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "popo Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 8 )
    {
        nx  = atoi( argv[4] );
        ny  = atoi( argv[5] );
        opt = atoi( argv[6] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.
        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[7] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;
        nlev = matlev->n[0];

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 7 )
    {
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );
        clev = NULL;

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 6 )
#else
    if ( argc == 6 )
#endif
    {
        opt = atoi( argv[4] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.
        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[5] );

        nx   = matx->n[0];
        ny   = maty->n[0];
        nlev = matlev->n[0];

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 5 )
    {
        opt  = atoi( argv[4] );
        clev = NULL;

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        nx = matx->n[0]; ny = maty->n[0];

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }
    else if ( argc == 3 )
    {
        Tcl_SetResult( interp, "unimplemented", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        Tcl_AppendResult( interp, "wrong # args: should be \"plot3dc ",
            "x y z nx ny opt clevel nlevel\", or a valid contraction ",
            "thereof.", (char *) NULL );
        return TCL_ERROR;
    }

    plot3dc( x, y, (const PLFLT * const *) z, nx, ny, opt, clev, nlev );

    if ( argc == 7 )
    {
        free( z );
    }
    else if ( argc == 5 || argc == 6 )
    {
        free( z );
    }
    else                        // argc == 3
    {
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plsurf3dCmd
//
// Processes plsurf3d Tcl command.
//
// We support 6 different invocation forms:
// 1)	plsurf3d x y z nx ny opt clevel nlevel (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 2)	plsurf3d x y z nx ny opt clevel (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 3)	plsurf3d x y z nx ny opt (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 4)	plsurf3d x y z opt clevel
// 5)	plsurf3d x y z opt
// 6)	plsurf3d z opt
//
// Form 1) is an exact mirror of the usual C API.  In form 2) we infer nlevel.
// In form 3, 5 and 6 clevel is set to NULL. In form 4 we infer nx, ny, and nlevel
// from the input data, in form 5 we infer nx and ny, and in form 6 we take
// the x and y arrays to just be integral spacing.
//--------------------------------------------------------------------------

static int
plsurf3dCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
             int argc, const char *argv[] )
{
    PLINT     nx, ny, opt, nlev = 10;
    PLFLT     *x, *y, **z;
    PLFLT     *clev;

    tclMatrix *matx, *maty, *matz, *matPtr, *matlev;
    int       i;

#ifdef PLPLOTTCLTK_NON_REDACTED_API
    if ( argc == 9 )
    {
        nlev = atoi( argv[8] );
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[7] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "popo Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 8 )
    {
        nx  = atoi( argv[4] );
        ny  = atoi( argv[5] );
        opt = atoi( argv[6] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.
        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[7] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;
        nlev = matlev->n[0];

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 7 )
    {
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );
        clev = NULL;

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 6 )
#else
    if ( argc == 6 )
#endif
    {
        opt = atoi( argv[4] );

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.
        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[5] );

        nx   = matx->n[0];
        ny   = maty->n[0];
        nlev = matlev->n[0];

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 5 )
    {
        opt  = atoi( argv[4] );
        clev = NULL;

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        nx = matx->n[0]; ny = maty->n[0];

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }
    else if ( argc == 3 )
    {
        Tcl_SetResult( interp, "unimplemented", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        Tcl_AppendResult( interp, "wrong # args: should be \"plsurf3d ",
            "x y z nx ny opt clevel nlevel\", or a valid contraction ",
            "thereof.", (char *) NULL );
        return TCL_ERROR;
    }

    plsurf3d( x, y, (const PLFLT * const *) z, nx, ny, opt, clev, nlev );

    if ( argc == 7 )
    {
        free( z );
    }
    else if ( argc == 5 )
    {
        free( z );
    }
    else                        // argc == 3
    {
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plsurf3dlCmd
//
// Processes plsurf3d Tcl command.
//
// We support 6 different invocation forms:
// 1)	plsurf3dl x y z nx ny opt clevel nlevel indexxmin indexxmax indexymin indexymax (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 2)	plsurf3dl x y z nx ny opt clevel indexxmin indexxmax indexymin indexymax (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 3)	plsurf3dl x y z nx ny opt indexxmin indexxmax indexymin indexymax (only if PLPLOTTCLTK_NON_REDACTED_API is #defined)
// 4)	plsurf3dl x y z opt clevel indexxmin indexymin indexymax
// 5)	plsurf3dl x y z opt indexxmin indexymin indexymax
// 6)	plsurf3dl z opt indexxmin indexymin indexymax
//
// Form 1) is an exact mirror of the usual C API.  In form 2) we infer nlevel.
// In form 3, 5 and 6 clevel is set to NULL. In form 4 we infer nx, ny, nlevel, and indexxmax
// from the input data, in form 5 we infer nx ny, and indexxmax, and in form 6 we take
// the x and y arrays to just be integral spacing and infer indexxmax.
//--------------------------------------------------------------------------

static int
plsurf3dlCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
              int argc, const char *argv[] )
{
    PLINT     nx, ny, opt, nlev = 10;
    PLFLT     *x, *y, **z;
    PLFLT     *clev;
    PLINT     indexxmin, indexxmax;

    tclMatrix *matx, *maty, *matz, *matPtr, *matlev;
    tclMatrix *indexymin, *indexymax;
    PLINT     *idxymin, *idxymax;

    int       i;

#ifdef PLPLOTTCLTK_NON_REDACTED_API
    if ( argc == 13 )
    {
        nlev = atoi( argv[8] );
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );

        indexxmin = atoi( argv[9] );
        indexxmax = atoi( argv[10] );
        CHECK_Tcl_GetMatrixPtr( indexymin, interp, argv[11] );
        CHECK_Tcl_GetMatrixPtr( indexymax, interp, argv[12] );
        if ( indexymin->type != TYPE_INT ||
             indexymax->type != TYPE_INT )
        {
            Tcl_SetResult( interp, "indexymin and indexymax must be integer matrices", TCL_STATIC );
            return TCL_ERROR;
        }

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[7] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float matrices", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev ||
             indexymin->dim != 1 || indexymin->n[0] != indexxmax ||
             indexymax->dim != 1 || indexymax->n[0] != indexxmax )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;

        idxymin = indexymin->idata;
        idxymax = indexymax->idata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 12 )
    {
        nx  = atoi( argv[4] );
        ny  = atoi( argv[5] );
        opt = atoi( argv[6] );

        indexxmin = atoi( argv[8] );
        indexxmax = atoi( argv[9] );
        CHECK_Tcl_GetMatrixPtr( indexymin, interp, argv[10] );
        CHECK_Tcl_GetMatrixPtr( indexymax, interp, argv[11] );
        if ( indexymin->type != TYPE_INT ||
             indexymax->type != TYPE_INT )
        {
            Tcl_SetResult( interp, "indexymin and indexymax must be integer matrices", TCL_STATIC );
            return TCL_ERROR;
        }

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.
        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[7] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float matrices", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev ||
             indexymin->dim != 1 || indexymin->n[0] != indexxmax ||
             indexymax->dim != 1 || indexymax->n[0] != indexxmax )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;
        nlev = matlev->n[0];

        idxymin = indexymin->idata;
        idxymax = indexymax->idata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 11 )
    {
        nx   = atoi( argv[4] );
        ny   = atoi( argv[5] );
        opt  = atoi( argv[6] );
        clev = NULL;

        indexxmin = atoi( argv[7] );
        indexxmax = atoi( argv[8] );
        CHECK_Tcl_GetMatrixPtr( indexymin, interp, argv[9] );
        CHECK_Tcl_GetMatrixPtr( indexymax, interp, argv[10] );
        if ( indexymin->type != TYPE_INT ||
             indexymax->type != TYPE_INT )
        {
            Tcl_SetResult( interp, "indexymin and indexymax must be integer matrices", TCL_STATIC );
            return TCL_ERROR;
        }

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float matrices", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             indexymin->dim != 1 || indexymin->n[0] != indexxmax ||
             indexymax->dim != 1 || indexymax->n[0] != indexxmax )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        idxymin = indexymin->idata;
        idxymax = indexymax->idata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 9 )
#else
    if ( argc == 9 )
#endif
    {
        indexxmin = atoi( argv[6] );
        CHECK_Tcl_GetMatrixPtr( indexymin, interp, argv[7] );
        CHECK_Tcl_GetMatrixPtr( indexymax, interp, argv[8] );
        if ( indexymin->type != TYPE_INT ||
             indexymax->type != TYPE_INT )
        {
            Tcl_SetResult( interp, "indexymin and indexymax must be integer matrices", TCL_STATIC );
            return TCL_ERROR;
        }
        indexxmax = indexymin->n[0];

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.
        CHECK_Tcl_GetMatrixPtr( matlev, interp, argv[5] );

        nx  = matx->n[0];
        ny  = maty->n[0];
        opt = atoi( argv[4] );

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT ||
             matlev->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y z and clevel must all be float matrices", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             matlev->dim != 1 || matlev->n[0] != nlev ||
             indexymin->dim != 1 || indexymin->n[0] != indexxmax ||
             indexymax->dim != 1 || indexymax->n[0] != indexxmax )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x    = matx->fdata;
        y    = maty->fdata;
        clev = matlev->fdata;
        nlev = matlev->n[0];

        idxymin = indexymin->idata;
        idxymax = indexymax->idata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }

    else if ( argc == 8 )
    {
        opt  = atoi( argv[4] );
        clev = NULL;

        indexxmin = atoi( argv[5] );
        CHECK_Tcl_GetMatrixPtr( indexymin, interp, argv[6] );
        CHECK_Tcl_GetMatrixPtr( indexymax, interp, argv[7] );
        if ( indexymin->type != TYPE_INT ||
             indexymax->type != TYPE_INT )
        {
            Tcl_SetResult( interp, "indexymin and indexymax must be integer matrices", TCL_STATIC );
            return TCL_ERROR;
        }
        indexxmax = indexymin->n[0];

        CHECK_Tcl_GetMatrixPtr( matx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( maty, interp, argv[2] );
        CHECK_Tcl_GetMatrixPtr( matz, interp, argv[3] );
        matPtr = matz;          // For dumb indexer macro, grrrr.

        if ( matx->type != TYPE_FLOAT ||
             maty->type != TYPE_FLOAT ||
             matz->type != TYPE_FLOAT )
        {
            Tcl_SetResult( interp, "x y and z must all be float", TCL_STATIC );
            return TCL_ERROR;
        }

        nx = matx->n[0]; ny = maty->n[0];

        if ( matx->dim != 1 || matx->n[0] != nx ||
             maty->dim != 1 || maty->n[0] != ny ||
             matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
             indexymin->dim != 1 || indexymin->n[0] != indexxmax ||
             indexymax->dim != 1 || indexymax->n[0] != indexxmax )
        {
            Tcl_SetResult( interp, "Inconsistent dimensions", TCL_STATIC );
            return TCL_ERROR;
        }

        x = matx->fdata;
        y = maty->fdata;

        idxymin = indexymin->idata;
        idxymax = indexymax->idata;

        z = (PLFLT **) malloc( (size_t) nx * sizeof ( PLFLT * ) );
        for ( i = 0; i < nx; i++ )
            z[i] = &matz->fdata[ I2D( i, 0 ) ];
    }
    else if ( argc == 2 )
    {
        Tcl_SetResult( interp, "unimplemented", TCL_STATIC );
        return TCL_ERROR;
    }
    else
    {
        Tcl_AppendResult( interp, "wrong # args: should be \"plsurf3dl ",
            "x y z nx ny opt clevel nlevel indexxmin indexxmax indexymin ",
            "indexymax\", or a valid contraction thereof.", (char *) NULL );
        return TCL_ERROR;
    }

    plsurf3dl( x, y, (const PLFLT * const *) z, nx, ny, opt, clev, nlev, indexxmin, indexxmax, idxymin, idxymax );

    if ( argc == 13 )
    {
        free( z );
    }
    else if ( argc == 9 || argc == 10 )
    {
        free( z );
    }
    else                        // argc == 3
    {
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plranddCmd
//
// Return a random number
//--------------------------------------------------------------------------

static int
plranddCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
            int argc, const char **argv )
{
    if ( argc != 1 )
    {
        Tcl_AppendResult( interp, "wrong # args: ",
            argv[0], " takes no arguments", (char *) NULL );
        return TCL_ERROR;
    }
    else
    {
        Tcl_SetObjResult( interp, Tcl_NewDoubleObj( (double) plrandd() ) );
        return TCL_OK;
    }
}

//--------------------------------------------------------------------------
// plsetoptCmd
//
// Processes plsetopt Tcl command.
//--------------------------------------------------------------------------

static int
plsetoptCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
             int argc, const char **argv )
{
    if ( argc < 2 || argc > 3 )
    {
        Tcl_AppendResult( interp, "wrong # args: should be \"",
            argv[0], " option ?argument?\"", (char *) NULL );
        return TCL_ERROR;
    }

    plsetopt( argv[1], argv[2] );

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plshadeCmd
//
// Processes plshade Tcl command.
// C version takes:
//    data, nx, ny, defined,
//    xmin, xmax, ymin, ymax,
//    sh_min, sh_max, sh_cmap, sh_color, sh_width,
//    min_col, min_wid, max_col, max_wid,
//    plfill, rect, pltr, pltr_data
//
// We will be getting data through a 2-d Matrix, which carries along
// nx and ny, so no need for those.  Toss defined since it's not supported
// anyway.  Toss plfill since it is the only valid choice.  Take an optional
// pltr spec just as for plcont or an alternative of NULL pltr, and add a
// wrapping specifier, as in plcont.  So the new command looks like:
//
// *INDENT-OFF*
//      plshade z xmin xmax ymin ymax
//          sh_min sh_max sh_cmap sh_color sh_width
//          min_col min_wid max_col max_wid
//          rect [[pltr x y] | NULL ] [wrap]
// *INDENT-ON*
//--------------------------------------------------------------------------

static int
plshadeCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
            int argc, const char *argv[] )
{
    tclMatrix  *matPtr, *matz, *mattrx = NULL, *mattry = NULL;
    PLFLT      **z, **zused, **zwrapped;
    PLFLT      xmin, xmax, ymin, ymax, sh_min, sh_max, sh_col;

    PLINT      sh_cmap   = 1;
    PLFLT      sh_wid    = 2.;
    PLINT      min_col   = 1, max_col = 0;
    PLFLT      min_wid   = 0., max_wid = 0.;
    PLINT      rect      = 1;
    const char *pltrname = "pltr0";
    void       ( *pltr )( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer );
    PLPointer  pltr_data = NULL;
    PLcGrid    cgrid1;
    PLcGrid2   cgrid2;
    PLINT      wrap = 0;
    int        nx, ny, i, j;

    if ( argc < 16 )
    {
        Tcl_AppendResult( interp, "bogus syntax for plshade, see doc.",
            (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( matz, interp, argv[1] );
    if ( matz->dim != 2 )
    {
        Tcl_SetResult( interp, "Must plot a 2-d matrix.", TCL_STATIC );
        return TCL_ERROR;
    }

    nx = matz->n[0];
    ny = matz->n[1];

    tclmateval_modx = nx;
    tclmateval_mody = ny;

    // convert matz to 2d-array so can use standard wrap approach
    // from now on in this code.
    plAlloc2dGrid( &z, nx, ny );
    for ( i = 0; i < nx; i++ )
    {
        for ( j = 0; j < ny; j++ )
        {
            z[i][j] = tclMatrix_feval( i, j, matz );
        }
    }

    xmin    = atof( argv[2] );
    xmax    = atof( argv[3] );
    ymin    = atof( argv[4] );
    ymax    = atof( argv[5] );
    sh_min  = atof( argv[6] );
    sh_max  = atof( argv[7] );
    sh_cmap = atoi( argv[8] );
    sh_col  = atof( argv[9] );
    sh_wid  = atof( argv[10] );
    min_col = atoi( argv[11] );
    min_wid = atoi( argv[12] );
    max_col = atoi( argv[13] );
    max_wid = atof( argv[14] );
    rect    = atoi( argv[15] );

    argc -= 16, argv += 16;

    if ( argc >= 3 )
    {
        pltrname = argv[0];
        CHECK_Tcl_GetMatrixPtr( mattrx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( mattry, interp, argv[2] );

        argc -= 3, argv += 3;
    }
    else if ( argc && !strcmp( argv[0], "NULL" ) )
    {
        pltrname = argv[0];
        argc    -= 1, argv += 1;
    }

    if ( argc )
    {
        wrap = atoi( argv[0] );
        argc--, argv++;
    }

    if ( argc )
    {
        Tcl_SetResult( interp, "plshade: bogus arg list", TCL_STATIC );
        return TCL_ERROR;
    }

// Figure out which coordinate transformation model is being used, and setup
// accordingly.

    if ( !strcmp( pltrname, "NULL" ) )
    {
        pltr  = NULL;
        zused = z;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }
    }
    else if ( !strcmp( pltrname, "pltr0" ) )
    {
        pltr  = pltr0;
        zused = z;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }
    }
    else if ( !strcmp( pltrname, "pltr1" ) )
    {
        pltr      = pltr1;
        cgrid1.xg = mattrx->fdata;
        cgrid1.nx = nx;
        cgrid1.yg = mattry->fdata;
        cgrid1.ny = ny;
        zused     = z;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( mattrx->dim != 1 || mattry->dim != 1 )
        {
            Tcl_SetResult( interp, "Must use 1-d coord arrays with pltr1.", TCL_STATIC );
            return TCL_ERROR;
        }

        pltr_data = &cgrid1;
    }
    else if ( !strcmp( pltrname, "pltr2" ) )
    {
        // printf( "plshade, setting up for pltr2\n" );
        if ( !wrap )
        {
            // printf( "plshade, no wrapping is needed.\n" );
            plAlloc2dGrid( &cgrid2.xg, nx, ny );
            plAlloc2dGrid( &cgrid2.yg, nx, ny );
            cgrid2.nx = nx;
            cgrid2.ny = ny;
            zused     = z;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
        }
        else if ( wrap == 1 )
        {
            plAlloc2dGrid( &cgrid2.xg, nx + 1, ny );
            plAlloc2dGrid( &cgrid2.yg, nx + 1, ny );
            plAlloc2dGrid( &zwrapped, nx + 1, ny );
            cgrid2.nx = nx + 1;
            cgrid2.ny = ny;
            zused     = zwrapped;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                    zwrapped[i][j]  = z[i][j];
                }
            }

            for ( j = 0; j < ny; j++ )
            {
                cgrid2.xg[nx][j] = cgrid2.xg[0][j];
                cgrid2.yg[nx][j] = cgrid2.yg[0][j];
                zwrapped[nx][j]  = zwrapped[0][j];
            }

            // z not used in executable path after this so free it before
            // nx value is changed.
            plFree2dGrid( z, nx, ny );

            nx++;
        }
        else if ( wrap == 2 )
        {
            plAlloc2dGrid( &cgrid2.xg, nx, ny + 1 );
            plAlloc2dGrid( &cgrid2.yg, nx, ny + 1 );
            plAlloc2dGrid( &zwrapped, nx, ny + 1 );
            cgrid2.nx = nx;
            cgrid2.ny = ny + 1;
            zused     = zwrapped;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                    zwrapped[i][j]  = z[i][j];
                }
            }

            for ( i = 0; i < nx; i++ )
            {
                cgrid2.xg[i][ny] = cgrid2.xg[i][0];
                cgrid2.yg[i][ny] = cgrid2.yg[i][0];
                zwrapped[i][ny]  = zwrapped[i][0];
            }

            // z not used in executable path after this so free it before
            // ny value is changed.
            plFree2dGrid( z, nx, ny );

            ny++;
        }
        else
        {
            Tcl_SetResult( interp, "Invalid wrap specifier, must be <empty>, 0, 1, or 2.", TCL_STATIC );
            return TCL_ERROR;
        }

        pltr      = pltr2;
        pltr_data = &cgrid2;
    }
    else
    {
        Tcl_AppendResult( interp,
            "Unrecognized coordinate transformation spec:",
            pltrname, ", must be NULL, pltr0, pltr1, or pltr2.",
            (char *) NULL );
        return TCL_ERROR;
    }

// Now go make the plot.

    plshade( (const PLFLT * const *) zused, nx, ny, NULL,
        xmin, xmax, ymin, ymax,
        sh_min, sh_max, sh_cmap, sh_col, sh_wid,
        min_col, min_wid, max_col, max_wid,
        plfill, rect, pltr, pltr_data );

// Now free up any space which got allocated for our coordinate trickery.

// zused points to either z or zwrapped.  In both cases the allocated size
// was nx by ny.  Now free the allocated space, and note in the case
// where zused points to zwrapped, the separate z space has been freed by
// previous wrap logic.
    plFree2dGrid( zused, nx, ny );

    if ( pltr == pltr1 )
    {
        // Hmm, actually, nothing to do here currently, since we just used the
        // Tcl Matrix data directly, rather than allocating private space.
    }
    else if ( pltr == pltr2 )
    {
        // printf( "plshade, freeing space for grids used in pltr2\n" );
        plFree2dGrid( cgrid2.xg, nx, ny );
        plFree2dGrid( cgrid2.yg, nx, ny );
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plshadesCmd
//
// Processes plshades Tcl command.
// C version takes:
//    data, nx, ny, defined,
//    xmin, xmax, ymin, ymax,
//    clevel, nlevel, fill_width, cont_color, cont_width,
//    plfill, rect, pltr, pltr_data
//
// We will be getting data through a 2-d Matrix, which carries along
// nx and ny, so no need for those.  Toss defined since it's not supported
// anyway.  clevel will be via a 1-d matrix, which carries along nlevel, so
// no need for that.  Toss plfill since it is the only valid choice.
// Take an optional pltr spec just as for plcont or an alternative of
// NULL pltr, and add a wrapping specifier, as in plcont.
// So the new command looks like:
//
// *INDENT-OFF*
//      plshades z xmin xmax ymin ymax
//          clevel, fill_width, cont_color, cont_width
//          rect [[pltr x y] | NULL] [wrap]
// *INDENT-ON*
//--------------------------------------------------------------------------

static int
plshadesCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
             int argc, const char *argv[] )
{
    tclMatrix  *matPtr, *matz, *mattrx = NULL, *mattry = NULL;
    tclMatrix  *matclevel = NULL;
    PLFLT      **z, **zused, **zwrapped;
    PLFLT      xmin, xmax, ymin, ymax;
    PLINT      cont_color = 0;
    PLFLT      fill_width = 0., cont_width = 0.;
    PLINT      rect       = 1;
    const char *pltrname  = "pltr0";
    void       ( *pltr )( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer );
    PLPointer  pltr_data = NULL;
    PLcGrid    cgrid1;
    PLcGrid2   cgrid2;
    PLINT      wrap = 0;
    int        nx, ny, nlevel, i, j;

    if ( argc < 11 )
    {
        Tcl_AppendResult( interp, "bogus syntax for plshades, see doc.",
            (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( matz, interp, argv[1] );
    if ( matz->dim != 2 )
    {
        Tcl_SetResult( interp, "Must plot a 2-d matrix.", TCL_STATIC );
        return TCL_ERROR;
    }

    nx = matz->n[0];
    ny = matz->n[1];

    tclmateval_modx = nx;
    tclmateval_mody = ny;

    // convert matz to 2d-array so can use standard wrap approach
    // from now on in this code.
    plAlloc2dGrid( &z, nx, ny );
    for ( i = 0; i < nx; i++ )
    {
        for ( j = 0; j < ny; j++ )
        {
            z[i][j] = tclMatrix_feval( i, j, matz );
        }
    }

    xmin = atof( argv[2] );
    xmax = atof( argv[3] );
    ymin = atof( argv[4] );
    ymax = atof( argv[5] );

    CHECK_Tcl_GetMatrixPtr( matclevel, interp, argv[6] );
    nlevel = matclevel->n[0];
    if ( matclevel->dim != 1 )
    {
        Tcl_SetResult( interp, "clevel must be 1-d matrix.", TCL_STATIC );
        return TCL_ERROR;
    }

    fill_width = atof( argv[7] );
    cont_color = atoi( argv[8] );
    cont_width = atof( argv[9] );
    rect       = atoi( argv[10] );

    argc -= 11, argv += 11;

    if ( argc >= 3 )
    {
        pltrname = argv[0];
        CHECK_Tcl_GetMatrixPtr( mattrx, interp, argv[1] );
        CHECK_Tcl_GetMatrixPtr( mattry, interp, argv[2] );

        argc -= 3, argv += 3;
    }
    else if ( argc && !strcmp( argv[0], "NULL" ) )
    {
        pltrname = argv[0];
        argc    -= 1, argv += 1;
    }

    if ( argc )
    {
        wrap = atoi( argv[0] );
        argc--, argv++;
    }

    if ( argc )
    {
        Tcl_SetResult( interp, "plshades: bogus arg list", TCL_STATIC );
        return TCL_ERROR;
    }

// Figure out which coordinate transformation model is being used, and setup
// accordingly.

    if ( !strcmp( pltrname, "NULL" ) )
    {
        pltr  = NULL;
        zused = z;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }
    }
    else if ( !strcmp( pltrname, "pltr0" ) )
    {
        pltr  = pltr0;
        zused = z;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }
    }
    else if ( !strcmp( pltrname, "pltr1" ) )
    {
        pltr      = pltr1;
        cgrid1.xg = mattrx->fdata;
        cgrid1.nx = nx;
        cgrid1.yg = mattry->fdata;
        cgrid1.ny = ny;
        zused     = z;

        // wrapping is only supported for pltr2.
        if ( wrap )
        {
            Tcl_SetResult( interp, "Must use pltr2 if want wrapping.", TCL_STATIC );
            return TCL_ERROR;
        }

        if ( mattrx->dim != 1 || mattry->dim != 1 )
        {
            Tcl_SetResult( interp, "Must use 1-d coord arrays with pltr1.", TCL_STATIC );
            return TCL_ERROR;
        }

        pltr_data = &cgrid1;
    }
    else if ( !strcmp( pltrname, "pltr2" ) )
    {
        // printf( "plshades, setting up for pltr2\n" );
        if ( !wrap )
        {
            // printf( "plshades, no wrapping is needed.\n" );
            plAlloc2dGrid( &cgrid2.xg, nx, ny );
            plAlloc2dGrid( &cgrid2.yg, nx, ny );
            cgrid2.nx = nx;
            cgrid2.ny = ny;
            zused     = z;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
        }
        else if ( wrap == 1 )
        {
            plAlloc2dGrid( &cgrid2.xg, nx + 1, ny );
            plAlloc2dGrid( &cgrid2.yg, nx + 1, ny );
            plAlloc2dGrid( &zwrapped, nx + 1, ny );
            cgrid2.nx = nx + 1;
            cgrid2.ny = ny;
            zused     = zwrapped;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                    zwrapped[i][j]  = z[i][j];
                }
            }

            for ( j = 0; j < ny; j++ )
            {
                cgrid2.xg[nx][j] = cgrid2.xg[0][j];
                cgrid2.yg[nx][j] = cgrid2.yg[0][j];
                zwrapped[nx][j]  = zwrapped[0][j];
            }

            // z not used in executable path after this so free it before
            // nx value is changed.
            plFree2dGrid( z, nx, ny );

            nx++;
        }
        else if ( wrap == 2 )
        {
            plAlloc2dGrid( &cgrid2.xg, nx, ny + 1 );
            plAlloc2dGrid( &cgrid2.yg, nx, ny + 1 );
            plAlloc2dGrid( &zwrapped, nx, ny + 1 );
            cgrid2.nx = nx;
            cgrid2.ny = ny + 1;
            zused     = zwrapped;

            matPtr = mattrx;
            for ( i = 0; i < nx; i++ )
                for ( j = 0; j < ny; j++ )
                    cgrid2.xg[i][j] = mattrx->fdata[ I2D( i, j ) ];

            matPtr = mattry;
            for ( i = 0; i < nx; i++ )
            {
                for ( j = 0; j < ny; j++ )
                {
                    cgrid2.yg[i][j] = mattry->fdata[ I2D( i, j ) ];
                    zwrapped[i][j]  = z[i][j];
                }
            }

            for ( i = 0; i < nx; i++ )
            {
                cgrid2.xg[i][ny] = cgrid2.xg[i][0];
                cgrid2.yg[i][ny] = cgrid2.yg[i][0];
                zwrapped[i][ny]  = zwrapped[i][0];
            }

            // z not used in executable path after this so free it before
            // ny value is changed.
            plFree2dGrid( z, nx, ny );

            ny++;
        }
        else
        {
            Tcl_SetResult( interp, "Invalid wrap specifier, must be <empty>, 0, 1, or 2.", TCL_STATIC );
            return TCL_ERROR;
        }

        pltr      = pltr2;
        pltr_data = &cgrid2;
    }
    else
    {
        Tcl_AppendResult( interp,
            "Unrecognized coordinate transformation spec:",
            pltrname, ", must be NULL, pltr0, pltr1, or pltr2.",
            (char *) NULL );
        return TCL_ERROR;
    }

// Now go make the plot.

    plshades( (const PLFLT * const *) zused, nx, ny, NULL,
        xmin, xmax, ymin, ymax,
        matclevel->fdata, nlevel, fill_width, cont_color, cont_width,
        plfill, rect, pltr, pltr_data );

// Now free up any space which got allocated for our coordinate trickery.

// zused points to either z or zwrapped.  In both cases the allocated size
// was nx by ny.  Now free the allocated space, and note in the case
// where zused points to zwrapped, the separate z space has been freed by
// previous wrap logic.
    plFree2dGrid( zused, nx, ny );

    if ( pltr == pltr1 )
    {
        // Hmm, actually, nothing to do here currently, since we just used the
        // Tcl Matrix data directly, rather than allocating private space.
    }
    else if ( pltr == pltr2 )
    {
        // printf( "plshades, freeing space for grids used in pltr2\n" );
        plFree2dGrid( cgrid2.xg, nx, ny );
        plFree2dGrid( cgrid2.yg, nx, ny );
    }

    plflush();
    return TCL_OK;
}

//--------------------------------------------------------------------------
// mapform
//
// Defines our coordinate transformation.
// x[], y[] are the coordinates to be plotted.
//--------------------------------------------------------------------------

static const char *transform_name; // Name of the procedure that transforms the
                                   // coordinates
static Tcl_Interp *tcl_interp;     // Pointer to the current interp
static int        return_code;     // Saved return code

void
mapform( PLINT n, PLFLT *x, PLFLT *y )
{
    int       i;
    char      *cmd;
    tclMatrix *xPtr, *yPtr;

    cmd = (char *) malloc( strlen( transform_name ) + 40 );

    // Build the (new) matrix commands and fill the matrices
    sprintf( cmd, "matrix %cx f %d", (char) 1, n );
    if ( Tcl_Eval( tcl_interp, cmd ) != TCL_OK )
    {
        return_code = TCL_ERROR;
        free( cmd );
        return;
    }
    sprintf( cmd, "matrix %cy f %d", (char) 1, n );
    if ( Tcl_Eval( tcl_interp, cmd ) != TCL_OK )
    {
        return_code = TCL_ERROR;
        free( cmd );
        return;
    }

    sprintf( cmd, "%cx", (char) 1 );
    xPtr = Tcl_GetMatrixPtr( tcl_interp, cmd );
    if ( xPtr == NULL )
    {
        return_code = TCL_ERROR;
        free( cmd );
        return;
    }

    sprintf( cmd, "%cy", (char) 1 );
    yPtr = Tcl_GetMatrixPtr( tcl_interp, cmd );
    if ( yPtr == NULL )
    {
        return_code = TCL_ERROR;
        free( cmd );
        return;
    }

    for ( i = 0; i < n; i++ )
    {
        xPtr->fdata[i] = x[i];
        yPtr->fdata[i] = y[i];
    }

    // Now call the Tcl procedure to do the work
    sprintf( cmd, "%s %d %cx %cy", transform_name, n, (char) 1, (char) 1 );
    return_code = Tcl_Eval( tcl_interp, cmd );
    if ( return_code != TCL_OK )
    {
        free( cmd );
        return;
    }

    // Don't forget to copy the results back into the original arrays
    //
    for ( i = 0; i < n; i++ )
    {
        x[i] = xPtr->fdata[i];
        y[i] = yPtr->fdata[i];
    }

    // Clean up, otherwise the next call will fail - [matrix] does not
    // overwrite existing commands
    //
    sprintf( cmd, "rename %cx {}; rename %cy {}", (char) 1, (char) 1 );
    return_code = Tcl_Eval( tcl_interp, cmd );

    free( cmd );
}

//--------------------------------------------------------------------------
// plmapCmd
//
// Processes plmap Tcl command.
// C version takes:
//    string, minlong, maxlong, minlat, maxlat
//
//  e.g. .p cmd plmap globe 0 360 -90 90
//--------------------------------------------------------------------------

static int
plmapCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
          int argc, const char *argv[] )
{
    PLFLT minlong, maxlong, minlat, maxlat;
    PLINT transform;
    PLINT idxname;

    return_code = TCL_OK;
    if ( argc == 6 )
    {
        transform      = 0;
        transform_name = NULL;
        idxname        = 1;
    }
    else if ( argc == 7 )
    {
        transform      = 1;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }
        idxname = 2;

        tcl_interp = interp;
    }
    else
    {
        return_code = TCL_ERROR;
    }

    if ( return_code == TCL_ERROR )
    {
        Tcl_AppendResult( interp, "bogus syntax for plmap, see doc.",
            (char *) NULL );
    }
    else
    {
        minlong = atof( argv[idxname + 1] );
        maxlong = atof( argv[idxname + 2] );
        minlat  = atof( argv[idxname + 3] );
        maxlat  = atof( argv[idxname + 4] );
        if ( transform && idxname == 2 )
        {
            plmap( &mapform, argv[idxname], minlong, maxlong, minlat, maxlat );
        }
        else
        {
            // No transformation given
            plmap( NULL, argv[idxname], minlong, maxlong, minlat, maxlat );
        }

        plflush();
    }

    return return_code;
}

//--------------------------------------------------------------------------
// GetEntries
//
// Return the list of plot entries (either from a list of from a matrix)
//--------------------------------------------------------------------------

static int *
GetEntries( Tcl_Interp *interp, const char *string, int *n )
{
    tclMatrix *mati;
    int       argc;
    // NULL returned on all failures.
    int       *entries = NULL;
    char      **argv;
    int       i;

    mati = Tcl_GetMatrixPtr( interp, string );
    if ( mati == NULL )
    {
        if ( Tcl_SplitList( interp, string, n, (const char ***) &argv ) == TCL_OK )
        {
            entries = (int *) malloc( ( *n ) * sizeof ( int ) );
            for ( i = 0; i < *n; i++ )
            {
                entries[i] = atoi( argv[i] );
            }
            Tcl_Free( (char *) argv );
        }
    }
    else
    {
        *n      = mati->n[0];
        entries = (int *) malloc( ( *n ) * sizeof ( int ) );
        for ( i = 0; i < *n; i++ )
        {
            entries[i] = mati->idata[i];
        }
    }

    return entries;
}

//--------------------------------------------------------------------------
// plmapfillCmd
//
// Processes plmapfill Tcl command.
// C version takes:
//    transform_proc, string, minlong, maxlong, minlat, maxlat, entries, nentries
//
//  e.g. .p cmd plmapfill globe 0 360 -90 90
//--------------------------------------------------------------------------

static int
plmapfillCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
              int argc, const char *argv[] )
{
    PLFLT  minlong, maxlong, minlat, maxlat;
    PLINT  transform;
    PLINT  idxname;
    PLINT  *entries;
    PLINT  nentries;
    double dminlong;

    return_code = TCL_OK;

    nentries = 0;
    entries  = NULL;

    switch ( argc )
    {
    case 6:     // No transform, no plotentries
        transform      = 0;
        idxname        = 1;
        transform_name = NULL;
        break;

    case 7:     // Transform OR plotentries, not both - ambiguity
                // Transformation name is either a name or empty
                // string or missing. So the argument pattern is
                // either one or two non-numeric strings, then a
                // numeric string.  In the former case all argument
                // indices are offset by one and a list (not a matrix)
                // of plotentries is given as the last argument.

        transform = 1;
        idxname   = 2;

        tcl_interp     = interp;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }
        else
        {
            if ( Tcl_GetDouble( interp, argv[2], &dminlong ) == TCL_OK )
            {
                transform = 0;
                idxname   = 1;
                entries   = GetEntries( interp, argv[6], &nentries );
                if ( !entries )
                    return_code = TCL_ERROR;
            }
        }
        break;

    case 8:     // Transform, plotentries
        transform      = 1;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }

        idxname = 2;

        entries = GetEntries( interp, argv[7], &nentries );
        if ( !entries )
            return_code = TCL_ERROR;
        tcl_interp = interp;
        break;
    default:
        return_code = TCL_ERROR;
    }

    if ( return_code == TCL_ERROR )
    {
        Tcl_AppendResult( interp, "bogus syntax for plmapfill, see doc.",
            (char *) NULL );
    }
    else
    {
        minlong = atof( argv[idxname + 1] );
        maxlong = atof( argv[idxname + 2] );
        minlat  = atof( argv[idxname + 3] );
        maxlat  = atof( argv[idxname + 4] );
        if ( transform && idxname == 2 )
        {
            plmapfill( &mapform, argv[idxname], minlong, maxlong, minlat, maxlat, entries, nentries );
        }
        else
        {
            // No transformation given
            plmapfill( NULL, argv[idxname], minlong, maxlong, minlat, maxlat, entries, nentries );
        }

        free( entries );

        plflush();
    }

    return return_code;
}

//--------------------------------------------------------------------------
// plmaplineCmd
//
// Processes plmapline Tcl command.
// C version takes:
//    transform_proc, string, minlong, maxlong, minlat, maxlat, entries, nentries
//
//  e.g. .p cmd plmapline globe 0 360 -90 90
//--------------------------------------------------------------------------

static int
plmaplineCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
              int argc, const char *argv[] )
{
    PLFLT  minlong, maxlong, minlat, maxlat;
    PLINT  transform;
    PLINT  idxname;
    PLINT  *entries;
    PLINT  nentries;
    double dminlong;

    return_code = TCL_OK;

    nentries = 0;
    entries  = NULL;

    //fprintf(stderr, "plmapline: %d\n", argc);
    switch ( argc )
    {
    case 6:     // No transform, no plotentries
        transform      = 0;
        transform_name = NULL;
        idxname        = 1;
        break;

    case 7:     // Transform OR plotentries, not both - ambiguity
                // Transformation name is either a name or empty
                // string or missing. So the argument pattern is
                // either one or two non-numeric strings, then a
                // numeric string.  In the former case all argument
                // indices are offset by one and a list (not a matrix)
                // of plotentries is given as the last argument.

        transform = 1;
        idxname   = 2;

        tcl_interp     = interp;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }
        else
        {
            if ( Tcl_GetDouble( interp, argv[2], &dminlong ) == TCL_OK )
            {
                transform = 0;
                idxname   = 1;
                entries   = GetEntries( interp, argv[6], &nentries );
                if ( !entries )
                    return_code = TCL_ERROR;
            }
        }
        break;

    case 8:     // Transform, plotentries
        transform      = 1;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }

        idxname = 2;

        tcl_interp = interp;
        entries    = GetEntries( interp, argv[7], &nentries );
        //fprintf(stderr, "plmapline: number entries %d\n", nentries);
        if ( !entries )
            return_code = TCL_ERROR;
        break;

    default:
        return_code = TCL_ERROR;
    }

    if ( return_code == TCL_ERROR )
    {
        Tcl_AppendResult( interp, "bogus syntax for plmapline, see doc.",
            (char *) NULL );
    }
    else
    {
        minlong = atof( argv[idxname + 1] );
        maxlong = atof( argv[idxname + 2] );
        minlat  = atof( argv[idxname + 3] );
        maxlat  = atof( argv[idxname + 4] );
        if ( transform && idxname == 2 )
        {
            plmapline( &mapform, argv[idxname], minlong, maxlong, minlat, maxlat, entries, nentries );
        }
        else
        {
            // No transformation given
            plmapline( NULL, argv[idxname], minlong, maxlong, minlat, maxlat, entries, nentries );
        }

        free( entries );

        plflush();
    }

    return return_code;
}

//--------------------------------------------------------------------------
// plmapstringCmd
//
// Processes plmapstring Tcl command.
// C version takes:
//    transform_proc, string, minlong, maxlong, minlat, maxlat, entries, nentries
//
//  e.g. .p cmd plmapstring globe "Town" 0 360 -90 90
//--------------------------------------------------------------------------

static int
plmapstringCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
                int argc, const char *argv[] )
{
    PLFLT      minlong, maxlong, minlat, maxlat;
    PLINT      transform;
    PLINT      idxname;
    PLINT      *entries;
    PLINT      nentries;
    const char *string;
    double     dminlong;

    return_code = TCL_OK;
    if ( argc < 7 || argc > 9 )
    {
        Tcl_AppendResult( interp, "bogus syntax for plmapstring, see doc.",
            (char *) NULL );
        return TCL_ERROR;
    }

    nentries = 0;
    entries  = NULL;

    switch ( argc )
    {
    case 7:     // No transform, no plotentries
        transform      = 0;
        idxname        = 1;
        transform_name = NULL;
        break;

    case 8:     // Transform OR plotentries, not both - ambiguity
                // Transformation name is either a name or empty
                // string or missing. So the argument pattern is
                // either one or two non-numeric strings, then a
                // numeric string.  In the former case all argument
                // indices are offset by one and a list (not a matrix)
                // of plotentries is given as the last argument.

        transform = 1;
        idxname   = 2;

        tcl_interp     = interp;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }
        else
        {
            if ( Tcl_GetDouble( interp, argv[3], &dminlong ) == TCL_OK )
            {
                transform = 0;
                idxname   = 1;
                entries   = GetEntries( interp, argv[7], &nentries );
                if ( !entries )
                    return_code = TCL_ERROR;
            }
        }
        break;

    case 9:     // Transform, plotentries
        transform      = 1;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }

        idxname = 2;

        tcl_interp = interp;
        entries    = GetEntries( interp, argv[8], &nentries );
        if ( !entries )
            return_code = TCL_ERROR;
        break;
    default:
        return_code = TCL_ERROR;
    }

    string  = argv[idxname + 1];
    minlong = atof( argv[idxname + 2] );
    maxlong = atof( argv[idxname + 3] );
    minlat  = atof( argv[idxname + 4] );
    maxlat  = atof( argv[idxname + 5] );
    if ( entries != NULL )
    {
        if ( transform && idxname == 2 )
        {
            plmapstring( &mapform, argv[idxname], string, minlong, maxlong, minlat, maxlat, entries, nentries );
        }
        else
        {
            // No transformation given
            plmapstring( NULL, argv[idxname], string, minlong, maxlong, minlat, maxlat, entries, nentries );
        }

        free( entries );
    }

    plflush();
    return return_code;
}

//--------------------------------------------------------------------------
// plmaptexCmd
//
// Processes plmaptex Tcl command.
// C version takes:
//    transform_proc, string, minlong, maxlong, minlat, maxlat, plotentry
//
//  e.g. .p cmd plmaptex globe "Town" 0 360 -90 90
//--------------------------------------------------------------------------

static int
plmaptexCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
             int argc, const char *argv[] )
{
    PLFLT      minlong, maxlong, minlat, maxlat;
    PLFLT      dx, dy, just;
    PLINT      transform;
    PLINT      idxname;
    PLINT      plotentry;
    const char *text;
    double     dminlong;

    return_code = TCL_OK;
    // N.B. plotentries is always required for the plmaptex case so no ambiguity below.
    switch ( argc )
    {
    case 11:     // No transformation.

        // For this case, argv[2] must be translatable into a double-precision number.
        if ( Tcl_GetDouble( interp, argv[2], &dminlong ) == TCL_OK )
        {
            transform = 0;
            idxname   = 1;
        }
        else
            return_code = TCL_ERROR;
        break;

    case 12:     // Transform
        transform      = 1;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }
        idxname = 2;
        break;
    default:
        return_code = TCL_ERROR;
    }

    if ( return_code == TCL_ERROR )
    {
        Tcl_AppendResult( interp, "bogus syntax for plmaptex, see doc.",
            (char *) NULL );
    }
    else
    {
        dx        = atof( argv[idxname + 1] );
        dy        = atof( argv[idxname + 2] );
        just      = atof( argv[idxname + 3] );
        text      = argv[idxname + 4];
        minlong   = atof( argv[idxname + 5] );
        maxlong   = atof( argv[idxname + 6] );
        minlat    = atof( argv[idxname + 7] );
        maxlat    = atof( argv[idxname + 8] );
        plotentry = atoi( argv[idxname + 9] );
        if ( transform && idxname == 2 )
        {
            plmaptex( &mapform, argv[idxname], dx, dy, just, text, minlong, maxlong, minlat, maxlat, plotentry );
        }
        else
        {
            // No transformation given
            plmaptex( NULL, argv[idxname], dx, dy, just, text, minlong, maxlong, minlat, maxlat, plotentry );
        }

        plflush();
    }

    return return_code;
}

//--------------------------------------------------------------------------
// plmeridiansCmd
//
// Processes plmeridians Tcl command.
// C version takes:
//    dlong, dlat, minlong, maxlong, minlat, maxlat
//
//  e.g. .p cmd plmeridians 1 ...
//--------------------------------------------------------------------------

static int
plmeridiansCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
                int argc, const char *argv[] )
{
    PLFLT dlong, dlat, minlong, maxlong, minlat, maxlat;
    PLINT transform;

    return_code = TCL_OK;

    if ( argc < 7 || argc > 8 )
    {
        Tcl_AppendResult( interp, "bogus syntax for plmeridians, see doc.",
            (char *) NULL );
        return TCL_ERROR;
    }

    if ( argc == 7 )
    {
        transform      = 0;
        transform_name = NULL;
        dlong          = atof( argv[1] );
        dlat           = atof( argv[2] );
        minlong        = atof( argv[3] );
        maxlong        = atof( argv[4] );
        minlat         = atof( argv[5] );
        maxlat         = atof( argv[6] );
    }
    else
    {
        dlong   = atof( argv[2] );
        dlat    = atof( argv[3] );
        minlong = atof( argv[4] );
        maxlong = atof( argv[5] );
        minlat  = atof( argv[6] );
        maxlat  = atof( argv[7] );

        transform      = 1;
        tcl_interp     = interp;
        transform_name = argv[1];
        if ( strlen( transform_name ) == 0 )
        {
            transform = 0;
        }
    }

    if ( transform )
    {
        plmeridians( &mapform, dlong, dlat, minlong, maxlong, minlat, maxlat );
    }
    else
    {
        plmeridians( NULL, dlong, dlat, minlong, maxlong, minlat, maxlat );
    }

    plflush();
    return TCL_OK;
}

static Tcl_Interp *tcl_xform_interp   = 0;
static char       *tcl_xform_procname = 0;
static const char *tcl_xform_template =
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION < 5
    "set result [%s ${_##_x} ${_##_y}] ; set _##_x [lindex $result 0] ; set _##_y [lindex $result 1]"
#else
    "set result [%s ${_##_x} ${_##_y}] ; lassign $result _##_x _##_y"
#endif
;

static char *tcl_xform_code = 0;

static void
Tcl_transform( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer PL_UNUSED( data ) )
{
    Tcl_Obj *objx, *objy;
    int     code;
    double  dx, dy;

// Set Tcl x to x
    objx = Tcl_NewDoubleObj( (double) x );
    Tcl_IncrRefCount( objx );
    Tcl_SetVar2Ex( tcl_xform_interp,
        "_##_x", NULL, objx, 0 );
    Tcl_DecrRefCount( objx );

// Set Tcl y to y
    objy = Tcl_NewDoubleObj( (double) y );
    Tcl_IncrRefCount( objy );
    Tcl_SetVar2Ex( tcl_xform_interp,
        "_##_y", NULL, objy, 0 );
    Tcl_DecrRefCount( objy );

//     printf( "objx=%x objy=%x\n", objx, objy );

//     printf( "Evaluating code: %s\n", tcl_xform_code );

// Call identified Tcl proc.  Forget data, Tcl can use namespaces and custom
// procs to manage transmission of the custom client data.
// Proc should return a two element list which is xt yt.
    code = Tcl_Eval( tcl_xform_interp, tcl_xform_code );

    if ( code != TCL_OK )
    {
        printf( "Unable to evaluate Tcl-side coordinate transform.\n" );
        printf( "code = %d\n", code );
        printf( "Error result: %s\n", Tcl_GetStringResult( tcl_xform_interp ) );
        return;
    }

    objx = Tcl_GetVar2Ex( tcl_xform_interp, "_##_x", NULL, 0 );
    objy = Tcl_GetVar2Ex( tcl_xform_interp, "_##_y", NULL, 0 );

// In case PLFLT != double, we have to make sure we perform the extraction in
// a safe manner.
    if ( Tcl_GetDoubleFromObj( tcl_xform_interp, objx, &dx ) != TCL_OK ||
         Tcl_GetDoubleFromObj( tcl_xform_interp, objy, &dy ) != TCL_OK )
    {
        printf( "Unable to extract Tcl results.\n" );
        return;
    }

    *xt = dx;
    *yt = dy;
}

//--------------------------------------------------------------------------
// plstransform
//
// Implement Tcl-side global coordinate transformation setting/restoring API.
//--------------------------------------------------------------------------

static int
plstransformCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
                 int argc, const char *argv[] )
{
    if ( argc == 1
         || strcmp( argv[1], "NULL" ) == 0 )
    {
        // The user has requested to clear the transform setting.
        plstransform( NULL, NULL );
        tcl_xform_interp = 0;
        if ( tcl_xform_procname )
        {
            free( tcl_xform_procname );
            tcl_xform_procname = 0;
        }
    }
    else
    {
        size_t len;

        tcl_xform_interp   = interp;
        tcl_xform_procname = plstrdup( argv[1] );

        len            = strlen( tcl_xform_template ) + strlen( tcl_xform_procname );
        tcl_xform_code = malloc( len );
        sprintf( tcl_xform_code, tcl_xform_template, tcl_xform_procname );

        plstransform( Tcl_transform, NULL );
    }

    return TCL_OK;
}

//--------------------------------------------------------------------------
// plgriddataCmd
//
// Processes plgriddata Tcl command.
//--------------------------------------------------------------------------
static int
plgriddataCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
               int argc, const char *argv[] )
{
    tclMatrix *arrx, *arry, *arrz, *xcoord, *ycoord, *zvalue;
    PLINT     pts, nx, ny, alg;
    PLFLT     optalg;
    PLFLT     **z;

    double    value;
    int       i, j;

    if ( argc != 9 )
    {
        Tcl_AppendResult( interp, "wrong # args: see documentation for ",
            argv[0], (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( arrx, interp, argv[1] );
    CHECK_Tcl_GetMatrixPtr( arry, interp, argv[2] );
    CHECK_Tcl_GetMatrixPtr( arrz, interp, argv[3] );
    CHECK_Tcl_GetMatrixPtr( xcoord, interp, argv[4] );
    CHECK_Tcl_GetMatrixPtr( ycoord, interp, argv[5] );
    CHECK_Tcl_GetMatrixPtr( zvalue, interp, argv[6] );
    sscanf( argv[7], "%d", &alg );

    sscanf( argv[8], "%lg", &value ); optalg = (PLFLT) value;

    if ( arrx->dim != 1 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 1 should be a \
one-dimensional matrix - ", argv[1], (char *) NULL );
        return TCL_ERROR;
    }
    if ( arry->dim != 1 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 2 should be a \
one-dimensional matrix - ", argv[2], (char *) NULL );
        return TCL_ERROR;
    }
    if ( arrz->dim != 1 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 3 should be a \
one-dimensional matrix - ", argv[3], (char *) NULL );
        return TCL_ERROR;
    }

    if ( xcoord->dim != 1 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 4 should be a \
one-dimensional matrix - ", argv[4], (char *) NULL );
        return TCL_ERROR;
    }
    if ( ycoord->dim != 1 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 5 should be a \
one-dimensional matrix - ", argv[5], (char *) NULL );
        return TCL_ERROR;
    }
    if ( zvalue->dim != 2 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 6 should be a \
two-dimensional matrix - ", argv[6], (char *) NULL );
        return TCL_ERROR;
    }

    pts = arrx->n[0];
    nx  = zvalue->n[0];
    ny  = zvalue->n[1];

    // convert zvalue to 2d-array so can use standard wrap approach
    // from now on in this code.
    plAlloc2dGrid( &z, nx, ny );

    // Interpolate the data
    plgriddata( arrx->fdata, arry->fdata, arrz->fdata, pts,
        xcoord->fdata, nx, ycoord->fdata, ny, z, alg, optalg );

    // Copy the result into the matrix
    for ( i = 0; i < nx; i++ )
    {
        for ( j = 0; j < ny; j++ )
        {
            zvalue->fdata[j + zvalue->n[1] * i] = z[i][j];
        }
    }

    plFree2dGrid( z, nx, ny );
    return TCL_OK;
}

//--------------------------------------------------------------------------
// plimageCmd
//
// Processes plimage Tcl command.
//--------------------------------------------------------------------------
static int
plimageCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
            int argc, const char *argv[] )
{
    tclMatrix *zvalue;
    PLINT     nx, ny;
    PLFLT     **pidata;
    PLFLT     xmin, xmax, ymin, ymax, zmin, zmax, Dxmin, Dxmax, Dymin, Dymax;

    double    value;
    int       i, j;

    if ( argc != 12 )
    {
        Tcl_AppendResult( interp, "wrong # args: see documentation for ",
            argv[0], (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( zvalue, interp, argv[1] );

    if ( zvalue->dim != 2 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 1 should be a \
two-dimensional matrix - ", argv[1], (char *) NULL );
        return TCL_ERROR;
    }

    sscanf( argv[2], "%lg", &value ); xmin   = (PLFLT) value;
    sscanf( argv[3], "%lg", &value ); xmax   = (PLFLT) value;
    sscanf( argv[4], "%lg", &value ); ymin   = (PLFLT) value;
    sscanf( argv[5], "%lg", &value ); ymax   = (PLFLT) value;
    sscanf( argv[6], "%lg", &value ); zmin   = (PLFLT) value;
    sscanf( argv[7], "%lg", &value ); zmax   = (PLFLT) value;
    sscanf( argv[8], "%lg", &value ); Dxmin  = (PLFLT) value;
    sscanf( argv[9], "%lg", &value ); Dxmax  = (PLFLT) value;
    sscanf( argv[10], "%lg", &value ); Dymin = (PLFLT) value;
    sscanf( argv[11], "%lg", &value ); Dymax = (PLFLT) value;

    nx = zvalue->n[0];
    ny = zvalue->n[1];

    plAlloc2dGrid( &pidata, nx, ny );

    for ( i = 0; i < nx; i++ )
    {
        for ( j = 0; j < ny; j++ )
        {
            pidata[i][j] = zvalue->fdata[j + i * ny];
        }
    }
    //
    // fprintf(stderr,"nx, ny: %d %d\n", nx, ny);
    // fprintf(stderr,"xmin, xmax: %.17g %.17g\n", xmin, xmax);
    // fprintf(stderr,"ymin, ymax: %.17g %.17g\n", ymin, ymax);
    // fprintf(stderr,"zmin, zmax: %.17g %.17g\n", zmin, zmax);
    // fprintf(stderr,"Dxmin, Dxmax: %.17g %.17g\n", Dxmin, Dxmax);
    // fprintf(stderr,"Dymin, Dymax: %.17g %.17g\n", Dymin, Dymax);
    //

    c_plimage( (const PLFLT * const *) pidata, nx, ny, xmin, xmax, ymin, ymax, zmin, zmax,
        Dxmin, Dxmax, Dymin, Dymax );

    plFree2dGrid( pidata, nx, ny );

    return TCL_OK;
}

//--------------------------------------------------------------------------
// plimagefrCmd
//
// Processes plimagefr Tcl command.
//
// Note:
// Very basic! No user-defined interpolation routines
//--------------------------------------------------------------------------
static int
plimagefrCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
              int argc, const char *argv[] )
{
    tclMatrix *zvalue;
    tclMatrix *xg;
    tclMatrix *yg;
    PLINT     nx, ny;
    PLFLT     **pidata;
    PLcGrid2  cgrid2;
    PLFLT     xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax;

    double    value;
    int       i, j;

    if ( argc != 12 && argc != 10 )
    {
        Tcl_AppendResult( interp, "wrong # args: see documentation for ",
            argv[0], (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( zvalue, interp, argv[1] );

    if ( zvalue->dim != 2 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 1 should be a \
two-dimensional matrix - ", argv[1], (char *) NULL );
        return TCL_ERROR;
    }

    xg = NULL;
    yg = NULL;
    if ( argc == 12 )
    {
        CHECK_Tcl_GetMatrixPtr( xg, interp, argv[10] );
        CHECK_Tcl_GetMatrixPtr( yg, interp, argv[11] );

        if ( xg->dim != 2 )
        {
            Tcl_AppendResult( interp, argv[0], ": argument 10 should be a \
two-dimensional matrix - ", argv[10], (char *) NULL );
            return TCL_ERROR;
        }

        if ( yg->dim != 2 )
        {
            Tcl_AppendResult( interp, argv[0], ": argument 11 should be a \
two-dimensional matrix - ", argv[11], (char *) NULL );
            return TCL_ERROR;
        }
    }

    sscanf( argv[2], "%lg", &value ); xmin     = (PLFLT) value;
    sscanf( argv[3], "%lg", &value ); xmax     = (PLFLT) value;
    sscanf( argv[4], "%lg", &value ); ymin     = (PLFLT) value;
    sscanf( argv[5], "%lg", &value ); ymax     = (PLFLT) value;
    sscanf( argv[6], "%lg", &value ); zmin     = (PLFLT) value;
    sscanf( argv[7], "%lg", &value ); zmax     = (PLFLT) value;
    sscanf( argv[8], "%lg", &value ); valuemin = (PLFLT) value;
    sscanf( argv[9], "%lg", &value ); valuemax = (PLFLT) value;

    nx = zvalue->n[0];
    ny = zvalue->n[1];

    plAlloc2dGrid( &pidata, nx, ny );

    for ( i = 0; i < nx; i++ )
    {
        for ( j = 0; j < ny; j++ )
        {
            pidata[i][j] = zvalue->fdata[j + i * ny];
        }
    }

    if ( xg != NULL )
    {
        plAlloc2dGrid( &cgrid2.xg, nx + 1, ny + 1 );
        plAlloc2dGrid( &cgrid2.yg, nx + 1, ny + 1 );

        cgrid2.nx = nx + 1;
        cgrid2.ny = ny + 1;
        for ( i = 0; i <= nx; i++ )
        {
            for ( j = 0; j <= ny; j++ )
            {
                cgrid2.xg[i][j] = xg->fdata[j + i * ( ny + 1 )];
                cgrid2.yg[i][j] = yg->fdata[j + i * ( ny + 1 )];
            }
        }
        c_plimagefr( (const PLFLT * const *) pidata, nx, ny, xmin, xmax, ymin, ymax, zmin, zmax,
            valuemin, valuemax, pltr2, (void *) &cgrid2 );
    }
    else
    {
        c_plimagefr( (const PLFLT * const *) pidata, nx, ny, xmin, xmax, ymin, ymax, zmin, zmax,
            valuemin, valuemax, NULL, NULL );
    }

    plFree2dGrid( pidata, nx, ny );
    if ( xg != NULL )
    {
        plFree2dGrid( cgrid2.xg, nx + 1, ny + 1 );
        plFree2dGrid( cgrid2.yg, nx + 1, ny + 1 );
    }

    return TCL_OK;
}

//--------------------------------------------------------------------------
// plstripcCmd
//
// Processes plstripc Tcl command.
//--------------------------------------------------------------------------
static int
plstripcCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
             int argc, const char *argv[] )
{
    int        i;
    int        id;
    const char *xspec;
    const char *yspec;
    const char *idName;
    tclMatrix  *colMat;
    tclMatrix  *styleMat;
    double     value;
    int        ivalue;
    PLFLT      xmin, xmax, xjump, ymin, ymax, xlpos, ylpos;
    PLBOOL     y_ascl, acc;
    PLINT      colbox, collab;
    PLINT      colline[4], styline[4];
    int        nlegend;
    const char **legline;
    const char *labx;
    const char *laby;
    const char *labtop;
    char       idvalue[20];

    if ( argc != 21 )
    {
        Tcl_AppendResult( interp, "wrong # args: see documentation for ",
            argv[0], (char *) NULL );
        return TCL_ERROR;
    }

    CHECK_Tcl_GetMatrixPtr( colMat, interp, argv[15] );
    CHECK_Tcl_GetMatrixPtr( styleMat, interp, argv[16] );

    if ( colMat->dim != 1 || colMat->idata == NULL )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 15 should be a \
one-dimensional integer matrix - ", argv[15], (char *) NULL );
        return TCL_ERROR;
    }

    if ( styleMat->dim != 1 || styleMat->idata == NULL )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 16 should be a \
one-dimensional integer matrix - ", argv[16], (char *) NULL );
        return TCL_ERROR;
    }

    idName = argv[1];
    xspec  = argv[2];
    yspec  = argv[3];

    sscanf( argv[4], "%lg", &value ); xmin    = (PLFLT) value;
    sscanf( argv[5], "%lg", &value ); xmax    = (PLFLT) value;
    sscanf( argv[6], "%lg", &value ); xjump   = (PLFLT) value;
    sscanf( argv[7], "%lg", &value ); ymin    = (PLFLT) value;
    sscanf( argv[8], "%lg", &value ); ymax    = (PLFLT) value;
    sscanf( argv[9], "%lg", &value ); xlpos   = (PLFLT) value;
    sscanf( argv[10], "%lg", &value ); ylpos  = (PLFLT) value;
    sscanf( argv[11], "%d", &ivalue ); y_ascl = (PLBOOL) ivalue;
    sscanf( argv[12], "%d", &ivalue ); acc    = (PLBOOL) ivalue;
    sscanf( argv[13], "%d", &ivalue ); colbox = ivalue;
    sscanf( argv[14], "%d", &ivalue ); collab = ivalue;

    labx   = argv[18];
    laby   = argv[19];
    labtop = argv[20];

    for ( i = 0; i < 4; i++ )
    {
        colline[i] = colMat->idata[i];
        styline[i] = styleMat->idata[i];
    }

    if ( Tcl_SplitList( interp, argv[17], &nlegend, &legline ) != TCL_OK )
    {
        return TCL_ERROR;
    }
    if ( nlegend < 4 )
    {
        Tcl_AppendResult( interp, argv[0], ": argument 18 should be a \
list of at least four items - ", argv[17], (char *) NULL );
        return TCL_ERROR;
    }

    c_plstripc( &id, xspec, yspec,
        xmin, xmax, xjump, ymin, ymax,
        xlpos, ylpos,
        y_ascl, acc,
        colbox, collab,
        colline, styline, legline,
        labx, laby, labtop );

    sprintf( idvalue, "%d", id );
    Tcl_SetVar( interp, idName, idvalue, 0 );

    Tcl_Free( (char *) legline );

    return TCL_OK;
}

//--------------------------------------------------------------------------
// labelform
//
// Call the Tcl custom label function.
//--------------------------------------------------------------------------

static Tcl_Obj *label_objs[4] = { NULL, NULL, NULL, NULL };   // Arguments for the Tcl procedure
                                                              // that handles the custom labels

void
labelform( PLINT axis, PLFLT value, char *string, PLINT string_length, PLPointer PL_UNUSED( data ) )
{
    int objc;

    label_objs[1] = Tcl_NewIntObj( axis );
    label_objs[2] = Tcl_NewDoubleObj( (double) value );

    Tcl_IncrRefCount( label_objs[1] );
    Tcl_IncrRefCount( label_objs[2] );

    // Call the Tcl procedure and store the result
    objc = 3;
    if ( label_objs[3] != NULL )
    {
        objc = 4;
    }

    return_code = Tcl_EvalObjv( tcl_interp, objc, label_objs, 0 );

    if ( return_code != TCL_OK )
    {
        strncpy( string, "ERROR", (size_t) string_length );
    }
    else
    {
        strncpy( string, Tcl_GetStringResult( tcl_interp ), (size_t) string_length );
    }

    Tcl_DecrRefCount( label_objs[1] );
    Tcl_DecrRefCount( label_objs[2] );
}

//--------------------------------------------------------------------------
// plslabelfuncCmd
//
// Processes plslabelfunc Tcl command.
// C version takes:
//    function, data
// (data argument is optional)
//--------------------------------------------------------------------------

static int
plslabelfuncCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
                 int argc, const char *argv[] )
{
    if ( argc < 2 || argc > 3 )
    {
        Tcl_AppendResult( interp, "bogus syntax for plslabelfunc, see doc.",
            (char *) NULL );
        return TCL_ERROR;
    }

    tcl_interp = interp;

    if ( label_objs[0] != NULL )
    {
        Tcl_DecrRefCount( label_objs[0] );
    }
    if ( label_objs[3] != NULL )
    {
        Tcl_DecrRefCount( label_objs[3] );
        label_objs[3] = NULL;
    }

    if ( strlen( argv[1] ) == 0 )
    {
        plslabelfunc( NULL, NULL );
        return TCL_OK;
    }
    else
    {
        plslabelfunc( labelform, NULL );
        label_objs[0] = Tcl_NewStringObj( argv[1], (int) strlen( argv[1] ) );
        Tcl_IncrRefCount( label_objs[0] );
    }

    if ( argc == 3 )
    {
        label_objs[3] = Tcl_NewStringObj( argv[2], (int) strlen( argv[2] ) ); // Should change with Tcl_Obj interface
        Tcl_IncrRefCount( label_objs[3] );
    }
    else
    {
        label_objs[3] = NULL;
    }

    return TCL_OK;
}

//--------------------------------------------------------------------------
// pllegendCmd
//
// Processes pllegend Tcl command.
// C version takes:
//    function, data
// (data argument is optional)
//--------------------------------------------------------------------------

static int *argv_to_ints( Tcl_Interp *interp, const char *list_numbers, int *number )
{
    int     i, retcode;
    int     *array;
    Tcl_Obj *list;
    Tcl_Obj *elem;

    list = Tcl_NewStringObj( list_numbers, ( -1 ) );

    retcode = Tcl_ListObjLength( interp, list, number );
    if ( retcode != TCL_OK || ( *number ) == 0 )
    {
        *number = 0;
        return NULL;
    }
    else
    {
        array = (int *) malloc( sizeof ( int ) * (size_t) ( *number ) );
        for ( i = 0; i < ( *number ); i++ )
        {
            Tcl_ListObjIndex( interp, list, i, &elem );
            Tcl_GetIntFromObj( interp, elem, &array[i] );
        }
    }
    return array;
}

static PLFLT *argv_to_PLFLTs( Tcl_Interp *interp, const char *list_numbers, int *number )
{
    int     i, retcode;
    PLFLT   *array;
    Tcl_Obj *list;
    Tcl_Obj *elem;
    double  ddata;

    list = Tcl_NewStringObj( list_numbers, ( -1 ) );

    retcode = Tcl_ListObjLength( interp, list, number );
    if ( retcode != TCL_OK || ( *number ) == 0 )
    {
        *number = 0;
        return NULL;
    }
    else
    {
        array = (PLFLT *) malloc( sizeof ( PLFLT ) * (size_t) ( *number ) );
        for ( i = 0; i < ( *number ); i++ )
        {
            Tcl_ListObjIndex( interp, list, i, &elem );
            Tcl_GetDoubleFromObj( interp, elem, &ddata );
            array[i] = (PLFLT) ddata;
        }
    }
    return array;
}

static char **argv_to_chars( Tcl_Interp *interp, const char *list_strings, int *number )
{
    int     i, retcode;
    char    **array;
    char    *string;
    int     length;
    int     idx;
    Tcl_Obj *list;
    Tcl_Obj *elem;

    list = Tcl_NewStringObj( list_strings, ( -1 ) );

    retcode = Tcl_ListObjLength( interp, list, number );
    if ( retcode != TCL_OK || ( *number ) == 0 )
    {
        *number = 0;
        return NULL;
    }
    else
    {
        array    = (char **) malloc( sizeof ( char* ) * (size_t) ( *number ) );
        array[0] = (char *) malloc( sizeof ( char ) * ( strlen( list_strings ) + 1 ) );
        idx      = 0;
        for ( i = 0; i < ( *number ); i++ )
        {
            Tcl_ListObjIndex( interp, list, i, &elem );
            string = Tcl_GetStringFromObj( elem, &length );

            array[i] = array[0] + idx;
            strncpy( array[i], string, (size_t) length );
            idx += length + 1;
            array[0][idx - 1] = '\0';
        }
    }
    return array;
}

static int
pllegendCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
             int argc, const char *argv[] )
{
    PLFLT   legend_width, legend_height;
    PLFLT   x, y, plot_width;
    PLINT   opt, position;
    PLINT   bg_color, bb_color, bb_style;
    PLINT   nrow, ncolumn;
    PLINT   nlegend;
    PLINT   *opt_array;
    PLFLT   text_offset, text_scale, text_spacing, text_justification;
    PLINT   *text_colors;
    PLINT   *box_colors, *box_patterns;
    PLFLT   *box_scales;
    PLINT   *line_colors, *line_styles;
    PLFLT   *box_line_widths, *line_widths;
    PLINT   *symbol_colors, *symbol_numbers;
    PLFLT   *symbol_scales;
    char    **text;
    char    **symbols;

    int     number_opts;
    int     number_texts;
    int     dummy;
    double  value;

    Tcl_Obj *data[2];

    if ( argc != 29 )
    {
        Tcl_AppendResult( interp, "bogus syntax for pllegend, see doc.",
            (char *) NULL );
        return TCL_ERROR;
    }

    sscanf( argv[1], "%d", &opt );
    sscanf( argv[2], "%d", &position );
    sscanf( argv[3], "%lg", &value ); x          = (PLFLT) value;
    sscanf( argv[4], "%lg", &value ); y          = (PLFLT) value;
    sscanf( argv[5], "%lg", &value ); plot_width = (PLFLT) value;
    sscanf( argv[6], "%d", &bg_color );
    sscanf( argv[7], "%d", &bb_color );
    sscanf( argv[8], "%d", &bb_style );
    sscanf( argv[9], "%d", &nrow );
    sscanf( argv[10], "%d", &ncolumn );
    opt_array = argv_to_ints( interp, argv[11], &number_opts );
    sscanf( argv[12], "%lg", &value ); text_offset        = (PLFLT) value;
    sscanf( argv[13], "%lg", &value ); text_scale         = (PLFLT) value;
    sscanf( argv[14], "%lg", &value ); text_spacing       = (PLFLT) value;
    sscanf( argv[15], "%lg", &value ); text_justification = (PLFLT) value;

    text_colors     = argv_to_ints( interp, argv[16], &dummy );
    text            = argv_to_chars( interp, argv[17], &number_texts );
    box_colors      = argv_to_ints( interp, argv[18], &dummy );
    box_patterns    = argv_to_ints( interp, argv[19], &dummy );
    box_scales      = argv_to_PLFLTs( interp, argv[20], &dummy );
    box_line_widths = argv_to_PLFLTs( interp, argv[21], &dummy );
    line_colors     = argv_to_ints( interp, argv[22], &dummy );
    line_styles     = argv_to_ints( interp, argv[23], &dummy );
    line_widths     = argv_to_PLFLTs( interp, argv[24], &dummy );
    symbol_colors   = argv_to_ints( interp, argv[25], &dummy );
    symbol_scales   = argv_to_PLFLTs( interp, argv[26], &dummy );
    symbol_numbers  = argv_to_ints( interp, argv[27], &dummy );
    symbols         = argv_to_chars( interp, argv[28], &dummy );

    nlegend = MIN( number_opts, number_texts );

    c_pllegend( &legend_width, &legend_height,
        opt, position, x, y, plot_width,
        bg_color, bb_color, bb_style,
        nrow, ncolumn,
        nlegend, opt_array,
        text_offset, text_scale, text_spacing,
        text_justification,
        text_colors, (const char * const *) text,
        box_colors, box_patterns,
        box_scales, box_line_widths,
        line_colors, line_styles,
        line_widths,
        symbol_colors, symbol_scales,
        symbol_numbers, (const char * const *) symbols );

    if ( opt_array != NULL )
        free( opt_array );
    if ( text_colors != NULL )
        free( text_colors );
    if ( text != NULL )
    {
        free( text[0] );
        free( text );
    }
    if ( box_colors != NULL )
        free( box_colors );
    if ( box_patterns != NULL )
        free( box_patterns );
    if ( box_scales != NULL )
        free( box_scales );
    if ( box_line_widths != NULL )
        free( box_line_widths );
    if ( line_colors != NULL )
        free( line_colors );
    if ( line_styles != NULL )
        free( line_styles );
    if ( line_widths != NULL )
        free( line_widths );
    if ( symbol_colors != NULL )
        free( symbol_colors );
    if ( symbol_scales != NULL )
        free( symbol_scales );
    if ( symbol_numbers != NULL )
        free( symbol_numbers );
    if ( symbols != NULL )
    {
        free( symbols[0] );
        free( symbols );
    }

    data[0] = Tcl_NewDoubleObj( (double) legend_width );
    data[1] = Tcl_NewDoubleObj( (double) legend_height );
    Tcl_SetObjResult( interp, Tcl_NewListObj( 2, data ) );

    return TCL_OK;
}

//--------------------------------------------------------------------------
// plcolorbarCmd
//
// Processes plcolorbar Tcl command.
//--------------------------------------------------------------------------

static int
plcolorbarCmd( ClientData PL_UNUSED( clientData ), Tcl_Interp *interp,
               int argc, const char *argv[] )
{
    PLFLT     colorbar_width, colorbar_height;
    PLINT     opt, position;
    PLFLT     x, y, x_length, y_length;
    PLINT     bg_color, bb_color, bb_style;
    PLFLT     low_cap_color, high_cap_color;
    PLINT     cont_color;
    PLFLT     cont_width;
    PLINT     n_label_opts;
    PLINT     n_labels;
    PLINT     *label_opts;
    char      **labels;
    PLINT     n_axis_opts;
    PLINT     n_ticks;
    PLINT     n_sub_ticks;
    PLINT     n_axes;
    char      **axis_opts;
    PLFLT     *ticks;
    PLINT     *sub_ticks;
    Tcl_Obj   *list_vectors;
    int       n_vectors;
    PLINT     *vector_sizes;
    PLFLT     **vector_values;
    int       retcode;
    int       i;
    int       length;
    Tcl_Obj   *vector;
    tclMatrix *vectorPtr;

    double    value;

    Tcl_Obj   *data[2];

    if ( argc != 20 )
    {
        Tcl_AppendResult( interp, "bogus syntax for plcolorbar, see doc.",
            (char *) NULL );
        return TCL_ERROR;
    }

    // The first two arguments, the resulting width and height are returned via Tcl_SetObjResult()
    sscanf( argv[1], "%d", &opt );
    sscanf( argv[2], "%d", &position );
    sscanf( argv[3], "%lg", &value ); x        = (PLFLT) value;
    sscanf( argv[4], "%lg", &value ); y        = (PLFLT) value;
    sscanf( argv[5], "%lg", &value ); x_length = (PLFLT) value;
    sscanf( argv[6], "%lg", &value ); y_length = (PLFLT) value;
    sscanf( argv[7], "%d", &bg_color );
    sscanf( argv[8], "%d", &bb_color );
    sscanf( argv[9], "%d", &bb_style );
    sscanf( argv[10], "%lg", &value ); low_cap_color  = (PLFLT) value;
    sscanf( argv[11], "%lg", &value ); high_cap_color = (PLFLT) value;
    sscanf( argv[12], "%d", &cont_color );
    sscanf( argv[13], "%lg", &value ); cont_width = (PLFLT) value;
    label_opts   = argv_to_ints( interp, argv[14], &n_label_opts );
    labels       = argv_to_chars( interp, argv[15], &n_labels );
    axis_opts    = argv_to_chars( interp, argv[16], &n_axis_opts );
    ticks        = argv_to_PLFLTs( interp, argv[17], &n_ticks );
    sub_ticks    = argv_to_ints( interp, argv[18], &n_sub_ticks );
    list_vectors = Tcl_NewStringObj( argv[19], ( -1 ) );

    // Check consistency
    if ( n_label_opts != n_labels )
    {
        Tcl_AppendResult( interp, "number of label options must equal number of labels.",
            (char *) NULL );
        return TCL_ERROR;
    }
    if ( n_axis_opts != n_ticks || n_axis_opts != n_sub_ticks )
    {
        Tcl_AppendResult( interp, "number of axis, tick and subtick options must be equal.",
            (char *) NULL );
        return TCL_ERROR;
    }
    n_axes = n_axis_opts;

    retcode = Tcl_ListObjLength( interp, list_vectors, &n_vectors );
    if ( retcode != TCL_OK || n_vectors == 0 )
    {
        Tcl_AppendResult( interp, "malformed list of vectors or no vector at all.",
            (char *) NULL );
        return TCL_ERROR;
    }
    else
    {
        vector_sizes  = (int *) malloc( sizeof ( int ) * (size_t) n_vectors );
        vector_values = (PLFLT **) malloc( sizeof ( PLFLT * ) * (size_t) n_vectors );
        for ( i = 0; i < n_vectors; i++ )
        {
            Tcl_ListObjIndex( interp, list_vectors, i, &vector );
            CHECK_Tcl_GetMatrixPtr( vectorPtr, interp, Tcl_GetStringFromObj( vector, &length ) );
            if ( vectorPtr->dim != 1 )
            {
                Tcl_AppendResult( interp, "element in list of vectors is not a vector.",
                    (char *) NULL );
                return TCL_ERROR;
            }
            vector_sizes[i]  = vectorPtr->n[0];
            vector_values[i] = vectorPtr->fdata;
        }
    }

    c_plcolorbar( &colorbar_width, &colorbar_height,
        opt, position, x, y,
        x_length, y_length,
        bg_color, bb_color, bb_style,
        low_cap_color, high_cap_color,
        cont_color, cont_width,
        n_labels, label_opts, (const char * const *) labels,
        n_axes, (const char * const *) axis_opts,
        ticks, sub_ticks,
        vector_sizes, (const PLFLT * const *) vector_values );

    if ( label_opts != NULL )
        free( label_opts );
    if ( labels != NULL )
    {
        free( labels[0] );
        free( labels );
    }
    if ( axis_opts != NULL )
    {
        free( axis_opts[0] );
        free( axis_opts );
    }
    if ( ticks != NULL )
        free( ticks );
    if ( sub_ticks != NULL )
        free( sub_ticks );
    if ( vector_values != NULL )
    {
        free( vector_sizes );
        free( vector_values );
    }

    Tcl_DecrRefCount( list_vectors );

    data[0] = Tcl_NewDoubleObj( (double) colorbar_width );
    data[1] = Tcl_NewDoubleObj( (double) colorbar_height );
    Tcl_SetObjResult( interp, Tcl_NewListObj( 2, data ) );

    return TCL_OK;
}
