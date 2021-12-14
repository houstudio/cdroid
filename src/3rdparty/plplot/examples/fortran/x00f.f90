!   Simple demo of a 2D line plot.
!
!   Copyright (C) 2011-2016  Alan W. Irwin
!
!   This file is part of PLplot.
!
!   PLplot is free software; you can redistribute it and/or modify
!   it under the terms of the GNU Library General Public License as published
!   by the Free Software Foundation; either version 2 of the License, or
!   (at your option) any later version.
!
!   PLplot is distributed in the hope that it will be useful,
!   but WITHOUT ANY WARRANTY; without even the implied warranty of
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!   GNU Library General Public License for more details.
!
!   You should have received a copy of the GNU Library General Public License
!   along with PLplot; if not, write to the Free Software
!   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

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
program x00f
    use plfortrandemolib

    integer, parameter :: NSIZE = 101

    real(kind=pl_test_flt), dimension(NSIZE) :: x, y

    real(kind=pl_test_flt) :: xmin = 0._pl_test_flt, xmax = 1._pl_test_flt, ymin = 0._pl_test_flt, ymax = 100._pl_test_flt
    ! integer          :: i
    integer :: plparseopts_rc

    ! Prepare data to be plotted.
    x = arange(NSIZE) / real(NSIZE-1,pl_test_flt)
    y = ymax * x**2

    ! Or alternatively, using a DO-loop
    !do i = 1,NSIZE
    !    x(i) = real( i - 1, pl_test_flt ) / real( NSIZE - 1, pl_test_flt )
    !    y(i) = ymax * x(i)**2
    !enddo

    ! Parse and process command line arguments
    plparseopts_rc = plparseopts( PL_PARSE_FULL )
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    ! Initialize plplot
    call plinit

    ! Create a labelled box to hold the plot.
    call plenv( xmin, xmax, ymin, ymax, 0, 0 )
    call pllab( "x", "y=100 x#u2#d", "Simple PLplot demo of a 2D line plot" )

    ! Plot the data that was prepared above.
    call plline( x, y )

    ! Close PLplot library
    call plend

end program x00f
