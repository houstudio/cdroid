!   Alpha color values demonstration
!
!   Copyright (C) 2008 Hazen Babcock
!   Copyright (C) 2008 Andrew Ross
!   Copyright (C) 2008-2016 Alan W. Irwin
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
!
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

program x30f
    use plplot

    implicit none

    integer, dimension(4) ::  red, green, blue
    real(kind=pl_test_flt), dimension (4) :: alpha, px, py
    real(kind=pl_test_flt), dimension (2) :: pos, rcoord, gcoord, bcoord, acoord
    logical, dimension(1) :: rev

    data red   / 0, 255, 0, 0 /
    data green / 0, 0, 255, 0 /
    data blue  / 0, 0, 0, 255 /
    data alpha / 1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt /
    data px / 0.1_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, 0.1_pl_test_flt /
    data py / 0.1_pl_test_flt, 0.1_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt /
    data pos / 0.0_pl_test_flt, 1.0_pl_test_flt /
    data rcoord / 1.0_pl_test_flt, 1.0_pl_test_flt /
    data gcoord / 0.0_pl_test_flt, 0.0_pl_test_flt /
    data bcoord / 0.0_pl_test_flt, 0.0_pl_test_flt /
    data acoord / 0.0_pl_test_flt, 1.0_pl_test_flt /
    data rev / .false. /

    integer i, j
    integer icol, r, g, b
    integer :: plparseopts_rc
    real(kind=pl_test_flt) :: a

    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    call plinit()

    call plscmap0n(4)

    call plscmap0a (red, green, blue, alpha)

    !
    ! Page 1:
    !
    ! This is a series of red, green and blue rectangles overlaid
    ! on each other with gradually increasing transparency.
    !

    ! Set up the window
    call pladv (0)
    call plvpor (0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt)
    call plwind (0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt)
    call plcol0 (0)
    call plbox ("", 1.0_pl_test_flt, 0, "", 1.0_pl_test_flt, 0)

    ! Draw the boxes
    do i = 1,9
        icol = mod(i-1,3) + 1

        ! Get a color, change its transparency and
        ! set it as the current color.
        call plgcol0a (icol, r, g, b, a)
        call plscol0a (icol, r, g, b, 1.0_pl_test_flt - real(i-1,kind=pl_test_flt)/9.0_pl_test_flt)
        call plcol0 (icol)

        ! Draw the rectangle
        call plfill (px, py)

        ! Shift the rectangles coordinates
        do j = 1,4
            px(j) = px(j) + 0.5_pl_test_flt/9.0_pl_test_flt
            py(j) = py(j) + 0.5_pl_test_flt/9.0_pl_test_flt
        enddo
    enddo
    !
    ! Page 2:
    !
    ! This is a bunch of boxes colored red, green or blue with a single
    ! large (red) box of linearly varying transparency overlaid. The
    ! overlaid box is completely transparent at the bottom and completely
    ! opaque at the top.
    !

    ! Set up the window
    call pladv(0)
    call plvpor(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
    call plwind(0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt)

    ! Draw the boxes. There are 25 of them drawn on a 5 x 5 grid.
    do i = 1,5
        ! Set box X position
        px(1) = 0.05_pl_test_flt + 0.2_pl_test_flt * real(i-1,kind=pl_test_flt)
        px(2) = px(1) + 0.1_pl_test_flt
        px(3) = px(2)
        px(4) = px(1)

        ! We don't want the boxes to be transparent, so since we changed
        ! the colors transparencies in the first example we have to change
        ! the transparencies back to completely opaque.
        icol = mod(i-1,3) + 1
        call plgcol0a (icol, r, g, b, a)
        call plscol0a (icol, r, g, b, 1.0_pl_test_flt)
        call plcol0 (icol)
        do j = 1, 5
            ! Set box y position and draw the box.
            py(1) = 0.05_pl_test_flt + 0.2_pl_test_flt * real(j-1,kind=pl_test_flt)
            py(2) = py(1)
            py(3) = py(1) + 0.1_pl_test_flt
            py(4) = py(3)
            call plfill(px, py)
        enddo
    enddo

    ! Create the color map with 128 colors and call plscmap1la to initialize
    ! the color values with a linearly varying red transparency (or alpha)
    call plscmap1n(128)
    call plscmap1la(.true., pos, rcoord, gcoord, bcoord, acoord, rev)

    ! Use that cmap1 to create a transparent red gradient for the whole
    ! window.
    px(1) = 0._pl_test_flt
    px(2) = 1._pl_test_flt
    px(3) = 1._pl_test_flt
    px(4) = 0._pl_test_flt

    py(1) = 0._pl_test_flt
    py(2) = 0._pl_test_flt
    py(3) = 1._pl_test_flt
    py(4) = 1._pl_test_flt

    call plgradient( px, py, 90._pl_test_flt )
    call plend()

end program x30f

