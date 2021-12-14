!   Simple line plot and multiple windows demo.
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
program x01f
    use plplot, double_PI => PL_PI
    use :: iso_fortran_env, only: stdout => output_unit
    use plfortrandemolib

    implicit none

    real(kind=pl_test_flt), parameter :: PI = double_PI
    real(pl_test_flt) :: xscale, yscale, xoff, yoff
    character(len=80) :: version
    integer :: digmax
    logical, parameter :: locate_mode = .false.
    logical, parameter :: pl_parse_dynamic = .false.
    logical, parameter :: pl_parse_static_length = .false.
    logical, parameter :: pl_parse_static = .false.
!   Declarations for general parsing of the command line
    integer arg_count, nargv
    character(len=200), dimension(0:20) :: argv_static
    character(len=200), dimension(:), allocatable :: argv_static_length
    character(len=:), dimension(:), allocatable :: argv_dynamic

    type(PLGraphicsIn) :: gin
    integer :: PLK_Escape
    integer :: plget_arguments_rc, plparseopts_rc
    data PLK_Escape /Z'1B'/

!   Process command-line arguments
    if(pl_parse_dynamic) then
       write(0,*) "pl_parse_dynamic = ", pl_parse_dynamic
       plget_arguments_rc = plget_arguments(argv_dynamic)
       if(plget_arguments_rc /= 0) stop "plget_arguments error"
       nargv = size(argv_dynamic) - 1
       write(*, '(a)') "argv before call to plparseopts(..., PL_PARSE_SKIP)"
       do arg_count = 0, nargv
          write(*,'(a,i4,a,a)') "i =", arg_count, ", argument = ", trim(argv_dynamic(arg_count))
       enddo
       plparseopts_rc = plparseopts(argv_dynamic, PL_PARSE_SKIP)
       if(plparseopts_rc /= 0) stop "plparseopts error"
       nargv = size(argv_dynamic) - 1

       write(*, '(a)') "argv after call to plparseopts(..., PL_PARSE_SKIP)"
       do arg_count = 0, nargv
          write(*,'(a,i4,a,a)') "i =", arg_count, ", argument = ", trim(argv_dynamic(arg_count))
       enddo
       deallocate(argv_dynamic)
    elseif(pl_parse_static_length) then
       write(0,*) "pl_parse_static_length = ", pl_parse_static_length
       plget_arguments_rc = plget_arguments(argv_static_length, 1)
       if(plget_arguments_rc /= 0) stop "plget_arguments error"
       nargv = size(argv_static_length) - 1
       write(*, '(a)') "argv before call to plparseopts(..., PL_PARSE_SKIP)"
       do arg_count = 0, nargv
          write(*,'(a,i4,a,a)') "i =", arg_count, ", argument = ", trim(argv_static_length(arg_count))
       enddo
       plparseopts_rc = plparseopts(argv_static_length, PL_PARSE_SKIP, 1)
       if(plparseopts_rc /= 0) stop "plparseopts error"
       nargv = size(argv_static_length) - 1

       write(*, '(a)') "argv after call to plparseopts(..., PL_PARSE_SKIP)"
       do arg_count = 0, nargv
          write(*,'(a,i4,a,a)') "i =", arg_count, ", argument = ", trim(argv_static_length(arg_count))
       enddo
       deallocate(argv_static_length)
    elseif(pl_parse_static) then
       write(0,*) "pl_parse_static = ", pl_parse_static
       plget_arguments_rc = plget_arguments(nargv, argv_static)
       if(plget_arguments_rc /= 0) stop "plget_arguments error"
       write(*, '(a)') "argv before call to plparseopts(..., PL_PARSE_SKIP)"
       do arg_count = 0, nargv
          write(*,'(a,i4,a,a)') "i =", arg_count, ", argument = ", trim(argv_static(arg_count))
       enddo
       plparseopts_rc = plparseopts(nargv, argv_static(0:nargv), PL_PARSE_SKIP)
       if(plparseopts_rc /= 0) stop "plparseopts error"

       write(*, '(a)') "argv after call to plparseopts(..., PL_PARSE_SKIP)"
       do arg_count = 0, nargv
          write(*,'(a,i4,a,a)') "i =", arg_count, ", argument = ", trim(argv_static(arg_count))
       enddo
    else
       plparseopts_rc = plparseopts(PL_PARSE_SKIP)
       if(plparseopts_rc .ne. 0) stop "plparseopts error"
    endif

    !  Print plplot version
    call plgver(version)
    write (*,'(a,a)') 'PLplot library version: ', trim(version)

    !  Initialize plplot
    !  Divide page into 2x2 plots
    call plstar(2,2)

    !  Set up the data
    !  Original case

    xscale = 6._pl_test_flt
    yscale = 1._pl_test_flt
    xoff = 0._pl_test_flt
    yoff = 0._pl_test_flt

    !  Do a plot

    call plot1()

    !  Set up the data

    xscale = 1._pl_test_flt
    yscale = 0.0014_pl_test_flt
    yoff = 0.0185_pl_test_flt

    !  Do a plot

    digmax = 5
    call plsyax(digmax,  0)
    call plot1()

    call plot2()
    call plot3()

    if( locate_mode ) then
        do while(.true.)

            ! plGetCursor returns zero value for some devices (e.g., xwin) when
            ! the user hits the escape key or hits a mouse button or key when outside
            ! a subwindow.
            if( plGetCursor( gin ) == 0 ) exit

            ! For some devices (e.g., xcairo) plGetCursor always returns 0, but if the user hits the escape
            ! key or hits a mouse button or key when outside a subwindow, the returned keysym is PLK_escape
            if( gin%keysym == PLK_escape ) exit

            write(stdout, "(a,i3,a,f0.6,a,f0.6,a,f0.6,a,f0.6)") &
                   "subwin = ", gin%subwindow, ", wx = ", gin%wX, ", wy = ", gin%wY, &
                   ", dx = ", gin%dX, ", dy = ", gin%dY
            write(stdout, "(a,z0,a,z0,a,a,z0,a,z0)") &
                   "keysym = 0x", gin%keysym, ", button = 0x", gin%button, ", string = '"//trim(gin%string)//"'", &
                   ", type = 0x", gin%type, ", state = 0x", gin%state
        enddo
    end if

    !  Don't forget to call PLEND to finish off!

    call plend()

contains

    !======================================================================
    subroutine plot1()

        real(pl_test_flt), dimension(1:60) :: x, y
        real(pl_test_flt) :: xmin, xmax, ymin, ymax

        x = xoff + xscale * arange(1,size(x)+1) / real(size(x),pl_test_flt)
        y = yoff + yscale * x ** 2

        xmin = minval(x)
        xmax = maxval(x)
        ymin = minval(y)
        ymax = maxval(y)

        !   Set up the viewport and window using PLENV. The range in X is
        !   0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
        !   scaled separately (just = 0), and we just draw a labelled
        !   box (axis = 0).

        call plcol0(1)
        call plenv( xmin, xmax, ymin, ymax, 0, 0 )
        call plcol0(2)
        call pllab( '(x)', '(y)', '#frPLplot Example 1 - y=x#u2' )

        !   Plot the data points
        !   Only plot every tenth!

        call plcol0(4)
        call plpoin( x(4::10), y(4::10), 9 )

        !   Draw the line through the data

        call plcol0(3)
        call plline( x, y )

    end subroutine plot1

    !======================================================================
    subroutine plot2()

        real(pl_test_flt), dimension(1:100) :: x, y

        !
        !   Set up the viewport and window using PLENV. The range in X is
        !   -2.0 to 10.0, and the range in Y is -0.4 to 2.0. The axes are
        !   scaled separately (just = 0), and we draw a box with axes
        !   (axis = 1).

        call plcol0(1)
        call plenv(-2.0_pl_test_flt, 10.0_pl_test_flt, -0.4_pl_test_flt, 1.2_pl_test_flt, 0, 1 )
        call plcol0(2)
        call pllab( '(x)', 'sin(x)/x', '#frPLplot Example 1 - Sinc Function' )

        !   Fill up the arrays


        x = ( arange(size(x)) - 19 ) / 6.0_pl_test_flt
        y = merge( sin(x) / x, 1.0_pl_test_flt, x /= 0.0_pl_test_flt )

        !   Draw the line

        call plcol0(3)
        call plwidth(2._pl_test_flt)
        call plline( x, y )
        call plwidth(1._pl_test_flt)

    end subroutine plot2

    !======================================================================
    subroutine plot3()

        !
        !   For the final graph we wish to override the default tick intervals,
        !   and so do not use_ PLENV

        real(pl_test_flt), dimension(1:101) :: x, y

        call pladv(0)

        !   Use_ standard viewport, and define X range from 0 to 360 degrees,
        !   Y range from -1.2 to 1.2.

        call plvsta()
        call plwind( 0.0_pl_test_flt, 360.0_pl_test_flt, -1.2_pl_test_flt, 1.2_pl_test_flt )

        !   Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.

        call plcol0(1)
        call plbox( 'bcnst', 60.0_pl_test_flt, 2, 'bcnstv', 0.2_pl_test_flt, 2 )

        !   Superimpose a dashed line grid, with 1.5 mm marks and spaces. With
        !   only a single mark and space element, we do not need arrays

        call plstyl( (/1500/), (/1500/) )
        call plcol0(2)
        call plbox( 'g', 30.0_pl_test_flt, 0, 'g', 0.2_pl_test_flt, 0 )
        call plstyl( (/integer ::/), (/integer ::/) )

        call plcol0(3)
        call pllab( 'Angle (degrees)', 'sine', '#frPLplot Example 1 - Sine function' )

        x = 3.6_pl_test_flt * arange(size(x))
        y = sin( x * PI/180.0_pl_test_flt )

        call plcol0(4)
        call plline( x, y )

    end subroutine plot3

end program x01f
