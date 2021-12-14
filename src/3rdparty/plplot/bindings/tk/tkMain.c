// Modified version of tkMain.c, from Tk 3.6.
// Maurice LeBrun
// 23-Jun-1994
//
// Copyright (C) 2004  Joao Cardoso
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
// Modifications include:
// 1. main() changed to pltkMain().
// 2. tcl_RcFileName -> RcFileName, now passed in through the argument list.
// 3. Tcl_AppInit -> AppInit, now passed in through the argument list.
// 4. Support for -e <script> startup option
//
// The original notes follow.
//

//
// main.c --
//
//	This file contains the main program for "wish", a windowing
//	shell based on Tk and Tcl.  It also provides a template that
//	can be used as the basis for main programs for other Tk
//	applications.
//
// Copyright (c) 1990-1993 The Regents of the University of California.
// All rights reserved.
//
// Permission is hereby granted, without written agreement and without
// license or royalty fees, to use, copy, modify, and distribute this
// software and its documentation for any purpose, provided that the
// above copyright notice and the following two paragraphs appear in
// all copies of this software.
//
// IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
// OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
// CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
// ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
// PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//

#include "plplotP.h"
#include "pltkd.h"
#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include <tk.h>
#ifdef HAVE_ITCL
# ifndef HAVE_ITCLDECLS_H
#  define RESOURCE_INCLUDED
# endif
# include <itcl.h>
#endif

// itk.h includes itclInt.h which includes tclInt.h ...disaster -mjl
// #ifdef HAVE_ITK
// #include <itk.h>
// #endif

// From itkDecls.h

EXTERN int Itk_Init _ANSI_ARGS_( ( Tcl_Interp * interp ) );

// From tclIntDecls.h

//#ifndef Tcl_Import_TCL_DECLARED
#if 0
EXTERN int Tcl_Import _ANSI_ARGS_( ( Tcl_Interp * interp,
                                     Tcl_Namespace * nsPtr, char * pattern,
                                     int allowOverwrite ) );
#endif

#ifndef Tcl_GetGlobalNamespace_TCL_DECLARE
EXTERN Tcl_Namespace * Tcl_GetGlobalNamespace _ANSI_ARGS_( (
                                                               Tcl_Interp * interp ) );
#endif

//
// Declarations for various library procedures and variables (don't want
// to include tkInt.h or tkConfig.h here, because people might copy this
// file out of the Tk source directory to make their own modified versions).
//

// these are defined in unistd.h, included by plplotP.h
// extern void		exit _ANSI_ARGS_((int status));
// extern int		isatty _ANSI_ARGS_((int fd));
// extern int		read _ANSI_ARGS_((int fd, char *buf, size_t size));
//
#if !defined ( _WIN32 )
extern char *           strrchr _ANSI_ARGS_( ( CONST char *string, int c ) );
#else
// On Windows we do not have a convenient console to work with
#define isatty( a )    0
#endif

//
// Global variables used by the main program:
//

static Tcl_Interp  *interp;     // Interpreter for this application.
static Tcl_DString command;     // Used to assemble lines of terminal input
                                // into Tcl commands.
static int         tty;         // Non-zero means standard input is a
                                // terminal-like device.  Zero means it's
                                // a file.
static char errorExitCmd[] = "exit 1";

//
// Command-line options:
//

static int         synchronize = 0;
static const char  *script     = NULL;
static const char  *fileName   = NULL;
static const char  *name       = NULL;
static const char  *display    = NULL;
static const char  *geometry   = NULL;

static Tk_ArgvInfo argTable[] = {
    { "-file",       TK_ARGV_STRING,   (char *) NULL, (char *) &fileName,
      "File from which to read commands" },
    { "-e",          TK_ARGV_STRING,   (char *) NULL, (char *) &script,
      "Script to execute on startup" },
    { "-geometry",   TK_ARGV_STRING,   (char *) NULL, (char *) &geometry,
      "Initial geometry for window" },
    { "-display",    TK_ARGV_STRING,   (char *) NULL, (char *) &display,
      "Display to use" },
    { "-name",       TK_ARGV_STRING,   (char *) NULL, (char *) &name,
      "Name to use for application" },
    { "-sync",       TK_ARGV_CONSTANT, (char *) 1,    (char *) &synchronize,
      "Use synchronous mode for display server" },
    { (char *) NULL, TK_ARGV_END,      (char *) NULL, (char *) NULL,
      (char *) NULL }
};

//
// Forward declarations for procedures defined later in this file:
//

static void Prompt _ANSI_ARGS_( ( Tcl_Interp * interploc, int partial ) );
static void StdinProc _ANSI_ARGS_( ( ClientData clientData,
                                     int mask ) );

//
//--------------------------------------------------------------------------
//
// main --
//
//	Main program for Wish.
//
// Results:
//	None. This procedure never returns (it exits the process when
//	it's done
//
// Side effects:
//	This procedure initializes the wish world and then starts
//	interpreting commands;  almost anything could happen, depending
//	on the script being interpreted.
//
//--------------------------------------------------------------------------
//

int
pltkMain( int argc, const char **argv, char *RcFileName,
          int ( *AppInit )( Tcl_Interp *interp ) )
{
    char       *args;
    const char *msg, *p;
    char       buf[20];
    int        code;

#ifdef PL_HAVE_PTHREAD
    XInitThreads();
#endif

    Tcl_FindExecutable( argv[0] );
    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory( interp );
#endif

    //
    // Parse command-line arguments.
    //
    //fprintf( stderr, "Before Tk_ParseArgv\n" );

    if ( Tk_ParseArgv( interp, (Tk_Window) NULL, &argc, argv, argTable, 0 )
         != TCL_OK )
    {
        fprintf( stderr, "%s\n", Tcl_GetStringResult( interp ) );
        exit( 1 );
    }
    //fprintf( stderr, "After Tk_ParseArgv\n" );
    if ( name == NULL )
    {
        if ( fileName != NULL )
        {
            p = fileName;
        }
        else
        {
            p = argv[0];
        }
        name = strrchr( p, '/' );
        if ( name != NULL )
        {
            name++;
        }
        else
        {
            name = p;
        }
    }

    //
    // If a display was specified, put it into the DISPLAY
    // environment variable so that it will be available for
    // any sub-processes created by us.
    //

    if ( display != NULL )
    {
        Tcl_SetVar2( interp, "env", "DISPLAY", display, TCL_GLOBAL_ONLY );
    }

    //
    // Initialize the Tk application.
    //

    //
    // This must be setup *before* calling Tk_Init,
    // and `name' has already been setup above
    //

    Tcl_SetVar( interp, "argv0", name, TCL_GLOBAL_ONLY );

    if ( Tcl_Init( interp ) == TCL_ERROR )
    {
        fprintf( stderr, "Tcl initialisation failed: %s\n", Tcl_GetStringResult( interp ) );
        return TCL_ERROR;
    }
    if ( Tk_Init( interp ) == TCL_ERROR )
    {
        fprintf( stderr, "Tk initialisation failed: %s\n", Tcl_GetStringResult( interp ) );
        return TCL_ERROR;
    }
#ifdef HAVE_ITCL
    if ( Itcl_Init( interp ) == TCL_ERROR )
    {
        fprintf( stderr, "Itcl initialisation failed: %s\n", Tcl_GetStringResult( interp ) );
        return TCL_ERROR;
    }
#endif
#ifdef HAVE_ITK
    if ( Itk_Init( interp ) == TCL_ERROR )
    {
        fprintf( stderr, "Itk initialisation failed: %s\n", Tcl_GetStringResult( interp ) );
        return TCL_ERROR;
    }

//
// Pulled in this next section from itkwish in itcl3.0.1.
//

    //
    //  This is itkwish, so import all [incr Tcl] commands by
    //  default into the global namespace.  Fix up the autoloader
    //  to do the same.
    //
    if ( Tcl_Import( interp, Tcl_GetGlobalNamespace( interp ),
             "::itk::*", /* allowOverwrite */ 1 ) != TCL_OK )
    {
        fprintf( stderr, "Itk initialisation failed: %s\n", Tcl_GetStringResult( interp ) );
        return TCL_ERROR;
    }

    if ( Tcl_Import( interp, Tcl_GetGlobalNamespace( interp ),
             "::itcl::*", /* allowOverwrite */ 1 ) != TCL_OK )
    {
        fprintf( stderr, "Itk initialisation failed: %s\n", Tcl_GetStringResult( interp ) );
        return TCL_ERROR;
    }

    if ( Tcl_Eval( interp, "auto_mkindex_parser::slavehook { _%@namespace import -force ::itcl::* ::itk::* }" ) != TCL_OK )
    {
        fprintf( stderr, "Itk initialisation failed: %s\n", Tcl_GetStringResult( interp ) );
        return TCL_ERROR;
    }
#endif

    //
    // Make command-line arguments available in the Tcl variables "argc"
    // and "argv".  Also set the "geometry" variable from the geometry
    // specified on the command line.
    //
    //fprintf( stderr, "Before Tcl_Merge\n" );

    args = Tcl_Merge( argc - 1, ( CONST char * CONST * )argv + 1 );
    Tcl_SetVar( interp, "argv", args, TCL_GLOBAL_ONLY );
    ckfree( args );
    sprintf( buf, "%d", argc - 1 );
    Tcl_SetVar( interp, "argc", buf, TCL_GLOBAL_ONLY );

    //fprintf( stderr, "After Tcl_Merge\n" );
    if ( geometry != NULL )
    {
        Tcl_SetVar( interp, "geometry", geometry, TCL_GLOBAL_ONLY );
    }

    //
    // Set the "tcl_interactive" variable.
    //

    tty = isatty( 0 );
    Tcl_SetVar( interp, "tcl_interactive",
        ( ( fileName == NULL ) && tty ) ? "1" : "0", TCL_GLOBAL_ONLY );

    //
    // Add a few application-specific commands to the application's
    // interpreter.
    //

    //
    // Invoke application-specific initialization.
    //
    //fprintf( stderr, "Before AppInit\n" );

    if ( ( *AppInit )( interp ) != TCL_OK )
    {
        fprintf( stderr, "(*AppInit) failed: %s\n", Tcl_GetStringResult( interp ) );
        return TCL_ERROR;
    }

    //
    // Set the geometry of the main window, if requested.
    //

    if ( geometry != NULL )
    {
        code = Tcl_VarEval( interp, "wm geometry . ", geometry, (char *) NULL );
        if ( code != TCL_OK )
        {
            fprintf( stderr, "%s\n", Tcl_GetStringResult( interp ) );
        }
    }

    //
    // Process the startup script, if any.
    //
    //fprintf( stderr, "Before startup\n" );

    if ( script != NULL )
    {
        code = Tcl_VarEval( interp, script, (char *) NULL );
        if ( code != TCL_OK )
        {
            goto error;
        }
        tty = 0;
    }

    //
    // Invoke the script specified on the command line, if any.
    //
    //fprintf( stderr, "Before source\n" );

    if ( fileName != NULL )
    {
        code = Tcl_VarEval( interp, "source \"", fileName, "\"", (char *) NULL );
        if ( code != TCL_OK )
        {
            goto error;
        }
        tty = 0;
    }
    else
    {
        //
        // Commands will come from standard input, so set up an event
        // handler for standard input.  Evaluate the .rc file, if one
        // has been specified, set up an event handler for standard
        // input, and print a prompt if the input device is a
        // terminal.
        //

        if ( RcFileName != NULL )
        {
            Tcl_DString buffer;
            char        *fullName;
            FILE        *f;

            fullName = Tcl_TildeSubst( interp, RcFileName, &buffer );
            if ( fullName == NULL )
            {
                fprintf( stderr, "%s\n", Tcl_GetStringResult( interp ) );
            }
            else
            {
                f = fopen( fullName, "r" );
                if ( f != NULL )
                {
                    code = Tcl_EvalFile( interp, fullName );
                    if ( code != TCL_OK )
                    {
                        fprintf( stderr, "%s\n", Tcl_GetStringResult( interp ) );
                    }
                    fclose( f );
                }
            }
            Tcl_DStringFree( &buffer );
        }
// Exclude UNIX-only feature
#if !defined ( MAC_TCL ) && !defined ( _WIN32 )
        Tk_CreateFileHandler( 0, TK_READABLE, StdinProc, (ClientData) 0 );
#endif
        if ( tty )
        {
            Prompt( interp, 0 );
        }
    }
    fflush( stdout );
    Tcl_DStringInit( &command );

    //
    // Loop infinitely, waiting for commands to execute.  When there
    // are no windows left, Tk_MainLoop returns and we exit.
    //

    //fprintf( stderr, "Before Tk_MainLoop\n" );
    Tk_MainLoop();

    //
    // Don't exit directly, but rather invoke the Tcl "exit" command.
    // This gives the application the opportunity to redefine "exit"
    // to do additional cleanup.
    //

    Tcl_Eval( interp, "exit" );
    exit( 1 );

error:
    msg = Tcl_GetVar( interp, "errorInfo", TCL_GLOBAL_ONLY );
    if ( msg == NULL )
    {
        msg = Tcl_GetStringResult( interp );
    }
    fprintf( stderr, "%s\n", msg );
    Tcl_Eval( interp, errorExitCmd );
    return 1;                   // Needed only to prevent compiler warnings.
}

//
//--------------------------------------------------------------------------
//
// StdinProc --
//
//	This procedure is invoked by the event dispatcher whenever
//	standard input becomes readable.  It grabs the next line of
//	input characters, adds them to a command being assembled, and
//	executes the command if it's complete.
//
// Results:
//	None.
//
// Side effects:
//	Could be almost arbitrary, depending on the command that's
//	typed.
//
//--------------------------------------------------------------------------
//

// ARGSUSED
static void
StdinProc( ClientData PL_UNUSED( clientData ), int PL_UNUSED( mask ) )
{
#define BUFFER_SIZE    4000
    char       input[BUFFER_SIZE + 1];
    static int gotPartial = 0;
    char       *cmd;
    int        code, count;
    const char *res;

#if !defined ( _WIN32 )
    count = (int) read( fileno( stdin ), input, BUFFER_SIZE );
#else
    count = fread( input, BUFFER_SIZE, sizeof ( char ), stdin );
#endif
    if ( count <= 0 )
    {
        if ( !gotPartial )
        {
            if ( tty )
            {
                Tcl_Eval( interp, "exit" );
                exit( 1 );
            }
            else
            {
#if !defined ( MAC_TCL ) && !defined ( _WIN32 )
                Tk_DeleteFileHandler( 0 );
#endif
            }
            return;
        }
        else
        {
            count = 0;
        }
    }
    cmd = Tcl_DStringAppend( &command, input, count );
    if ( count != 0 )
    {
        if ( ( input[count - 1] != '\n' ) && ( input[count - 1] != ';' ) )
        {
            gotPartial = 1;
            goto prompt;
        }
        if ( !Tcl_CommandComplete( cmd ) )
        {
            gotPartial = 1;
            goto prompt;
        }
    }
    gotPartial = 0;

    //
    // Disable the stdin file handler while evaluating the command;
    // otherwise if the command re-enters the event loop we might
    // process commands from stdin before the current command is
    // finished.  Among other things, this will trash the text of the
    // command being evaluated.
    //
#if !defined ( MAC_TCL ) && !defined ( _WIN32 )
    Tk_CreateFileHandler( 0, 0, StdinProc, (ClientData) 0 );
#endif
    code = Tcl_RecordAndEval( interp, cmd, 0 );
#if !defined ( MAC_TCL ) && !defined ( _WIN32 )
    Tk_CreateFileHandler( 0, TK_READABLE, StdinProc, (ClientData) 0 );
#endif
    Tcl_DStringFree( &command );
    res = Tcl_GetStringResult( interp );
    if ( *res != 0 )
    {
        if ( ( code != TCL_OK ) || ( tty ) )
        {
            printf( "%s\n", res );
        }
    }

    //
    // Output a prompt.
    //

prompt:
    if ( tty )
    {
        Prompt( interp, gotPartial );
    }
}

//
//--------------------------------------------------------------------------
//
// Prompt --
//
//	Issue a prompt on standard output, or invoke a script
//	to issue the prompt.
//
// Results:
//	None.
//
// Side effects:
//	A prompt gets output, and a Tcl script may be evaluated
//	in interp.
//
//--------------------------------------------------------------------------
//

static void
Prompt( interploc, partial )
Tcl_Interp * interploc;               // Interpreter to use for prompting.
int partial;                          // Non-zero means there already
                                      // exists a partial command, so use
                                      // the secondary prompt.
{
    const char *promptCmd;
    int        code;

    promptCmd = Tcl_GetVar( interploc,
        partial ? "tcl_prompt2" : "tcl_prompt1", TCL_GLOBAL_ONLY );
    if ( promptCmd == NULL )
    {
defaultPrompt:
        if ( !partial )
        {
            fputs( "% ", stdout );
        }
    }
    else
    {
        code = Tcl_Eval( interploc, promptCmd );
        if ( code != TCL_OK )
        {
            Tcl_AddErrorInfo( interploc,
                "\n    (script that generates prompt)" );
            fprintf( stderr, "%s\n", Tcl_GetStringResult( interploc ) );
            goto defaultPrompt;
        }
    }
    fflush( stdout );
}
