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
!--------------------------------------------------------------------------
!   main
!
!   Does a series of 3-d plots for a given data set, with different
!   viewing options in each plot.
!--------------------------------------------------------------------------

program x18f
    use plplot, double_PI => PL_PI, double_TWOPI => PL_TWOPI
    implicit none
    real(kind=pl_test_flt), parameter :: PI = double_PI
    real(kind=pl_test_flt), parameter :: TWOPI = double_TWOPI

    integer, parameter :: NPTS = 1000

    integer            :: i, k
    real(kind=pl_test_flt), dimension(NPTS) :: x, y, z, r
    character(len=80)  :: title

    integer            :: opt(4) = (/    1,          0,          1,          0       /)
    real(kind=pl_test_flt)   :: alt(4) = (/ 20.0_pl_test_flt, 35.0_pl_test_flt, 50.0_pl_test_flt, 65.0_pl_test_flt /)
    real(kind=pl_test_flt)   :: az(4)  = (/ 30.0_pl_test_flt, 40.0_pl_test_flt, 50.0_pl_test_flt, 60.0_pl_test_flt /)

    integer, dimension(NPTS) :: ia = (/(i,i=1,NPTS)/)
    integer :: plparseopts_rc

    !   Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !   Initialize plplot

    call plinit()

    do k = 1, 4
        call test_poly(k, alt(k), az(k))
    enddo

    !   From the mind of a sick and twisted physicist...

    z = -1._pl_test_flt + 2._pl_test_flt * real(ia-1,kind=pl_test_flt) / real(NPTS,kind=pl_test_flt)

    !   Pick one ...

    !   r = 1. - real(ia-1,kind=pl_test_flt) / real(NPTS,kind=pl_test_flt)
    r = z

    x = r * cos( 2._pl_test_flt * PI * 6._pl_test_flt * real(ia-1,kind=pl_test_flt) / real(NPTS,kind=pl_test_flt) )
    y = r * sin( 2._pl_test_flt * PI * 6._pl_test_flt * real(ia-1,kind=pl_test_flt) / real(NPTS,kind=pl_test_flt) )

    do k = 1, 4
        call pladv(0)
        call plvpor(0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 0.9_pl_test_flt)
        call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -0.9_pl_test_flt, 1.1_pl_test_flt)
        call plcol0(1)
        call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, &
               -1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt,  &
               1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt, &
               alt(k), az(k))
        call plbox3('bnstu', 'x axis', 0.0_pl_test_flt, 0, &
               'bnstu', 'y axis', 0.0_pl_test_flt, 0, &
               'bcdmnstuv', 'z axis', 0.0_pl_test_flt, 0)

        call plcol0(2)

        if ( opt(k).gt. 0 ) then
            call plline3(x, y, z)
        else
            !U+22C5 DOT OPERATOR.
            call plstring3( x, y, z, "⋅" )
        endif

        call plcol0(3)
        write( title, '(a,i2,a,i2)') '#frPLplot Example 18 - Alt=', nint(alt(k)), ', Az=', nint(az(k))
        call plmtex('t', 1.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, title)
    enddo

    call plend()
contains

    subroutine test_poly(k, alt, az)
        integer          :: k
        real(kind=pl_test_flt) :: alt, az

        real(kind=pl_test_flt) :: x(5), y(5), z(5)
        integer          :: i, j
        logical          :: draw(4,4) = &
               reshape( &
               (/  .true., .true., .true., .true., &
               .true., .false., .true., .false., &
               .false., .true., .false., .true., &
               .true., .true., .false., .false. /), (/4,4/) )
        integer, dimension(0:20)          :: ia = (/(j,j=0,20)/)
        real(kind=pl_test_flt), dimension(0:20) :: theta, phi

        theta = TWOPI * ia /20._pl_test_flt
        phi   = PI * ia / 20.1_pl_test_flt

        call pladv(0)
        call plvpor(0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 0.9_pl_test_flt)
        call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -0.9_pl_test_flt, 1.1_pl_test_flt)
        call plcol0(1)
        call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, &
               -1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, &
               1.0_pl_test_flt,  -1.0_pl_test_flt, 1.0_pl_test_flt, &
               alt, az)
        call plbox3('bnstu', 'x axis', 0.0_pl_test_flt, 0, &
               'bnstu', 'y axis', 0.0_pl_test_flt, 0, &
               'bcdmnstuv', 'z axis', 0.0_pl_test_flt, 0)

        call plcol0(2)


        !    x = r sin(phi) cos(theta)
        !    y = r sin(phi) sin(theta)
        !    z = r cos(phi)
        !    r = 1 :=)

        do i=0,19
            do j=0,19
                x(1) = sin( phi(j) ) * cos( theta(i) )
                y(1) = sin( phi(j) ) * sin( theta(i) )
                z(1) = cos( phi(j) )

                x(2) = sin( phi(j+1) ) * cos( theta(i) )
                y(2) = sin( phi(j+1) ) * sin( theta(i) )
                z(2) = cos( phi(j+1) )

                x(3) = sin( phi(j+1) ) * cos( theta(i+1) )
                y(3) = sin( phi(j+1) ) * sin( theta(i+1) )
                z(3) = cos( phi(j+1) )

                x(4) = sin( phi(j) ) * cos( theta(i+1) )
                y(4) = sin( phi(j) ) * sin( theta(i+1) )
                z(4) = cos( phi(j) )

                x(5) = sin( phi(j) ) * cos( theta(i) )
                y(5) = sin( phi(j) ) * sin( theta(i) )
                z(5) = cos( phi(j) )

                call plpoly3(x, y, z, draw(:,k), .true.)
            enddo
        enddo

        call plcol0(3)
        call plmtex('t', 1.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, 'unit radius sphere' )
    end subroutine test_poly
end program x18f
