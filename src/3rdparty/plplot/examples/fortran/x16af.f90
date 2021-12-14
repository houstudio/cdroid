!      Demonstration of plshade plotting
!      Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
!
!      Copyright (C) 2004-2016  Alan W. Irwin
!
!      This file is part of PLplot.
!
!      PLplot is free software; you can redistribute it and/or modify
!      it under the terms of the GNU Library General Public License as
!      published by the Free Software Foundation; either version 2 of the
!      License, or (at your option) any later version.
!
!      PLplot is distributed in the hope that it will be useful,
!      but WITHOUT ANY WARRANTY; without even the implied warranty of
!      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!      GNU Library General Public License for more details.
!
!      You should have received a copy of the GNU Library General Public
!      License along with PLplot; if not, write to the Free Software
!      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

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
program x16af
    use plplot
    implicit none
    integer :: plparseopts_rc

    !      Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    call plscmap0n(3)

    !      Initialize plplot

    call plinit()

    !      Rectangular coordinate plot

    call rect()

    !      Polar coordinate plot

    call polar()

    call plend

contains

    !      Plot function using the identity transform

    subroutine rect()

        use plplot
        implicit none
        integer   xdim, ydim, NX, NY, NCONTR
        !      xdim and ydim are the static dimensions of the 2D arrays while
        !      NX and NY are the defined area.
        parameter (xdim = 99, NX = 35, ydim = 100, NY = 46, NCONTR = 14)

        real(kind=pl_test_flt)    z(xdim, ydim)
        real(kind=pl_test_flt)    zmin, zmax, x, y
        real(kind=pl_test_flt)    shade_min, shade_max, sh_color
        integer   i, j, sh_cmap
        integer   min_color, max_color
        real(kind=pl_test_flt)    sh_width, min_width, max_width

        !      Set up for plshade call

        sh_cmap   = 1
        min_color = 1
        min_width = 0
        max_color = 0
        max_width = 0

        !      Set up data arrays

        do i = 1, NX
            x = (i - 1 - (NX/2)) / real(NX/2,kind=pl_test_flt)
            do j = 1, NY
                y = (j - 1 - (NY/2)) / real(NY/2,kind=pl_test_flt) - 1.0_pl_test_flt
                z(i,j) = x*x - y*y + (x - y) / (x*x + y*y + 0.1_pl_test_flt)
            enddo
        enddo

        call a2mnmx(z, NX, NY, zmin, zmax, xdim)

        !      Plot using identity transform

        call pladv(0)
        call plvpor(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
        call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt)

        do i = 1, NCONTR
            shade_min = zmin + (zmax - zmin) * real(i - 1,kind=pl_test_flt) / &
                   real(NCONTR,kind=pl_test_flt)
            shade_max = zmin + (zmax - zmin) * real(i,kind=pl_test_flt)     / &
                   real(NCONTR,kind=pl_test_flt)
            sh_color = real(i - 1,kind=pl_test_flt) / real(NCONTR - 1,kind=pl_test_flt)
            sh_width = 2
            call plpsty(0)
            call plshade(z(:NX,:NY), &
                   -1._pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt, &
                   shade_min, shade_max, &
                   sh_cmap, sh_color, sh_width, &
                   min_color, min_width, max_color, max_width, .true. )
        enddo

        call plcol0(1)
        call plbox('bcnst', 0.0_pl_test_flt, 0, 'bcnstv', 0.0_pl_test_flt, 0)
        call plcol0(2)
        call pllab('distance', 'altitude', 'Bogon flux')

    end subroutine rect

    !      Routine for demonstrating use_ of transformation arrays in contour plots.

    subroutine polar()

        use plplot, double_TWOPI => PL_TWOPI
        implicit none
        real(kind=pl_test_flt), parameter :: TWOPI = double_TWOPI
        integer   xdim, ydim, NX, NY, NCONTR, NBDRY
        !      xdim and ydim are the static dimensions of the 2D arrays while
        !      NX and NY are the defined area.
        parameter (xdim = 99, NX = 40, ydim = 100, NY = 64)
        parameter (NCONTR = 14, NBDRY=200)

        real(kind=pl_test_flt)    z(xdim, ydim)
        real(kind=pl_test_flt)    xg(xdim, ydim+1), yg(xdim, ydim+1), &
               xtm(NBDRY), ytm(NBDRY)
        real(kind=pl_test_flt)    xmin, xmax, ymin, ymax, zmin, zmax
        real(kind=pl_test_flt)    xpmin, xpmax, ypmin, ypmax
        real(kind=pl_test_flt)    r, theta, rmax, x0, y0
        real(kind=pl_test_flt)    eps, q1, d1, q1i, d1i, q2, d2, q2i, d2i
        real(kind=pl_test_flt)    div1, div1i, div2, div2i

        real(kind=pl_test_flt)    shade_min, shade_max, sh_color
        real(kind=pl_test_flt)    xtick, ytick
        integer   nxsub, nysub
        integer   ncolbox, ncollab
        integer   i, j
        integer   sh_cmap
        integer   min_color, max_color
        real(kind=pl_test_flt) sh_width, min_width, max_width
        character(len=8) xopt, yopt

        !      Set up for plshade call

        sh_cmap = 1
        min_color = 1
        min_width = 0
        max_color = 0
        max_width = 0

        !      Set up r-theta grids
        !      Tack on extra cell in theta to handle periodicity.

        do i = 1, NX
            r = i - 0.5_pl_test_flt
            do j = 1, NY
                theta = TWOPI/real(NY,kind=pl_test_flt) * (j-0.5_pl_test_flt)
                xg(i,j) = r * cos(theta)
                yg(i,j) = r * sin(theta)
            enddo
            xg(i, NY+1) = xg(i, 1)
            yg(i, NY+1) = yg(i, 1)
        enddo
        call a2mnmx(xg, NX, NY, xmin, xmax, xdim)
        call a2mnmx(yg, NX, NY, ymin, ymax, xdim)

        rmax = r
        x0 = (xmin + xmax)/2._pl_test_flt
        y0 = (ymin + ymax)/2._pl_test_flt

        !      Potential inside a conducting cylinder (or sphere) by method of images.
        !      Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
        !      Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
        !      Also put in smoothing term at small distances.

        eps = 2._pl_test_flt

        q1 = 1._pl_test_flt
        d1 = r/4._pl_test_flt

        q1i = - q1*r/d1
        d1i = r**2/d1

        q2 = -1._pl_test_flt
        d2 = r/4._pl_test_flt

        q2i = - q2*r/d2
        d2i = r**2/d2

        do i = 1, NX
            do j = 1, NY
                div1 = sqrt((xg(i,j)-d1)**2 + (yg(i,j)-d1)**2 + eps**2)
                div1i = sqrt((xg(i,j)-d1i)**2 + (yg(i,j)-d1i)**2 + eps**2)

                div2 = sqrt((xg(i,j)-d2)**2 + (yg(i,j)+d2)**2 + eps**2)
                div2i = sqrt((xg(i,j)-d2i)**2 + (yg(i,j)+d2i)**2 + eps**2)

                z(i,j) = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i
            enddo
        enddo

        call a2mnmx(z, NX, NY, zmin, zmax, xdim)

        !      Advance graphics frame and get ready to plot.

        ncolbox = 1
        ncollab = 2

        call pladv(0)
        call plcol0(ncolbox)

        !      Scale window to user coordinates.
        !      Make a bit larger so the boundary does not get clipped.

        eps = 0.05_pl_test_flt
        xpmin = xmin - abs(xmin)*eps
        xpmax = xmax + abs(xmax)*eps
        ypmin = ymin - abs(ymin)*eps
        ypmax = ymax + abs(ymax)*eps

        call plvpas(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt, 1.0_pl_test_flt)
        call plwind(xpmin, xpmax, ypmin, ypmax)

        xopt = ' '
        yopt = ' '
        xtick = 0._pl_test_flt
        nxsub = 0
        ytick = 0._pl_test_flt
        nysub = 0

        call plbox(xopt, xtick, nxsub, yopt, ytick, nysub)

        !      Call plotter once for z < 0 (dashed), once for z > 0 (solid lines).

        do i = 1, NCONTR
            shade_min = zmin + (zmax - zmin) * real(i - 1,kind=pl_test_flt) / &
                   real(NCONTR,kind=pl_test_flt)
            shade_max = zmin + (zmax - zmin) * real(i,kind=pl_test_flt)     / &
                   real(NCONTR,kind=pl_test_flt)
            sh_color = real(i - 1,kind=pl_test_flt) / real(NCONTR - 1,kind=pl_test_flt)
            sh_width = 2
            call plpsty(0)

            call plshade(z(:NX,:NY), &
                   -1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt, &
                   shade_min, shade_max, &
                   sh_cmap, sh_color, sh_width, &
                   min_color, min_width, max_color, max_width, &
                   .false., xg(:NX,:NY), yg(:NX,:NY) )
        enddo

        !      Draw boundary.

        do i = 1, NBDRY
            theta = (TWOPI)/(NBDRY-1) * real(i-1,kind=pl_test_flt)
            xtm(i) = x0 + rmax * cos(theta)
            ytm(i) = y0 + rmax * sin(theta)
        enddo
        call plcol0(ncolbox)
        call plline(xtm, ytm)

        call plcol0(ncollab)
        call pllab(' ', ' ', &
               'Shielded potential of charges in a conducting sphere')

    end subroutine polar

    !----------------------------------------------------------------------------
    !      Subroutine a2mnmx
    !      Minimum and the maximum elements of a 2-d array.

    subroutine a2mnmx(f, nx, ny, fmin, fmax, xdim)
        use plplot
        implicit none

        integer   i, j, nx, ny, xdim
        real(kind=pl_test_flt)    f(xdim, ny), fmin, fmax

        fmax = f(1, 1)
        fmin = fmax
        do j = 1, ny
            do  i = 1, nx
                fmax = max(fmax, f(i, j))
                fmin = min(fmin, f(i, j))
            enddo
        enddo
    end subroutine a2mnmx
end program x16af
