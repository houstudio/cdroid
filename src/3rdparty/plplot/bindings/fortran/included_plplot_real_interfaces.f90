    !***********************************************************************
    !  included_plplot_real_interfaces.f90
    !
    !  Copyright (C) 2005-2016  Arjen Markus
    !  Copyright (C) 2006-2018 Alan W. Irwin
    !
    !  This file is part of PLplot.
    !
    !  PLplot is free software; you can redistribute it and/or modify
    !  it under the terms of the GNU Library General Public License as published
    !  by the Free Software Foundation; either version 2 of the License, or
    !  (at your option) any later version.
    !
    !  PLplot is distributed in the hope that it will be useful,
    !  but WITHOUT ANY WARRANTY; without even the implied warranty of
    !  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    !  GNU Library General Public License for more details.
    !
    !  You should have received a copy of the GNU Library General Public License
    !  along with PLplot; if not, write to the Free Software
    !  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
    !
    !
    !  This file defines the Fortran interfaces for the the subset of the
    !  PLplot API which has at least one real argument.  Keeping this part
    !  of the interface definitions separate allows convenient defining of
    !  these interfaces for both single and double precision real
    !  arguments.  See plplot_bindings.f90 for the definition of the
    !  Fortran interfaces for the remaining part of the PLplot API.
    !
    !***********************************************************************

    private :: matrix_to_c

    ! Private interfaces for wp-precision callbacks

    abstract interface
        subroutine plmapform_proc( x, y )
            import :: wp
            real(kind=wp), dimension(:), intent(inout) :: x, y
        end subroutine plmapform_proc
    end interface
    procedure(plmapform_proc), pointer :: plmapform
    private:: plmapform_proc, plmapform

    abstract interface
        subroutine pllabeler_proc( axis, value, label )
            import :: wp
            integer, intent(in) :: axis
            real(kind=wp), intent(in) :: value
            character(len=*), intent(out) :: label
        end subroutine pllabeler_proc
    end interface
    procedure(pllabeler_proc), pointer :: pllabeler
    private:: pllabeler_proc, pllabeler

    abstract interface
        subroutine pllabeler_proc_data( axis, value, label, data )
            import :: wp, c_ptr
            implicit none
            integer, intent(in) :: axis
            real(kind=wp), intent(in) :: value
            character(len=*), intent(out) :: label
            type(c_ptr), intent(in) :: data
        end subroutine pllabeler_proc_data
    end interface
    procedure(pllabeler_proc_data), pointer :: pllabeler_data
    private:: pllabeler_proc_data, pllabeler_data

    abstract interface
        subroutine pltransform_proc( x, y, tx, ty )
            import :: wp
            implicit none
            real(kind=wp), intent(in) :: x, y
            real(kind=wp), intent(out) :: tx, ty
        end subroutine pltransform_proc
    end interface
    procedure(pltransform_proc), pointer :: pltransform
    private:: pltransform_proc, pltransform

    abstract interface
        subroutine pltransform_proc_data( x, y, tx, ty, data )
            import :: wp, c_ptr
            implicit none
            real(kind=wp), intent(in) :: x, y
            real(kind=wp), intent(out) :: tx, ty
            type(c_ptr), intent(in) :: data
        end subroutine pltransform_proc_data
    end interface
    procedure(pltransform_proc_data), pointer :: pltransform_data
    private:: pltransform_proc_data, pltransform_data

    ! Normally interface blocks describing the C routines that are
    ! called by this Fortran binding are embedded as part of module
    ! procedures, but when more than one module procedure uses such
    ! interface blocks there is a requirement (enforced at least by
    ! the nagfor compiler) that those interface blocks be consistent.
    ! We could comply with that requirement by embedding such multiply
    ! used interface blocks as part of module procedures using
    ! duplicated code, but that is inefficient (in terms of the number
    ! of lines of code to be compiled) and implies a maintenance issue
    ! (to keep that code duplicated whenever there are changes on the
    ! C side).  To deal with those two potential issues we collect
    ! here in alphabetical order all interface blocks describing C
    ! routines that are called directly by more than one module
    ! procedure below.

    interface
        subroutine interface_plcont( z, nx, ny, kx, lx, ky, ly, clevel, nlevel, transform, data ) &
               bind(c,name='c_plcont')
            import :: c_funptr, c_ptr
            import :: private_plint, private_plflt
            implicit none
            integer(kind=private_plint), value, intent(in) :: nx, ny, kx, lx, ky, ly, nlevel
            type(c_ptr), dimension(*), intent(in)  :: z
            real(kind=private_plflt), dimension(*), intent(in) :: clevel
            type(c_ptr), value, intent(in) :: data
            interface
                subroutine transform( x, y, tx, ty, data ) bind(c)
                    import :: private_plflt
                    import :: c_ptr
                    implicit none
                    real(kind=private_plflt), value, intent(in) :: x, y
                    real(kind=private_plflt), intent(out) :: tx, ty
                    type(c_ptr), value, intent(in) :: data
                end subroutine transform
            end interface
        end subroutine interface_plcont
    end interface
    private :: interface_plcont

    interface
        function interface_plf2evalr( ix, iy, data ) bind(c, name = 'plf2evalr' )
            import :: c_ptr
            import :: private_plint, private_plflt
            implicit none
            real(kind=private_plflt) :: interface_plf2evalr
            integer(kind=private_plint), value, intent(in) :: ix, iy
            type(c_ptr), value, intent(in) :: data
        end function interface_plf2evalr
    end interface
    private :: interface_plf2evalr

    interface
        subroutine interface_plfcont( lookup, grid, nx, ny, kx, lx, ky, ly, clevel, nlevel, transform, data ) &
               bind(c,name='plfcont')
            import :: c_funptr, c_ptr
            import :: private_plint, private_plflt
            implicit none
            integer(kind=private_plint), value, intent(in) :: nx, ny, kx, lx, ky, ly, nlevel
            real(kind=private_plflt), dimension(*), intent(in) :: clevel
            type(c_ptr), value, intent(in) :: grid
            type(c_ptr), value, intent(in) :: data
            interface
                function lookup( ix, iy, data ) bind(c)
                    import :: c_ptr
                    import :: private_plflt, private_plint
                    implicit none
                    real(kind=private_plflt) :: lookup
                    integer(kind=private_plint), value, intent(in) :: ix, iy
                    type(c_ptr), value, intent(in) :: data
                end function lookup
            end interface
            interface
                subroutine transform( x, y, tx, ty, data ) bind(c)
                    import :: private_plflt
                    import :: c_ptr
                    implicit none
                    real(kind=private_plflt), value, intent(in)  :: x, y
                    real(kind=private_plflt), intent(out) :: tx, ty
                    type(c_ptr), value, intent(in) :: data
                end subroutine transform
            end interface
        end subroutine interface_plfcont
    end interface
    private :: interface_plfcont

    interface
        subroutine interface_plfill( n, x, y ) bind( c, name='c_plfill')
            import :: private_plint, private_plflt
            implicit none
            integer(kind=private_plint), value, intent(in) :: n
            real(kind=private_plflt), dimension(*), intent(in) :: x, y
        end subroutine interface_plfill
    end interface
    private :: interface_plfill

    interface
        subroutine interface_plfvect( lookup, fgrid1, fgrid2, nx, ny, scale, transform, data ) bind(c, name = 'plfvect' )
            import :: c_ptr, c_funptr
            import :: private_plint, private_plflt, PLfGrid, PLcGrid
            implicit none
            integer(kind=private_plint), value, intent(in) :: nx, ny
            type(PLfGrid), intent(in) :: fgrid1, fgrid2
            real(kind=private_plflt), value, intent(in) :: scale
            type(c_ptr), value, intent(in) :: data ! Not used in this case
            interface
                function lookup( ix, iy, data ) bind(c)
                    import :: c_ptr
                    import :: private_plint, private_plflt
                    implicit none
                    real(kind=private_plflt) :: lookup
                    integer(kind=private_plint), value, intent(in) :: ix, iy
                    type(c_ptr), value, intent(in) :: data
                end function lookup
            end interface
            interface
                subroutine transform( x, y, tx, ty, data ) bind(c)
                    import :: private_plflt
                    import :: c_ptr
                    implicit none
                    real(kind=private_plflt), value, intent(in) :: x, y
                    real(kind=private_plflt), intent(out) :: tx, ty
                    type(c_ptr), value, intent(in) :: data
                end subroutine transform
            end interface
        end subroutine interface_plfvect
    end interface
    private :: interface_plfvect

    interface
        subroutine interface_plimagefr( idata, nx, ny, &
               xmin, xmax, ymin, ymax, &
               zmin, zmax, valuemin, valuemax, transform, data ) bind(c,name='c_plimagefr')
            import :: c_ptr
            import :: private_plint, private_plflt
            implicit none
            integer(kind=private_plint), value, intent(in) :: nx, ny
            real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax
            type(c_ptr), dimension(*), intent(in) :: idata
            type(c_ptr), value, intent(in) :: data
            interface
                subroutine transform( x, y, tx, ty, data ) bind(c)
                    import :: private_plflt
                    import :: c_ptr
                    implicit none
                    real(kind=private_plflt), value, intent(in) :: x, y
                    real(kind=private_plflt), intent(out) :: tx, ty
                    type(c_ptr), value, intent(in) :: data
                end subroutine transform
            end interface
        end subroutine interface_plimagefr
    end interface
    private :: interface_plimagefr

    interface
        subroutine interface_plimagefr_null( idata, nx, ny, &
               xmin, xmax, ymin, ymax, &
               zmin, zmax, valuemin, valuemax) bind(c,name='plimagefr_null')
            import :: c_ptr
            import :: private_plint, private_plflt
            implicit none
            integer(kind=private_plint), value, intent(in) :: nx, ny
            real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax
            type(c_ptr), dimension(*), intent(in) :: idata
        end subroutine interface_plimagefr_null
    end interface
    private :: interface_plimagefr_null

    interface
        subroutine interface_plmap( proc, name, minx, maxx, miny, maxy ) &
               bind(c, name = 'c_plmap' )
            import :: c_funptr, private_plflt
            implicit none
            type(c_funptr), value, intent(in) :: proc
            character(len=1), dimension(*), intent(in) :: name
            real(kind=private_plflt), value, intent(in) :: minx, maxx, miny, maxy
        end subroutine interface_plmap
    end interface
    private interface_plmap

    interface
        subroutine interface_plmapfill( proc, name, minx, maxx, miny, maxy, plotentries, nplotentries ) &
               bind(c, name = 'c_plmapfill' )
            import :: c_funptr, private_plflt, c_ptr, private_plint
            implicit none
            type(c_funptr), value, intent(in) :: proc
            character(len=1), dimension(*), intent(in) :: name
            real(kind=private_plflt), value, intent(in) :: minx, maxx, miny, maxy
            type(c_ptr), value, intent(in) :: plotentries
            integer(kind=private_plint), value, intent(in) :: nplotentries
        end subroutine interface_plmapfill
    end interface
    private :: interface_plmapfill

    interface
        subroutine interface_plmapline( proc, name, minx, maxx, miny, maxy, plotentries, nplotentries ) &
               bind(c, name = 'c_plmapline' )
            import :: c_funptr, private_plflt, c_ptr, private_plint
            implicit none
            type(c_funptr), value, intent(in) :: proc
            character(len=1), dimension(*), intent(in) :: name
            real(kind=private_plflt), value, intent(in) :: minx, maxx, miny, maxy
            type(c_ptr), value, intent(in) :: plotentries
            integer(kind=private_plint), value, intent(in) :: nplotentries
        end subroutine interface_plmapline
    end interface
    private :: interface_plmapline

    interface
        subroutine interface_plmapstring( proc, name, string, minx, maxx, miny, maxy, plotentries, nplotentries ) &
               bind(c, name = 'c_plmapstring' )
            import :: c_funptr, private_plflt, c_ptr, private_plint
            implicit none
            type(c_funptr), value, intent(in) :: proc
            character(len=1), dimension(*), intent(in) :: name, string
            real(kind=private_plflt), value, intent(in) :: minx, maxx, miny, maxy
            type(c_ptr), value, intent(in) :: plotentries
            integer(kind=private_plint), value, intent(in) :: nplotentries
        end subroutine interface_plmapstring
    end interface
    private :: interface_plmapstring

    interface
        subroutine interface_plmaptex( proc, name, dx, dy, just, text, minx, maxx, miny, maxy, plotentry ) &
               bind(c, name = 'c_plmaptex' )
            import :: c_funptr, private_plflt, c_ptr, private_plint
            implicit none
            type(c_funptr), value, intent(in) :: proc
            character(len=1), dimension(*), intent(in) :: name, text
            real(kind=private_plflt), value, intent(in) :: dx, dy, just, minx, maxx, miny, maxy
            integer(kind=private_plint), value, intent(in) :: plotentry
        end subroutine interface_plmaptex
    end interface
    private :: interface_plmaptex

    interface
        subroutine interface_plmeridians( proc, dlong, dlat, minlong, maxlong, minlat, maxlat ) &
               bind(c, name = 'c_plmeridians' )
            import :: c_funptr, private_plflt
            implicit none
            type(c_funptr), value, intent(in) :: proc
            real(kind=private_plflt), value, intent(in) :: dlong, dlat, minlong, maxlong, minlat, maxlat
        end subroutine interface_plmeridians
    end interface
    private :: interface_plmeridians

    interface
        subroutine interface_plshade( a, nx, ny, defined, xmin, xmax, ymin, ymax, &
               shade_min, shade_max, sh_cmap, sh_color, sh_width, &
               min_color, min_width, max_color, max_width, &
               fill, rectangular, transform, data ) bind(c, name = 'c_plshade' )
            import :: c_ptr, c_funptr, c_null_ptr
            import :: private_plint, private_plbool, private_plflt, PLcGrid
            implicit none
            type(c_ptr), dimension(*), intent(in)  :: a
            integer(kind=private_plint), value, intent(in) :: nx, ny, sh_cmap, min_color, max_color
            integer(kind=private_plbool), value, intent(in) :: rectangular
            real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax
            real(kind=private_plflt), value, intent(in) :: sh_width, min_width, max_width
            real(kind=private_plflt), value, intent(in) :: shade_min, shade_max, sh_color
            type(c_ptr), value, intent(in) :: data
            type(c_ptr), value, intent(in) :: defined ! Not used in this case
            interface
                subroutine fill( n, x, y ) bind(c)
                    import :: private_plint, private_plflt
                    integer(kind=private_plint), value, intent(in) :: n
                    real(kind=private_plflt), dimension(*), intent(in) :: x, y
                end subroutine fill
            end interface
            interface
                subroutine transform( x, y, tx, ty, data ) bind(c)
                    import :: private_plflt
                    import :: c_ptr
                    implicit none
                    real(kind=private_plflt), value, intent(in) :: x, y
                    real(kind=private_plflt), intent(out) :: tx, ty
                    type(c_ptr), value, intent(in) :: data
                end subroutine transform
            end interface
        end subroutine interface_plshade
    end interface
    private :: interface_plshade

    interface
        subroutine interface_plshade_null( a, nx, ny, defined, xmin, xmax, ymin, ymax, &
               shade_min, shade_max, sh_cmap, sh_color, sh_width, &
               min_color, min_width, max_color, max_width, &
               fill, rectangular) bind(c, name = 'plshade_null' )
            import :: c_ptr, c_funptr, c_null_ptr
            import :: private_plint, private_plbool, private_plflt, PLcGrid
            implicit none
            type(c_ptr), dimension(*), intent(in)  :: a
            integer(kind=private_plint), value, intent(in) :: nx, ny, sh_cmap, min_color, max_color
            integer(kind=private_plbool), value, intent(in) :: rectangular
            real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax
            real(kind=private_plflt), value, intent(in) :: sh_width, min_width, max_width
            real(kind=private_plflt), value, intent(in) :: shade_min, shade_max, sh_color
            type(c_ptr), value, intent(in) :: defined ! Not used in this case
            interface
                subroutine fill( n, x, y ) bind(c)
                    import :: private_plint, private_plflt
                    integer(kind=private_plint), value, intent(in) :: n
                    real(kind=private_plflt), dimension(*), intent(in) :: x, y
                end subroutine fill
            end interface
        end subroutine interface_plshade_null
    end interface
    private :: interface_plshade_null

    interface
        subroutine interface_plshades( a, nx, ny, defined, xmin, xmax, ymin, ymax, &
               clevel, nlevel, fill_width, cont_color, cont_width, &
               fill, rectangular, transform, data ) bind(c, name = 'c_plshades' )
            import :: c_ptr, c_funptr, c_null_ptr
            import :: private_plint, private_plbool, private_plflt, PLcGrid
            implicit none
            type(c_ptr), dimension(*), intent(in)  :: a
            integer(kind=private_plint), value, intent(in) :: nx, ny, cont_color, nlevel
            integer(kind=private_plbool), value, intent(in) :: rectangular
            real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax, fill_width
            real(kind=private_plflt), dimension(*), intent(in) :: clevel
            real(kind=private_plflt), value, intent(in) :: cont_width
            type(c_ptr), value, intent(in) :: data
            type(c_ptr), value, intent(in) :: defined ! Not used in this case
            interface
                subroutine fill( n, x, y ) bind(c)
                    import :: private_plint, private_plflt
                    integer(kind=private_plint), value, intent(in) :: n
                    real(kind=private_plflt), dimension(*), intent(in) :: x, y
                end subroutine fill
            end interface
            interface
                subroutine transform( x, y, tx, ty, data ) bind(c)
                    import :: private_plflt
                    import :: c_ptr
                    implicit none
                    real(kind=private_plflt), value, intent(in) :: x, y
                    real(kind=private_plflt), intent(out) :: tx, ty
                    type(c_ptr), value, intent(in) :: data
                end subroutine transform
            end interface
        end subroutine interface_plshades
    end interface
    private :: interface_plshades

    interface
        subroutine interface_plshades_null( a, nx, ny, defined, xmin, xmax, ymin, ymax, &
               clevel, nlevel, fill_width, cont_color, cont_width, &
               fill, rectangular ) bind(c, name = 'plshades_null' )
            import :: c_ptr, c_funptr, c_null_ptr
            import :: private_plint, private_plbool, private_plflt, PLcGrid
            implicit none
            type(c_ptr), dimension(*), intent(in)  :: a
            integer(kind=private_plint), value, intent(in) :: nx, ny, cont_color, nlevel
            integer(kind=private_plbool), value, intent(in) :: rectangular
            real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax, fill_width
            real(kind=private_plflt), dimension(*), intent(in) :: clevel
            real(kind=private_plflt), value, intent(in) :: cont_width
            type(c_ptr), value, intent(in) :: defined ! Not used in this case
            interface
                subroutine fill( n, x, y ) bind(c)
                    import :: private_plint, private_plflt
                    integer(kind=private_plint), value, intent(in) :: n
                    real(kind=private_plflt), dimension(*), intent(in) :: x, y
                end subroutine fill
            end interface
        end subroutine interface_plshades_null
    end interface
    private :: interface_plshades_null

    interface
        subroutine interface_pltr0( x, y, tx, ty, data ) bind(c, name = 'pltr0' )
            import :: private_plflt, c_ptr
            implicit none
            real(kind=private_plflt), value, intent(in) :: x, y
            real(kind=private_plflt), intent(out) :: tx, ty
            type(c_ptr), value, intent(in) :: data
        end subroutine interface_pltr0
    end interface
    private :: interface_pltr0

    interface
        subroutine interface_pltr1( x, y, tx, ty, data ) bind(c, name = 'pltr1' )
            import :: private_plflt, c_ptr
            implicit none
            real(kind=private_plflt), value, intent(in) :: x, y
            real(kind=private_plflt), intent(out) :: tx, ty
            type(c_ptr), value, intent(in) :: data
        end subroutine interface_pltr1
    end interface
    private :: interface_pltr1

    interface
        subroutine interface_pltr2f( x, y, tx, ty, data ) bind(c, name = 'pltr2f' )
            import :: c_ptr
            import :: private_plflt
            implicit none
            real(kind=private_plflt), value, intent(in) :: x, y
            real(kind=private_plflt), intent(out) :: tx, ty
            type(c_ptr), value, intent(in) :: data
        end subroutine interface_pltr2f
    end interface
    private :: interface_pltr2f

    interface
        subroutine interface_plvect( u, v, nx, ny, scale, transform, data ) bind(c, name = 'c_plvect' )
            import :: c_funptr, c_ptr
            import :: private_plint, private_plflt
            implicit none
            integer(kind=private_plint), value, intent(in) :: nx, ny
            real(kind=private_plflt), value, intent(in) :: scale
            type(c_ptr), dimension(*), intent(in) :: u, v
            type(c_ptr), value, intent(in) :: data
            interface
                subroutine transform( x, y, tx, ty, data ) bind(c)
                    import :: private_plflt
                    import :: c_ptr
                    implicit none
                    real(kind=private_plflt), value, intent(in) :: x, y
                    real(kind=private_plflt), intent(out) :: tx, ty
                    type(c_ptr), value, intent(in) :: data
                end subroutine transform
        end interface
    end subroutine interface_plvect
    end interface
    private :: interface_plvect

    ! Interface blocks for module procedures
    ! These interface blocks are ordered by the names of the module
    ! procedures inside them.  (Recall that the collating sequence has
    ! numbers first, followed by underscores, followed by lower-case
    ! letters.  So "fill3_impl" sorts before "fill_impl".)

    interface pl_setcontlabelparam
        module procedure pl_setcontlabelparam_impl
    end interface pl_setcontlabelparam
    private :: pl_setcontlabelparam_impl

    interface plarc
        module procedure plarc_impl
    end interface plarc
    private :: plarc_impl

    interface plaxes
        module procedure plaxes_impl
    end interface plaxes
    private :: plaxes_impl

    interface plbin
        module procedure plbin_impl
    end interface plbin
    private :: plbin_impl

    interface plbox3
        module procedure plbox3_impl
    end interface plbox3
    private :: plbox3_impl

    interface plbox
        module procedure plbox_impl
    end interface plbox
    private :: plbox_impl

    interface plbtime
        module procedure plbtime_impl
    end interface plbtime
    private :: plbtime_impl

    interface plcalc_world
        module procedure plcalc_world_impl
    end interface plcalc_world
    private :: plcalc_world_impl

    interface plcol1
        module procedure plcol1_impl
    end interface plcol1
    private :: plcol1_impl

    interface plcolorbar
        module procedure plcolorbar_impl
    end interface plcolorbar
    private :: plcolorbar_impl

    interface plconfigtime
        module procedure plconfigtime_impl
    end interface plconfigtime
    private :: plconfigtime_impl

    interface plcont
        module procedure plcont_impl_0
        module procedure plcont_impl_1
        module procedure plcont_impl_2
        module procedure plcont_impl_tr
        module procedure plcont_impl
        module procedure plcont_impl_data
    end interface plcont
    private :: plcont_impl_0
    private :: plcont_impl_1
    private :: plcont_impl_2
    private :: plcont_impl_tr
    private :: plcont_impl
    private :: plcont_impl_data

    interface plctime
        module procedure plctime_impl
    end interface plctime
    private :: plctime_impl

    interface plenv0
        module procedure plenv0_impl
    end interface plenv0
    private :: plenv0_impl

    interface plenv
        module procedure plenv_impl
    end interface plenv
    private :: plenv_impl

    interface plerrx
        module procedure plerrx_impl
    end interface plerrx
    private :: plerrx_impl

    interface plerry
        module procedure plerry_impl
    end interface plerry
    private :: plerry_impl

    interface plfill3
        module procedure plfill3_impl
    end interface plfill3
    private :: plfill3_impl

    interface plfill
        module procedure plfill_impl
    end interface plfill
    private :: plfill_impl

    interface plgchr
        module procedure plgchr_impl
    end interface plgchr
    private :: plgchr_impl

    interface plgcmap1_range
        module procedure plgcmap1_range_impl
    end interface plgcmap1_range
    private :: plgcmap1_range_impl

    interface plgcol0a
        module procedure plgcol0a_impl
    end interface plgcol0a
    private :: plgcol0a_impl

    interface plgcolbga
        module procedure plgcolbga_impl
    end interface plgcolbga
    private :: plgcolbga_impl

    interface plgdidev
        module procedure plgdidev_impl
    end interface plgdidev
    private :: plgdidev_impl

    interface plgdiori
        module procedure plgdiori_impl
    end interface plgdiori
    private :: plgdiori_impl

    interface plgdiplt
        module procedure plgdiplt_impl
    end interface plgdiplt
    private :: plgdiplt_impl

    interface plgpage
        module procedure plgpage_impl
    end interface plgpage
    private :: plgpage_impl

    interface plgradient
        module procedure plgradient_impl
    end interface plgradient
    private :: plgradient_impl

    interface plgriddata
        module procedure plgriddata_impl
    end interface plgriddata
    private :: plgriddata_impl

    interface plgspa
        module procedure plgspa_impl
    end interface plgspa
    private :: plgspa_impl

    interface plgvpd
        module procedure plgvpd_impl
    end interface plgvpd
    private :: plgvpd_impl

    interface plgvpw
        module procedure plgvpw_impl
    end interface plgvpw
    private :: plgvpw_impl

    interface plhist
        module procedure plhist_impl
    end interface plhist
    private :: plhist_impl

    interface plhlsrgb
        module procedure plhlsrgb_impl
    end interface plhlsrgb
    private :: plhlsrgb_impl

    interface plimage
        module procedure plimage_impl
    end interface plimage
    private :: plimage_impl

    interface plimagefr
        module procedure plimagefr_impl_1
        module procedure plimagefr_impl_2
        module procedure plimagefr_impl_null
        module procedure plimagefr_impl_tr
        module procedure plimagefr_impl
        module procedure plimagefr_impl_data
    end interface plimagefr
    private :: plimagefr_impl_1, plimagefr_impl_2, plimagefr_impl_null, plimagefr_impl_tr, plimagefr_impl, plimagefr_impl_data

    interface pljoin
        module procedure pljoin_impl
    end interface pljoin
    private :: pljoin_impl

    interface pllegend
        module procedure pllegend_impl
    end interface pllegend
    private :: pllegend_impl

    interface pllightsource
        module procedure pllightsource_impl
    end interface pllightsource
    private :: pllightsource_impl

    interface plline3
        module procedure plline3_impl
    end interface plline3
    private :: plline3_impl

    interface plline
        module procedure plline_impl
    end interface plline
    private :: plline_impl

    interface plmap
        module procedure plmap_impl
        module procedure plmap_impl_null
    end interface plmap
    private :: plmap_impl
    private :: plmap_impl_null

    interface plmapfill
        module procedure plmapfill_impl
        module procedure plmapfill_impl_null
    end interface plmapfill
    private :: plmapfill_impl
    private :: plmapfill_impl_null

    interface plmapline
        module procedure plmapline_impl
        module procedure plmapline_impl_null
    end interface plmapline
    private :: plmapline_impl
    private :: plmapline_impl_null

    interface plmapstring
        module procedure plmapstring_impl
        module procedure plmapstring_impl_null
    end interface plmapstring
    private :: plmapstring_impl
    private :: plmapstring_impl_null

    interface plmaptex
        module procedure plmaptex_impl
        module procedure plmaptex_impl_null
    end interface plmaptex
    private :: plmaptex_impl
    private :: plmaptex_impl_null

    interface plmeridians
        module procedure plmeridians_impl
        module procedure plmeridians_impl_null
    end interface plmeridians
    private :: plmeridians_impl
    private :: plmeridians_impl_null

    interface plmesh
        module procedure plmesh_impl
    end interface plmesh
    private :: plmesh_impl

    interface plmeshc
        module procedure plmeshc_impl
    end interface plmeshc
    private :: plmeshc_impl

    interface plmtex3
        module procedure plmtex3_impl
    end interface plmtex3
    private :: plmtex3_impl

    interface plmtex
        module procedure plmtex_impl
    end interface plmtex
    private :: plmtex_impl

    interface plot3d
        module procedure plot3d_impl
    end interface plot3d
    private :: plot3d_impl

    interface plot3dc
        module procedure plot3dc_impl
    end interface plot3dc
    private :: plot3dc_impl

    interface plot3dcl
        module procedure plot3dcl_impl
    end interface plot3dcl
    private :: plot3dcl_impl

    interface plpath
        module procedure plpath_impl
    end interface plpath
    private :: plpath_impl

    interface plpoin3
        module procedure plpoin3_impl
    end interface plpoin3
    private :: plpoin3_impl

    interface plpoin
        module procedure plpoin_impl
    end interface plpoin
    private :: plpoin_impl

    interface plpoly3
        module procedure plpoly3_impl
    end interface plpoly3
    private :: plpoly3_impl

    interface plptex3
        module procedure plptex3_impl
    end interface plptex3
    private :: plptex3_impl

    interface plptex
        module procedure plptex_impl
    end interface plptex
    private :: plptex_impl

    interface plrgbhls
        module procedure plrgbhls_impl
    end interface plrgbhls
    private :: plrgbhls_impl

    interface plschr
        module procedure plschr_impl
    end interface plschr
    private :: plschr_impl

    interface plscmap0a
        module procedure plscmap0a_impl
    end interface plscmap0a
    private :: plscmap0a_impl

    interface plscmap1_range
        module procedure plscmap1_range_impl
    end interface plscmap1_range
    private :: plscmap1_range_impl

    interface plscmap1a
        module procedure plscmap1a_impl
    end interface plscmap1a
    private :: plscmap1a_impl

    interface plscmap1l
        module procedure plscmap1l_impl
    end interface plscmap1l
    private :: plscmap1l_impl

    interface plscmap1la
        module procedure plscmap1la_impl
    end interface plscmap1la
    private :: plscmap1la_impl

    interface plscol0a
        module procedure plscol0a_impl
    end interface plscol0a
    private :: plscol0a_impl

    interface plscolbga
        module procedure plscolbga_impl
    end interface plscolbga
    private :: plscolbga_impl

    interface plsdidev
        module procedure plsdidev_impl
    end interface plsdidev
    private :: plsdidev_impl

    interface plsdimap
        module procedure plsdimap_impl
    end interface plsdimap
    private :: plsdimap_impl

    interface plsdiori
        module procedure plsdiori_impl
    end interface plsdiori
    private :: plsdiori_impl

    interface plsdiplt
        module procedure plsdiplt_impl
    end interface plsdiplt
    private :: plsdiplt_impl

    interface plsdiplz
        module procedure plsdiplz_impl
    end interface plsdiplz
    private :: plsdiplz_impl

    interface plshade
        module procedure plshade_impl_0
        module procedure plshade_impl_1
        module procedure plshade_impl_2
        module procedure plshade_impl_tr
        module procedure plshade_impl
        module procedure plshade_impl_data
    end interface plshade
    private :: plshade_impl_0
    private :: plshade_impl_1
    private :: plshade_impl_2
    private :: plshade_impl_tr
    private :: plshade_impl
    private :: plshade_impl_data

    interface plshades
        module procedure plshades_impl_0
        module procedure plshades_impl_1
        module procedure plshades_impl_2
        module procedure plshades_impl_tr
        module procedure plshades_impl
        module procedure plshades_impl_data
    end interface plshades
    private :: plshades_impl_0
    private :: plshades_impl_1
    private :: plshades_impl_2
    private :: plshades_impl_tr
    private :: plshades_impl
    private :: plshades_impl_data

    interface plsmaj
        module procedure plsmaj_impl
    end interface plsmaj
    private :: plsmaj_impl

    interface plsmin
        module procedure plsmin_impl
    end interface plsmin
    private :: plsmin_impl

    interface plspage
        module procedure plspage_impl
    end interface plspage
    private :: plspage_impl

    interface plssym
        module procedure plssym_impl
    end interface plssym
    private :: plssym_impl

    interface plstring3
        module procedure plstring3_impl
    end interface plstring3
    private :: plstring3_impl

    interface plstring
        module procedure plstring_impl
    end interface plstring
    private :: plstring_impl

    interface plstripa
        module procedure plstripa_impl
    end interface plstripa
    private :: plstripa_impl

    interface plstripc
        module procedure plstripc_impl
    end interface plstripc
    private :: plstripc_impl

    interface plsurf3d
        module procedure plsurf3d_impl
    end interface plsurf3d
    private :: plsurf3d_impl

    interface plsurf3dl
        module procedure plsurf3dl_impl
    end interface plsurf3dl
    private :: plsurf3dl_impl

    interface plsvpa
        module procedure plsvpa_impl
    end interface plsvpa
    private :: plsvpa_impl

    ! Another variant defined in the plplot module.
    interface plsvect
        module procedure plsvect_impl
    end interface plsvect
    private :: plsvect_impl

    interface plsym
        module procedure plsym_impl
    end interface plsym
    private :: plsym_impl

    interface plvasp
        module procedure plvasp_impl
    end interface plvasp
    private :: plvasp_impl

    interface plvect
        module procedure plvect_impl_0
        module procedure plvect_impl_1
        module procedure plvect_impl_2
        module procedure plvect_impl_tr
        module procedure plvect_impl
        module procedure plvect_impl_data
    end interface plvect
    private :: plvect_impl_0
    private :: plvect_impl_1
    private :: plvect_impl_2
    private :: plvect_impl_tr
    private :: plvect_impl
    private :: plvect_impl_data

    interface plvpas
        module procedure plvpas_impl
    end interface plvpas
    private :: plvpas_impl

    interface plvpor
        module procedure plvpor_impl
    end interface plvpor
    private :: plvpor_impl

    interface plw3d
        module procedure plw3d_impl
    end interface plw3d
    private :: plw3d_impl

    interface plwidth
        module procedure plwidth_impl
    end interface plwidth
    private :: plwidth_impl

    interface plwind
        module procedure plwind_impl
    end interface plwind
    private :: plwind_impl

contains

    ! Private utility routine that depends on real precision:
    subroutine matrix_to_c( array, carray, caddress )
        real(kind=wp), dimension(:,:), intent(in) :: array
        real(kind=private_plflt), dimension(:,:), allocatable, target, intent(out) :: carray

        type(c_ptr), dimension(:), allocatable, intent(out) :: caddress

        integer :: i_local

        allocate( carray(size(array,2),size(array,1)) )
        allocate( caddress(size(array,1)) )

        carray = transpose( array )

        do i_local = 1,size(array,1)
            caddress(i_local) = c_loc(carray(1,i_local))
        enddo
    end subroutine matrix_to_c

    ! Module procedures:

    subroutine pl_setcontlabelparam_impl( offset, size, spacing, active )
        real(kind=wp), intent(in) :: offset, size, spacing
        integer, intent(in) :: active

        interface
            subroutine interface_pl_setcontlabelparam( offset, size, spacing, active) bind(c,name='c_pl_setcontlabelparam')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: active
                real(kind=private_plflt), value, intent(in) :: offset, size, spacing
            end subroutine interface_pl_setcontlabelparam
        end interface

        call interface_pl_setcontlabelparam( real(offset,kind=private_plflt), real(size,kind=private_plflt), &
               real(spacing,kind=private_plflt), int(active,kind=private_plint) )
    end subroutine pl_setcontlabelparam_impl

    subroutine plarc_impl( x, y, a, b, angle1, angle2, rotate, fill )
        real(kind=wp), intent(in) :: x, y, a, b, angle1, angle2, rotate
        logical, intent(in) :: fill

        interface
            subroutine interface_plarc( x, y, a, b, angle1, angle2, rotate, fill ) bind(c,name='c_plarc')
                import :: private_plbool, private_plflt
                implicit none
                integer(kind=private_plbool), value, intent(in) :: fill
                real(kind=private_plflt), value, intent(in) :: x, y, a, b, angle1, angle2, rotate
            end subroutine interface_plarc
        end interface

        call interface_plarc( real(x,kind=private_plflt), real(y,kind=private_plflt), real(a,kind=private_plflt), &
               real(b,kind=private_plflt), real(angle1,kind=private_plflt), real(angle2,kind=private_plflt), &
               real(rotate,kind=private_plflt), int(merge(1,0,fill),kind=private_plbool) )
    end subroutine plarc_impl

    subroutine plaxes_impl(x0, y0, xopt,xtick,nxsub,yopt,ytick,nysub)
        real(kind=wp), intent(in) :: x0, y0, xtick, ytick
        integer, intent(in) :: nxsub, nysub
        character*(*), intent(in) :: xopt,yopt

        interface
            subroutine interface_plaxes(x0, y0, xopt,xtick,nxsub,yopt,ytick,nysub) bind(c,name='c_plaxes')
                import :: private_plint, private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: x0, y0, xtick, ytick
                integer(kind=private_plint), value, intent(in) :: nxsub, nysub
                character(len=1), dimension(*), intent(in) :: xopt, yopt
            end subroutine interface_plaxes
        end interface

        call interface_plaxes( &
               real(x0,kind=private_plflt), real(y0,kind=private_plflt), &
               trim(xopt)//c_null_char, real(xtick,kind=private_plflt), int(nxsub,kind=private_plint), &
               trim(yopt)//c_null_char, real(ytick,kind=private_plflt), int(nysub,kind=private_plint) )
    end subroutine plaxes_impl

    subroutine plbin_impl( x, y, center )
        real(kind=wp), dimension(:), intent(in) :: x, y
        integer, intent(in) :: center

        integer(kind=private_plint) :: sz_local

        interface
            subroutine interface_plbin( sz, x, y, center ) bind(c,name='c_plbin')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: sz, center
                real(kind=private_plflt), dimension(*), intent(in) :: x, y
            end subroutine interface_plbin
        end interface

        sz_local = size(x,kind=private_plint)
        if( sz_local /= size(y,kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plbin: inconsistent sizes for x and y"
        end if
        call interface_plbin( sz_local, real(x,kind=private_plflt), real(y,kind=private_plflt), &
               int(center,kind=private_plint) )
    end subroutine plbin_impl

    subroutine plbox3_impl(xopt,xlabel,xtick,nxsub,yopt,ylabel,ytick,nysub, &
           zopt,zlabel,ztick,nzsub)

        real(kind=wp), intent(in) :: xtick, ytick, ztick
        character*(*), intent(in) :: xopt, xlabel, yopt, ylabel, zopt, zlabel
        integer, intent(in) :: nxsub, nysub, nzsub

        interface
            subroutine interface_plbox3(xopt,xlabel,xtick,nxsub,yopt,ylabel,ytick,nysub, &
                   zopt,zlabel,ztick,nzsub) bind(c,name='c_plbox3')
                import :: private_plint, private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in):: xtick, ytick, ztick
                integer(kind=private_plint), value, intent(in) :: nxsub, nysub, nzsub
                character(len=1), dimension(*), intent(in) :: xopt, yopt, zopt, xlabel, ylabel, zlabel
            end subroutine interface_plbox3
        end interface

        call interface_plbox3( trim(xopt)//c_null_char, trim(xlabel)//c_null_char, real(xtick,kind=private_plflt), &
               int(nxsub,kind=private_plint),                                                      &
               trim(yopt)//c_null_char, trim(ylabel)//c_null_char, real(ytick,kind=private_plflt), &
               int(nysub,kind=private_plint),                                                      &
               trim(zopt)//c_null_char, trim(zlabel)//c_null_char, real(ztick,kind=private_plflt), &
               int(nzsub,kind=private_plint) )
    end subroutine plbox3_impl

    subroutine plbox_impl(xopt,xtick,nxsub,yopt,ytick,nysub)
        real(kind=wp), intent(in) :: xtick, ytick
        integer, intent(in) :: nxsub, nysub
        character*(*), intent(in) :: xopt,yopt

        interface
            subroutine interface_plbox(xopt,xtick,nxsub,yopt,ytick,nysub) bind(c,name='c_plbox')
                import :: private_plint, private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xtick, ytick
                integer(kind=private_plint), value, intent(in) :: nxsub, nysub
                character(len=1), dimension(*), intent(in) :: xopt, yopt
            end subroutine interface_plbox
        end interface

        call interface_plbox( trim(xopt)//c_null_char, real(xtick,kind=private_plflt), int(nxsub,kind=private_plint), &
               trim(yopt)//c_null_char, real(ytick,kind=private_plflt), int(nysub,kind=private_plint) )
    end subroutine plbox_impl

    subroutine plbtime_impl( year, month, day, hour, min, sec, ctime )
        real(kind=wp), intent(in) :: ctime
        integer, intent(out) :: year, month, day, hour, min
        real(kind=wp), intent(out) :: sec

        integer(kind=private_plint) :: year_out, month_out, day_out, hour_out, min_out
        real(kind=private_plflt) :: sec_out

        interface
            subroutine interface_plbtime( year, month, day, hour, min, sec, ctime ) bind(c,name='c_plbtime')
                import :: private_plint, private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: ctime
                integer(kind=private_plint), intent(out) :: year, month, day, hour, min
                real(kind=private_plflt), intent(out) :: sec
            end subroutine interface_plbtime
        end interface

        call interface_plbtime( year_out, month_out, day_out, hour_out, min_out, sec_out, real(ctime, kind=private_plflt))
        year = int(year_out)
        month = int(month_out)
        day = int(day_out)
        hour = int(hour_out)
        min = int(min_out)
        sec = real(sec_out, kind=wp)

    end subroutine plbtime_impl

    subroutine plcalc_world_impl( rx, ry, wx, wy, window )
        integer, intent(out) :: window
        real(kind=wp), intent(in) :: rx, ry
        real(kind=wp), intent(out) :: wx, wy

        real(kind=private_plflt) :: wx_out, wy_out
        integer(kind=private_plint) window_out


        interface
            subroutine interface_plcalc_world( rx, ry, wx, wy, window ) bind(c,name='c_plcalc_world')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), intent(out) :: window
                real(kind=private_plflt), value, intent(in) :: rx, ry
                real(kind=private_plflt), intent(out) :: wx, wy
            end subroutine interface_plcalc_world
        end interface

        call interface_plcalc_world( real(rx,kind=private_plflt), real(ry,kind=private_plflt), wx_out, wy_out, window_out )
        window = int(window_out)
        wx = real(wx_out, kind=wp)
        wy = real(wy_out, kind=wp)

    end subroutine plcalc_world_impl

    subroutine plcol1_impl( col )
        real(kind=wp), intent(in) :: col

        interface
            subroutine interface_plcol1( col ) bind(c,name='c_plcol1')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: col
            end subroutine interface_plcol1
        end interface

        call interface_plcol1( real(col,kind=private_plflt) )
    end subroutine plcol1_impl

    subroutine plcolorbar_impl( &
           colorbar_width, colorbar_height, &
           opt, position, x, y, &
           x_length, y_length, bg_color, bb_color, bb_style, &
           low_cap_color, high_cap_color, &
           cont_color, cont_width, &
           label_opts, labels, &
           axis_opts, ticks, sub_ticks, n_values, values )


        real(kind=wp), intent(in) :: x_length, y_length, x, y, low_cap_color, high_cap_color, cont_width
        real(kind=wp), dimension(:, :), intent(in) :: values
        integer, intent(in) :: position, opt, bg_color, bb_color, bb_style, cont_color
        integer, dimension(:), intent(in) :: label_opts, sub_ticks, n_values
        real(kind=wp), dimension(:), intent(in) :: ticks
        character(len=*), dimension(:), intent(in) :: labels, axis_opts
        real(kind=wp), intent(out) :: colorbar_width, colorbar_height

        integer :: n_labels_local, n_axes_local
        real(kind=private_plflt) :: colorbar_width_out, colorbar_height_out
        real(kind=private_plflt), dimension(:,:), allocatable :: values_c_local
        type(c_ptr), dimension(:), allocatable :: values_address_local
        character(len=1), dimension(:,:), allocatable :: cstring_labels_local, cstring_axis_opts_local
        type(c_ptr), dimension(:), allocatable :: cstring_address_labels_local, cstring_address_axis_opts_local

        interface
            subroutine interface_plcolorbar( &
                   colorbar_width, colorbar_height, &
                   opt, position, x, y, &
                   x_length, y_length, bg_color, bb_color, bb_style, &
                   low_cap_color, high_cap_color, &
                   cont_color, cont_width, &
                   n_labels, label_opts, labels, &
                   n_axes, axis_opts, ticks, sub_ticks, n_values, values ) &
                   bind(c,name='c_plcolorbar')

                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none

                real(kind=private_plflt), value, intent(in) :: x_length, y_length, x, y, &
                       low_cap_color, high_cap_color, cont_width
                integer(kind=private_plint), value, intent(in) :: position, opt, bg_color, bb_color, bb_style, cont_color
                integer(kind=private_plint), value, intent(in) :: n_labels, n_axes
                real(kind=private_plflt), dimension(*), intent(in) :: ticks
                integer(kind=private_plint), dimension(*), intent(in) :: label_opts, sub_ticks, n_values
                type(c_ptr), dimension(*), intent(in) :: values

                type(c_ptr), dimension(*), intent(in) :: labels, axis_opts
                real(kind=private_plflt), intent(out) :: colorbar_width, colorbar_height

            end subroutine interface_plcolorbar
        end interface


        ! Determine number of label entries and demand consistent
        ! array sizes.

        n_labels_local = size(label_opts)
        if( n_labels_local /= size(labels) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plcolorbar: inconsistent sizes for the following arrays:"
            write(error_unit, "(a)") "label_opts"
            write(error_unit, "(a)") "labels"
        end if

        n_axes_local = size(axis_opts)
        if( &
               n_axes_local /= size(ticks) .or. &
               n_axes_local /= size(sub_ticks) .or. &
               n_axes_local /= size(n_values) .or. &
               n_axes_local /= size(values,1) &
               ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plcolorbar: inconsistent sizes for the following arrays:"
            write(error_unit, "(a)") "axis_opts"
            write(error_unit, "(a)") "ticks"
            write(error_unit, "(a)") "sub_ticks"
            write(error_unit, "(a)") "n_values"
            write(error_unit, "(a)") "first dimension of values"
        end if

        if(maxval(n_values) > size(values,2) ) then
            write(error_unit, "(a)") "Plplot Fortran Severe Warning: plcolorbar: maximum of n_values > second dimension of values"
            return
        end if

        call matrix_to_c( values, values_c_local, values_address_local )

        call character_array_to_c( cstring_labels_local, cstring_address_labels_local, labels )
        call character_array_to_c( cstring_axis_opts_local, cstring_address_axis_opts_local, axis_opts )

        call interface_plcolorbar( &
               colorbar_width_out, colorbar_height_out, &
               int(opt,kind=private_plint), int(position,kind=private_plint), &
               real(x,kind=private_plflt), real(y,kind=private_plflt), &
               real(x_length,kind=private_plflt), real(y_length,kind=private_plflt), &
               int(bg_color,kind=private_plint), &
               int(bb_color,kind=private_plint), int(bb_style,kind=private_plint), &
               real(low_cap_color,kind=private_plflt), real(high_cap_color,kind=private_plflt), &
               int(cont_color,kind=private_plint), real(cont_width,kind=private_plflt), &
               int(n_labels_local, kind=private_plint), int(label_opts, kind=private_plint), &
               cstring_address_labels_local, &
               int(n_axes_local, kind=private_plint), &
               cstring_address_axis_opts_local, real(ticks, kind=private_plflt), int(sub_ticks, kind=private_plint), &
               int(n_values, kind=private_plint), values_address_local &
               )
        colorbar_width = real(colorbar_width_out, kind=wp)
        colorbar_height = real(colorbar_height_out, kind=wp)

    end subroutine plcolorbar_impl

    subroutine plconfigtime_impl( scale, offset1, offset2, ccontrol, ifbtime_offset, year, month, day, hour, min, sec )
        integer, intent(in) :: ccontrol, year, month, day, hour, min
        logical, intent(in) :: ifbtime_offset
        real(kind=wp), intent(in) :: scale, offset1, offset2, sec

        interface
            subroutine interface_plconfigtime( scale, offset1, offset2, ccontrol, ifbtime_offset, &
                   year, month, day, hour, min, sec) bind(c,name='c_plconfigtime')
                import :: private_plint, private_plbool, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: ccontrol, year, month, day, hour, min
                integer(kind=private_plbool), value, intent(in) :: ifbtime_offset
                real(kind=private_plflt), value, intent(in) :: scale, offset1, offset2, sec
            end subroutine interface_plconfigtime
        end interface

        call interface_plconfigtime( &
               real(scale, kind=private_plflt), real(offset1, kind=private_plflt), real(offset2, kind=private_plflt), &
               int(ccontrol, kind=private_plint), int(merge(1,0,ifbtime_offset),kind=private_plbool), &
               int(year, kind=private_plint), int(month, kind=private_plint), int(day, kind=private_plint), &
               int(hour, kind=private_plint), int(min, kind=private_plint), real(sec, kind=private_plflt) )
    end subroutine plconfigtime_impl

    subroutine plcont_impl_0( z, kx, lx, ky, ly, clevel )
        integer, intent(in) :: kx, lx, ky, ly
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), dimension(:), intent(in) :: clevel

        real(kind=private_plflt), dimension(:,:), allocatable, target :: z_in
        type(PLfGrid), target :: fgrid_local

        allocate( z_in(size(z,1),size(z,2)) )
        z_in = z
        fgrid_local%f  = c_loc(z_in)
        fgrid_local%nx = size(z_in,1)
        fgrid_local%ny = size(z_in,2)

        call interface_plfcont( interface_plf2evalr, c_loc(fgrid_local), size(z,1,kind=private_plint), &
               size(z,2,kind=private_plint), kx, lx, ky, ly, real(clevel, kind=private_plflt), &
               size(clevel,kind=private_plint), interface_pltr0, c_null_ptr )
        deallocate(z_in)
    end subroutine plcont_impl_0

    subroutine plcont_impl_1( z, kx, lx, ky, ly, clevel, xg, yg )
        integer, intent(in) :: kx, lx, ky, ly
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), dimension(:), intent(in) :: clevel
        real(kind=wp), dimension(:), intent(in) :: xg, yg

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: z_in
        real(kind=private_plflt), dimension(:), allocatable, target :: xg_in, yg_in
        type(PLfGrid), target :: fgrid_local
        type(PLcGrid), target :: cgrid_local

        nx_in = size(z,1, kind=private_plint)
        ny_in = size(z,2, kind=private_plint)
        if(nx_in /= size(xg, kind=private_plint) .or. ny_in /= size(yg, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plcont: inconsistent sizes for z, xg, and/or yg"
        end if

        allocate( z_in(nx_in, ny_in) )
        z_in = z
        fgrid_local%f  = c_loc(z_in)
        fgrid_local%nx = nx_in
        fgrid_local%ny = ny_in

        allocate( xg_in(nx_in), yg_in(ny_in) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in
        cgrid_local%ny = ny_in
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call interface_plfcont( interface_plf2evalr, c_loc(fgrid_local), nx_in, ny_in, &
               kx, lx, ky, ly, real(clevel, kind=private_plflt), size(clevel,kind=private_plint), &
               interface_pltr1, c_loc(cgrid_local) )
    end subroutine plcont_impl_1

    subroutine plcont_impl_2( z, kx, lx, ky, ly, clevel, xg, yg )
        integer, intent(in) :: kx, lx, ky, ly
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), dimension(:), intent(in) :: clevel
        real(kind=wp), dimension(:,:), intent(in) :: xg, yg

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: z_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: xg_in, yg_in
        type(PLfGrid), target :: fgrid_local
        type(PLcGrid), target :: cgrid_local

        nx_in = size(z,1, kind=private_plint)
        ny_in = size(z,2, kind=private_plint)
        if( &
               nx_in /= size(xg, 1, kind=private_plint) .or. ny_in /= size(xg, 2, kind=private_plint) .or. &
               nx_in /= size(yg, 1, kind=private_plint) .or. ny_in /= size(xg, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plcont: inconsistent sizes for z, xg and/or yg"
        end if

        allocate( z_in(nx_in, ny_in) )
        z_in = z
        fgrid_local%f  = c_loc(z_in)
        fgrid_local%nx = nx_in
        fgrid_local%ny = ny_in

        allocate( xg_in(nx_in,ny_in), yg_in(nx_in,ny_in) )
        xg_in = xg
        yg_in = yg

        cgrid_local%nx = nx_in
        cgrid_local%ny = ny_in
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call interface_plfcont( interface_plf2evalr, c_loc(fgrid_local), nx_in, ny_in, &
               kx, lx, ky, ly, real(clevel, kind=private_plflt), size(clevel,kind=private_plint), &
               interface_pltr2f, c_loc(cgrid_local) )
    end subroutine plcont_impl_2

    subroutine plcont_impl_tr( z, kx, lx, ky, ly, clevel, tr )
        integer, intent(in) :: kx, lx, ky, ly
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), dimension(:), intent(in) :: clevel
        real(kind=wp), dimension(:), intent(in) :: tr

        real(kind=private_plflt), dimension(6), target :: tr_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: z_in
        type(PLfGrid), target :: fgrid_local

        allocate( z_in(size(z,1),size(z,2)) )
        z_in = z
        fgrid_local%f  = c_loc(z_in)
        fgrid_local%nx = size(z,1)
        fgrid_local%ny = size(z,2)

        tr_in = tr(1:6)

        call interface_plfcont( interface_plf2evalr, c_loc(fgrid_local), size(z,1,kind=private_plint), &
               size(z,2,kind=private_plint), kx, lx, ky, ly, real(clevel, kind=private_plflt), &
               size(clevel,kind=private_plint), plplot_private_pltr, c_loc(tr_in) )
    end subroutine plcont_impl_tr

    subroutine plcont_impl( z, kx, lx, ky, ly, clevel, proc )
        integer, intent(in) :: kx, lx, ky, ly
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), dimension(:), intent(in) :: clevel
        procedure(pltransform_proc) :: proc

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local

        call matrix_to_c( z, z_c_local, z_address_local )
        pltransform => proc

        call interface_plcont( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), &
               kx, lx, ky, ly, real(clevel, kind=private_plflt), size(clevel,kind=private_plint), &
               pltransformf2c, c_null_ptr )
    end subroutine plcont_impl

    subroutine plcont_impl_data( z, kx, lx, ky, ly, clevel, proc, data )
        integer, intent(in) :: kx, lx, ky, ly
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), dimension(:), intent(in) :: clevel
        procedure(pltransform_proc_data) :: proc
        type(c_ptr), intent(in) :: data

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local

        call matrix_to_c( z, z_c_local, z_address_local )
        pltransform_data => proc

        call interface_plcont( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), &
               kx, lx, ky, ly, real(clevel, kind=private_plflt), size(clevel,kind=private_plint), &
               pltransformf2c_data, data )
    end subroutine plcont_impl_data

    subroutine plctime_impl( year, month, day, hour, min, sec, ctime )
        integer, intent(in) :: year, month, day, hour, min
        real(kind=wp), intent(in) :: sec
        real(kind=wp), intent(out) :: ctime

        real(kind=private_plflt) :: ctime_out

        interface
            subroutine interface_plctime( year, month, day, hour, min, sec, ctime ) bind(c,name='c_plctime')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: year, month, day, hour, min
                real(kind=private_plflt), value, intent(in) :: sec
                real(kind=private_plflt), intent(out) :: ctime
            end subroutine interface_plctime
        end interface

        call interface_plctime( &
               int(year, kind=private_plint), int(month, kind=private_plint), int(day, kind=private_plint), &
               int(hour, kind=private_plint), int(min, kind=private_plint), real(sec, kind=private_plflt), ctime_out )
        ! Transform output real values.
        ctime = real(ctime_out, kind=wp)

    end subroutine plctime_impl

    subroutine plenv0_impl( xmin, xmax, ymin, ymax, just, axis )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        integer, intent(in) :: just, axis

        interface
            subroutine interface_plenv0( xmin, xmax, ymin, ymax, just, axis ) bind(c, name='c_plenv0')
                import :: private_plint, private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax
                integer(kind=private_plint), value, intent(in) :: just, axis
            end subroutine interface_plenv0
        end interface

        call interface_plenv0( real(xmin,private_plflt), real(xmax,private_plflt), &
               real(ymin,private_plflt), real(ymax,private_plflt), &
               int(just,private_plint), int(axis,private_plint) )
    end subroutine plenv0_impl

    subroutine plenv_impl( xmin, xmax, ymin, ymax, just, axis )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        integer, intent(in) :: just, axis

        interface
            subroutine interface_plenv( xmin, xmax, ymin, ymax, just, axis ) bind(c, name='c_plenv')
                import :: private_plint, private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax
                integer(kind=private_plint), value, intent(in) :: just, axis
            end subroutine interface_plenv
        end interface

        call interface_plenv( real(xmin,private_plflt), real(xmax,private_plflt), &
               real(ymin,private_plflt), real(ymax,private_plflt), &
               int(just,private_plint), int(axis,private_plint) )
    end subroutine plenv_impl

    subroutine plerrx_impl( xmin, xmax, y )
        real(kind=wp), dimension(:), intent(in) :: xmin, xmax, y

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plerrx( n, xmin, xmax, y ) bind( c, name='c_plerrx')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                real(kind=private_plflt), dimension(*), intent(in) :: xmin, xmax, y
            end subroutine interface_plerrx
        end interface

        n_local = size(y,kind=private_plint)
        if( n_local /= size(xmin, kind=private_plint) .or. n_local /= size(xmax, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plerrx: inconsistent sizes for xmin, xmax, and/or y"
        end if

        call interface_plerrx( n_local, real(xmin,private_plflt), real(xmax,private_plflt), real(y,private_plflt) )
    end subroutine plerrx_impl

    subroutine plerry_impl( x, ymin, ymax )
        real(kind=wp), dimension(:), intent(in) :: x, ymin, ymax

        integer(kind=private_plint) n_local

        interface
            subroutine interface_plerry( n, x, ymin, ymax ) bind( c, name='c_plerry')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                real(kind=private_plflt), dimension(*), intent(in) :: x, ymin, ymax
            end subroutine interface_plerry
        end interface

        n_local = size(x,kind=private_plint)
        if( n_local /= size(ymin, kind=private_plint) .or. n_local /= size(ymax, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plerry: inconsistent sizes for x, ymin, and/or ymax"
        end if

        call interface_plerry( n_local, real(x,private_plflt), real(ymin,private_plflt), real(ymax,private_plflt) )
    end subroutine plerry_impl

    subroutine plfill3_impl( x, y, z )
        real(kind=wp), dimension(:), intent(in) :: x, y, z

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plfill3( n, x, y, z ) bind( c, name='c_plfill3')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, z
            end subroutine interface_plfill3
        end interface

        n_local = size(x,kind=private_plint)
        if( n_local /= size(y, kind=private_plint) .or. n_local /= size(z, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plfill3: inconsistent sizes for x, y, and/or z"
        end if

        call interface_plfill3( n_local, &
               real(x,private_plflt), real(y,private_plflt) , real(z,private_plflt) )
    end subroutine plfill3_impl

    subroutine plfill_impl( x, y )
        real(kind=wp), dimension(:), intent(in) :: x, y

        integer(kind=private_plint) :: n_local

        n_local = size(x,kind=private_plint)
        if( n_local /= size(y, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plfill: inconsistent sizes for x and y"
        end if

        call interface_plfill( n_local, real(x,private_plflt), real(y,private_plflt) )
    end subroutine plfill_impl

    subroutine plgchr_impl( chrdef, chrht )
        real(kind=wp), intent(out) :: chrdef, chrht

        real(kind=private_plflt) :: chrdef_out, chrht_out

        interface
            subroutine interface_plgchr( chrdef, chrht ) bind(c,name='c_plgchr')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), intent(out) :: chrdef, chrht
            end subroutine interface_plgchr
        end interface

        call interface_plgchr( chrdef_out, chrht_out )
        chrdef = real(chrdef_out, kind=wp)
        chrht = real(chrht_out, kind=wp)

    end subroutine plgchr_impl

    subroutine plgcmap1_range_impl( min_color, max_color )
        real(kind=wp), intent(out) :: min_color, max_color

        real(kind=private_plflt) :: min_color_out, max_color_out

        interface
            subroutine interface_plgcmap1_range( min_color, max_color ) bind(c,name='c_plgcmap1_range')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), intent(out) :: min_color, max_color
            end subroutine interface_plgcmap1_range
        end interface

        call interface_plgcmap1_range( min_color_out, max_color_out )
        min_color = real(min_color_out, kind=wp)
        max_color = real(max_color_out, kind=wp)

    end subroutine plgcmap1_range_impl

    subroutine plgcol0a_impl( icol, r, g, b, a )
        integer, intent(in) :: icol
        integer, intent(out) :: r, g, b
        real(kind=wp), intent(out) :: a

        integer(kind=private_plint) :: r_out, g_out, b_out
        real(kind=private_plflt) :: a_out

        interface
            subroutine interface_plgcol0a( icol, r, g, b, a ) bind(c,name='c_plgcol0a')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: icol
                integer(kind=private_plint), intent(out) :: r, g, b
                real(kind=private_plflt), intent(out) :: a
            end subroutine interface_plgcol0a
        end interface

        call interface_plgcol0a( int(icol,kind=private_plint), r_out, g_out, b_out, a_out )
        r = int(r_out)
        g = int(g_out)
        b = int(b_out)
        a = real(a_out, kind=private_plflt)
    end subroutine plgcol0a_impl

    subroutine plgcolbga_impl( r, g, b, a )
        integer, intent(out) :: r, g, b
        real(kind=wp), intent(out) :: a

        integer(kind=private_plint) :: r_out, g_out, b_out
        real(kind=private_plflt) :: a_out

        interface
            subroutine interface_plgcolbga( r, g, b, a ) bind(c,name='c_plgcolbga')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), intent(out) :: r, g, b
                real(kind=private_plflt), intent(out) :: a
            end subroutine interface_plgcolbga
        end interface

        call interface_plgcolbga( r_out, g_out, b_out, a_out )
        r = int(r_out)
        g = int(g_out)
        b = int(b_out)
        a = real(a_out, kind=private_plflt)
    end subroutine plgcolbga_impl

    subroutine plgdidev_impl( mar, aspect, jx, jy )
        real(kind=wp), intent(out) :: mar, aspect, jx, jy

        real(kind=private_plflt) :: mar_out, aspect_out, jx_out, jy_out

        interface
            subroutine interface_plgdidev( mar, aspect, jx, jy ) bind(c,name='c_plgdidev')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), intent(out) :: mar, aspect, jx, jy
            end subroutine interface_plgdidev
        end interface

        call interface_plgdidev( mar_out, aspect_out, jx_out, jy_out )
        mar    = real(mar_out, kind=wp)
        aspect = real(aspect_out, kind=wp)
        jx     = real(jx_out, kind=wp)
        jy     = real(jy_out, kind=wp)
    end subroutine plgdidev_impl

    subroutine plgdiori_impl( rot )
        real(kind=wp), intent(out) :: rot

        real(kind=private_plflt) :: rot_out

        interface
            subroutine interface_plgdiori( rot ) bind(c,name='c_plgdiori')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), intent(out) :: rot
            end subroutine interface_plgdiori
        end interface

        call interface_plgdiori( rot_out )
        rot = real(rot_out, kind=wp)
    end subroutine plgdiori_impl

    subroutine plgdiplt_impl( xmin, xmax, ymin, ymax )
        real(kind=wp), intent(out) :: xmin, xmax, ymin, ymax

        real(kind=private_plflt) :: xmin_out, xmax_out, ymin_out, ymax_out

        interface
            subroutine interface_plgdiplt( xmin, xmax, ymin, ymax ) bind(c,name='c_plgdiplt')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), intent(out) :: xmin, xmax, ymin, ymax
            end subroutine interface_plgdiplt
        end interface

        call interface_plgdiplt( xmin_out, xmax_out, ymin_out, ymax_out )
        xmin = real(xmin_out, kind=wp)
        xmax = real(xmax_out, kind=wp)
        ymin = real(ymin_out, kind=wp)
        ymax = real(ymax_out, kind=wp)
    end subroutine plgdiplt_impl

    subroutine plgpage_impl( xpmm, ypmm, xwid, ywid, xoff, yoff )
        integer, intent(out) :: xwid, ywid, xoff, yoff
        real(kind=wp), intent(out) :: xpmm, ypmm

        integer(kind=private_plint) :: xwid_out, ywid_out, xoff_out, yoff_out
        real(kind=private_plflt) :: xpmm_out, ypmm_out

        interface
            subroutine interface_plgpage( xpmm, ypmm, xwid, ywid, xoff, yoff ) bind(c,name='c_plgpage')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), intent(out) :: xwid, ywid, xoff, yoff
                real(kind=private_plflt), intent(out) :: xpmm, ypmm
            end subroutine interface_plgpage
        end interface

        call interface_plgpage( xpmm_out, ypmm_out, xwid_out, ywid_out, xoff_out, yoff_out )
        xwid = int(xwid_out)
        ywid = int(ywid_out)
        xoff = int(xoff_out)
        yoff = int(yoff_out)
        xpmm = real(xpmm_out, kind=wp)
        ypmm = real(ypmm_out, kind=wp)
    end subroutine plgpage_impl

    subroutine plgradient_impl( x, y, angle )
        real(kind=wp), dimension(:), intent(in) :: x, y
        real(kind=wp), intent(in) :: angle

        integer(kind=private_plint) :: sz_local

        interface
            subroutine interface_plgradient( sz, x, y, angle ) bind(c,name='c_plgradient')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: sz
                real(kind=private_plflt), dimension(*), intent(in) :: x, y
                real(kind=private_plflt), value, intent(in) :: angle
            end subroutine interface_plgradient
        end interface

        sz_local = size(x,kind=private_plint)
        if( sz_local /= size(y, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plgradient: inconsistent sizes for x and y"
        end if

        call interface_plgradient( sz_local, &
               real(x,kind=private_plflt), real(y,kind=private_plflt), &
               real(angle,kind=private_plflt) )
    end subroutine plgradient_impl

    subroutine plgriddata_impl( x, y, z, xg, yg, zg, type, data )
        integer, intent(in) :: type
        real(kind=wp), intent(in) :: data
        real(kind=wp), dimension(:), intent(in) :: x, y, z, xg, yg
        real(kind=wp), dimension(:, :), intent(out) :: zg

        real(kind=private_plflt), dimension(:,:), allocatable, target :: transpose_local
        type(c_ptr), dimension(:), allocatable :: transpose_address_local
        integer(kind=private_plint) :: npts_local, nptsx_local, nptsy_local
        integer :: i_local

        interface
            subroutine interface_plgriddata( x, y, z, npts, &
                   xg, nptsx, yg, nptsy, zg, type, data ) bind(c,name='c_plgriddata')
                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: npts, nptsx, nptsy, type
                real(kind=private_plflt), value, intent(in) :: data
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, z, xg, yg
                type(c_ptr), dimension(*), intent(out) :: zg
            end subroutine interface_plgriddata
        end interface

        npts_local = size(x, kind=private_plint)

        if( &
               npts_local /= size(y, kind=private_plint) .or. &
               npts_local /= size(z, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plgriddata: inconsistent sizes for x, y, and/or z"
        end if

        nptsx_local = size(xg, kind=private_plint)
        nptsy_local = size(yg, kind=private_plint)

        if( &
               nptsx_local /= size(zg, 1, kind=private_plint) .or. &
               nptsy_local /= size(zg, 2, kind=private_plint) ) then
            write(error_unit, "(a)") &
                   "Plplot Fortran Warning: plgriddata: inconsistent sizes for "// &
                   "xg and first dimension of zg or yg and second dimension of zg"
        end if

        ! Prepare array areas to be written to by C version of plgriddata
        ! following relevant parts of code in matrix_to_c.
        allocate( transpose_local(nptsy_local, nptsx_local) )
        allocate( transpose_address_local(nptsx_local) )
        do i_local = 1, nptsx_local
            transpose_address_local(i_local) = c_loc(transpose_local(1,i_local))
        enddo

        call interface_plgriddata( &
               real(x,kind=private_plflt), real(y,kind=private_plflt), real(z,kind=private_plflt), npts_local, &
               real(xg,kind=private_plflt), nptsx_local, real(yg,kind=private_plflt), nptsy_local, &
               transpose_address_local, int(type, kind=private_plint), real(data, kind=private_plflt) )

        zg = real(transpose(transpose_local), kind=wp)
        deallocate(transpose_local, transpose_address_local)
    end subroutine plgriddata_impl

    subroutine plgspa_impl( xmin, xmax, ymin, ymax )
        real(kind=wp), intent(out) :: xmin, xmax, ymin, ymax

        real(kind=private_plflt) :: xmin_out, xmax_out, ymin_out, ymax_out

        interface
            subroutine interface_plgspa( xmin, xmax, ymin, ymax ) bind(c,name='c_plgspa')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), intent(out) :: xmin, xmax, ymin, ymax
            end subroutine interface_plgspa
        end interface

        call interface_plgspa( xmin_out, xmax_out, ymin_out, ymax_out )
        xmin = real(xmin_out, kind=private_plflt)
        xmax = real(xmax_out, kind=private_plflt)
        ymin = real(ymin_out, kind=private_plflt)
        ymax = real(ymax_out, kind=private_plflt)
    end subroutine plgspa_impl

    subroutine plgvpd_impl( xmin, xmax, ymin, ymax )
        real(kind=wp), intent(out) :: xmin, xmax, ymin, ymax

        real(kind=private_plflt) :: xmin_out, xmax_out, ymin_out, ymax_out

        interface
            subroutine interface_plgvpd( xmin, xmax, ymin, ymax ) bind(c,name='c_plgvpd')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), intent(out) :: xmin, xmax, ymin, ymax
            end subroutine interface_plgvpd
        end interface

        call interface_plgvpd( xmin_out, xmax_out, ymin_out, ymax_out )
        xmin = real(xmin_out, kind=private_plflt)
        xmax = real(xmax_out, kind=private_plflt)
        ymin = real(ymin_out, kind=private_plflt)
        ymax = real(ymax_out, kind=private_plflt)
    end subroutine plgvpd_impl

    subroutine plgvpw_impl( xmin, xmax, ymin, ymax )
        real(kind=wp), intent(out) :: xmin, xmax, ymin, ymax

        real(kind=private_plflt) :: xmin_out, xmax_out, ymin_out, ymax_out
        interface
            subroutine interface_plgvpw( xmin, xmax, ymin, ymax ) bind(c,name='c_plgvpw')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), intent(out) :: xmin, xmax, ymin, ymax
            end subroutine interface_plgvpw
        end interface

        call interface_plgvpw( xmin_out, xmax_out, ymin_out, ymax_out )
        xmin = real(xmin_out, kind=private_plflt)
        xmax = real(xmax_out, kind=private_plflt)
        ymin = real(ymin_out, kind=private_plflt)
        ymax = real(ymax_out, kind=private_plflt)
    end subroutine plgvpw_impl

    subroutine plhist_impl( data, datmin, datmax, nbin, oldwin )
        real(kind=wp), dimension(:), intent(in) :: data
        real(kind=wp), intent(in) :: datmin, datmax
        integer, intent(in) :: nbin, oldwin

        interface
            subroutine interface_plhist( n, data, datmin, datmax, nbin, oldwin ) bind(c,name='c_plhist')
                import :: private_plint, private_plflt
                implicit none
                real(kind=private_plflt), dimension(*), intent(in) :: data
                real(kind=private_plflt), value, intent(in) :: datmin, datmax
                integer(kind=private_plint), value, intent(in) :: n, nbin, oldwin
            end subroutine interface_plhist
        end interface

        call interface_plhist( size(data,kind=private_plint), real(data,kind=private_plflt),     &
               real(datmin,kind=private_plflt), real(datmax,kind=private_plflt), &
               int(nbin,kind=private_plint), int(oldwin,kind=private_plint) )
    end subroutine plhist_impl

    subroutine plhlsrgb_impl( h, l, s, r, g, b )
        real(kind=wp), intent(in) :: h, l, s
        real(kind=wp), intent(out) :: r, g, b

        real(kind=private_plflt) :: r_out, g_out, b_out

        interface
            subroutine interface_plhlsrgb( h, l, s, r, g, b ) bind(c,name='c_plhlsrgb')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: h, l, s
                real(kind=private_plflt), intent(out) :: r, g, b
            end subroutine interface_plhlsrgb
        end interface

        call interface_plhlsrgb( real(h,kind=private_plflt), real(l,kind=private_plflt), real(s,kind=private_plflt), &
               r_out, g_out, b_out )
        r = real(r_out, kind=wp)
        g = real(g_out, kind=wp)
        b = real(b_out, kind=wp)
    end subroutine plhlsrgb_impl

    subroutine plimage_impl( idata, xmin, xmax, ymin, ymax, zmin, zmax, Dxmin, Dxmax, Dymin, Dymax )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, Dxmin, Dxmax, Dymin, Dymax
        real(kind=wp), dimension(:, :), intent(in) :: idata

        real(kind=private_plflt), dimension(:,:), allocatable :: idata_local
        type(c_ptr), dimension(:), allocatable :: idata_address_local

        interface
            subroutine interface_plimage( idata, nx, ny, &
                   xmin, xmax, ymin, ymax, &
                   zmin, zmax, Dxmin, Dxmax, Dymin, Dymax ) bind(c,name='c_plimage')
                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny
                real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, Dxmin, Dxmax, Dymin, Dymax
                type(c_ptr), dimension(*), intent(in) :: idata
            end subroutine interface_plimage
        end interface

        call matrix_to_c( idata, idata_local, idata_address_local )

        call interface_plimage( &
               idata_address_local, size(idata, 1, kind=private_plint), size(idata, 2, kind=private_plint), &
               real(xmin, kind=private_plflt), real(xmax, kind=private_plflt), &
               real(ymin, kind=private_plflt), real(ymax, kind=private_plflt), &
               real(zmin, kind=private_plflt), real(zmax, kind=private_plflt), &
               real(Dxmin, kind=private_plflt), real(Dxmax, kind=private_plflt), &
               real(Dymin, kind=private_plflt), real(Dymax, kind=private_plflt) &
               )
    end subroutine plimage_impl

    subroutine plimagefr_impl_1( idata, xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax, xg, yg )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax
        real(kind=wp), dimension(:, :), intent(in) :: idata
        real(kind=wp), dimension(:), intent(in) :: xg, yg

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:), allocatable, target :: xg_in, yg_in
        type(PLcGrid), target :: cgrid_local
        real(kind=private_plflt), dimension(:,:), allocatable :: idata_local
        type(c_ptr), dimension(:), allocatable :: idata_address_local

        nx_in = size(idata,1, kind=private_plint)
        ny_in = size(idata,2, kind=private_plint)
        if(nx_in + 1 /= size(xg, kind=private_plint) .or. ny_in + 1 /= size(yg, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plimagefr: inconsistent sizes for idata, xg, and/or yg"
        end if

        allocate( xg_in(nx_in+1), yg_in(ny_in+1) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in + 1
        cgrid_local%ny = ny_in + 1
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call matrix_to_c( idata, idata_local, idata_address_local )

        call interface_plimagefr( &
               idata_address_local, nx_in, ny_in, &
               real(xmin, kind=private_plflt), real(xmax, kind=private_plflt), &
               real(ymin, kind=private_plflt), real(ymax, kind=private_plflt), &
               real(zmin, kind=private_plflt), real(zmax, kind=private_plflt), &
               real(valuemin, kind=private_plflt), real(valuemax, kind=private_plflt), &
               interface_pltr1, c_loc(cgrid_local) )
    end subroutine plimagefr_impl_1

    subroutine plimagefr_impl_2( idata, xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax, xg, yg )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax
        real(kind=wp), dimension(:, :), intent(in) :: idata
        real(kind=wp), dimension(:, :), intent(in) :: xg, yg

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:, :), allocatable, target :: xg_in, yg_in
        type(PLcGrid), target :: cgrid_local
        real(kind=private_plflt), dimension(:,:), allocatable :: idata_local
        type(c_ptr), dimension(:), allocatable :: idata_address_local

        nx_in = size(idata,1, kind=private_plint)
        ny_in = size(idata,2, kind=private_plint)
        if( &
               nx_in + 1 /= size(xg, 1, kind=private_plint) .or. ny_in + 1 /= size(xg, 2, kind=private_plint) .or. &
               nx_in + 1 /= size(yg, 1, kind=private_plint) .or. ny_in + 1 /= size(xg, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plimagefr: inconsistent sizes for idata, xg and/or yg"
        end if

        allocate( xg_in(nx_in+1,ny_in+1), yg_in(nx_in+1,ny_in+1) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in + 1
        cgrid_local%ny = ny_in + 1
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call matrix_to_c( idata, idata_local, idata_address_local )

        call interface_plimagefr( &
               idata_address_local, nx_in, ny_in, &
               real(xmin, kind=private_plflt), real(xmax, kind=private_plflt), &
               real(ymin, kind=private_plflt), real(ymax, kind=private_plflt), &
               real(zmin, kind=private_plflt), real(zmax, kind=private_plflt), &
               real(valuemin, kind=private_plflt), real(valuemax, kind=private_plflt), &
               interface_pltr2f, c_loc(cgrid_local) )
    end subroutine plimagefr_impl_2

    ! Uses NULL C callback and NULL associated data (which has a special meaning at
    ! the C level).
    subroutine plimagefr_impl_null( idata, xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax
        real(kind=wp), dimension(:, :), intent(in) :: idata

        real(kind=private_plflt), dimension(:,:), allocatable :: idata_local
        type(c_ptr), dimension(:), allocatable :: idata_address_local

        call matrix_to_c( idata, idata_local, idata_address_local )

        call interface_plimagefr_null( &
               idata_address_local, &
               size(idata, 1, kind=private_plint), size(idata, 2, kind=private_plint), &
               real(xmin, kind=private_plflt), real(xmax, kind=private_plflt), &
               real(ymin, kind=private_plflt), real(ymax, kind=private_plflt), &
               real(zmin, kind=private_plflt), real(zmax, kind=private_plflt), &
               real(valuemin, kind=private_plflt), real(valuemax, kind=private_plflt) )
    end subroutine plimagefr_impl_null

    subroutine plimagefr_impl_tr( idata, xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax, tr )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax
        real(kind=wp), dimension(:, :), intent(in) :: idata
        real(kind=wp), dimension(:), intent(in) :: tr

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable :: idata_local
        type(c_ptr), dimension(:), allocatable :: idata_address_local
        real(kind=private_plflt), dimension(6), target :: tr_in

        nx_in = size(idata,1, kind=private_plint)
        ny_in = size(idata,2, kind=private_plint)

        call matrix_to_c( idata, idata_local, idata_address_local )
        tr_in = tr(1:6)

        call interface_plimagefr( &
               idata_address_local, nx_in, ny_in, &
               real(xmin, kind=private_plflt), real(xmax, kind=private_plflt), &
               real(ymin, kind=private_plflt), real(ymax, kind=private_plflt), &
               real(zmin, kind=private_plflt), real(zmax, kind=private_plflt), &
               real(valuemin, kind=private_plflt), real(valuemax, kind=private_plflt), &
               plplot_private_pltr, c_loc(tr_in) )
    end subroutine plimagefr_impl_tr

    subroutine plimagefr_impl( idata, xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax, proc )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax
        real(kind=wp), dimension(:, :), intent(in) :: idata
        procedure(pltransform_proc) :: proc

        real(kind=private_plflt), dimension(:,:), allocatable :: idata_local
        type(c_ptr), dimension(:), allocatable :: idata_address_local

        call matrix_to_c( idata, idata_local, idata_address_local )
        pltransform => proc

        call interface_plimagefr( &
               idata_address_local, &
               size(idata, 1, kind=private_plint), size(idata, 2, kind=private_plint), &
               real(xmin, kind=private_plflt), real(xmax, kind=private_plflt), &
               real(ymin, kind=private_plflt), real(ymax, kind=private_plflt), &
               real(zmin, kind=private_plflt), real(zmax, kind=private_plflt), &
               real(valuemin, kind=private_plflt), real(valuemax, kind=private_plflt), &
               pltransformf2c, c_null_ptr )
    end subroutine plimagefr_impl

    subroutine plimagefr_impl_data( idata, xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax, proc, data )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax
        real(kind=wp), dimension(:, :), intent(in) :: idata
        procedure(pltransform_proc_data) :: proc
        type(c_ptr), intent(in) :: data

        real(kind=private_plflt), dimension(:,:), allocatable :: idata_local
        type(c_ptr), dimension(:), allocatable :: idata_address_local

        call matrix_to_c( idata, idata_local, idata_address_local )
        pltransform_data => proc

        call interface_plimagefr( &
               idata_address_local, &
               size(idata, 1, kind=private_plint), size(idata, 2, kind=private_plint), &
               real(xmin, kind=private_plflt), real(xmax, kind=private_plflt), &
               real(ymin, kind=private_plflt), real(ymax, kind=private_plflt), &
               real(zmin, kind=private_plflt), real(zmax, kind=private_plflt), &
               real(valuemin, kind=private_plflt), real(valuemax, kind=private_plflt), &
               pltransformf2c_data, data )
    end subroutine plimagefr_impl_data

    subroutine pljoin_impl( x1, y1, x2, y2 )
        real(kind=wp), intent(in) :: x1, y1, x2, y2

        interface
            subroutine interface_pljoin( x1, y1, x2, y2 ) bind(c,name='c_pljoin')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: x1, y1, x2, y2
            end subroutine interface_pljoin
        end interface

        call interface_pljoin( real(x1,kind=private_plflt), real(y1,kind=private_plflt), &
               real(x2,kind=private_plflt), real(y2,kind=private_plflt) )
    end subroutine pljoin_impl

    subroutine pllegend_impl( &
           legend_width, legend_height, &
           opt, position, x, y, &
           plot_width, bg_color, bb_color, bb_style, &
           nrow, ncolumn, opt_array, &
           text_offset, text_scale, text_spacing, &
           text_justification, text_colors, text, &
           box_colors, box_patterns, box_scales, &
           box_line_widths, &
           line_colors, line_styles, line_widths, &
           symbol_colors, symbol_scales, &
           symbol_numbers, symbols )

        real(kind=wp), intent(in) :: plot_width, x, y
        real(kind=wp), intent(in) :: text_offset, text_scale, text_spacing, text_justification
        integer, intent(in) :: position, opt, bg_color, bb_color, bb_style
        integer, intent(in) :: nrow, ncolumn
        real(kind=wp), intent(out) :: legend_width, legend_height

        character(len=*), dimension(:), intent(in) :: text, symbols

        integer, dimension(:), intent(in) :: opt_array, text_colors, box_colors
        integer, dimension(:), intent(in) :: box_patterns
        real(kind=wp), dimension(:), intent(in) :: box_line_widths
        integer, dimension(:), intent(in) :: line_colors, line_styles
        real(kind=wp), dimension(:), intent(in) :: line_widths
        integer, dimension(:), intent(in) :: symbol_colors, symbol_numbers
        real(kind=wp), dimension(:), intent(in) :: box_scales, symbol_scales

        integer(kind=private_plint) :: nlegend_local
        real(kind=private_plflt) :: legend_width_out, legend_height_out
        character(len=1), dimension(:,:), allocatable :: cstring_text_local, cstring_symbols_local
        type(c_ptr), dimension(:), allocatable :: cstring_address_text_local, cstring_address_symbols_local

        interface
            subroutine interface_pllegend( &
                   legend_width, legend_height, &
                   opt, position, x, y, &
                   plot_width, bg_color, bb_color, bb_style, &
                   nrow, ncolumn, nlegend, opt_array, &
                   text_offset, text_scale, text_spacing, &
                   text_justification, text_colors, text, &
                   box_colors, box_patterns, box_scales, &
                   box_line_widths, &
                   line_colors, line_styles, line_widths, &
                   symbol_colors, symbol_scales, &
                   symbol_numbers, symbols ) &
                   bind(c,name='c_pllegend')

                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none

                real(kind=private_plflt), value, intent(in) :: plot_width, x, y
                real(kind=private_plflt), value, intent(in) :: text_offset, text_scale, text_spacing, text_justification
                integer(kind=private_plint), value, intent(in) :: position, opt, bg_color, bb_color, bb_style
                integer(kind=private_plint), value, intent(in) :: nrow, ncolumn, nlegend

                type(c_ptr), dimension(*), intent(in) :: text, symbols

                integer(kind=private_plint), dimension(*), intent(in) :: opt_array, text_colors, box_colors
                integer(kind=private_plint), dimension(*), intent(in) :: box_patterns
                real(kind=private_plflt), dimension(*), intent(in) :: box_line_widths
                integer(kind=private_plint), dimension(*), intent(in) :: line_colors, line_styles
                real(kind=private_plflt), dimension(*), intent(in) :: line_widths
                integer(kind=private_plint), dimension(*), intent(in) :: symbol_colors, symbol_numbers
                real(kind=private_plflt), dimension(*), intent(in) :: box_scales, symbol_scales
                real(kind=private_plflt), intent(out) :: legend_width, legend_height
            end subroutine interface_pllegend
        end interface


        ! Determine number of legend entries and demand consistent
        ! array sizes.

        nlegend_local = size(opt_array)
        if( &
               nlegend_local /= size(text_colors) .or. &
               nlegend_local /= size(text) .or. &
               nlegend_local /= size(box_colors) .or. &
               nlegend_local /= size(box_patterns) .or. &
               nlegend_local /= size(box_scales) .or. &
               nlegend_local /= size(box_line_widths) .or. &
               nlegend_local /= size(line_colors) .or. &
               nlegend_local /= size(line_styles) .or. &
               nlegend_local /= size(line_widths) .or. &
               nlegend_local /= size(symbol_colors) .or. &
               nlegend_local /= size(symbol_scales) .or. &
               nlegend_local /= size(symbol_numbers) .or. &
               nlegend_local /= size(symbols) &
               ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: pllegend: inconsistent sizes for the following arrays:"
            write(error_unit, "(a)") "opt_array"
            write(error_unit, "(a)") "text_colors"
            write(error_unit, "(a)") "text"
            write(error_unit, "(a)") "box_colors"
            write(error_unit, "(a)") "box_patterns"
            write(error_unit, "(a)") "box_scales"
            write(error_unit, "(a)") "box_line_widths"
            write(error_unit, "(a)") "line_colors"
            write(error_unit, "(a)") "line_styles"
            write(error_unit, "(a)") "line_widths"
            write(error_unit, "(a)") "symbol_colors"
            write(error_unit, "(a)") "symbol_scales"
            write(error_unit, "(a)") "symbol_numbers"
            write(error_unit, "(a)") "symbols"
        end if

        call character_array_to_c( cstring_text_local, cstring_address_text_local, text )
        call character_array_to_c( cstring_symbols_local, cstring_address_symbols_local, symbols )

        call interface_pllegend( &
               legend_width_out, legend_height_out, &
               int(opt,kind=private_plint), int(position,kind=private_plint), &
               real(x,kind=private_plflt), real(y,kind=private_plflt), &
               real(plot_width,kind=private_plflt), int(bg_color,kind=private_plint), &
               int(bb_color,kind=private_plint), int(bb_style,kind=private_plint), &
               int(nrow,kind=private_plint), int(ncolumn,kind=private_plint), &
               int(nlegend_local,kind=private_plint), int(opt_array,kind=private_plint), &
               real(text_offset,kind=private_plflt), real(text_scale,kind=private_plflt), &
               real(text_spacing,kind=private_plflt), &
               real(text_justification,kind=private_plflt), int(text_colors,kind=private_plint), &
               cstring_address_text_local, &
               int(box_colors,kind=private_plint), int(box_patterns,kind=private_plint), &
               real(box_scales,kind=private_plflt), &
               real(box_line_widths,kind=private_plflt), &
               int(line_colors,kind=private_plint), int(line_styles,kind=private_plint), &
               real(line_widths,kind=private_plflt), &
               int(symbol_colors,kind=private_plint), real(symbol_scales,kind=private_plflt), &
               int(symbol_numbers,kind=private_plint), cstring_address_symbols_local )

        legend_width = real(legend_width_out, kind=wp)
        legend_height = real(legend_height_out, kind=wp)

    end subroutine pllegend_impl

    subroutine pllightsource_impl( x, y, z )
        real(kind=wp), intent(in) :: x, y, z

        interface
            subroutine interface_pllightsource( x, y, z ) bind(c,name='c_pllightsource')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: x, y, z
            end subroutine interface_pllightsource
        end interface

        call interface_pllightsource( real(x,kind=private_plflt), real(y,kind=private_plflt), real(z,kind=private_plflt) )
    end subroutine pllightsource_impl

    subroutine plline3_impl( x, y, z )
        real(kind=wp), dimension(:), intent(in) :: x, y, z

        integer(kind=private_plint) :: sz_local

        interface
            subroutine interface_plline3( sz, x, y, z ) bind(c,name='c_plline3')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: sz
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, z
            end subroutine interface_plline3
        end interface

        sz_local = size(x,kind=private_plint)
        if( sz_local /= size(y, kind=private_plint) .or. sz_local /= size(z, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plline3: inconsistent sizes for x, y, and/or z"
        end if

        call interface_plline3( sz_local, real(x,kind=private_plflt), real(y,kind=private_plflt), &
               real(z,kind=private_plflt) )
    end subroutine plline3_impl

    subroutine plline_impl( x, y )
        real(kind=wp), dimension(:), intent(in) :: x, y

        integer(kind=private_plint) :: sz_local

        interface
            subroutine interface_plline( sz, x, y ) bind(c,name='c_plline')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: sz
                real(kind=private_plflt), dimension(*), intent(in) :: x, y
            end subroutine interface_plline
        end interface

        sz_local = size(x,kind=private_plint)
        if( sz_local /= size(y, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plline: inconsistent sizes for x and y"
        end if

        call interface_plline( sz_local, real(x,kind=private_plflt), real(y,kind=private_plflt) )
    end subroutine plline_impl

    subroutine plmap_impl( proc, name, minx, maxx, miny, maxy )
        procedure(plmapform_proc) :: proc
        character*(*), intent(in) :: name
        real(kind=wp), intent(in) :: minx, maxx, miny, maxy

        type(c_funptr) :: c_funloc_local

         plmapform => proc
        c_funloc_local = c_funloc(plmapformf2c)

        call interface_plmap( c_funloc_local, trim(name)//c_null_char, &
               real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
               real(miny, kind=private_plflt), real(maxy, kind=private_plflt) )
    end subroutine plmap_impl

    subroutine plmap_impl_null( name, minx, maxx, miny, maxy )
        character*(*), intent(in) :: name
        real(kind=wp), intent(in) :: minx, maxx, miny, maxy

        call interface_plmap( c_null_funptr, trim(name)//c_null_char, &
               real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
               real(miny, kind=private_plflt), real(maxy, kind=private_plflt) )
    end subroutine plmap_impl_null

    subroutine plmapfill_impl( proc, name, minx, maxx, miny, maxy, plotentries )
        procedure(plmapform_proc) :: proc
        character*(*), intent(in) :: name
        real(kind=wp), intent(in) :: minx, maxx, miny, maxy
        integer, dimension(:), optional, target, intent(in) :: plotentries

        plmapform => proc

        if ( present(plotentries) ) then
            call interface_plmapfill( c_funloc(plmapformf2c), trim(name)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_loc(plotentries), size(plotentries, kind=private_plint) )
        else
            call interface_plmapfill( c_funloc(plmapformf2c), trim(name)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_null_ptr, 0_private_plint )
        endif
    end subroutine plmapfill_impl

    subroutine plmapfill_impl_null( name, minx, maxx, miny, maxy, plotentries )
        character*(*), intent(in) :: name
        real(kind=wp), intent(in) :: minx, maxx, miny, maxy
        integer, dimension(:), optional, target, intent(in) :: plotentries

        if ( present(plotentries) ) then
            call interface_plmapfill( c_null_funptr, trim(name)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_loc(plotentries), size(plotentries, kind=private_plint) )
        else
            call interface_plmapfill( c_null_funptr, trim(name)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_null_ptr, 0_private_plint )
        endif
    end subroutine plmapfill_impl_null

    subroutine plmapline_impl( proc, name, minx, maxx, miny, maxy, plotentries )
        procedure(plmapform_proc) :: proc
        character*(*), intent(in) :: name
        real(kind=wp), intent(in) :: minx, maxx, miny, maxy
        integer, dimension(:), optional, target, intent(in) :: plotentries

        plmapform => proc

        if ( present(plotentries) ) then
            call interface_plmapline( c_funloc(plmapformf2c), trim(name)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_loc(plotentries), size(plotentries, kind=private_plint) )
        else
            call interface_plmapline( c_funloc(plmapformf2c), trim(name)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_null_ptr, 0_private_plint )
        endif
    end subroutine plmapline_impl

    subroutine plmapline_impl_null( name, minx, maxx, miny, maxy, plotentries )
        character*(*), intent(in) :: name
        real(kind=wp), intent(in) :: minx, maxx, miny, maxy
        integer, dimension(:), optional, target, intent(in) :: plotentries

        if ( present(plotentries) ) then
            call interface_plmapline( c_null_funptr, trim(name)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_loc(plotentries), size(plotentries, kind=private_plint) )
        else
            call interface_plmapline( c_null_funptr, trim(name)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_null_ptr, 0_private_plint )
        endif
    end subroutine plmapline_impl_null

    subroutine plmapstring_impl( proc, name, string, minx, maxx, miny, maxy, plotentries )
        procedure(plmapform_proc) :: proc
        character*(*), intent(in) :: name, string
        real(kind=wp), intent(in) :: minx, maxx, miny, maxy
        integer, dimension(:), optional, target, intent(in) :: plotentries

        plmapform => proc

        if ( present(plotentries) ) then
            call interface_plmapstring( c_funloc(plmapformf2c), &
                   trim(name)//c_null_char, trim(string)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_loc(plotentries), size(plotentries, kind=private_plint) )
        else
            call interface_plmapstring( c_funloc(plmapformf2c), &
                   trim(name)//c_null_char, trim(string)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_null_ptr, 0_private_plint )
        endif
    end subroutine plmapstring_impl

    subroutine plmapstring_impl_null( name, string, minx, maxx, miny, maxy, plotentries )
        character*(*), intent(in) :: name, string
        real(kind=wp), intent(in) :: minx, maxx, miny, maxy
        integer, dimension(:), optional, target, intent(in) :: plotentries

        if ( present(plotentries) ) then
            call interface_plmapstring( c_null_funptr, &
                   trim(name)//c_null_char, trim(string)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_loc(plotentries), size(plotentries, kind=private_plint) )
        else
            call interface_plmapstring( c_null_funptr, &
                   trim(name)//c_null_char, trim(string)//c_null_char, &
                   real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
                   real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
                   c_null_ptr, 0_private_plint )
        endif
    end subroutine plmapstring_impl_null

    subroutine plmaptex_impl( proc, name, dx, dy, just, text, minx, maxx, miny, maxy, plotentry )
        procedure(plmapform_proc) :: proc
        character*(*), intent(in) :: name, text
        real(kind=wp), intent(in) :: dx, dy, just, minx, maxx, miny, maxy
        integer, intent(in) :: plotentry

        plmapform => proc

        call interface_plmaptex( c_funloc(plmapformf2c), &
               trim(name)//c_null_char, &
               real(dx, kind=private_plflt), real(dy, kind=private_plflt), &
               real(just, kind=private_plflt), trim(text)//c_null_char, &
               real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
               real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
               int(plotentry, kind=private_plint) )
    end subroutine plmaptex_impl

    subroutine plmaptex_impl_null( name, dx, dy, just, text, minx, maxx, miny, maxy, plotentry )
        character*(*), intent(in) :: name, text
        real(kind=wp), intent(in) :: dx, dy, just, minx, maxx, miny, maxy
        integer, intent(in) :: plotentry

        call interface_plmaptex( c_null_funptr, &
               trim(name)//c_null_char, &
               real(dx, kind=private_plflt), real(dy, kind=private_plflt), &
               real(just, kind=private_plflt), trim(text)//c_null_char, &
               real(minx, kind=private_plflt), real(maxx, kind=private_plflt), &
               real(miny, kind=private_plflt), real(maxy, kind=private_plflt), &
               int(plotentry, kind=private_plint) )
    end subroutine plmaptex_impl_null

    subroutine plmeridians_impl( proc, dlong, dlat, minlong, maxlong, minlat, maxlat )
        procedure(plmapform_proc) :: proc
        real(kind=wp), intent(in) :: dlong, dlat, minlong, maxlong, minlat, maxlat

        plmapform => proc

        call interface_plmeridians( c_funloc(plmapformf2c), &
               real(dlong, kind=private_plflt), real(dlat, kind=private_plflt), &
               real(minlong, kind=private_plflt), real(maxlong, kind=private_plflt), &
               real(minlat, kind=private_plflt), real(maxlat, kind=private_plflt) )
    end subroutine plmeridians_impl

    subroutine plmeridians_impl_null( dlong, dlat, minlong, maxlong, minlat, maxlat )
        real(kind=wp), intent(in) :: dlong, dlat, minlong, maxlong, minlat, maxlat

        call interface_plmeridians( c_null_funptr, &
               real(dlong, kind=private_plflt), real(dlat, kind=private_plflt), &
               real(minlong, kind=private_plflt), real(maxlong, kind=private_plflt), &
               real(minlat, kind=private_plflt), real(maxlat, kind=private_plflt) )
    end subroutine plmeridians_impl_null

    subroutine plmesh_impl( x, y, z, opt )
        integer, intent(in) :: opt
        real(kind=wp), dimension(:), intent(in) :: x, y
        real(kind=wp), dimension(:,:), intent(in) :: z

        real(kind=private_plflt), dimension(:,:), allocatable :: zz_local
        type(c_ptr), dimension(:), allocatable :: zaddress_local

        integer(kind=private_plint) :: nx_local, ny_local

        interface
            subroutine interface_plmesh( x, y, zaddress, nx, ny, opt ) bind(c,name='c_plmesh')
                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny, opt
                real(kind=private_plflt), dimension(*), intent(in) :: x, y
                type(c_ptr), dimension(*), intent(in) :: zaddress
            end subroutine interface_plmesh
        end interface

        nx_local = size(x,kind=private_plint)
        ny_local = size(y,kind=private_plint)

        if( nx_local /= size(z, 1, kind=private_plint) .or. ny_local /= size(z, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plmesh: inconsistent sizes for x, y, and/or z"
        end if

        call matrix_to_c( z, zz_local, zaddress_local )

        call interface_plmesh( real(x,kind=private_plflt), real(y,kind=private_plflt), zaddress_local, &
               nx_local, ny_local, int(opt,kind=private_plint))

    end subroutine plmesh_impl

    subroutine plmeshc_impl( x, y, z, opt, clevel )
        integer, intent(in) :: opt
        real(kind=wp), dimension(:), intent(in) :: x, y, clevel
        real(kind=wp), dimension(:,:), intent(in) :: z

        real(kind=private_plflt), dimension(:,:), allocatable :: zz_local
        type(c_ptr), dimension(:), allocatable :: zaddress_local

        integer(kind=private_plint) :: nx_local, ny_local

        interface
            subroutine interface_plmeshc( x, y, zaddress, nx, ny, opt, clevel, nlevel ) bind(c,name='c_plmeshc')
                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny, opt, nlevel
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, clevel
                type(c_ptr), dimension(*), intent(in) :: zaddress
            end subroutine interface_plmeshc
        end interface

        nx_local = size(x,kind=private_plint)
        ny_local = size(y,kind=private_plint)

        if( nx_local /= size(z, 1, kind=private_plint) .or. ny_local /= size(z, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plmeshc: inconsistent sizes for x, y, and/or z"
        end if

        call matrix_to_c( z, zz_local, zaddress_local )

        call interface_plmeshc( real(x,kind=private_plflt), real(y,kind=private_plflt), zaddress_local, &
               nx_local, ny_local, int(opt,kind=private_plint), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint) )

    end subroutine plmeshc_impl

    subroutine plmtex3_impl( side, disp, pos, just, text )
        real(kind=wp), intent(in) :: disp, pos, just
        character*(*), intent(in) :: side, text

        interface
            subroutine interface_plmtex3( side, disp, pos, just, text ) bind(c,name='c_plmtex3')
                import :: private_plflt
                implicit none
                character(len=1), dimension(*), intent(in) :: side, text
                real(kind=private_plflt), value, intent(in) :: disp, pos, just
            end subroutine interface_plmtex3
        end interface

        call interface_plmtex3( trim(side)//c_null_char, real(disp,kind=private_plflt), real(pos,kind=private_plflt), &
               real(just,kind=private_plflt), trim(text)//c_null_char )
    end subroutine plmtex3_impl

    subroutine plmtex_impl( side, disp, pos, just, text )
        real(kind=wp), intent(in) :: disp, pos, just
        character*(*), intent(in) :: side, text

        interface
            subroutine interface_plmtex( side, disp, pos, just, text ) bind(c,name='c_plmtex')
                import :: private_plflt
                implicit none
                character(len=1), dimension(*), intent(in) :: side, text
                real(kind=private_plflt), value, intent(in) :: disp, pos, just
            end subroutine interface_plmtex
        end interface

        call interface_plmtex( trim(side)//c_null_char, real(disp,kind=private_plflt), real(pos,kind=private_plflt), &
               real(just,kind=private_plflt), trim(text)//c_null_char )
    end subroutine plmtex_impl

    subroutine plot3d_impl( x, y, z, opt, side)
        logical, intent(in) :: side
        integer, intent(in) :: opt
        real(kind=wp), dimension(:), intent(in) :: x, y
        real(kind=wp), dimension(:,:), intent(in) :: z

        real(kind=private_plflt), dimension(:,:), allocatable :: zz_local
        type(c_ptr), dimension(:), allocatable :: zaddress_local

        integer(kind=private_plint) :: nx_local, ny_local

        interface
            subroutine interface_plot3d( x, y, zaddress, nx, ny, opt, side ) bind(c,name='c_plot3d')
                import :: c_ptr
                import :: private_plint, private_plbool, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny, opt
                integer(kind=private_plbool), value, intent(in) :: side
                real(kind=private_plflt), dimension(*), intent(in) :: x, y
                type(c_ptr), dimension(*), intent(in) :: zaddress
            end subroutine interface_plot3d
        end interface

        nx_local = size(x,kind=private_plint)
        ny_local = size(y,kind=private_plint)

        if( nx_local /= size(z, 1, kind=private_plint) .or. ny_local /= size(z, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plot3d: inconsistent sizes for x, y, and/or z"
        end if

        call matrix_to_c( z, zz_local, zaddress_local )

        call interface_plot3d( real(x,kind=private_plflt), real(y,kind=private_plflt), zaddress_local,  &
               nx_local, ny_local, int(opt,kind=private_plint), &
               int(merge(1,0,side),kind=private_plbool) )

    end subroutine plot3d_impl

    subroutine plot3dc_impl( x, y, z, opt, clevel )
        integer, intent(in) :: opt
        real(kind=wp), dimension(:), intent(in) :: x, y, clevel
        real(kind=wp), dimension(:,:), intent(in) :: z

        real(kind=private_plflt), dimension(:,:), allocatable :: zz_local
        type(c_ptr), dimension(:), allocatable :: zaddress_local

        integer(kind=private_plint) :: nx_local, ny_local

        interface
            subroutine interface_plot3dc( x, y, zaddress, nx, ny, opt, clevel, nlevel ) bind(c,name='c_plot3dc')
                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny, opt, nlevel
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, clevel
                type(c_ptr), dimension(*), intent(in) :: zaddress
            end subroutine interface_plot3dc
        end interface

        nx_local = size(x,kind=private_plint)
        ny_local = size(y,kind=private_plint)

        if( nx_local /= size(z, 1, kind=private_plint) .or. ny_local /= size(z, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plot3dc: inconsistent sizes for x, y, and/or z"
        end if

        call matrix_to_c( z, zz_local, zaddress_local )

        call interface_plot3dc( real(x,kind=private_plflt), real(y,kind=private_plflt), zaddress_local,  &
               nx_local, ny_local, int(opt,kind=private_plint), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint) )

    end subroutine plot3dc_impl

    subroutine plot3dcl_impl( x, y, z, opt, clevel, indexxmin, indexymin, indexymax )
        integer, intent(in) :: opt
        real(kind=wp), dimension(:), intent(in) :: x, y, clevel
        real(kind=wp), dimension(:,:), intent(in) :: z
        integer, intent(in) :: indexxmin
        integer, dimension(:), intent(in) :: indexymin, indexymax

        real(kind=private_plflt), dimension(:,:), allocatable :: zz_local
        type(c_ptr), dimension(:), allocatable :: zaddress_local

        integer(kind=private_plint) :: nx_local, ny_local, indexxmax_local

        interface
            subroutine interface_plot3dcl( x, y, zaddress, nx, ny, opt, clevel, nlevel, &
                   indexxmin, indexxmax, indexymin, indexymax ) bind(c,name='c_plot3dcl')
                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny, opt, nlevel, indexxmin, indexxmax
                integer(kind=private_plint), dimension(*), intent(in) :: indexymin, indexymax
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, clevel
                type(c_ptr), dimension(*), intent(in) :: zaddress
            end subroutine interface_plot3dcl
        end interface

        nx_local = size(x,kind=private_plint)
        ny_local = size(y,kind=private_plint)

        if( nx_local /= size(z, 1, kind=private_plint) .or. ny_local /= size(z, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plot3dcl: inconsistent sizes for x, y, and/or z"
        end if

        indexxmax_local = size(indexymin)
        if( indexxmax_local /= size(indexymax, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plot3dcl: inconsistent sizes for indexymin and indeyxmax"
        end if

        call matrix_to_c( z, zz_local, zaddress_local )

        call interface_plot3dcl( real(x,kind=private_plflt), real(y,kind=private_plflt), zaddress_local, &
               nx_local, ny_local, int(opt,kind=private_plint), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint), &
               int(indexxmin,kind=private_plint), indexxmax_local, &
               int(indexymin,kind=private_plint), int(indexymax,kind=private_plint) )

    end subroutine plot3dcl_impl

    subroutine plpath_impl( n, x1, y1, x2, y2 )
        integer, intent(in) :: n
        real(kind=wp), intent(in) :: x1, y1, x2, y2

        interface
            subroutine interface_plpath( n, x1, y1, x2, y2 ) bind(c,name='c_plpath')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                real(kind=private_plflt), value, intent(in) :: x1, y1, x2, y2
            end subroutine interface_plpath
        end interface

        call interface_plpath( int(n,kind=private_plint), real(x1,kind=private_plflt), &
               real(y1,kind=private_plflt), real(x2,kind=private_plflt), real(y2,kind=private_plflt) )
    end subroutine plpath_impl

    subroutine plpoin3_impl( x, y, z, code )
        integer, intent(in) :: code
        real(kind=wp), dimension(:), intent(in) :: x, y, z

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plpoin3( n, x, y, z, code ) bind(c,name='c_plpoin3')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n, code
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, z
            end subroutine interface_plpoin3
        end interface

        n_local = size(x,kind=private_plint)
        if( n_local /= size(y, kind=private_plint) .or. n_local /= size(z, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plpoin3: inconsistent sizes for x, y, and/or z"
        end if

        call interface_plpoin3( n_local, real(x,kind=private_plflt), real(y,kind=private_plflt), &
               real(z,kind=private_plflt), int(code,kind=private_plint) )
    end subroutine plpoin3_impl

    subroutine plpoin_impl( x, y, code )
        integer, intent(in) :: code
        real(kind=wp), dimension(:), intent(in) :: x, y

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plpoin( n, x, y, code ) bind(c,name='c_plpoin')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n, code
                real(kind=private_plflt), dimension(*), intent(in) :: x, y
            end subroutine interface_plpoin
        end interface

        n_local = size(x,kind=private_plint)
        if( n_local /= size(y, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plpoin: inconsistent sizes for x and y"
        end if

        call interface_plpoin( n_local, real(x,kind=private_plflt), real(y,kind=private_plflt), &
               int(code,kind=private_plint) )
    end subroutine plpoin_impl

    subroutine plpoly3_impl( x, y, z, draw, ifcc )
        logical, intent(in) :: ifcc
        logical, dimension(:), intent(in) :: draw
        real(kind=wp), dimension(:), intent(in) :: x, y, z

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plpoly3( n, x, y, z, draw, ifcc ) bind(c,name='c_plpoly3')
                import :: private_plint, private_plbool, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                integer(kind=private_plbool), value, intent(in) :: ifcc
                integer(kind=private_plbool), dimension(*), intent(in) :: draw
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, z
            end subroutine interface_plpoly3
        end interface

        n_local = size(x,kind=private_plint)
        if( n_local /= size(y, kind=private_plint) .or. n_local /= size(z, kind=private_plint) .or. &
               n_local /= size(draw, kind=private_plint) + 1 ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plpoly3: inconsistent sizes for x, y, z, and/or draw"
        end if

        call interface_plpoly3( n_local, &
               real(x,kind=private_plflt), real(y,kind=private_plflt), real(z,kind=private_plflt), &
               int(merge(1,0,draw),kind=private_plbool), int(merge(1,0,ifcc),kind=private_plbool) )
    end subroutine plpoly3_impl

    subroutine plptex3_impl( wx, wy, wz, dx, dy, dz, sx, sy, sz, just, text )

        real(kind=wp), intent(in) :: wx, wy, wz, dx, dy, dz, sx, sy, sz, just
        character*(*), intent(in) :: text

        interface
            subroutine interface_plptex3( wx, wy, wz, dx, dy, dz, sx, sy, sz, just, text ) bind(c,name='c_plptex3')
                import :: private_plflt
                implicit none
                character(len=1), dimension(*), intent(in) :: text
                real(kind=private_plflt), value, intent(in) :: wx, wy, wz, dx, dy, dz, sx, sy, sz, just
            end subroutine interface_plptex3
        end interface

        call interface_plptex3( real(wx,kind=private_plflt), real(wy,kind=private_plflt), real(wz,kind=private_plflt), &
               real(dx,kind=private_plflt), real(dy,kind=private_plflt), real(dz,kind=private_plflt), &
               real(sx,kind=private_plflt), real(sy,kind=private_plflt), real(sz,kind=private_plflt), &
               real(just,kind=private_plflt), trim(text)//c_null_char )

    end subroutine plptex3_impl

    subroutine plptex_impl( x, y, dx, dy, just, text )

        real(kind=wp), intent(in) :: x, y, dx, dy, just
        character*(*), intent(in) :: text

        interface
            subroutine interface_plptex( x, y, dx, dy, just, text ) bind(c,name='c_plptex')
                import :: private_plflt
                implicit none
                character(len=1), dimension(*), intent(in) :: text
                real(kind=private_plflt), value, intent(in) :: x, y, dx, dy, just
            end subroutine interface_plptex
        end interface

        call interface_plptex( real(x,kind=private_plflt), real(y,kind=private_plflt), real(dx,kind=private_plflt), &
               real(dy,kind=private_plflt), real(just,kind=private_plflt), trim(text)//c_null_char )

    end subroutine plptex_impl

    subroutine plrgbhls_impl( r, g, b, h, l, s )
        real(kind=wp), intent(in) :: r, g, b
        real(kind=wp), intent(out) :: h, l, s

        real(kind=private_plflt) :: h_out, l_out, s_out

        interface
            subroutine interface_plrgbhls( r, g, b, h, l, s ) bind(c,name='c_plrgbhls')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: r, g, b
                real(kind=private_plflt), intent(out) :: h, l, s
            end subroutine interface_plrgbhls
        end interface

        call interface_plrgbhls( real(r,kind=private_plflt), real(g,kind=private_plflt), real(b,kind=private_plflt), &
               h_out, l_out, s_out )
        h = real(h_out, kind=wp)
        l = real(l_out, kind=wp)
        s = real(s_out, kind=wp)
    end subroutine plrgbhls_impl

    subroutine plschr_impl( chrdef, chrht )
        real(kind=wp), intent(in) :: chrdef, chrht

        interface
            subroutine interface_plschr( chrdef, chrht ) bind(c,name='c_plschr')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: chrdef, chrht
            end subroutine interface_plschr
        end interface

        call interface_plschr( real(chrdef,kind=private_plflt), real(chrht,kind=private_plflt) )
    end subroutine plschr_impl

    subroutine plscmap0a_impl( r, g, b, a )
        integer, dimension(:), intent(in) :: r, g, b
        real(kind=wp), dimension(:), intent(in) :: a

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plscmap0a( r, g, b, a, n ) bind(c,name='c_plscmap0a')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), dimension(*), intent(in) :: r, g, b
                real(kind=private_plflt), dimension(*), intent(in) :: a
                integer(kind=private_plint), value, intent(in) :: n
            end subroutine interface_plscmap0a
        end interface

        n_local = size(r,kind=private_plint)
        if( n_local /= size(g, kind=private_plint) .or. n_local /= size(b, kind=private_plint) .or. &
               n_local /= size(a, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plscmap0a: inconsistent sizes for r, g, b, and/or a"
        end if

        call interface_plscmap0a( int(r,kind=private_plint), int(g,kind=private_plint), int(b,kind=private_plint), &
               real(a,kind=private_plflt), n_local )
    end subroutine plscmap0a_impl

    subroutine plscmap1_range_impl( chrdef, chrht )
        real(kind=wp), intent(in) :: chrdef, chrht

        interface
            subroutine interface_plscmap1_range( chrdef, chrht ) bind(c,name='c_plscmap1_range')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: chrdef, chrht
            end subroutine interface_plscmap1_range
        end interface

        call interface_plscmap1_range( real(chrdef,kind=private_plflt), real(chrht,kind=private_plflt) )
    end subroutine plscmap1_range_impl

    subroutine plscmap1a_impl( r, g, b, a )
        integer, dimension(:), intent(in) :: r, g, b
        real(kind=wp), dimension(:), intent(in) :: a

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plscmap1a( r, g, b, a, n ) bind(c,name='c_plscmap1a')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), dimension(*), intent(in) :: r, g, b
                real(kind=private_plflt), dimension(*), intent(in) :: a
                integer(kind=private_plint), value, intent(in) :: n
            end subroutine interface_plscmap1a
        end interface

        n_local = size(r,kind=private_plint)
        if( n_local /= size(g, kind=private_plint) .or. n_local /= size(b, kind=private_plint) .or. &
               n_local /= size(a, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plscmap1a: inconsistent sizes for r, g, b, and/or a"
        end if

        call interface_plscmap1a( int(r,kind=private_plint), int(g,kind=private_plint), int(b,kind=private_plint), &
               real(a,kind=private_plflt), n_local )
    end subroutine plscmap1a_impl

    subroutine plscmap1l_impl( rgbtype, intensity, coord1, coord2, coord3, alt_hue_path)
        logical, intent(in) :: rgbtype
        real(kind=wp), dimension(:), intent(in) :: intensity, coord1, coord2, coord3
        logical, dimension(:), optional, intent(in) :: alt_hue_path

        integer(kind=private_plint) :: npts_local
        integer(kind=private_plbool), dimension(:), allocatable, target :: ialt_hue_path_local

        interface
            subroutine interface_plscmap1l( rgbtype, npts, intensity, coord1, coord2, coord3, alt_hue_path ) &
                   bind(c,name='c_plscmap1l')
                import :: c_ptr
                import :: private_plint, private_plbool, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: npts
                integer(kind=private_plbool), value, intent(in) :: rgbtype
                real(kind=private_plflt), dimension(*), intent(in) :: intensity, coord1, coord2, coord3
                !integer(kind=private_plbool), dimension(*), intent(in) :: alt_hue_path
                type(c_ptr), value, intent(in) :: alt_hue_path
            end subroutine interface_plscmap1l
        end interface

        npts_local = size(intensity,kind=private_plint)
        if ( present(alt_hue_path) ) then
            if( npts_local /= size(coord1, kind=private_plint) .or. &
                   npts_local /= size(coord2, kind=private_plint) .or. &
                   npts_local /= size(coord3, kind=private_plint) .or. &
                   npts_local /= size(alt_hue_path, kind=private_plint) + 1 ) then
                write(error_unit, "(a)") "Plplot Fortran Warning: plscmap1l: inconsistent sizes for &
                       &intensity, coord1, coord2, coord3, and/or alt_hue_path"
            end if
            allocate( ialt_hue_path_local(size(alt_hue_path)) )
            ialt_hue_path_local = int(merge(1,0,alt_hue_path),kind=private_plbool)

            call interface_plscmap1l( int(merge(1,0,rgbtype),kind=private_plbool), npts_local, &
                   real(intensity,kind=private_plflt), real(coord1,kind=private_plflt), &
                   real(coord2,kind=private_plflt), real(coord3,kind=private_plflt), &
                   c_loc(ialt_hue_path_local) )
            deallocate( ialt_hue_path_local )
        else
            if( npts_local /= size(coord1, kind=private_plint) .or. &
                   npts_local /= size(coord2, kind=private_plint) .or. &
                   npts_local /= size(coord3, kind=private_plint) ) then
                write(error_unit, "(a)") "Plplot Fortran Warning: plscmap1l: inconsistent sizes for &
                       &intensity, coord1, coord2, and/or coord3"
            end if
            call interface_plscmap1l( int(merge(1,0,rgbtype),kind=private_plbool), npts_local, &
                   real(intensity,kind=private_plflt), real(coord1,kind=private_plflt), &
                   real(coord2,kind=private_plflt), real(coord3,kind=private_plflt), &
                   c_null_ptr )
        endif
    end subroutine plscmap1l_impl

    subroutine plscmap1la_impl( rgbtype, intensity, coord1, coord2, coord3, alpha, alt_hue_path)
        logical, intent(in) :: rgbtype
        real(kind=wp), dimension(:), intent(in) :: intensity, coord1, coord2, coord3, alpha
        logical, dimension(:), optional, intent(in) :: alt_hue_path

        integer(kind=private_plint) :: npts_local
        integer(kind=private_plbool), dimension(:), allocatable, target :: ialt_hue_path_local

        interface
            subroutine interface_plscmap1la( rgbtype, n, intensity, coord1, coord2, coord3, alpha, alt_hue_path ) &
                   bind(c,name='c_plscmap1la')
                import :: c_ptr
                import :: private_plint,private_plbool, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                integer(kind=private_plbool), value, intent(in) :: rgbtype
                real(kind=private_plflt), dimension(*), intent(in) :: intensity, coord1, coord2, coord3, alpha
                !integer(kind=private_plbool), dimension(*), intent(in) :: alt_hue_path
                type(c_ptr), value, intent(in) :: alt_hue_path
            end subroutine interface_plscmap1la
        end interface

        npts_local = size(intensity, kind=private_plint)
        if ( present(alt_hue_path) ) then
            if( npts_local /= size(coord1, kind=private_plint) .or. &
                   npts_local /= size(coord2, kind=private_plint) .or. &
                   npts_local /= size(coord3, kind=private_plint) .or. &
                   npts_local /= size(alpha, kind=private_plint) .or. &
                   npts_local /= size(alt_hue_path, kind=private_plint) + 1 ) then
                write(error_unit, "(a)") "Plplot Fortran Warning: plscmap1la: inconsistent sizes for &
                       &intensity, coord1, coord2, coord3, alpha, and/or alt_hue_path"
            end if
            allocate( ialt_hue_path_local(size(alt_hue_path)) )
            ialt_hue_path_local = int(merge(1,0,alt_hue_path),kind=private_plbool)

            call interface_plscmap1la( int(merge(1,0,rgbtype),kind=private_plbool), npts_local, &
                   real(intensity,kind=private_plflt), real(coord1,kind=private_plflt), &
                   real(coord2,kind=private_plflt), real(coord3,kind=private_plflt), &
                   real(alpha,kind=private_plflt), c_loc(ialt_hue_path_local) )
            deallocate(ialt_hue_path_local)
        else
            if( npts_local /= size(coord1, kind=private_plint) .or. &
                   npts_local /= size(coord2, kind=private_plint) .or. &
                   npts_local /= size(coord3, kind=private_plint) .or. &
                   npts_local /= size(alpha, kind=private_plint) ) then
                write(error_unit, "(a)") "Plplot Fortran Warning: plscmap1la: inconsistent sizes for &
                       &intensity, coord1, coord2, coord3, and/or alpha"
            end if
            call interface_plscmap1la( int(merge(1,0,rgbtype),kind=private_plbool), npts_local, &
                   real(intensity,kind=private_plflt), real(coord1,kind=private_plflt), &
                   real(coord2,kind=private_plflt), real(coord3,kind=private_plflt), &
                   real(alpha,kind=private_plflt), c_null_ptr )
        endif
    end subroutine plscmap1la_impl

    subroutine plscol0a_impl( icol, r, g, b, a )
        integer, intent(in) :: icol, r, g, b
        real(kind=wp), intent(in) :: a

        interface
            subroutine interface_plscol0a( icol, r, g, b, a ) bind(c,name='c_plscol0a')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: icol, r, g, b
                real(kind=private_plflt), value, intent(in) :: a
            end subroutine interface_plscol0a
        end interface

        call interface_plscol0a( int(icol,kind=private_plint), int(r,kind=private_plint), int(g,kind=private_plint), &
               int(b,kind=private_plint), real(a,kind=private_plflt) )
    end subroutine plscol0a_impl

    subroutine plscolbga_impl( r, g, b, a )
        integer, intent(in) :: r, g, b
        real(kind=wp), intent(in) :: a

        interface
            subroutine interface_plscolbga( r, g, b, a ) bind(c,name='c_plscolbga')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: r, g, b
                real(kind=private_plflt), value, intent(in) :: a
            end subroutine interface_plscolbga
        end interface

        call interface_plscolbga( int(r,kind=private_plint), int(g,kind=private_plint), &
               int(b,kind=private_plint), real(a,kind=private_plflt) )
    end subroutine plscolbga_impl

    subroutine plsdidev_impl( mar, aspect, jx, jy )
        real(kind=wp), intent(in) :: mar, aspect, jx, jy

        interface
            subroutine interface_plsdidev( mar, aspect, jx, jy ) bind(c,name='c_plsdidev')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: mar, aspect, jx, jy
            end subroutine interface_plsdidev
        end interface

        call interface_plsdidev( real(mar,kind=private_plflt), real(aspect,kind=private_plflt), &
               real(jx,kind=private_plflt), real(jy,kind=private_plflt) )
    end subroutine plsdidev_impl

    subroutine plsdimap_impl( dimxmi, dimxmax, diymin, dimymax, dimxpmm, diypmm )
        real(kind=wp), intent(in) :: dimxmi, dimxmax, diymin, dimymax, dimxpmm, diypmm

        interface
            subroutine interface_plsdimap( dimxmi, dimxmax, diymin, dimymax, dimxpmm, diypmm ) bind(c,name='c_plsdimap')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: dimxmi, dimxmax, diymin, dimymax, dimxpmm, diypmm
            end subroutine interface_plsdimap
        end interface

        call interface_plsdimap( real(dimxmi,kind=private_plflt), real(dimxmax,kind=private_plflt), &
               real(diymin,kind=private_plflt), real(dimymax,kind=private_plflt), &
               real(dimxpmm,kind=private_plflt), real(diypmm,kind=private_plflt) )
    end subroutine plsdimap_impl

    subroutine plsdiori_impl( rot )
        real(kind=wp), intent(in) :: rot

        interface
            subroutine interface_plsdiori( rot) bind(c,name='c_plsdiori')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: rot
            end subroutine interface_plsdiori
        end interface

        call interface_plsdiori( real(rot,kind=private_plflt) )
    end subroutine plsdiori_impl

    subroutine plsdiplt_impl( xmin, ymin, xmax, ymax )
        real(kind=wp), intent(in) :: xmin, ymin, xmax, ymax

        interface
            subroutine interface_plsdiplt( xmin, ymin, xmax, ymax ) bind(c,name='c_plsdiplt')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xmin, ymin, xmax, ymax
            end subroutine interface_plsdiplt
        end interface

        call interface_plsdiplt( &
               real(xmin,kind=private_plflt), real(ymin,kind=private_plflt), &
               real(xmax,kind=private_plflt), real(ymax,kind=private_plflt) )
    end subroutine plsdiplt_impl

    subroutine plsdiplz_impl( xmin, ymin, xmax, ymax )
        real(kind=wp), intent(in) :: xmin, ymin, xmax, ymax

        interface
            subroutine interface_plsdiplz( xmin, ymin, xmax, ymax ) bind(c,name='c_plsdiplz')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xmin, ymin, xmax, ymax
            end subroutine interface_plsdiplz
        end interface

        call interface_plsdiplz( &
               real(xmin,kind=private_plflt), real(ymin,kind=private_plflt), &
               real(xmax,kind=private_plflt), real(ymax,kind=private_plflt) )
    end subroutine plsdiplz_impl

    subroutine plshade_impl_0( z, xmin, xmax, ymin, ymax, shade_min, shade_max, sh_cmap, sh_color, sh_width, &
           min_color, min_width, max_color, max_width, rectangular )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: sh_width, min_width, max_width
        real(kind=wp), intent(in) :: shade_min, shade_max, sh_color
        integer, intent(in) :: sh_cmap, min_color, max_color
        logical, intent(in) :: rectangular

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local

        call matrix_to_c( z, z_c_local, z_address_local )

        call interface_plshade_null( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(shade_min,kind=private_plflt), real(shade_max,kind=private_plflt), &
               int(sh_cmap,kind=private_plint), real(sh_color,kind=private_plflt), &
               real(sh_width,kind=private_plflt), &
               int(min_color,kind=private_plint), real(min_width,kind=private_plflt), &
               int(max_color,kind=private_plint), real(max_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool) )
    end subroutine plshade_impl_0

    subroutine plshade_impl_1( z, xmin, xmax, ymin, ymax, shade_min, shade_max, sh_cmap, sh_color, sh_width, &
           min_color, min_width, max_color, max_width, rectangular, xg, yg )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: sh_width, min_width, max_width
        real(kind=wp), intent(in) :: shade_min, shade_max, sh_color
        real(kind=wp), dimension(:), intent(in) :: xg, yg
        integer, intent(in) :: sh_cmap, min_color, max_color
        logical, intent(in) :: rectangular

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:), allocatable, target :: xg_in, yg_in
        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local
        type(PLcGrid), target :: cgrid_local

        call matrix_to_c( z, z_c_local, z_address_local )

        nx_in = size(z,1,kind=private_plint)
        ny_in = size(z,2,kind=private_plint)
        if(nx_in /= size(xg, kind=private_plint) .or. ny_in /= size(yg, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plshade: inconsistent sizes for z, xg, and/or yg"
        end if
        allocate( xg_in(nx_in), yg_in(ny_in) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in
        cgrid_local%ny = ny_in
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call interface_plshade( z_address_local, nx_in, ny_in, c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(shade_min,kind=private_plflt), real(shade_max,kind=private_plflt), &
               int(sh_cmap,kind=private_plint), real(sh_color,kind=private_plflt), &
               real(sh_width,kind=private_plflt), &
               int(min_color,kind=private_plint), real(min_width,kind=private_plflt), &
               int(max_color,kind=private_plint), real(max_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), &
               interface_pltr1, c_loc(cgrid_local) )
    end subroutine plshade_impl_1

    subroutine plshade_impl_2( z, xmin, xmax, ymin, ymax, shade_min, shade_max, sh_cmap, sh_color, sh_width, &
           min_color, min_width, max_color, max_width, rectangular, xg, yg )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: sh_width, min_width, max_width
        real(kind=wp), intent(in) :: shade_min, shade_max, sh_color
        real(kind=wp), dimension(:,:), intent(in) :: xg, yg
        integer, intent(in) :: sh_cmap, min_color, max_color
        logical, intent(in) :: rectangular

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: xg_in, yg_in
        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local
        type(PLcGrid), target :: cgrid_local

        call matrix_to_c( z, z_c_local, z_address_local )

        nx_in = size(z,1, kind=private_plint)
        ny_in = size(z,2, kind=private_plint)
        if( &
               nx_in /= size(xg, 1, kind=private_plint) .or. ny_in /= size(xg, 2, kind=private_plint) .or. &
               nx_in /= size(yg, 1, kind=private_plint) .or. ny_in /= size(xg, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plshade: inconsistent sizes for z, xg and/or yg"
        end if
        allocate( xg_in(nx_in, ny_in), yg_in(nx_in, ny_in) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in
        cgrid_local%ny = ny_in
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call interface_plshade( z_address_local, nx_in, ny_in, c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(shade_min,kind=private_plflt), real(shade_max,kind=private_plflt), &
               int(sh_cmap,kind=private_plint), real(sh_color,kind=private_plflt), &
               real(sh_width,kind=private_plflt), &
               int(min_color,kind=private_plint), real(min_width,kind=private_plflt), &
               int(max_color,kind=private_plint), real(max_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), &
               interface_pltr2f, c_loc(cgrid_local) )
    end subroutine plshade_impl_2

    subroutine plshade_impl_tr( z, xmin, xmax, ymin, ymax, shade_min, shade_max, sh_cmap, sh_color, sh_width, &
           min_color, min_width, max_color, max_width, rectangular, tr )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: sh_width, min_width, max_width
        real(kind=wp), intent(in) :: shade_min, shade_max, sh_color
        logical, intent(in) :: rectangular
        real(kind=wp), dimension(:), intent(in) :: tr
        integer, intent(in) :: sh_cmap, min_color, max_color

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local
        real(kind=private_plflt), dimension(6), target :: tr_in

        call matrix_to_c( z, z_c_local, z_address_local )
        tr_in = tr(1:6)

        call interface_plshade( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(shade_min,kind=private_plflt), real(shade_max,kind=private_plflt), &
               int(sh_cmap,kind=private_plint), real(sh_color,kind=private_plflt), &
               real(sh_width,kind=private_plflt), &
               int(min_color,kind=private_plint), real(min_width,kind=private_plflt), &
               int(max_color,kind=private_plint), real(max_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), plplot_private_pltr, c_loc(tr_in) )
    end subroutine plshade_impl_tr

    subroutine plshade_impl( z, xmin, xmax, ymin, ymax, shade_min, shade_max, sh_cmap, sh_color, sh_width, &
           min_color, min_width, max_color, max_width, rectangular, proc )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: sh_width, min_width, max_width
        real(kind=wp), intent(in) :: shade_min, shade_max, sh_color
        integer, intent(in) :: sh_cmap, min_color, max_color
        logical, intent(in) :: rectangular
        procedure(pltransform_proc) :: proc

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local

        call matrix_to_c( z, z_c_local, z_address_local )
        pltransform => proc

        call interface_plshade( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(shade_min,kind=private_plflt), real(shade_max,kind=private_plflt), &
               int(sh_cmap,kind=private_plint), real(sh_color,kind=private_plflt), &
               real(sh_width,kind=private_plflt), &
               int(min_color,kind=private_plint), real(min_width,kind=private_plflt), &
               int(max_color,kind=private_plint), real(max_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), pltransformf2c, c_null_ptr )
    end subroutine plshade_impl

    subroutine plshade_impl_data( z, xmin, xmax, ymin, ymax, shade_min, shade_max, sh_cmap, sh_color, sh_width, &
           min_color, min_width, max_color, max_width, rectangular, proc, data )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: sh_width, min_width, max_width
        real(kind=wp), intent(in) :: shade_min, shade_max, sh_color
        integer, intent(in) :: sh_cmap, min_color, max_color
        logical, intent(in) :: rectangular
        procedure(pltransform_proc_data) :: proc
        type(c_ptr), intent(in) :: data

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local

        call matrix_to_c( z, z_c_local, z_address_local )
        pltransform_data => proc

        call interface_plshade( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(shade_min,kind=private_plflt), real(shade_max,kind=private_plflt), &
               int(sh_cmap,kind=private_plint), real(sh_color,kind=private_plflt), &
               real(sh_width,kind=private_plflt), &
               int(min_color,kind=private_plint), real(min_width,kind=private_plflt), &
               int(max_color,kind=private_plint), real(max_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), pltransformf2c_data, data )
    end subroutine plshade_impl_data

    subroutine plshades_impl_0( z, xmin, xmax, ymin, ymax, clevel, fill_width, cont_color, cont_width, &
           rectangular )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: fill_width, cont_width
        real(kind=wp), dimension(:), intent(in) :: clevel
        integer, intent(in) :: cont_color
        logical, intent(in) :: rectangular

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local

        call matrix_to_c( z, z_c_local, z_address_local )

        call interface_plshades_null( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint), &
               real(fill_width,kind=private_plflt), &
               int(cont_color,kind=private_plint), real(cont_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool) )
    end subroutine plshades_impl_0

    subroutine plshades_impl_1( z, xmin, xmax, ymin, ymax, clevel, fill_width, cont_color, cont_width, &
           rectangular, xg, yg )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: fill_width, cont_width
        real(kind=wp), dimension(:), intent(in) :: clevel
        integer, intent(in) :: cont_color
        logical, intent(in) :: rectangular
        real(kind=wp), dimension(:), intent(in) :: xg, yg

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:), allocatable, target :: xg_in, yg_in
        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local
        type(PLcGrid), target :: cgrid_local

        call matrix_to_c( z, z_c_local, z_address_local )

        nx_in = size(z,1, kind=private_plint)
        ny_in = size(z,2, kind=private_plint)
        if( &
               nx_in /= size(xg, kind=private_plint) .or. ny_in /= size(yg, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plshades: inconsistent sizes for z, xg and/or yg"
        end if
        allocate( xg_in(nx_in), yg_in(ny_in) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in
        cgrid_local%ny = ny_in
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call interface_plshades( z_address_local, nx_in, ny_in, c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint), &
               real(fill_width,kind=private_plflt), &
               int(cont_color,kind=private_plint), real(cont_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), &
               interface_pltr1, c_loc(cgrid_local) )
    end subroutine plshades_impl_1

    subroutine plshades_impl_2( z, xmin, xmax, ymin, ymax, clevel, fill_width, cont_color, cont_width, &
           rectangular, xg, yg )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: fill_width, cont_width
        real(kind=wp), dimension(:), intent(in) :: clevel
        integer, intent(in) :: cont_color
        logical, intent(in) :: rectangular
        real(kind=wp), dimension(:,:), intent(in) :: xg, yg

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: xg_in, yg_in
        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local
        type(PLcGrid), target :: cgrid_local

        call matrix_to_c( z, z_c_local, z_address_local )

        nx_in = size(z,1, kind=private_plint)
        ny_in = size(z,2, kind=private_plint)
        if( &
               nx_in /= size(xg, 1, kind=private_plint) .or. ny_in /= size(xg, 2, kind=private_plint) .or. &
               nx_in /= size(yg, 1, kind=private_plint) .or. ny_in /= size(xg, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plshades: inconsistent sizes for z, xg and/or yg"
        end if

        allocate( xg_in(nx_in,ny_in), yg_in(nx_in,ny_in) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in
        cgrid_local%ny = ny_in
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call interface_plshades( z_address_local, nx_in, ny_in, c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint), &
               real(fill_width,kind=private_plflt), &
               int(cont_color,kind=private_plint), real(cont_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), &
               interface_pltr2f, c_loc(cgrid_local) )
    end subroutine plshades_impl_2

    subroutine plshades_impl_tr( z, xmin, xmax, ymin, ymax, clevel, fill_width, cont_color, cont_width, &
           rectangular, tr )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: fill_width, cont_width
        real(kind=wp), dimension(:), intent(in) :: clevel
        integer, intent(in) :: cont_color
        logical, intent(in) :: rectangular
        real(kind=wp), dimension(:), intent(in) :: tr

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local
        real(kind=private_plflt), dimension(6), target :: tr_in

        call matrix_to_c( z, z_c_local, z_address_local )
        tr_in = tr(1:6)

        call interface_plshades( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint), &
               real(fill_width,kind=private_plflt), &
               int(cont_color,kind=private_plint), real(cont_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), plplot_private_pltr, c_loc(tr_in) )
    end subroutine plshades_impl_tr

    subroutine plshades_impl( z, xmin, xmax, ymin, ymax, clevel, fill_width, cont_color, cont_width, &
           rectangular, proc )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: fill_width, cont_width
        real(kind=wp), dimension(:), intent(in) :: clevel
        integer, intent(in) :: cont_color
        logical, intent(in) :: rectangular
        procedure(pltransform_proc) :: proc

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local

        call matrix_to_c( z, z_c_local, z_address_local )
        pltransform => proc

        call interface_plshades( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint), &
               real(fill_width,kind=private_plflt), &
               int(cont_color,kind=private_plint), real(cont_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), pltransformf2c, c_null_ptr )
    end subroutine plshades_impl

    subroutine plshades_impl_data( z, xmin, xmax, ymin, ymax, clevel, fill_width, cont_color, cont_width, &
           rectangular, proc, data )
        real(kind=wp), dimension(:,:), intent(in) :: z
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax
        real(kind=wp), intent(in) :: fill_width, cont_width
        real(kind=wp), dimension(:), intent(in) :: clevel
        integer, intent(in) :: cont_color
        logical, intent(in) :: rectangular
        procedure(pltransform_proc_data) :: proc
        type(c_ptr), intent(in) :: data

        real(kind=private_plflt), dimension(:,:), allocatable :: z_c_local
        type(c_ptr), dimension(:), allocatable :: z_address_local

        call matrix_to_c( z, z_c_local, z_address_local )
        pltransform_data => proc

        call interface_plshades( z_address_local, size(z,1,kind=private_plint), size(z,2,kind=private_plint), c_null_ptr, &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint), &
               real(fill_width,kind=private_plflt), &
               int(cont_color,kind=private_plint), real(cont_width,kind=private_plflt), &
               interface_plfill, int(merge(1,0,rectangular),kind=private_plbool), pltransformf2c_data, data )
    end subroutine plshades_impl_data

    subroutine plsmaj_impl( def, scale )
        real(kind=wp), intent(in) :: def, scale

        interface
            subroutine interface_plsmaj( def, scale ) bind(c,name='c_plsmaj')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: def, scale
            end subroutine interface_plsmaj
        end interface

        call interface_plsmaj( real(def,kind=private_plflt), real(scale,kind=private_plflt) )

    end subroutine plsmaj_impl

    subroutine plsmin_impl( def, scale )
        real(kind=wp), intent(in) :: def, scale

        interface
            subroutine interface_plsmin( def, scale ) bind(c,name='c_plsmin')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: def, scale
            end subroutine interface_plsmin
        end interface

        call interface_plsmin( real(def,kind=private_plflt), real(scale,kind=private_plflt) )

    end subroutine plsmin_impl

    subroutine plspage_impl( xp, yp, xleng, yleng, xoff, yoff )
        integer, intent(in) :: xleng, yleng, xoff, yoff
        real(kind=wp), intent(in) :: xp, yp

        interface
            subroutine interface_plspage( xp, yp, xleng, yleng, xoff, yoff ) bind(c,name='c_plspage')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: xleng, yleng, xoff, yoff
                real(kind=private_plflt), value, intent(in) :: xp, yp
            end subroutine interface_plspage
        end interface

        call interface_plspage( real(xp,kind=private_plflt), real(yp,kind=private_plflt), &
               int(xleng,kind=private_plint), int(yleng,kind=private_plint), &
               int(xoff,kind=private_plint), int(yoff,kind=private_plint) )

    end subroutine plspage_impl

    subroutine plssym_impl( def, scale )

        real(kind=wp), intent(in) :: def, scale

        interface
            subroutine interface_plssym( def, scale ) bind(c,name='c_plssym')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: def, scale
            end subroutine interface_plssym
        end interface

        call interface_plssym( real(def,kind=private_plflt), real(scale,kind=private_plflt) )
    end subroutine plssym_impl

    subroutine plstring3_impl( x, y, z, string )

        real(kind=wp), dimension (:), intent(in) :: x, y, z
        character(len=*), intent(in) :: string

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plstring3( n, x, y, z, string ) bind(c,name='c_plstring3')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, z
                character(len=1), dimension(*), intent(in) :: string
            end subroutine interface_plstring3
        end interface

        n_local = size(x, kind=private_plint)
        if(n_local /= size(y, kind=private_plint) .or. n_local /= size(z, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plstring3: inconsistent sizes for x, y, and/or z"
        end if

        call interface_plstring3( n_local, real(x,kind=private_plflt), real(y,kind=private_plflt), real(z,kind=private_plflt), &
               trim(string)//c_null_char )
    end subroutine plstring3_impl

    subroutine plstring_impl( x, y, string )

        real(kind=wp), dimension (:), intent(in) :: x, y
        character(len=*), intent(in) :: string

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plstring( n, x, y, string ) bind(c,name='c_plstring')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                real(kind=private_plflt), dimension(*), intent(in) :: x, y
                character(len=1), dimension(*), intent(in) :: string
            end subroutine interface_plstring
        end interface

        n_local = size(x, kind=private_plint)
        if(n_local /= size(y, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plstring: inconsistent sizes for x and y"
        end if

        call interface_plstring( n_local, real(x,kind=private_plflt), real(y,kind=private_plflt), &
               trim(string)//c_null_char )
    end subroutine plstring_impl

    subroutine plstripa_impl( id, pen, x, y )

        integer, intent(in) :: id, pen
        real(kind=wp), intent(in) :: x, y

        interface
            subroutine interface_plstripa( id, pen, x, y ) bind(c,name='c_plstripa')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: id, pen
                real(kind=private_plflt), value, intent(in) :: x, y
            end subroutine interface_plstripa
        end interface

        call interface_plstripa( int(id, kind=private_plint), int(pen, kind=private_plint), &
               real(x,kind=private_plflt), real(y,kind=private_plflt) )
    end subroutine plstripa_impl

    subroutine plstripc_impl( &
           id, xspec, yspec, &
           xmin, xmax, xjump, ymin, ymax, &
           xlpos, ylpos, &
           y_ascl, acc, &
           colbox, collab, &
           colline, styline, legline, &
           labx, laby, labtop )

        logical, intent(in) :: y_ascl, acc
        integer, intent(in) :: colbox, collab
        integer, dimension(:), intent(in) :: colline, styline
        real(kind=wp), intent(in) :: xmin, xmax, xjump, ymin, ymax, xlpos, ylpos
        character(len=*), intent(in) :: xspec, yspec, labx, laby, labtop
        character(len=*), dimension(:), intent(in) :: legline
        integer, intent(out) :: id

        integer(kind=private_plint) :: id_out, n_pens_local
        character(len=1), dimension(:,:), allocatable :: cstring_legline_local
        type(c_ptr), dimension(:), allocatable :: cstring_address_legline_local

        interface
            subroutine interface_plstripc( &
                   id, xspec, yspec, &
                   xmin, xmax, xjump, ymin, ymax, &
                   xlpos, ylpos, &
                   y_ascl, acc, &
                   colbox, collab, &
                   colline, styline, legline, &
                   labx, laby, labtop ) bind(c,name='c_plstripc')
                import :: c_ptr
                import :: private_plint, private_plbool, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: colbox, collab
                integer(kind=private_plint), dimension(*), intent(in) :: colline, styline
                integer(kind=private_plbool), value, intent(in) :: y_ascl, acc
                real(kind=private_plflt), value, intent(in) :: xmin, xmax, xjump, ymin, ymax, xlpos, ylpos
                character(len=1), dimension(*), intent(in) :: xspec, yspec, labx, laby, labtop
                type(c_ptr), dimension(*), intent(in) :: legline
                integer(kind=private_plint), intent(out) :: id

            end subroutine interface_plstripc
        end interface

        n_pens_local = size(colline, kind=private_plint)
        if( &
               n_pens_local /= 4 .or. &
               n_pens_local /= size(styline, kind=private_plint) .or. &
               n_pens_local /= size(legline, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Severe Warning: plstripc: sizes of colline, styline, and/or legline are not 4"
            return
        endif

        call character_array_to_c( cstring_legline_local, cstring_address_legline_local, legline )

        call interface_plstripc( &
               id_out, trim(xspec)//c_null_char, trim(yspec)//c_null_char, &
               real(xmin, kind=private_plflt), real(xmax, kind=private_plflt), real(xjump, kind=private_plflt), &
               real(ymin, kind=private_plflt), real(ymax, kind=private_plflt), &
               real(xlpos, kind=private_plflt), real(ylpos, kind=private_plflt), &
               int(merge(1,0,y_ascl),kind=private_plbool), int(merge(1,0,acc),kind=private_plbool),&
               int(colbox, kind=private_plint), int(collab, kind=private_plint), &
               int(colline, kind=private_plint), int(styline, kind=private_plint), &
               cstring_address_legline_local, &
               trim(labx)//c_null_char, trim(laby)//c_null_char, trim(labtop)//c_null_char )
        id = int(id_out, kind=private_plint)
    end subroutine plstripc_impl

    subroutine plsurf3d_impl( x, y, z, opt, clevel )
        integer, intent(in) :: opt
        real(kind=wp), dimension(:), intent(in) :: x, y, clevel
        real(kind=wp), dimension(:,:), intent(in) :: z

        real(kind=private_plflt), dimension(:,:), allocatable :: zz_local
        type(c_ptr), dimension(:), allocatable :: zaddress_local

        integer(kind=private_plint) :: nx_local, ny_local

        interface
            subroutine interface_plsurf3d( x, y, zaddress, nx, ny, opt, clevel, nlevel ) bind(c,name='c_plsurf3d')
                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny, opt, nlevel
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, clevel
                type(c_ptr), dimension(*), intent(in) :: zaddress
            end subroutine interface_plsurf3d
        end interface

        nx_local = size(x,kind=private_plint)
        ny_local = size(y,kind=private_plint)

        if( nx_local /= size(z, 1, kind=private_plint) .or. ny_local /= size(z, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsurf3d: inconsistent sizes for x, y, and/or z"
        end if

        call matrix_to_c( z, zz_local, zaddress_local )

        call interface_plsurf3d( real(x,kind=private_plflt), real(y,kind=private_plflt), zaddress_local,  &
               nx_local, ny_local, int(opt,kind=private_plint), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint) )

    end subroutine plsurf3d_impl

    subroutine plsurf3dl_impl( x, y, z, opt, clevel, indexxmin, indexymin, indexymax )
        integer, intent(in) :: opt
        real(kind=wp), dimension(:), intent(in) :: x, y, clevel
        real(kind=wp), dimension(:,:), intent(in) :: z
        integer, intent(in) :: indexxmin
        integer, dimension(:), intent(in) :: indexymin, indexymax

        real(kind=private_plflt), dimension(:,:), allocatable :: zz_local
        type(c_ptr), dimension(:), allocatable :: zaddress_local

        integer(kind=private_plint) :: nx_local, ny_local, indexxmax_local

        interface
            subroutine interface_plsurf3dl( x, y, zaddress, nx, ny, opt, clevel, nlevel, &
                   indexxmin, indexxmax, indexymin, indexymax ) bind(c,name='c_plsurf3dl')
                import :: c_ptr
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny, opt, nlevel, indexxmin, indexxmax
                integer(kind=private_plint), dimension(*), intent(in) :: indexymin, indexymax
                real(kind=private_plflt), dimension(*), intent(in) :: x, y, clevel
                type(c_ptr), dimension(*), intent(in) :: zaddress
            end subroutine interface_plsurf3dl
        end interface

        nx_local = size(x,kind=private_plint)
        ny_local = size(y,kind=private_plint)
        if( nx_local /= size(z, 1, kind=private_plint) .or. ny_local /= size(z, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsurf3dl: inconsistent sizes for x, y, and/or z"
        end if

        indexxmax_local = size(indexymin)
        if( indexxmax_local /= size(indexymax, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsurf3dl: inconsistent sizes for indexymin and indeyxmax"
        end if

        call matrix_to_c( z, zz_local, zaddress_local )

        call interface_plsurf3dl( real(x,kind=private_plflt), real(y,kind=private_plflt), zaddress_local, &
               nx_local, ny_local, int(opt,kind=private_plint), &
               real(clevel,kind=private_plflt), size(clevel,kind=private_plint), &
               int(indexxmin,kind=private_plint), indexxmax_local, &
               int(indexymin,kind=private_plint), int(indexymax,kind=private_plint) )

    end subroutine plsurf3dl_impl

    subroutine plsvpa_impl( xmin, xmax, ymin, ymax )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax

        interface
            subroutine interface_plsvpa( xmin, xmax, ymin, ymax ) bind(c,name='c_plsvpa')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax
            end subroutine interface_plsvpa
        end interface

        call interface_plsvpa( real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt) )
    end subroutine plsvpa_impl

    ! Another variant defined in the plplot module.
    subroutine plsvect_impl( arrowx, arrowy, fill )
        logical, intent(in) :: fill
        real(kind=wp), dimension(:), intent(in) :: arrowx, arrowy

        integer(kind=private_plint) :: npts_local

        interface
            subroutine interface_plsvect( arrowx, arrowy, npts,  fill ) bind(c,name='c_plsvect')
                import :: private_plint, private_plbool, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: npts
                integer(kind=private_plbool), value, intent(in) :: fill
                real(kind=private_plflt), dimension(*), intent(in) :: arrowx, arrowy
            end subroutine interface_plsvect
        end interface

        npts_local = size(arrowx, kind=private_plint)
        if(npts_local /= size(arrowy, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsvect: sizes of arrowx and arrowy are not consistent"
        end if

        call interface_plsvect( real(arrowx, kind=private_plflt), real(arrowy, kind=private_plflt),  &
               npts_local, int(merge(1,0,fill), kind=private_plbool) )
    end subroutine plsvect_impl

    subroutine plsym_impl( x, y, code )
        integer, intent(in) :: code
        real(kind=wp), dimension(:), intent(in) :: x, y

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plsym( n, x, y, code ) bind(c,name='c_plsym')
                import :: private_plint, private_plflt
                implicit none
                integer(kind=private_plint), value, intent(in) :: n, code
                real(kind=private_plflt), dimension(*), intent(in) :: x, y
            end subroutine interface_plsym
        end interface

        n_local = size(x, kind=private_plint)
        if(n_local /= size(y, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsym: inconsistent sizes for x and y"
        end if

        call interface_plsym( n_local, real(x,kind=private_plflt), real(y,kind=private_plflt), &
               int(code,kind=private_plint) )
    end subroutine plsym_impl

    subroutine plvasp_impl( aspect)
        real(kind=wp), intent(in) :: aspect

        interface
            subroutine interface_plvasp( aspect ) bind(c,name='c_plvasp')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: aspect
            end subroutine interface_plvasp
        end interface

        call interface_plvasp( real(aspect,kind=private_plflt) )
    end subroutine plvasp_impl

    subroutine plvect_impl_0( u, v, scale )
        real(kind=wp), dimension(:,:), intent(in) :: u, v
        real(kind=wp), intent(in) :: scale

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: u_in, v_in
        type(PLfGrid), target :: fgrid1, fgrid2

        nx_in = size(u,1,kind=private_plint)
        ny_in = size(u,2,kind=private_plint)
        if( nx_in /= size(v,1,kind=private_plint) .or. ny_in /= size(v,2,kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plvect: inconsistent sizes for u and v"
        end if

        allocate( u_in(nx_in,ny_in) )
        allocate( v_in(nx_in,ny_in) )
        u_in = u
        v_in = v
        fgrid1%f  = c_loc(u_in)
        fgrid1%nx = nx_in
        fgrid1%ny = ny_in
        fgrid2%f  = c_loc(v_in)
        fgrid2%nx = nx_in
        fgrid2%ny = ny_in

        call interface_plfvect( interface_plf2evalr, fgrid1, fgrid2, nx_in, ny_in, &
               real(scale,kind=private_plflt), interface_pltr0, c_null_ptr )
    end subroutine plvect_impl_0

    subroutine plvect_impl_1( u, v, scale, xg, yg )
        real(kind=wp), dimension(:,:), intent(in) :: u, v
        real(kind=wp), dimension(:), intent(in) :: xg, yg
        real(kind=wp), intent(in) :: scale

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:), allocatable, target :: xg_in, yg_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: u_in, v_in
        type(PLfGrid), target :: fgrid1, fgrid2
        type(PLcGrid), target :: cgrid_local

        nx_in = size(u,1,kind=private_plint)
        ny_in = size(u,2,kind=private_plint)
        if(nx_in /= size(v,1,kind=private_plint) .or. ny_in /= size(v,2,kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plvect: inconsistent sizes for u and v"
        end if

        if(nx_in /= size(xg, kind=private_plint) .or. ny_in /= size(yg, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plvect: inconsistent sizes for u, xg, and/or yg"
        end if

        allocate( u_in(nx_in, ny_in) )
        allocate( v_in(nx_in, ny_in) )
        u_in = u
        v_in = v
        fgrid1%f  = c_loc(u_in)
        fgrid1%nx = nx_in
        fgrid1%ny = ny_in

        fgrid2%f  = c_loc(v_in)
        fgrid2%nx = nx_in
        fgrid2%ny = ny_in

        allocate( xg_in(nx_in), yg_in(ny_in) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in
        cgrid_local%ny = ny_in
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call interface_plfvect( interface_plf2evalr, fgrid1, fgrid2, nx_in, ny_in, &
               real(scale,kind=private_plflt), interface_pltr1, c_loc(cgrid_local) )
    end subroutine plvect_impl_1

    subroutine plvect_impl_2( u, v, scale, xg, yg )
        real(kind=wp), dimension(:,:), intent(in) :: u, v
        real(kind=wp), dimension(:,:), intent(in) :: xg, yg
        real(kind=wp), intent(in) :: scale

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: u_in, v_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: xg_in, yg_in
        type(PLfGrid), target :: fgrid1, fgrid2
        type(PLcGrid), target :: cgrid_local

        nx_in = size(u,1,kind=private_plint)
        ny_in = size(u,2,kind=private_plint)
        if(nx_in /= size(v,1,kind=private_plint) .or. ny_in /= size(v,2,kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plvect: inconsistent sizes for u and v"
        end if

        if( &
               nx_in /= size(xg, 1, kind=private_plint) .or. ny_in /= size(xg, 2, kind=private_plint) .or. &
               nx_in /= size(yg, 1, kind=private_plint) .or. ny_in /= size(xg, 2, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plvect: inconsistent sizes for u, xg, and/or yg"
        end if

        allocate( u_in(nx_in, ny_in) )
        allocate( v_in(nx_in, ny_in) )
        u_in = u
        v_in = v
        fgrid1%f  = c_loc(u_in)
        fgrid1%nx = nx_in
        fgrid1%ny = ny_in

        fgrid2%f  = c_loc(v_in)
        fgrid2%nx = nx_in
        fgrid2%ny = ny_in

        allocate( xg_in(nx_in, ny_in), yg_in(nx_in, ny_in) )
        xg_in = xg
        yg_in = yg
        cgrid_local%nx = nx_in
        cgrid_local%ny = ny_in
        cgrid_local%xg = c_loc(xg_in)
        cgrid_local%yg = c_loc(yg_in)

        call interface_plfvect( interface_plf2evalr, fgrid1, fgrid2, nx_in, ny_in, &
               real(scale,kind=private_plflt), interface_pltr2f, c_loc(cgrid_local) )
    end subroutine plvect_impl_2

    subroutine plvect_impl_tr( u, v, scale, tr )
        real(kind=wp), dimension(:,:), intent(in) :: u, v
        real(kind=wp), intent(in) :: scale
        real(kind=wp), dimension(:), intent(in) :: tr

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable, target :: u_in, v_in
        real(kind=private_plflt), dimension(6), target :: tr_in
        type(PLfGrid), target :: fgrid1, fgrid2

        nx_in = size(u,1,kind=private_plint)
        ny_in = size(u,2,kind=private_plint)
        if(nx_in /= size(v,1,kind=private_plint) .or. ny_in /= size(v,2,kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plvect: inconsistent sizes for u and v"
        end if

        allocate( u_in(nx_in,ny_in) )
        allocate( v_in(nx_in,ny_in) )
        u_in = u
        v_in = v
        fgrid1%f  = c_loc(u_in)
        fgrid1%nx = nx_in
        fgrid1%ny = ny_in
        fgrid2%f  = c_loc(v_in)
        fgrid2%nx = nx_in
        fgrid2%ny = ny_in

        tr_in = tr(1:6)

        call interface_plfvect( interface_plf2evalr, fgrid1, fgrid2, nx_in, ny_in, &
               real(scale,kind=private_plflt), plplot_private_pltr, c_loc(tr_in) )
    end subroutine plvect_impl_tr

    subroutine plvect_impl( u, v, scale, proc )
        real(kind=wp), dimension(:,:), intent(in) :: u, v
        real(kind=wp), intent(in) :: scale
        procedure(pltransform_proc) :: proc

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable :: u_local, v_local
        type(c_ptr), dimension(:), allocatable :: u_address_local, v_address_local

        nx_in = size(u,1,kind=private_plint)
        ny_in = size(u,2,kind=private_plint)
        if( nx_in /= size(v,1,kind=private_plint) .or. ny_in /= size(v,2,kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plvect: inconsistent sizes for u and v"
        end if

        call matrix_to_c( u, u_local, u_address_local )
        call matrix_to_c( v, v_local, v_address_local )
        pltransform => proc

        call interface_plvect( u_address_local, v_address_local, nx_in, ny_in, &
               real(scale,kind=private_plflt), pltransformf2c, c_null_ptr )
    end subroutine plvect_impl

    subroutine plvect_impl_data( u, v, scale, proc, data )
        real(kind=wp), dimension(:,:), intent(in) :: u, v
        real(kind=wp), intent(in) :: scale
        procedure(pltransform_proc_data) :: proc
        type(c_ptr), intent(in) :: data

        integer(kind=private_plint) :: nx_in, ny_in
        real(kind=private_plflt), dimension(:,:), allocatable :: u_local, v_local
        type(c_ptr), dimension(:), allocatable :: u_address_local, v_address_local

        nx_in = size(u,1,kind=private_plint)
        ny_in = size(u,2,kind=private_plint)
        if( nx_in /= size(v,1,kind=private_plint) .or. ny_in /= size(v,2,kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plvect: inconsistent sizes for u and v"
        end if

        call matrix_to_c( u, u_local, u_address_local )
        call matrix_to_c( v, v_local, v_address_local )
        pltransform_data => proc

        call interface_plvect( u_address_local, v_address_local, nx_in, ny_in, &
               real(scale,kind=private_plflt), pltransformf2c_data, data )
    end subroutine plvect_impl_data

    subroutine plvpas_impl( xmin, xmax, ymin, ymax, aspect )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax, aspect

        interface
            subroutine interface_plvpas( xmin, xmax, ymin, ymax, aspect ) bind(c,name='c_plvpas')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax, aspect
            end subroutine interface_plvpas
        end interface

        call interface_plvpas( real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt), real(aspect,kind=private_plflt) )
    end subroutine plvpas_impl

    subroutine plvpor_impl( xmin, xmax, ymin, ymax )
        real(kind=wp), intent(in) :: xmin, xmax, ymin, ymax

        interface
            subroutine interface_plvpor( xmin, xmax, ymin, ymax ) bind(c,name='c_plvpor')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax
            end subroutine interface_plvpor
        end interface

        call interface_plvpor( real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt)  )
    end subroutine plvpor_impl

    subroutine plw3d_impl( basex, basey, height, xmin, xmax, ymin, ymax, zmin, zmax, alt, az )
        real(kind=wp), intent(in) :: basex, basey, height, xmin, xmax, ymin, ymax, zmin, zmax, alt, az

        interface
            subroutine interface_plw3d( basex, basey, height, xmin, xmax, ymin, ymax, zmin, zmax, alt, az ) bind(c,name='c_plw3d')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: basex, basey, height, xmin, xmax, ymin, ymax, zmin, zmax, alt, az
            end subroutine interface_plw3d
        end interface

        call interface_plw3d( real(basex,kind=private_plflt), real(basey,kind=private_plflt), real(height,kind=private_plflt), &
               real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), real(ymin,kind=private_plflt),     &
               real(ymax,kind=private_plflt), real(zmin,kind=private_plflt), real(zmax,kind=private_plflt),     &
               real(alt,kind=private_plflt), real(az,kind=private_plflt) )
    end subroutine plw3d_impl

    subroutine plwidth_impl( width )
        real(kind=wp), intent(in) :: width

        interface
            subroutine interface_plwidth( width ) bind(c,name='c_plwidth')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: width
            end subroutine interface_plwidth
        end interface

        call interface_plwidth( real(width,kind=private_plflt) )
    end subroutine plwidth_impl

    subroutine plwind_impl( xmin, xmax, ymin, ymax )
        real(kind=wp), intent(in):: xmin, xmax, ymin, ymax

        interface
            subroutine interface_plwind( xmin, xmax, ymin, ymax ) bind(c,name='c_plwind')
                import :: private_plflt
                implicit none
                real(kind=private_plflt), value, intent(in) :: xmin, xmax, ymin, ymax
            end subroutine interface_plwind
        end interface

        call interface_plwind( real(xmin,kind=private_plflt), real(xmax,kind=private_plflt), &
               real(ymin,kind=private_plflt), real(ymax,kind=private_plflt) )
    end subroutine plwind_impl
