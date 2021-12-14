!***********************************************************************
!  plplot_double.f90
!
!  Copyright (C) 2005-2016  Arjen Markus
!  Copyright (C) 2006-2016 Alan W. Irwin
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
!***********************************************************************

module plplot_double
    use iso_c_binding, only: c_ptr, c_char, c_null_char, c_null_ptr, c_loc, c_funptr, c_null_funptr, c_funloc, c_associated
    use iso_fortran_env, only: error_unit
    use plplot_types, only: private_plflt, private_plint, private_plbool, private_double, PLcGrid, PLfGrid
    use plplot_private_exposed
    use plplot_private_utilities, only: character_array_to_c
    implicit none

    integer, parameter :: wp = private_double
    private :: c_ptr, c_char, c_null_char, c_null_ptr, c_loc, c_funptr, c_null_funptr, c_funloc
    private :: error_unit
    private :: private_plflt, private_plint, private_plbool, private_double, PLcGrid, PLfGrid
    private :: character_array_to_c
    private :: wp

    ! Private interfaces for wp-precision callbacks
    private :: plmapformf2c, pllabelerf2c, pllabelerf2c_data, pltransformf2c, pltransformf2c_data

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
    ! procedure.

    interface
        subroutine interface_plslabelfunc( proc, data ) bind(c, name = 'c_plslabelfunc' )
            import :: c_funptr, c_ptr
            type(c_funptr), value, intent(in) :: proc
            type(c_ptr), value, intent(in) :: data
        end subroutine interface_plslabelfunc
    end interface
    private :: interface_plslabelfunc

    interface
        subroutine interface_plstransform( proc, data ) bind(c, name = 'c_plstransform' )
            import :: c_funptr, c_ptr
            type(c_funptr), value, intent(in) :: proc
            type(c_ptr), value, intent(in) :: data
        end subroutine interface_plstransform
    end interface
    private :: interface_plstransform

    ! Routines that have floating-point attributes that nevertheless
    ! cannot be disambiguated so we only provide them for the
    ! double-precision case (rather than using a separate naming
    ! convention for these routines or some other complexity for users
    ! to distinguish the double- and single-precision cases).

    interface plrandd
        ! Only provide double-precison version because of
        ! disambiguation problems with the corresponding
        ! single-precision versions.
        module procedure plrandd_impl
    end interface plrandd
    private :: plrandd_impl

    interface plslabelfunc
        ! Only provide double-precison versions because of
        ! disambiguation problems with the corresponding
        ! single-precision versions.
        module procedure plslabelfunc_impl_data
        module procedure plslabelfunc_impl
        module procedure plslabelfunc_impl_null
    end interface plslabelfunc
    private :: plslabelfunc_impl_data
    private :: plslabelfunc_impl
    private :: plslabelfunc_impl_null

    interface plstransform
        ! Only provide double-precison versions because of
        ! disambiguation problems with the corresponding
        ! single-precision versions.
        module procedure plstransform_impl_data
        module procedure plstransform_impl
        module procedure plstransform_impl_null
    end interface plstransform
    private :: plstransform_impl_data
    private :: plstransform_impl
    private :: plstransform_impl_null

    ! Routines that have floating-point attributes that can
    ! be disambiguated.
    include 'included_plplot_real_interfaces.f90'

    ! Routines that have floating-point attributes that nevertheless
    ! cannot be disambiguated so we only provide them for the
    ! double-precision case (rather than using a separate naming
    ! convention for these routines or some other complexity for users
    ! to distinguish the double- and single-precision cases).

    ! Return type is not part of the disambiguation so we provide
    ! one explicit double-precision version rather than both types.
    function plrandd_impl()

        real(kind=wp) :: plrandd_impl !function type

        interface
            function interface_plrandd() bind(c,name='c_plrandd')
                import :: private_plflt
                implicit none
                real(kind=private_plflt) :: interface_plrandd !function type
            end function interface_plrandd
        end interface

        plrandd_impl = real(interface_plrandd(), kind=wp)
    end function plrandd_impl

    ! Only provide double-precison version because of disambiguation
    ! problems with the corresponding single-precision version.
    subroutine plslabelfunc_impl_data( proc, data )
        procedure(pllabeler_proc_data) :: proc
        type(c_ptr), value, intent(in) :: data
        pllabeler_data => proc
        call interface_plslabelfunc( c_funloc(pllabelerf2c_data), data )
    end subroutine plslabelfunc_impl_data

    ! Only provide double-precison version because of disambiguation
    ! problems with the corresponding single-precision version.
    subroutine plslabelfunc_impl( proc )
        procedure(pllabeler_proc) :: proc
        pllabeler => proc
        call interface_plslabelfunc( c_funloc(pllabelerf2c), c_null_ptr )
    end subroutine plslabelfunc_impl

    subroutine plslabelfunc_impl_null
        call interface_plslabelfunc( c_null_funptr, c_null_ptr )
    end subroutine plslabelfunc_impl_null

    ! Only provide double-precison version because of disambiguation
    ! problems with the corresponding single-precision version.
    subroutine plstransform_impl_data( proc, data )
        procedure(pltransform_proc_data) :: proc
        type(c_ptr), value, intent(in) :: data
        pltransform_data => proc
        call interface_plstransform( c_funloc(pltransformf2c_data), data )
    end subroutine plstransform_impl_data

    ! Only provide double-precison version because of disambiguation
    ! problems with the corresponding single-precision version.
    subroutine plstransform_impl( proc )
        procedure(pltransform_proc) :: proc
        pltransform => proc
        call interface_plstransform( c_funloc(pltransformf2c), c_null_ptr )
    end subroutine plstransform_impl

    subroutine plstransform_impl_null
        call interface_plstransform( c_null_funptr, c_null_ptr )
    end subroutine plstransform_impl_null

    ! plflt-precision callback routines that are called from C and which wrap a call to wp-precision Fortran routines.

    subroutine plmapformf2c( n, x, y ) bind(c, name = 'plplot_double_private_plmapformf2c')
        integer(kind=private_plint), value, intent(in) :: n
        real(kind=private_plflt), dimension(n), intent(inout) :: x, y

        real(kind=wp), dimension(:), allocatable :: x_inout, y_inout

        allocate(x_inout(n), y_inout(n))

        x_inout = real(x, kind=wp)
        y_inout = real(y, kind=wp)

        call plmapform( x_inout, y_inout )
        x = real(x_inout, kind=private_plflt)
        y = real(y_inout, kind=private_plflt)
    end subroutine plmapformf2c

    subroutine pllabelerf2c( axis, value, label, length, data ) bind(c, name = 'plplot_double_private_pllabelerf2c')
        integer(kind=private_plint), value, intent(in) :: axis, length
        real(kind=private_plflt), value, intent(in) :: value
        character(len=1), dimension(*), intent(out) :: label
        type(c_ptr), value, intent(in) :: data

        character(len=:), allocatable :: label_out
        integer :: trimmed_length

        if ( c_associated(data) ) then
            write(*,*) 'PLPlot: error in pllabelerf2c - data argument should be NULL'
            stop
        endif

        allocate(character(length) :: label_out)
        call pllabeler( int(axis), real(value,kind=wp), label_out )
        trimmed_length = min(length,len_trim(label_out) + 1)
        label(1:trimmed_length) = transfer(trim(label_out(1:length))//c_null_char, " ", trimmed_length)
        deallocate(label_out)
    end subroutine pllabelerf2c

    subroutine pllabelerf2c_data( axis, value, label, length, data ) bind(c, name = 'plplot_double_private_pllabelerf2c_data')
        integer(kind=private_plint), value, intent(in) :: axis, length
        real(kind=private_plflt), value, intent(in) :: value
        character(len=1), dimension(*), intent(out) :: label
        type(c_ptr), value, intent(in) :: data

        character(len=:), allocatable :: label_out
        integer :: trimmed_length

        allocate(character(length) :: label_out)
        call pllabeler_data( int(axis), real(value,kind=wp), label_out, data )
        trimmed_length = min(length,len_trim(label_out) + 1)
        label(1:trimmed_length) = transfer(trim(label_out(1:length))//c_null_char, " ", trimmed_length)
        deallocate(label_out)
    end subroutine pllabelerf2c_data

    subroutine pltransformf2c( x, y, tx, ty, data ) bind(c, name = 'plplot_double_private_pltransformf2c')
        real(kind=private_plflt), value, intent(in) :: x, y
        real(kind=private_plflt), intent(out) :: tx, ty
        type(c_ptr), value, intent(in) :: data

        real(kind=wp) :: tx_out, ty_out

        if ( c_associated(data) ) then
            write(*,*) 'PLPlot: error in pltransfrom2c - data argument should be NULL'
            stop
        endif

        call pltransform( real(x,kind=wp), real(y,kind=wp), tx_out, ty_out )
        tx = tx_out
        ty = ty_out
    end subroutine pltransformf2c

    subroutine pltransformf2c_data( x, y, tx, ty, data ) bind(c, name = 'plplot_double_private_pltransformf2c_data')
        real(kind=private_plflt), value, intent(in) :: x, y
        real(kind=private_plflt), intent(out) :: tx, ty
        type(c_ptr), value, intent(in) :: data

        real(kind=wp) :: tx_out, ty_out

        call pltransform_data( real(x,kind=wp), real(y,kind=wp), tx_out, ty_out, data )
        tx = tx_out
        ty = ty_out
    end subroutine pltransformf2c_data

end module plplot_double
