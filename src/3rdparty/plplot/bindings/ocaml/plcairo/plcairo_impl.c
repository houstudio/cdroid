//
// Copyright 2008  Hezekiah M. Carty
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

#include <plplotP.h>

// The "usual" OCaml includes
#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/misc.h>
#include <caml/mlvalues.h>
#include <caml/bigarray.h>

#include <plplot.h>

#include <cairo.h>

//* OCaml Cairo integration.
// Get a Cairo context from OCaml.  These #defines are from the
// OCaml Cairo bindings.
#define wPointer_val( t, val )    ( *( (t **) Data_custom_val( val ) ) )
#define cairo_t_val( v )          wPointer_val( cairo_t, v )

value ml_set_plplot_cairo_context( value context )
{
    CAMLparam1( context );
    pl_cmd( PLESC_DEVINIT, cairo_t_val( context ) );
    CAMLreturn( Val_unit );
}

