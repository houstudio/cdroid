!    Generates polar plot with, 1-1 scaling
!
!    Copyright (C) 2004-2016 Alan W. Irwin
!
!    This file is part of PLplot.
!
!    PLplot is free software; you can redistribute it and/or modify
!    it under the terms of the GNU Library General Public License as
!    published by the Free Software Foundation; either version 2 of the
!    License, or (at your option) any later version.
!
!    PLplot is distributed in the hope that it will be useful,
!    but WITHOUT ANY WARRANTY; without even the implied warranty of
!    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!    GNU Library General Public License for more details.
!
!    You should have received a copy of the GNU Library General Public
!    License along with PLplot; if not, write to the Free Software
!    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

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
program x03f
    use plplot, double_PI => PL_PI
    use plfortrandemolib
    implicit none

    real(kind=pl_test_flt), parameter :: PI = double_PI
    character (len=3) :: text
    real(kind=pl_test_flt), dimension(0:360) :: x0, y0, x, y
    real(kind=pl_test_flt) :: dtr, theta, dx, dy, offset
    integer :: i
    integer :: plparseopts_rc
    !    Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !   Set orientation to portrait - note not all device drivers
    !   support this, in particular most interactive drivers do not.
    call plsori(1)

    dtr = PI/180.0_pl_test_flt
    x0 = cos(dtr * arange(361))
    y0 = sin(dtr * arange(361))

    !    Initialize PLplot

    call plinit()

    !    Set up viewport and window, but do not draw box

    call plenv(-1.3_pl_test_flt, 1.3_pl_test_flt, -1.3_pl_test_flt, 1.3_pl_test_flt, 1, -2)
    !   Draw circles for polar grid
    do i = 1,10
        call plarc(0.0_pl_test_flt, 0.0_pl_test_flt, 0.1_pl_test_flt*i, 0.1_pl_test_flt*i, &
               0.0_pl_test_flt, 360.0_pl_test_flt, 0.0_pl_test_flt, .false.)
    enddo
    call plcol0(2)
    do i = 0,11
        theta = 30.0_pl_test_flt*i
        dx = cos(dtr*theta)
        dy = sin(dtr*theta)

        !      Draw radial spokes for polar grid

        call pljoin(0.0_pl_test_flt, 0.0_pl_test_flt, dx, dy)
        write (text,'(i3)') nint(theta)

        !      Write labels for angle

        text = adjustl(text)

        if (theta .lt. 9.99) then
            offset = 0.45
        elseif (theta .lt. 99.9) then
            offset = 0.30
        else
            offset = 0.15
        endif
        !      Slightly off zero to avoid floating point logic flips at
        !      90 and 270 deg.
        if (dx >= -0.00001_pl_test_flt) then
            call plptex(dx, dy, dx, dy, -offset, text)
        else
            call plptex(dx, dy, -dx, -dy, 1._pl_test_flt+offset, text)
        end if
    enddo
    !    Draw the graph

    x = x0 * sin(5.0_pl_test_flt * dtr * arange(361))
    y = y0 * sin(5.0_pl_test_flt * dtr * arange(361))

    call plcol0(3)
    call plline(x,y)

    call plcol0(4)
    call plmtex('t', 2.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, &
           '#frPLplot Example 3 - r(#gh)=sin 5#gh')

    !    Close the plot at end

    call plend
end program x03f
