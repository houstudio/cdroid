!   3-d plot demo
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
program x08f
    use plplot, double_PI => PL_PI
    use plfortrandemolib

    implicit none

    real(kind=pl_test_flt), parameter :: PI = double_PI
    integer :: i, j, k, ifshade

    !   xdim is the leading dimension of z, xpts <= xdim is the leading
    !   dimension of z that is defined.
    integer, parameter :: xdim=99, ydim=100, xpts=35, ypts=45
    real(kind=pl_test_flt)   :: x(xdim), y(ydim), z(xdim,ypts), xx, yy, r
    real(kind=pl_test_flt)   :: zlimited(xdim,ypts)
    integer, parameter :: indexxmin = 1
    integer, parameter :: indexxmax = xpts
    integer            :: indexymin(xpts), indexymax(xpts)

    ! parameters of ellipse (in x, y index coordinates) that limits the data.
    ! x0, y0 correspond to the exact floating point centre of the index
    ! range.
    ! Note: using the Fortran convention of starting indices at 1
    real(kind=pl_test_flt), parameter :: x0 = 0.5_pl_test_flt * ( xpts + 1 )
    real(kind=pl_test_flt), parameter :: a  = 0.9_pl_test_flt * ( x0 - 1.0_pl_test_flt )
    real(kind=pl_test_flt), parameter :: y0 = 0.5_pl_test_flt * ( ypts + 1 )
    real(kind=pl_test_flt), parameter :: b  = 0.7_pl_test_flt * ( y0 - 1.0_pl_test_flt )
    real(kind=pl_test_flt)            :: square_root

    character (len=80) :: title(2) = &
           (/'#frPLplot Example 8 - Alt=60, Az=30 ', &
           '#frPLplot Example 8 - Alt=40, Az=-30'/)
    real(kind=pl_test_flt)   :: alt(2) = (/60.0_pl_test_flt, 40.0_pl_test_flt/)
    real(kind=pl_test_flt)   :: az(2)  = (/30.0_pl_test_flt,-30.0_pl_test_flt/)
    integer            :: rosen
    integer, parameter :: nlevel = 10
    integer :: plparseopts_rc
    real(kind=pl_test_flt)   :: zmin, zmax, step, clevel(nlevel)

    real(kind=pl_test_flt)   :: dx, dy

    !   Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    rosen = 0

    !   x(1:xpts) = (arange(xpts) - (xpts-1)/2.0_pl_test_flt) / ((xpts-1)/2.0_pl_test_flt)
    !   y(1:ypts) = (arange(ypts) - (ypts-1)/2.0_pl_test_flt) / ((ypts-1)/2.0_pl_test_flt)
    !

    dx = 2.0_pl_test_flt / (xpts - 1)
    dy = 2.0_pl_test_flt / (ypts - 1)

    do i = 1,xpts
        x(i) = -1.0_pl_test_flt + (i-1) * dx
    enddo

    do j = 1,ypts
        y(j) = -1.0_pl_test_flt + (j-1) * dy
    enddo

    if ( rosen == 1 ) then
        x = 1.5_pl_test_flt * x
        y = y + 0.5_pl_test_flt
    endif

    do i=1,xpts
        xx = x(i)
        do j=1,ypts
            yy = y(j)
            if (rosen == 1) then
                z(i,j) = (1._pl_test_flt - xx)**2 + 100._pl_test_flt*(yy - xx**2)**2

                ! The log argument may be zero for just the right grid.
                if (z(i,j) > 0._pl_test_flt) then
                    z(i,j) = log(z(i,j))
                else
                    z(i,j) = -5._pl_test_flt
                endif
            else
                ! Sombrero function
                r = sqrt(xx**2 + yy**2)
                z(i,j) = exp(-r**2) * cos(2.0_pl_test_flt*PI*r)
            endif
        enddo
    enddo

    zlimited = huge(1.0_pl_test_flt)
    do i = indexxmin, indexxmax
        square_root = sqrt( 1.0_pl_test_flt - min( 1.0_pl_test_flt, (( i - x0 ) / a) ** 2 ) )
        ! Add 0.5 to find nearest integer and therefore preserve symmetry
        ! with regard to lower and upper bound of y range.
        indexymin(i) = max( 1, int( 0.5_pl_test_flt + y0 - b * square_root ) )
        ! indexymax calculated with the convention that it is 1
        ! greater than highest valid index.
        indexymax(i) = min( ypts, 1 + int( 0.5_pl_test_flt + y0 + b * square_root ) )

        do j = indexymin(i),indexymax(i)
            zlimited(i,j) = z(i,j)
        enddo
    enddo

    zmin = minval( z(1:xpts,:) )
    zmax = maxval( z(1:xpts,:) )

    step = (zmax-zmin)/(nlevel+1)
    clevel = zmin + step * arange(1,nlevel+1)

    call plinit()
    call pllightsource(1._pl_test_flt, 1._pl_test_flt, 1._pl_test_flt)
    do k=1,2
        do ifshade = 0, 4
            call pladv(0)
            call plvpor(0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 0.9_pl_test_flt )
            call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -0.9_pl_test_flt, 1.1_pl_test_flt )
            call plcol0(3)
            call plmtex('t', 1.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, title(k))
            call plcol0(1)
            if (rosen ==1) then
                call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, -1.5_pl_test_flt, &
                       1.5_pl_test_flt, -0.5_pl_test_flt, 1.5_pl_test_flt, zmin, zmax, alt(k),az(k))
            else
                call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, &
                       1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt, zmin, zmax, alt(k),az(k))
            endif
            call plbox3('bnstu','x axis', 0.0_pl_test_flt, 0, &
                   'bnstu', 'y axis', 0.0_pl_test_flt, 0, &
                   'bcdmnstuv','z axis', 0.0_pl_test_flt, 0)
            call plcol0(2)

            select case (ifshade)
            case( 0 )
                ! diffuse light surface plot
                call cmap1_init(1)
                call plsurf3d(x(:xpts), y(:ypts), z(:xpts,:ypts), &
                       0, clevel(nlevel:1))

            case( 1 )
                ! magnitude colored plot
                call cmap1_init(0)
                call plsurf3d(x(:xpts), y(:ypts), z(:xpts,:ypts), &
                       MAG_COLOR, clevel(nlevel:1))

            case( 2 )
                ! magnitude colored plot with faceted squares
                call cmap1_init(0)
                call plsurf3d(x(:xpts), y(:ypts), z(:xpts,:ypts), &
                       ior(MAG_COLOR, FACETED), clevel(nlevel:1))

            case( 3 )
                ! magnitude colored plot with contours
                call cmap1_init(0)
                call plsurf3d(x(:xpts), y(:ypts), z(:xpts,:ypts), &
                       ior(MAG_COLOR, ior(SURF_CONT, BASE_CONT)), clevel)

            case( 4 )
                ! magnitude colored plot with contours and index limits
                call cmap1_init(0)
                ! N.B. indexxmin, indexymin, and indexymax are
                ! calculated above using one-based indexing, but must
                ! substract one from all of them below to convert to
                ! zero-based indexing.  Zero-based indexing is assumed
                ! for those 3 arguments by the Fortran binding to make
                ! that binding consistent with the rest of our
                ! bindings and our core C code in this regard.
                call plsurf3dl(x(:xpts), y(:ypts), zlimited(:xpts,:ypts), &
                       ior(MAG_COLOR, ior(SURF_CONT, BASE_CONT)), clevel, &
                       indexxmin-1, indexymin-1, indexymax-1 )
            case default
                stop 'x08f: bad logic'
            end select
        enddo
    enddo
    call plend
contains

    !----------------------------------------------------------------------------
    subroutine cmap1_init(gray)

        !   For gray.eq.1, basic grayscale variation from half-dark
        !   to light.  Otherwise, hue variations around the front of the
        !   colour wheel from blue to green to red with constant lightness
        !   and saturation.

        integer          :: gray
        real(kind=pl_test_flt) :: i(0:1), h(0:1), l(0:1), s(0:1)

        !   left boundary
        i(0) = 0._pl_test_flt
        !   right boundary
        i(1) = 1._pl_test_flt
        if (gray == 1) then
            !       hue -- low: red (arbitrary if s=0)
            h(0) = 0.0_pl_test_flt
            !       hue -- high: red (arbitrary if s=0)
            h(1) = 0.0_pl_test_flt
            !       lightness -- low: half-dark
            l(0) = 0.5_pl_test_flt
            !       lightness -- high: light
            l(1) = 1.0_pl_test_flt
            !       minimum saturation
            s(0) = 0.0_pl_test_flt
            !       minimum saturation
            s(1) = 0.0_pl_test_flt
        else
            !       This combination of hues ranges from blue to cyan to green to yellow
            !       to red (front of colour wheel) with constant lightness = 0.6
            !       and saturation = 0.8.

            !       hue -- low: blue
            h(0) = 240._pl_test_flt
            !       hue -- high: red
            h(1) = 0.0_pl_test_flt
            !       lightness -- low:
            l(0) = 0.6_pl_test_flt
            !       lightness -- high:
            l(1) = 0.6_pl_test_flt
            !       saturation
            s(0) = 0.8_pl_test_flt
            !       minimum saturation
            s(1) = 0.8_pl_test_flt
        endif
        call plscmap1n(256)
        call plscmap1l(.false., i, h, l, s)
    end subroutine cmap1_init
end program x08f
