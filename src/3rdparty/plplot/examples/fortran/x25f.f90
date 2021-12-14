!   Filling and clipping polygons.
!
!   Copyright (C) 2005 Arjen Markus
!   Copyright (C) 2008 Andrew Ross
!   Copyright (C) 2008-2016 Alan W. Irwin
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
!
! --------------------------------------------------------------------------
! main
!
! Test program for filling polygons and proper clipping
! --------------------------------------------------------------------------

program x25f

    use plplot
    implicit none

    integer          :: i, j, k
    integer          :: npts
    integer :: plparseopts_rc
    real(kind=pl_test_flt) :: xextreme(2,10)
    real(kind=pl_test_flt) :: yextreme(2,10)
    real(kind=pl_test_flt) :: x0(10)
    real(kind=pl_test_flt) :: y0(10)

    data ( xextreme(1,i), xextreme(2,i), yextreme(1,i), yextreme(2,i), i=1,9) / &
           -120.0_pl_test_flt,      120.0_pl_test_flt,     -120.0_pl_test_flt,      120.0_pl_test_flt, &
           -120.0_pl_test_flt,      120.0_pl_test_flt,       20.0_pl_test_flt,      120.0_pl_test_flt, &
           -120.0_pl_test_flt,      120.0_pl_test_flt,      -20.0_pl_test_flt,      120.0_pl_test_flt, &
           -80.0_pl_test_flt,       80.0_pl_test_flt,      -20.0_pl_test_flt,      120.0_pl_test_flt, &
           -220.0_pl_test_flt,     -120.0_pl_test_flt,     -120.0_pl_test_flt,      120.0_pl_test_flt, &
           -20.0_pl_test_flt,       20.0_pl_test_flt,     -120.0_pl_test_flt,      120.0_pl_test_flt, &
           -20.0_pl_test_flt,       20.0_pl_test_flt,      -20.0_pl_test_flt,       20.0_pl_test_flt, &
           -80.0_pl_test_flt,       80.0_pl_test_flt,      -80.0_pl_test_flt,       80.0_pl_test_flt, &
           20.0_pl_test_flt,      120.0_pl_test_flt,     -120.0_pl_test_flt,      120.0_pl_test_flt/

    npts = 0

    !  Parse and process command line arguments

    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !  Initialize plplot

    call plssub(3,3)
    call plinit()

    do k = 1,2
        do j = 1,4

            select case ( j )
            case ( 1 ) !  Polygon 1: a diamond
                x0(1) =    0.0_pl_test_flt
                y0(1) = -100.0_pl_test_flt
                x0(2) = -100.0_pl_test_flt
                y0(2) =    0.0_pl_test_flt
                x0(3) =    0.0_pl_test_flt
                y0(3) =  100.0_pl_test_flt
                x0(4) =  100.0_pl_test_flt
                y0(4) =    0.0_pl_test_flt
                npts = 4

            case( 2 )
                !  Polygon 1: a diamond - reverse direction
                x0(4) =    0.0_pl_test_flt
                y0(4) = -100.0_pl_test_flt
                x0(3) = -100.0_pl_test_flt
                y0(3) =    0.0_pl_test_flt
                x0(2) =    0.0_pl_test_flt
                y0(2) =  100.0_pl_test_flt
                x0(1) =  100.0_pl_test_flt
                y0(1) =    0.0_pl_test_flt
                npts = 4

            case( 3 )
                !  Polygon 2: a square with punctures
                x0(1)  = -100.0_pl_test_flt
                y0(1)  = -100.0_pl_test_flt
                x0(2)  = -100.0_pl_test_flt
                y0(2)  =  -80.0_pl_test_flt
                x0(3)  =   80.0_pl_test_flt
                y0(3)  =    0.0_pl_test_flt
                x0(4)  = -100.0_pl_test_flt
                y0(4)  =   80.0_pl_test_flt
                x0(5)  = -100.0_pl_test_flt
                y0(5)  =  100.0_pl_test_flt
                x0(6)  =  -80.0_pl_test_flt
                y0(6)  =  100.0_pl_test_flt
                x0(7)  =    0.0_pl_test_flt
                y0(7)  =   80.0_pl_test_flt
                x0(8)  =   80.0_pl_test_flt
                y0(8)  =  100.0_pl_test_flt
                x0(9)  =  100.0_pl_test_flt
                y0(9)  =  100.0_pl_test_flt
                x0(10) =  100.0_pl_test_flt
                y0(10) = -100.0_pl_test_flt
                npts = 10

            case( 4 )
                !  Polygon 2: a square with punctures - reversed direction
                x0(10) = -100.0_pl_test_flt
                y0(10) = -100.0_pl_test_flt
                x0(9)  = -100.0_pl_test_flt
                y0(9)  =  -80.0_pl_test_flt
                x0(8)  =   80.0_pl_test_flt
                y0(8)  =    0.0_pl_test_flt
                x0(7)  = -100.0_pl_test_flt
                y0(7)  =   80.0_pl_test_flt
                x0(6)  = -100.0_pl_test_flt
                y0(6)  =  100.0_pl_test_flt
                x0(5)  =  -80.0_pl_test_flt
                y0(5)  =  100.0_pl_test_flt
                x0(4)  =    0.0_pl_test_flt
                y0(4)  =   80.0_pl_test_flt
                x0(3)  =   80.0_pl_test_flt
                y0(3)  =  100.0_pl_test_flt
                x0(2)  =  100.0_pl_test_flt
                y0(2)  =  100.0_pl_test_flt
                x0(1)  =  100.0_pl_test_flt
                y0(1)  = -100.0_pl_test_flt
                npts = 10
            end select

            do i = 1,9
                call pladv(0)
                call plvsta()
                call plwind(xextreme(1,i), xextreme(2,i), yextreme(1,i), yextreme(2,i))

                call plcol0(2)
                call plbox('bc', 1.0_pl_test_flt, 0, 'bcnv', 10.0_pl_test_flt, 0)
                call plcol0(1)
                call plpsty(0)
                if(k.eq.1) then
                    call plfill(x0(1:npts),y0(1:npts))
                else
                    call plgradient(x0(1:npts),y0(1:npts),45._pl_test_flt)
                endif
                call plcol0(2)
                call pllsty(1)
                call plline(x0(1:npts),y0(1:npts))
            end do
        end do
    end do

    !  Don't forget to call plend() to finish off!
    call plend

end program x25f
