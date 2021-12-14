!   Mesh plot demo
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
program x11f
    use plplot
    use plfortrandemolib
    implicit none

    integer, parameter :: xpts=35, ypts=46
    integer            :: i, j, k, ifshade

    real(kind=pl_test_flt)   :: x(xpts), y(ypts), z(xpts,ypts), xx, yy

    character(len=80)  :: title(2) = &
           (/ '#frPLplot Example 11 - Alt=33, Az=24, Opt=3 ', &
           '#frPLplot Example 11 - Alt=17, Az=115, Opt=3'  /)
    real(kind=pl_test_flt)   :: alt(2) = (/ 33.0_pl_test_flt,  17.0_pl_test_flt/)
    real(kind=pl_test_flt)   :: az(2)  = (/ 24.0_pl_test_flt, 115.0_pl_test_flt/)

    integer            :: opt(2) = (/ 3, 3 /)
    integer, parameter :: nlevel = 10
    integer :: plparseopts_rc
    real(kind=pl_test_flt)   :: zmin, zmax, step, clevel(nlevel)

    !    Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    x = 3._pl_test_flt * (arange(xpts) - (xpts/2)) / real(xpts/2,kind=pl_test_flt)
    y = 3._pl_test_flt * (arange(ypts) - (ypts/2)) / real(ypts/2,kind=pl_test_flt)

    do i=1,xpts
        xx = x(i)
        do j=1,ypts
            yy = y(j)
            z(i,j) = 3._pl_test_flt * (1._pl_test_flt-xx)*(1._pl_test_flt-xx) * &
                   exp(-(xx**2) - (yy+1._pl_test_flt)*(yy+1._pl_test_flt)) - &
                   10._pl_test_flt * (xx/5._pl_test_flt - xx**3 - yy**5) * exp(-xx**2-yy**2) - &
                   1._pl_test_flt/3._pl_test_flt * exp(-(xx+1._pl_test_flt)*(xx+1._pl_test_flt) - (yy**2))

        enddo
    enddo
    if (.false.) then
        !       Jungfraujoch/Interlaken
        z = max(z, -1._pl_test_flt)
    endif

    zmin = minval(z)
    zmax = maxval(z)

    step = (zmax-zmin)/(nlevel+1)
    clevel = zmin + step * arange(1,nlevel+1)

    call plinit()
    call cmap1_init(0)

    do k=1,2
        do ifshade = 0, 3
            call pladv(0)
            call plcol0(1)
            call plvpor(0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 0.9_pl_test_flt )
            call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.5_pl_test_flt )
            call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.2_pl_test_flt, -3.0_pl_test_flt, &
                   3.0_pl_test_flt, -3.0_pl_test_flt, 3.0_pl_test_flt, zmin, zmax, alt(k),az(k))
            call plbox3('bnstu', 'x axis', 0.0_pl_test_flt, 0, &
                   'bnstu', 'y axis', 0.0_pl_test_flt, 0, &
                   'bcdmnstuv', 'z axis', 0.0_pl_test_flt, 0)
            call plcol0(2)

            select case (ifshade)
            case (0) ! wireframe plot
                call plmesh(x(:xpts), y(:ypts), z(:xpts,:ypts), opt(k))

            case (1) ! magnitude colored wireframe plot
                call plmesh(x(:xpts), y(:ypts), z(:xpts,:ypts), ior(opt(k), MAG_COLOR))

            case (2) ! magnitude colored wireframe plot with sides
                call plot3d(x(:xpts), y(:ypts), z(:xpts,:ypts), &
                       ior(opt(k), MAG_COLOR), .true.)

            case (3) ! magnitude colored wireframe plot with base contour
                call plmeshc(x(:xpts), y(:ypts), z(:xpts,:ypts), &
                       ior(opt(k), ior(MAG_COLOR, BASE_CONT)), clevel)

            case default
                stop 'x11f: bad logic'
            end select
            call plcol0(3)
            call plmtex('t', 1.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, title(k))
        enddo
    enddo
    call plend
contains

    !----------------------------------------------------------------------------
    subroutine cmap1_init(gray)
        !    For gray.eq.1, basic grayscale variation from half-dark
        !    to light.  Otherwise, hue variations around the front of the
        !    colour wheel from blue to green to red with constant lightness
        !    and saturation.

        integer gray
        real(kind=pl_test_flt) i(0:1), h(0:1), l(0:1), s(0:1)

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

end program x11f
