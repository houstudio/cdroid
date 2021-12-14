!   Demonstration program for PLplot: Bar chart example.
!
!   Copyright (C) 2004-2016  Alan W. Irwin
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
program x12f
    use plplot
    implicit none

    character(len=20) :: string
    integer :: i
    integer :: plparseopts_rc
    real(kind=pl_test_flt) :: y0(10)
    real(kind=pl_test_flt) :: pos(5)   = (/0.0_pl_test_flt, 0.25_pl_test_flt, 0.5_pl_test_flt, 0.75_pl_test_flt, 1.0_pl_test_flt/)
    real(kind=pl_test_flt) :: red(5)   = (/0.0_pl_test_flt, 0.25_pl_test_flt, 0.5_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt/)
    real(kind=pl_test_flt) :: green(5) = (/1.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, 1.0_pl_test_flt/)
    real(kind=pl_test_flt) :: blue(5)  = (/1.0_pl_test_flt, 1.0_pl_test_flt, 0.5_pl_test_flt, 0.25_pl_test_flt, 0.0_pl_test_flt/)

    !   Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    call plinit()

    call pladv(0)
    call plvsta
    call plwind( 1980._pl_test_flt, 1990._pl_test_flt, 0._pl_test_flt, 35._pl_test_flt )
    call plbox( 'bc', 1._pl_test_flt, 0, 'bcnv', 10._pl_test_flt, 0 )
    call plcol0(2)
    call pllab( 'Year', 'Widget Sales (millions)', '#frPLplot Example 12' )

    y0 = (/ 5, 15, 12, 24, 28, 30, 20, 8, 12, 3 /)

    call plscmap1l(.true.,pos,red,green,blue)

    do i = 0, 9
        !       call plcol0(i + 1)
        call plcol1(real(i,kind=pl_test_flt)/9.0_pl_test_flt)
        call plpsty(0)
        call plfbox( 1980._pl_test_flt+i, y0(i+1) )
        write (string, '(i0)') int(y0(i+1))
        call plptex( 1980._pl_test_flt+i+0.5_pl_test_flt, y0(i+1)+1._pl_test_flt, &
               1._pl_test_flt, 0._pl_test_flt, 0.5_pl_test_flt, string )
        write (string, '(i0)')1980+i
        call plmtex( 'b', 1._pl_test_flt, (i+1)*0.1_pl_test_flt-0.05_pl_test_flt, 0.5_pl_test_flt, string )
    enddo

    !    Don't forget to call PLEND to finish off!
    call plend
contains

    subroutine plfbox(x0, y0)
        real(kind=pl_test_flt) x0, y0, x(4), y(4)

        x(1) = x0
        y(1) = 0._pl_test_flt
        x(2) = x0
        y(2) = y0
        x(3) = x0+1._pl_test_flt
        y(3) = y0
        x(4) = x0+1._pl_test_flt
        y(4) = 0._pl_test_flt
        call plfill(x, y)
        call plcol0(1)
        call pllsty(1)
        call plline(x, y)
    end subroutine plfbox
end program x12f
