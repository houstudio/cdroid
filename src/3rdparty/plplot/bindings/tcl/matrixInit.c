//  Tcl Matrix initializer.
//  Vince Darley
//
//  Copyright (C) 2004  Joao Cardoso
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
//

#include "pldll.h"
#include "tclMatrix.h"

int Matrix_Init( Tcl_Interp *interp )
{
#ifdef USE_TCL_STUBS
//
// We hard-wire 8.1 here, rather than TCL_VERSION, TK_VERSION because
// we really don't mind which version of Tcl, Tk we use as long as it
// is 8.1 or newer.  Otherwise if we compiled against 8.2, we couldn't
// be loaded into 8.1
//
    Tcl_InitStubs( interp, "8.1", 0 );
#endif
    // matrix -- matrix	support	command
    Tcl_CreateCommand( interp, "matrix", (Tcl_CmdProc *) Tcl_MatrixCmd,
        (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL );

    Tcl_PkgProvide( interp, "Matrix", "0.1" );
    return TCL_OK;
}
