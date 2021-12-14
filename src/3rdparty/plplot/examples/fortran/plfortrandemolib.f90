!      Copyright (C) 2011  Arjen Markus
!      Copyright (C) 2005  Andrew Ross
!      Copyright (C) 2016  Alan W. Irwin
!
!      This file is part of PLplot.
!
!      PLplot is free software; you can redistribute it and/or modify
!      it under the terms of the GNU Library General Public License as
!      published by the Free Software Foundation; either version 2 of the
!      License, or (at your option) any later version.
!
!      PLplot is distributed in the hope that it will be useful,
!      but WITHOUT ANY WARRANTY; without even the implied warranty of
!      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!      GNU Library General Public License for more details.
!
!      You should have received a copy of the GNU Library General Public
!      License along with PLplot; if not, write to the Free Software
!      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

!     N.B. the pl_test_flt parameter used in this code is only
!     provided by the plplot module to allow convenient developer
!     testing of either kind(1.0) or kind(1.0d0) floating-point
!     precision regardless of the floating-point precision of the
!     PLplot C libraries.  We do not guarantee the value of this test
!     parameter so it should not be used by users, and instead user
!     code should replace the pl_test_flt parameter by whatever
!     kind(1.0) or kind(1.0d0) precision is most convenient for them.
!     For further details on floating-point precision issues please
!     consult README_precision in this directory.
!
!      Module containing auxiliary routines for the demo programs

module plfortrandemolib
    use plplot
    implicit none

    interface arange
        module procedure arange_3
        module procedure arange_2
        module procedure arange_1
    end interface arange

contains

    ! Function to return an arithmetic series (modelled after the Numpy
    ! function of the same name)
    !
    function arange_3( start, stop, step )
        integer, intent(in) :: start, stop, step

        real(kind=pl_test_flt), dimension((stop-start)/step) :: arange_3

        integer             :: i

        arange_3 = (/ (real(i, pl_test_flt), i = start, stop-step, step) /)
    end function arange_3

    function arange_2( start, stop )
        integer, intent(in) :: start, stop

        real(kind=pl_test_flt), dimension((stop-start)) :: arange_2

        arange_2 = arange_3( start, stop, 1 )
    end function arange_2

    function arange_1( stop )
        integer, intent(in) :: stop

        real(kind=pl_test_flt), dimension(stop) :: arange_1

        arange_1 = arange_3( 0, stop, 1 )
    end function arange_1

end module plfortrandemolib
