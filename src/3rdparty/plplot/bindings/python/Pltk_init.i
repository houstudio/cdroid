// Swig instructions to help implement the Pltk_init extension module

//Copyright (C) 2017 Alan W. Irwin

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

%module Pltk_init
%include typemaps.i

%{
#define NPY_NO_DEPRECATED_API    NPY_1_7_API_VERSION
#include <Python.h>
#include <arrayobject.h>
#include "plplot.h"
#include "plplotP.h"

#include <tcl.h>
#include "pltk.h"

// Initialize the PLtk extension.  The single argument of this routine
// is the address of the Tcl interpreter into which the Pltk extension
// is to be injected.

    void Pltk_init( long x )
    {
        Tcl_Interp *interp = (Tcl_Interp *) x;

        if ( Pltk_Init( interp ) == TCL_ERROR )
        {
            printf( "Initialization of Pltk Tcl extension failed!\n" );
        }
    }
%}

// Interface the above routine with swig
void Pltk_init( long x );

