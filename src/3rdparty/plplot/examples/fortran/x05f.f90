!   Draws a histogram from sample data
!
!   Copyright (C) 2004-2016 Alan W. Irwin
!
!   This file is part of PLplot.
!
!   PLplot is free software; you can redistribute it and/or modify
!   it under the terms of the GNU Library General Public License as
!   published by the Free Software Foundation; either version 2 of the
!   License, or (at your option) any later version.
!
!   PLplot is distributed in the hope that it will be useful,
!   but WITHOUT ANY WARRANTY; without even the implied warranty of
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!   GNU Library General Public License for more details.
!
!   You should have received a copy of the GNU Library General Public
!   License along with PLplot; if not, write to the Free Software
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
program x04f
    use plplot, double_PI => PL_PI
    use plfortrandemolib
    implicit none

    real(kind=pl_test_flt), parameter :: PI = double_PI
    integer, parameter :: NPTS = 2047
    integer :: plparseopts_rc
    real(kind=pl_test_flt) :: data(NPTS), delta

    !   Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !   Initialize plplot
    call plinit()

    !   Fill up data points
    delta = 2.0_pl_test_flt * PI / real(NPTS,kind=pl_test_flt)
    data = sin(delta*arange(NPTS))

    call plcol0(1)
    call plhist(data, -1.1_pl_test_flt, 1.1_pl_test_flt, 44, 0)
    call plcol0(2)
    call pllab('#frValue','#frFrequency', &
           '#frPLplot Example 5 - Probability function of Oscillator')

    call plend
end program x04f
