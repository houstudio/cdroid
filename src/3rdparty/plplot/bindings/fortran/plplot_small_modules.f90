!***********************************************************************
!  plplot_small_modules.f90
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
!***********************************************************************

module plplot_types
    use iso_c_binding, only: c_ptr, c_int32_t, c_float, c_double
    implicit none
    private :: c_ptr, c_int32_t, c_float, c_double

    ! Specify Fortran types used by the various modules below.

    ! N.B. It is those modules' responsibility to keep these precision values
    ! private.

    ! These types are used along with function overloading so that
    ! applications do not need a specific real type at all (under the
    ! constraint that all real arguments must have consistent real type
    ! for a particular call to a routine in the Fortran binding of
    ! PLplot.)

    ! This include file only defines the private_plflt parameter at the
    ! moment which is configured to be either c_float or c_double
    ! to agree with the configured real precision (PLFLT) of the PLplot
    ! C library.
    include 'included_plplot_configured_types.f90'

    ! The idea here is to match the Fortran integer type with the
    ! corresponding C types for PLINT (normally int32_t), PLBOOL
    ! (currently typedefed to PLINT) and PLUNICODE (normally
    ! uint32_t).  In the past we have used 4 for this purpose with
    ! good success for both the gfortran and Intel compilers.  That
    ! is, kind=4 corresponded to 4-byte integers for those compilers.
    ! But kind=4 may not do that for other compilers so we are now
    ! using a more standards-compliant approach as recommended by
    ! Wadud Miah of the NAG group.

    ! The kind c_int32_t defined in ISO_C_BINDING is meant to match the
    ! C type int32_t, which is used for PLINT and PLBOOL. As there
    ! is no equivalent for unsigned integers in Fortran, we use this
    ! kind for PLUNICODE as well.
    integer, parameter :: private_plint  = c_int32_t
    integer, parameter :: private_plbool  = c_int32_t
    integer, parameter :: private_plunicode  = c_int32_t

    ! Define parameters for specific real precisions, so that we can
    ! specify equivalent interfaces for all precisions (kinds)
    integer, parameter :: private_single  = c_float
    integer, parameter :: private_double  = c_double

    ! The PLfGrid and PLcGrid types transfer information about a multidimensional
    ! array to the plcontour/plshade family of routines.

    type, bind(c) :: PLfGrid
        type(c_ptr) :: f
        integer(kind=private_plint) :: nx, ny, nz
    end type PLfGrid

    type, bind(c) :: PLcGrid
        type(c_ptr) :: xg, yg, zg
        integer(kind=private_plint) :: nx, ny, nz
    end type PLcGrid

end module plplot_types

module plplot_private_utilities
    use iso_c_binding, only: c_ptr, c_char, c_null_char, c_loc, c_size_t, c_f_pointer
    use iso_fortran_env, only: error_unit
    implicit none
    private :: c_ptr, c_char, c_null_char, c_loc, c_size_t, c_f_pointer, error_unit

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
        ! Use standard C library function strlen to determine C string length excluding terminating NULL.
        function interface_strlen(s) bind(c, name='strlen')
            import c_ptr, c_size_t
            implicit none
            integer(c_size_t) :: interface_strlen
            type(c_ptr), intent(in), value :: s
        end function interface_strlen
    end interface
    private :: interface_strlen

contains

    subroutine character_array_to_c( cstring_array, cstring_address, character_array )
        ! Translate from Fortran character_array to an array of C strings (cstring_array), where the
        ! address of the start of each C string is stored in the cstring_address vector.
        ! N.B. cstring_array is only an argument to keep those allocatable data in scope for the calling
        ! routine.
        character(len=*), dimension(:), intent(in) :: character_array
        character(len=1), dimension(:,:), allocatable, target, intent(out) :: cstring_array
        type(c_ptr), dimension(:), allocatable, intent(out) :: cstring_address

        integer :: j_local, length_local, number_local, length_column_local

        ! length of character string
        length_local = len(character_array)
        ! number of character strings in array
        number_local = size(character_array)

        ! Leave room for trailing c_null_char if the Fortran character string is
        ! filled with non-blank characters to the end.
        allocate( cstring_array(length_local+1, number_local) )
        allocate( cstring_address(number_local) )

        do j_local = 1, number_local
            length_column_local = len(trim(character_array(j_local))) + 1
            ! Drop all trailing blanks in Fortran character string when converting to C string.
            cstring_array(1:length_column_local, j_local) = &
                   transfer(trim(character_array(j_local))//c_null_char, " ", length_column_local)
            cstring_address(j_local) = c_loc(cstring_array(1,j_local))
        enddo

    end subroutine character_array_to_c

    function c_to_character_array( character_array, cstring_address_array )
        ! Translate from an array of pointers to NULL-terminated C strings (cstring_address_array)
        ! to a Fortran character array (character_array).
        integer :: c_to_character_array
        character(len=*), dimension(:), intent(out) :: character_array
        type(c_ptr), dimension(:), intent(in) :: cstring_address_array

        integer :: i_local, j_local, length_local, number_local, length_column_local
        ! Array for accessing string pointed to by an element of cstring_address_array
        character(kind=c_char), pointer :: string_ptr(:)

        length_local = len(character_array)
        number_local = size(cstring_address_array)
        if(number_local > size(character_array)) then
            write(error_unit, *) "Error in c_to_character_array: size of character_array too small to hold converted result."
        endif

        do j_local = 1, number_local
            length_column_local = interface_strlen(cstring_address_array(j_local))
            if(length_column_local > length_local) then
                write(error_unit, *) &
                    "Error in c_to_character_array: length of character_array too small to hold converted result."
                c_to_character_array = 1
                return
            endif
            ! Copy contents of string addressed by cstring_address_array(j_local) and of
            ! length length_column_local to string_ptr pointer array which
            ! is dynamically allocated as needed.
            call c_f_pointer(cstring_address_array(j_local), string_ptr, [length_column_local])
            do i_local = 1, length_column_local
                character_array(j_local)(i_local:i_local) = string_ptr(i_local)
            enddo
            ! append blanks to character_array element
            character_array(j_local)(length_column_local+1:) = " "
        enddo
        c_to_character_array = 0
    end function c_to_character_array

    subroutine copystring2f( fstring, cstring )
        character(len=*), intent(out) :: fstring
        character(len=1), dimension(:), intent(in) :: cstring

        integer :: i_local

        fstring = ' '
        do i_local = 1,min(len(fstring),size(cstring))
            if ( cstring(i_local) /= c_null_char ) then
                fstring(i_local:i_local) = cstring(i_local)
            else
                exit
            endif
        enddo
    end subroutine copystring2f

    function max_cstring_length(cstring_address_array)
        ! Find maximum length (excluding the NULL-terminating character)
        ! of the C strings pointed to by cstring_address_array
        integer :: max_cstring_length
        type(c_ptr), dimension(:), intent(in) :: cstring_address_array

        integer :: j_local, number_local
        number_local = size(cstring_address_array)

        max_cstring_length = 0
        do j_local = 1, number_local
            max_cstring_length = max(max_cstring_length, interface_strlen(cstring_address_array(j_local)))
        enddo
    end function max_cstring_length

end module plplot_private_utilities

module plplot_graphics
    use plplot_types, only: private_plint, private_plflt, private_double
    use plplot_private_utilities, only: copystring2f
    implicit none
    private :: private_plint, private_plflt, private_double

    ! This is a public derived Fortran type that contains all the
    ! information in private_PLGraphicsIn below, but in standard
    ! Fortran form rather than C form.
    type :: PLGraphicsIn
        integer                   :: type           ! of event (CURRENTLY UNUSED)
        integer                   :: state          ! key or button mask
        integer                   :: keysym         ! key selected
        integer                   :: button         ! mouse button selected
        integer                   :: subwindow      ! subwindow (alias subpage, alias subplot) number
        character(len=16)         :: string         ! Fortran character string
        integer                   :: pX, pY         ! absolute device coordinates of pointer
        real(kind=private_double) :: dX, dY         ! relative device coordinates of pointer
        real(kind=private_double) :: wX, wY         ! world coordinates of pointer
    end type PLGraphicsIn

    interface plGetCursor
        module procedure plGetCursor_impl
    end interface plGetCursor
    private :: plGetCursor_impl

contains

    function plGetCursor_impl( gin )

        ! According to a gfortran build error message the combination of bind(c) and
        ! private attributes is not allowed for a derived type so to keep
        ! private_PLGraphicsIn actually private declare it inside the function
        ! rather than before the contains.

        ! This derived type is a direct equivalent of the C struct because
        ! of the bind(c) attribute and interoperable nature of all the
        ! types. (See <https://gcc.gnu.org/onlinedocs/gfortran/Derived-Types-and-struct.html> for
        ! further discussion.)

        ! Note the good alignment (offset is a multiple of 8 bytes) of the
        ! trailing dX, dY, wX, and wY for the case when private_plflt refers
        ! to double precision.
        type, bind(c) :: private_PLGraphicsIn
            integer(kind=private_plint)     :: type           ! of event (CURRENTLY UNUSED)
            integer(kind=private_plint)     :: state          ! key or button mask
            integer(kind=private_plint)     :: keysym         ! key selected
            integer(kind=private_plint)     :: button         ! mouse button selected
            integer(kind=private_plint)     :: subwindow      ! subwindow (alias subpage, alias subplot) number
            character(len=1), dimension(16) :: string         ! NULL-terminated character string
            integer(kind=private_plint)     :: pX, pY         ! absolute device coordinates of pointer
            real(kind=private_plflt)        :: dX, dY         ! relative device coordinates of pointer
            real(kind=private_plflt)        :: wX, wY         ! world coordinates of pointer
        end type private_PLGraphicsIn


        type(PLGraphicsIn), intent(out) :: gin
        integer :: plGetCursor_impl  !function type

        type(private_PLGraphicsIn) :: gin_out

        interface
            function interface_plGetCursor( gin ) bind(c,name='plGetCursor')
                import :: private_PLGraphicsIn, private_plint
                implicit none
                integer(kind=private_plint) :: interface_plGetCursor !function type
                type(private_PLGraphicsIn), intent(out) :: gin
            end function interface_plGetCursor
        end interface

        plGetCursor_impl = int(interface_plGetCursor( gin_out ))
        ! Copy all gin_out elements to corresponding gin elements with
        ! appropriate type conversions.
        gin%type = int(gin_out%type)
        gin%state = int(gin_out%state)
        gin%keysym = int(gin_out%keysym)
        gin%button = int(gin_out%button)
        gin%subwindow = int(gin_out%subwindow)
        call copystring2f( gin%string, gin_out%string )
        gin%pX = int(gin_out%pX)
        gin%pY = int(gin_out%pY)
        gin%dX = real(gin_out%dX, kind=private_double)
        gin%dY = real(gin_out%dY, kind=private_double)
        gin%wX = real(gin_out%wX, kind=private_double)
        gin%wY = real(gin_out%wY, kind=private_double)
    end function plGetCursor_impl

end module plplot_graphics

! The bind(c) attribute exposes the pltr routine which ought to be private
module plplot_private_exposed
    use iso_c_binding, only: c_ptr, c_f_pointer
    use plplot_types, only: private_plflt
    implicit none
    private :: c_ptr, private_plflt
contains
    subroutine plplot_private_pltr( x, y, tx, ty, tr_in ) bind(c)
        real(kind=private_plflt), value, intent(in) :: x, y
        real(kind=private_plflt), intent(out) :: tx, ty
        type(c_ptr), value, intent(in) :: tr_in

        real(kind=private_plflt), dimension(:), pointer :: tr

        call c_f_pointer( tr_in, tr, [6] )

        tx = tr(1) * x + tr(2) * y + tr(3)
        ty = tr(4) * x + tr(5) * y + tr(6)
    end subroutine plplot_private_pltr

end module plplot_private_exposed
