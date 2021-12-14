!   Pie chart demo.
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
program x13f
    use plplot, double_PI => PL_PI
    implicit none
    real(kind=pl_test_flt), parameter :: PI = double_PI
    real(kind=pl_test_flt)   :: just, dx, dy, x(500), y(500)
    integer            ::  i, j, dthet, theta0, theta1, theta
    integer :: plparseopts_rc
    character(len=20), dimension(5) :: text = &
           (/ 'Maurice ', 'Geoffrey', 'Alan    ', 'Rafael  ', 'Vince   '/)
    real(kind=pl_test_flt)   :: per(5) = &
           (/ 10._pl_test_flt , 32._pl_test_flt , 12._pl_test_flt , 30._pl_test_flt , 16._pl_test_flt /)

    !   Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !   Ask user to specify the output device.
    call plinit()

    call pladv(0)
    !   Ensure window has aspect ratio of one so circle is
    !   plotted as a circle.
    call plvasp(1.0_pl_test_flt)
    call plwind(0._pl_test_flt, 10._pl_test_flt, 0._pl_test_flt, 10._pl_test_flt)
    !   call plenv( 0._pl_test_flt, 10._pl_test_flt, 0._pl_test_flt, 10._pl_test_flt, 1, -2 )
    call plcol0(2)

    !   n.b. all theta quantities scaled by 2*pi/500 to be integers to avoid
    !   floating point logic problems.
    theta0 = 0
    dthet = 1

    do i = 0, 4
        j = 0
        x(j+1) = 5._pl_test_flt
        y(j+1) = 5._pl_test_flt
        j = j + 1
        !       n.b. the theta quantities multiplied by 2*pi/500 afterward so
        !       in fact per is interpreted as a percentage.
        theta1 = int(theta0 + 5*per(i+1))
        if (i .eq. 4) theta1 = 500
        do theta = theta0, theta1, dthet
            x(j+1) = 5 + 3*cos((2._pl_test_flt*pi/500._pl_test_flt)*theta)
            y(j+1) = 5 + 3*sin((2._pl_test_flt*pi/500._pl_test_flt)*theta)
            j = j + 1
        enddo

        call plcol0(i+1)
        call plpsty( mod(i+3, 8) + 1 )
        call plfill(x(:j), y(:j))
        call plcol0(1)
        call plline(x(:j), y(:j))

        just = (2._pl_test_flt*pi/500._pl_test_flt)*(theta0 + theta1) / 2._pl_test_flt
        dx = 0.25_pl_test_flt * cos(just)
        dy = 0.25_pl_test_flt * sin(just)
        if ((theta0  + theta1) .lt. 250 .or. &
               (theta0 + theta1) .gt. 750) then
            just = 0._pl_test_flt
        else
            just = 1._pl_test_flt
        endif
        call plptex( x(j/2+1)+dx, y(j/2+1)+dy, 1._pl_test_flt, 0._pl_test_flt, just, text(i+1) )
        theta0 = theta - dthet
    enddo

    call plfont(2)
    call plschr( 0._pl_test_flt, 1.3_pl_test_flt)
    call plptex( 5._pl_test_flt, 9._pl_test_flt, 1._pl_test_flt, 0._pl_test_flt, 0.5_pl_test_flt, 'Percentage of Sales' )

    call plend
end program x13f
