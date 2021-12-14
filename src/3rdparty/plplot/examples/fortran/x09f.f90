!   Contour plot demo.
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
!   Does several contour plots using different coordinate mappings.

program x09f
    use plplot, double_PI => PL_PI, double_TWOPI => PL_TWOPI
    use plfortrandemolib
    use iso_c_binding, only: c_ptr, c_loc, c_f_pointer
    implicit none

    real(kind=pl_test_flt), parameter :: PI = double_PI
    real(kind=pl_test_flt), parameter :: TWOPI = double_TWOPI
    integer i, j

    !   xdim and ydim are the absolute static dimensions.
    !   nptsx, and nptsy are the (potentially dynamic) defined area of the 2D
    !   arrays that is actually used.
    integer, parameter :: xdim=99, ydim=100, nptsx=35, nptsy=46
    integer :: plparseopts_rc

    real(kind=pl_test_flt)   :: z(xdim, ydim), w(xdim, ydim), &
           xg1(xdim), yg1(ydim), &
           xg2(xdim, ydim), yg2(xdim, ydim)
    real(kind=pl_test_flt)   :: xc(nptsx), yc(nptsy)
    real(kind=pl_test_flt)   :: xx, yy, argx, argy, distort
    real(kind=pl_test_flt)   :: tr(6)

    real(kind=pl_test_flt)   :: clevel(11) = &
           (/ -1._pl_test_flt, -0.8_pl_test_flt, -0.6_pl_test_flt, -0.4_pl_test_flt, -0.2_pl_test_flt, &
           0._pl_test_flt,  0.2_pl_test_flt,  0.4_pl_test_flt,  0.6_pl_test_flt,  0.8_pl_test_flt, 1._pl_test_flt /)

    type mypltr_data_type
        ! Only contains data required by the mypltr_data callback
        real(kind=pl_test_flt), dimension(6) :: tr_data
    end type mypltr_data_type

    type(mypltr_data_type), target :: data

    ! Use tr plcont callback?
    logical, parameter :: tr_callback = .false.
    ! Use pltr0 (identity transformation) callback? (only meaningful
    ! if tr_callback is .false.).
    logical, parameter :: identity_callback = .false.

    !   Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    tr = (/ 2._pl_test_flt/real(nptsx-1,kind=pl_test_flt), 0.0_pl_test_flt, -1.0_pl_test_flt, &
           0.0_pl_test_flt, 2._pl_test_flt/real(nptsy-1,kind=pl_test_flt), -1.0_pl_test_flt /)

    !   Calculate the data matrices.
    xc = (arange(nptsx) - (nptsx/2)) / real(nptsx/2,kind=pl_test_flt)
    yc = (arange(nptsy) - (nptsy/2)) / real(nptsy/2,kind=pl_test_flt) - 1.0_pl_test_flt

    do i=1,nptsx
        do j=1,nptsy
            z(i,j) = xc(i)**2 - yc(j)**2
            w(i,j) = 2._pl_test_flt*xc(i)*yc(j)
        enddo
    enddo

    !   Build the 1-d coord arrays.
    distort = 0.4_pl_test_flt

    xg1(1:nptsx) = coord_function( arange(nptsx) / real(nptsx-1,kind=pl_test_flt),  distort )
    yg1(1:nptsy) = coord_function( arange(nptsy) / real(nptsy-1,kind=pl_test_flt), -distort )

    !   Build the 2-d coord arrays.
    do i=1,nptsx
        xx = -1._pl_test_flt + real(i-1,kind=pl_test_flt)*2._pl_test_flt/real(nptsx-1,kind=pl_test_flt)
        argx = 0.5_pl_test_flt*PI*xx
        do j=1,nptsy
            yy = -1._pl_test_flt + real(j-1,kind=pl_test_flt)*2._pl_test_flt/real(nptsy-1,kind=pl_test_flt)
            argy = 0.5_pl_test_flt*PI*yy
            xg2(i,j) = xx + distort*cos(argx)*cos(argy)
            yg2(i,j) = yy - distort*cos(argx)*cos(argy)
        enddo
    enddo

    call plinit

    !   Plot using identity transform
    call pl_setcontlabelformat(4, 3)
    call pl_setcontlabelparam(0.006_pl_test_flt, 0.3_pl_test_flt, 0.1_pl_test_flt, 1)
    if(.not. tr_callback .and. identity_callback) then
        call plenv( &
               real(0,kind=pl_test_flt), real(nptsx-1,kind=pl_test_flt), &
               real(0,kind=pl_test_flt), real(nptsy-1,kind=pl_test_flt), 0, 0)
    else
        call plenv(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt, 0, 0)
    endif
    call plcol0(2)
    if(tr_callback) then
        call plcont(z(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel, tr)
    elseif(identity_callback) then
        call plcont(z(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel)
    else
        data%tr_data = tr
        call plcont(z(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel, mypltr_data, c_loc(data))
    endif
    call plstyl( (/1500/), (/1500/) )
    call plcol0(3)
    if(tr_callback) then
        call plcont(w(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel, tr)
    elseif(identity_callback) then
        call plcont(w(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel)
    else
        call plcont(w(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel, mypltr)
    endif
    call plstyl( (/integer ::/), (/integer ::/) )
    call plcol0(1)
    call pllab('X Coordinate', 'Y Coordinate', 'Streamlines of flow')
    call pl_setcontlabelparam(0.006_pl_test_flt, 0.3_pl_test_flt, 0.1_pl_test_flt, 0)

    !   Plot using 1d coordinate transform
    call plenv(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt, 0, 0)
    call plcol0(2)
    call plcont(z(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel, xg1(1:nptsx), yg1(1:nptsy))
    call plstyl( (/1500/), (/1500/) )
    call plcol0(3)
    call plcont(w(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel, xg1(1:nptsx), yg1(1:nptsy))
    call plstyl( (/integer ::/), (/integer ::/) )
    call plcol0(1)
    call pllab('X Coordinate', 'Y Coordinate', 'Streamlines of flow')

    !   Plot using 2d coordinate transform
    call plenv(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt, 0, 0)
    call plcol0(2)
    call plcont(z(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel, xg2(1:nptsx,1:nptsy), yg2(1:nptsx,1:nptsy))
    call plstyl( (/1500/), (/1500/) )
    call plcol0(3)
    call plcont(w(1:nptsx,1:nptsy), 1, nptsx, 1, nptsy, clevel, xg2(1:nptsx,1:nptsy), yg2(1:nptsx,1:nptsy))
    call plstyl( (/integer ::/), (/integer ::/) )
    call plcol0(1)
    call pllab('X Coordinate', 'Y Coordinate', 'Streamlines of flow')

    call polar()
    call potential()

    call plend
contains

    !----------------------------------------------------------------------------
    !   Auxiliary function to compute the coordinates

    elemental real(kind=pl_test_flt) function coord_function( coord, factor )
        real(kind=pl_test_flt), intent(in) :: coord
        real(kind=pl_test_flt), intent(in) :: factor

        real(kind=pl_test_flt)             :: tcoord

        tcoord         = -1.0_pl_test_flt + coord * 2.0_pl_test_flt
        coord_function = tcoord + factor*cos(0.5_pl_test_flt*PI*tcoord)
    end function coord_function

    ! Callback function that relies on global tr.
    subroutine mypltr( x, y, xt, yt )

        ! These callback arguments must have exactly these attributes.
        real(kind=pl_test_flt), intent(in) ::  x, y
        real(kind=pl_test_flt), intent(out) :: xt, yt

        xt = tr(1) * x + tr(2) * y + tr(3)
        yt = tr(4) * x + tr(5) * y + tr(6)

    end subroutine mypltr

    ! Callback function that uses data argument to pass tr information.
    subroutine mypltr_data( x, y, xt, yt, data )

        ! These callback arguments must have exactly these attributes.
        real(kind=pl_test_flt), intent(in) ::  x, y
        real(kind=pl_test_flt), intent(out) :: xt, yt
        type(c_ptr), intent(in) :: data

        type(mypltr_data_type), pointer :: d
        call c_f_pointer(data, d)

        xt = d%tr_data(1) * x + d%tr_data(2) * y + d%tr_data(3)
        yt = d%tr_data(4) * x + d%tr_data(5) * y + d%tr_data(6)

    end subroutine mypltr_data

    !----------------------------------------------------------------------------
    !   polar contour plot example.
    subroutine polar()
        integer, parameter :: PERIMETERPTS = 100

        !   xdim and ydim are the absolute static size of the 2D arrays.
        !   RPTS and THETAPTS are associated with the part of the
        !   2D arrays that are defined.
        integer, parameter :: xdim=99, RPTS = 40
        integer, parameter :: ydim=100, THETAPTS = 40
        integer, parameter :: NLEVEL=10
        integer            :: i,j
        real(kind=pl_test_flt)   :: xg(xdim, ydim),  yg(xdim, ydim), &
               z(xdim, ydim), px(PERIMETERPTS), py(PERIMETERPTS), &
               lev(NLEVEL), r, theta, delta

        call plenv(-1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, 1._pl_test_flt, 0, -2)
        call plcol0(1)

        !   perimeter.
        delta = 2._pl_test_flt*PI/(PERIMETERPTS-1)
        px = cos(delta*arange(PERIMETERPTS))
        py = sin(delta*arange(PERIMETERPTS))

        call plline(px, py)

        !   create data to be contoured.
        do j = 1, THETAPTS
            theta = (2._pl_test_flt*PI/real(THETAPTS-1,kind=pl_test_flt))*real(j-1,kind=pl_test_flt)
            do i = 1, RPTS
                r = (i-1)/real(RPTS-1,kind=pl_test_flt)
                xg(i,j) = r*cos(theta)
                yg(i,j) = r*sin(theta)
                z(i,j) = r
            enddo
        enddo

        !   create contour values.
        lev = 0.05_pl_test_flt + 0.10_pl_test_flt * arange(nlevel)

        !   plot the (polar) contours.
        call plcol0(2)
        call plcont(z, 1, RPTS, 1, THETAPTS, lev, xg, yg)
        call plcol0(1)
        call pllab('', '', 'Polar Contour Plot')
    end subroutine polar

    !----------------------------------------------------------------------------
    !    shielded potential contour plot example
    subroutine potential()

        integer          :: i, j, nx, ny, kx, lx, ky, ly, &
               nlevel, ilevgt, ilevlt, nlevlt, nlevgt, &
               ncollin, ncolbox, ncollab, &
               nxsub, nysub
        real(kind=pl_test_flt) :: r, theta, rmax, x0, &
               y0, xmin, xmax, eps, q1, d1, &
               ymin, ymax, &
               q1i, d1i, q2, d2, q2i, d2i, div1, div1i, div2, div2i, &
               zmin, zmax, dz, xpmin, xpmax, ypmin, ypmax, &
               xtick, ytick, delta

        !    xdim and ydim are the absolute static size of the 2D arrays.
        !    NCX and NCY are associated with the part of the
        !    2D arrays that are defined.
        integer, parameter :: xdim=99, NCX=40, ydim=100, NCY=64, NPLT=100

        real(kind=pl_test_flt)   :: z(xdim, ydim), ztmp(xdim, ydim+1)
        real(kind=pl_test_flt)   :: xg(xdim, ydim+1), yg(xdim, ydim+1), xtm(NPLT), ytm(NPLT)

        real(kind=pl_test_flt)   :: clevel(20)
        character(len=8)   :: xopt, yopt

        nx = NCX
        ny = NCY

        kx = 1
        lx = nx
        ky = 1
        ly = ny

        !    Set up r-theta grids
        !    Tack on extra cell in theta to handle periodicity.

        do i = 1, nx
            r = i - 0.5_pl_test_flt
            do j = 1, ny
                theta = TWOPI/real(ny-1,kind=pl_test_flt) * (j-0.5_pl_test_flt)
                xg(i,j) = r * cos(theta)
                yg(i,j) = r * sin(theta)
            enddo
            xg(i, ny+1) = xg(i, 1)
            yg(i, ny+1) = yg(i, 1)
        enddo

        xmax = maxval( xg(1:nx,1:ny) )
        xmin = minval( xg(1:nx,1:ny) )
        ymax = maxval( yg(1:nx,1:ny) )
        ymin = minval( yg(1:nx,1:ny) )

        rmax = r
        x0 = (xmin + xmax)/2._pl_test_flt
        y0 = (ymin + ymax)/2._pl_test_flt

        !    Potential inside a conducting cylinder (or sphere) by method of images.
        !    Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
        !    Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
        !    Also put in smoothing term at small distances.

        eps = 2._pl_test_flt

        q1 = 1._pl_test_flt
        d1 = r/4._pl_test_flt

        q1i = - q1*r/d1
        d1i = r**2/d1

        q2 = -1._pl_test_flt
        d2 = r/4._pl_test_flt

        q2i = - q2*r/d2
        d2i = r**2/d2

        do i = 1, nx
            do j = 1, ny
                div1  = sqrt((xg(i,j)-d1)**2  + (yg(i,j)-d1)**2  + eps**2)
                div1i = sqrt((xg(i,j)-d1i)**2 + (yg(i,j)-d1i)**2 + eps**2)

                div2  = sqrt((xg(i,j)-d2)**2  + (yg(i,j)+d2)**2  + eps**2)
                div2i = sqrt((xg(i,j)-d2i)**2 + (yg(i,j)+d2i)**2 + eps**2)

                z(i,j) = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i
            enddo
        enddo

        !   Tack on extra cell in theta to handle periodicity.

        ztmp(:,1:ny) = z
        ztmp(:,ny+1:ny+1) = z(:,1:1)

        zmax = maxval( z(1:nx,1:ny) )
        zmin = minval( z(1:nx,1:ny) )

        !   Set up contour levels.

        nlevel = 20
        dz = abs(zmax - zmin)/real(nlevel,kind=pl_test_flt)
        clevel(1:nlevel) = zmin + (arange(1,nlevel+1) - 0.5_pl_test_flt) * dz

        !   Split contours into two parts, z > 0, and z < 0.
        !   Dashed contours will be at levels 'ilevlt' through 'ilevlt+nlevlt'.
        !   Solid  contours will be at levels 'ilevgt' through 'ilevgt+nlevgt'.
        !
        !   Since the array clevel is ordered, we can find the level
        !   where the values become positive by counting the non-positive
        !   elements

        ilevlt = 1
        nlevlt = count( clevel(1:nlevel) <= 0.0_pl_test_flt )
        ilevgt = ilevlt + nlevlt
        nlevgt = nlevel - nlevlt

        !   Advance graphics frame and get ready to plot.

        ncollin = 11
        ncolbox = 1
        ncollab = 2

        call pladv(0)
        call plcol0(ncolbox)

        !   Scale window to user coordinates.
        !   Make a bit larger so the boundary doesn't get clipped.

        eps = 0.05_pl_test_flt
        xpmin = xmin - abs(xmin)*eps
        xpmax = xmax + abs(xmax)*eps
        ypmin = ymin - abs(ymin)*eps
        ypmax = ymax + abs(ymax)*eps

        call plvpas(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt, 1.0_pl_test_flt )
        call plwind(xpmin, xpmax, ypmin, ypmax)

        xopt = ' '
        yopt = ' '
        xtick = 0._pl_test_flt
        nxsub = 0
        ytick = 0._pl_test_flt
        nysub = 0

        call plbox(xopt, xtick, nxsub, yopt, ytick, nysub)

        !   Call plotter once for z < 0 (dashed), once for z > 0 (solid lines).

        call plcol0(ncollin)
        if (nlevlt > 0) then
            call pllsty(2)
            call plcont(ztmp, kx, lx, ky, ly+1, &
                   clevel(ilevlt:nlevlt), xg, yg)
        endif
        if (nlevgt > 0) then
            call pllsty(1)
            call plcont(ztmp, kx, lx, ky, ly+1, &
                   clevel(ilevgt:ilevgt-1+nlevgt), xg, yg)
        endif

        !   Draw boundary.

        delta = TWOPI/(NPLT-1)
        xtm = x0 + rmax * cos(delta*arange(NPLT))
        ytm = y0 + rmax * sin(delta*arange(NPLT))

        call plcol0(ncolbox)
        call plline(xtm, ytm)

        call plcol0(ncollab)
        call pllab('', '', 'Shielded potential of charges in a conducting sphere')
    end subroutine potential
end program x09f
