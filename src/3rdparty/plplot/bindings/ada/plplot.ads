-- Maintain backward compatibility of the Ada "standard" binding after changing the
-- name to PLplot_Standard from PLplot.

-- Copyright (C) 2016 Jerry Bauck

-- This file is part of PLplot.

-- PLplot is free software; you can redistribute it and/or modify
-- it under the terms of the GNU Library General Public License as published
-- by the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.

-- PLplot is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Library General Public License for more details.

-- You should have received a copy of the GNU Library General Public License
-- along with PLplot; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

-- The preferred name for the "standard" Ada binding is PLplot_Standard. It replaces
-- the earlier identical package that was named PLplot. This file simply renames that
-- package so that applications which use the old binding name, PLplot, will still
-- function as they did before the name change. The name PLplot is thus deprecated
-- although with this file progams that with-use the PLplot name should always work.

with PLplot_Standard;
package PLplot renames PLplot_Standard;
