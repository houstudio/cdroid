!      Grid data demo.
!
!      Copyright (C) 2004  Joao Cardoso
!      Copyright (C) 2008  Andrew Ross
!      Copyright (C) 2008-2016 Alan W. Irwin
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
program x21f
    use plplot, double_PI => PL_PI
    implicit none
    real(kind=pl_test_flt), parameter :: PI = double_PI

    integer pts, xp, yp, nl, knn_order, randn, rosen
    real(kind=pl_test_flt) threshold, wmin
    parameter (pts = 500)
    parameter (xp = 25)
    parameter (yp = 20)
    parameter (nl = 16)
    parameter (knn_order = 20)
    parameter (threshold = 1.001_pl_test_flt)
    parameter (wmin = -1e3_pl_test_flt)
    parameter (randn = 0)
    parameter (rosen = 0)

    real(kind=pl_test_flt) xmin, xmax, ymin, ymax

    real(kind=pl_test_flt) x(pts), y(pts), z(pts), clev(nl)
    real(kind=pl_test_flt) xg(xp), yg(yp), zg(xp,yp)
    real(kind=pl_test_flt) zmin, zmax, lzmin, lzmax
    integer i, j, k
    integer alg
    character(len=80) title(6)
    data title /'Cubic Spline Approximation', &
           'Delaunay Linear Interpolation', &
           'Natural Neighbors Interpolation', &
           'KNN Inv. Distance Weighted', &
           '3NN Linear Interpolation', &
           '4NN Around Inv. Dist. Weighted'/

    real(kind=pl_test_flt) opt(6)
    data opt /0._pl_test_flt, 0._pl_test_flt, 0._pl_test_flt, 0._pl_test_flt, 0._pl_test_flt, 0._pl_test_flt/

    real(kind=pl_test_flt) xt, yt

    real(kind=pl_test_flt) r
    integer ii, jj
    integer :: plparseopts_rc
    real(kind=pl_test_flt) dist, d

    xmin = -0.2_pl_test_flt
    ymin = -0.2_pl_test_flt
    xmax = 0.6_pl_test_flt
    ymax = 0.6_pl_test_flt

    !      call plMergeOpts(options, "x21c options", NULL);
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    opt(3) = wmin
    opt(4) = real(knn_order,kind=pl_test_flt)
    opt(5) = threshold

    ! Initialize plplot

    call plinit

    call cmap1_init

    call plseed(5489)

    do i=1,pts
        xt = (xmax-xmin)*plrandd()
        yt = (ymax-ymin)*plrandd()
        if (randn.eq.0) then
            x(i) = xt + xmin
            y(i) = yt + ymin
        else
            x(i) = sqrt(-2._pl_test_flt*log(xt)) * cos(2._pl_test_flt*PI*yt) + xmin
            y(i) = sqrt(-2._pl_test_flt*log(xt)) * sin(2._pl_test_flt*PI*yt) + ymin
        endif
        if (rosen.eq.0) then
            r = sqrt(x(i)*x(i) + y(i)*y(i))
            z(i) = exp(-r*r)*cos(2._pl_test_flt*PI*r)
        else
            z(i) = log((1._pl_test_flt-x(i))**2 + 100._pl_test_flt*(y(i)-x(i)**2)**2)
        endif
    enddo

    zmin = z(1)
    zmax = z(1)
    do i=2,pts
        zmax = max(zmax,z(i))
        zmin = min(zmin,z(i))
    enddo

    do i=1,xp
        xg(i) = xmin + (xmax-xmin)*(i-1._pl_test_flt)/(xp-1._pl_test_flt)
    enddo
    do i=1,yp
        yg(i) = ymin + (ymax-ymin)*(i-1._pl_test_flt)/(yp-1._pl_test_flt)
    enddo

    call plcol0(1)
    call plenv(xmin, xmax, ymin, ymax, 2, 0)
    call plcol0(15)
    call pllab("X", "Y", "The original data sampling")
    do i=1,pts
        call plcol1( ( z(i) - zmin ) / ( zmax - zmin ) )
        !     The following plstring call should be the the equivalent of
        !     plpoin( 1, &x[i], &y[i], 5 ); Use plstring because it is
        !     not deprecated like plpoin and has much more powerful
        !     capabilities.  N.B. symbol 141 works for Hershey devices
        !     (e.g., -dev xwin) only if plfontld( 0 ) has been called
        !     while symbol 727 works only if plfontld( 1 ) has been
        !     called.  The latter is the default which is why we use 727
        !     here to represent a centred X (multiplication) symbol.
        !     This dependence on plfontld is one of the limitations of
        !     the Hershey escapes for PLplot, but the upside is you get
        !     reasonable results for both Hershey and Unicode devices.
        call plstring( x(i:i), y(i:i), '#(727)' );
    enddo
    call pladv(0)

    call plssub(3,2)

    do k=1,2
        call pladv(0)
        do alg=1,6

            call plgriddata(x, y, z, xg, yg, zg, alg, opt(alg))

            !     - CSA can generate NaNs (only interpolates? !).
            !     - DTLI and NNI can generate NaNs for points outside the convex hull
            !     of the data points.
            !     - NNLI can generate NaNs if a sufficiently thick triangle is not found
            !
            !     PLplot should be NaN/Inf aware, but changing it now is quite a job...
            !     so, instead of not plotting the NaN regions, a weighted average over
            !     the neighbors is done.
            !

            if ((alg.eq.GRID_CSA).or.(alg.eq.GRID_DTLI).or. &
                   (alg.eq.GRID_NNLI).or.(alg.eq.GRID_NNI)) then

                do i=1,xp
                    do j=1,yp
                        if (myisnan(zg(i,j))) then
                            !     average (IDW) over the 8 neighbors

                            zg(i,j) = 0._pl_test_flt
                            dist = 0._pl_test_flt

                            ii=i-1
                            do while ((ii.le.i+1).and.(ii.le.xp))
                                jj = j-1
                                do while ((jj.le.j+1).and.(jj.le.yp))
                                    if ((ii.ge.1) .and. (jj.ge.1) .and. &
                                           (.not.myisnan(zg(ii,jj))) ) then
                                        if (abs(ii-i) + abs(jj-j) .eq. 1) then
                                            d = 1._pl_test_flt
                                        else
                                            d = 1.4142_pl_test_flt
                                        endif
                                        zg(i,j) = zg(i,j) + zg(ii,jj)/(d*d)
                                        dist = dist + d
                                    endif
                                    jj = jj+1
                                enddo
                                ii = ii+1
                            enddo
                            if (dist.ne.0._pl_test_flt) then
                                zg(i,j) = zg(i,j) / dist
                            else
                                zg(i,j) = zmin
                            endif
                        endif
                    enddo
                enddo
            endif

            call a2mnmx(zg, xp, yp, lzmin, lzmax, xp)

            lzmin = min(lzmin, zmin)
            lzmax = max(lzmax, zmax)

            lzmin = lzmin - 0.01_pl_test_flt
            lzmax = lzmax + 0.01_pl_test_flt

            call plcol0(1)
            call pladv(alg)

            if (k.eq.1) then

                do i=1,nl
                    clev(i) = lzmin + (lzmax-lzmin)/(nl-1._pl_test_flt)*(i-1._pl_test_flt)
                enddo
                call plenv0(xmin, xmax, ymin, ymax, 2, 0)
                call plcol0(15)
                call pllab("X", "Y", title(alg))
                call plshades(zg, xmin, xmax, ymin, &
                       ymax, clev, 1._pl_test_flt, 0, 1._pl_test_flt, .true. )
                call plcol0(2)
            else

                do i = 1,nl
                    clev(i) = lzmin + (lzmax-lzmin)/(nl-1._pl_test_flt)*(i-1._pl_test_flt)
                enddo
                call plvpor(0._pl_test_flt, 1._pl_test_flt, 0._pl_test_flt, 0.9_pl_test_flt)
                call plwind(-1.1_pl_test_flt, 0.75_pl_test_flt, -0.65_pl_test_flt, 1.20_pl_test_flt)
                !
                !     For the comparison to be fair, all plots should have the
                !     same z values, but to get the max/min of the data generated
                !     by all algorithms would imply two passes. Keep it simple.
                !
                !     plw3d(1., 1., 1., xmin, xmax, ymin, ymax, zmin, zmax, 30, -60);
                !

                call plw3d(1._pl_test_flt, 1._pl_test_flt, 1._pl_test_flt, xmin, xmax, ymin, ymax,  &
                       lzmin, lzmax, 30._pl_test_flt, -40._pl_test_flt)
                call plbox3("bntu", "X", 0._pl_test_flt, 0, &
                       "bntu", "Y", 0._pl_test_flt, 0, &
                       "bcdfntu", "Z", 0.5_pl_test_flt, 0)
                call plcol0(15)
                call pllab("", "", title(alg))
                call plot3dc(xg, yg, zg, ior(ior(DRAW_LINEXY, &
                       MAG_COLOR), BASE_CONT), clev)
            endif
        enddo
    enddo

    call plend

contains

    subroutine cmap1_init
        use plplot
        implicit none
        real(kind=pl_test_flt) i(2), h(2), l(2), s(2)

        i(1) = 0._pl_test_flt
        i(2) = 1._pl_test_flt

        h(1) = 240._pl_test_flt
        h(2) = 0._pl_test_flt

        l(1) = 0.6_pl_test_flt
        l(2) = 0.6_pl_test_flt

        s(1) = 0.8_pl_test_flt
        s(2) = 0.8_pl_test_flt

        call plscmap1n(256)
        call plscmap1l(.false., i, h, l, s)
    end subroutine cmap1_init


    !----------------------------------------------------------------------------
    !      Subroutine a2mnmx
    !      Minimum and the maximum elements of a 2-d array.

    subroutine a2mnmx(f, nx, ny, fmin, fmax, xdim)
        use plplot
        implicit none

        integer   i, j, nx, ny, xdim
        real(kind=pl_test_flt) f(xdim, ny), fmin, fmax

        fmax = f(1, 1)
        fmin = fmax
        do j = 1, ny
            do  i = 1, nx
                fmax = max(fmax, f(i, j))
                fmin = min(fmin, f(i, j))
            enddo
        enddo
    end subroutine a2mnmx

    include 'plfortrandemos.inc'

end program x21f
