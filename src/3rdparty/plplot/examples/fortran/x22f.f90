!      Vector plot demo.
!
!      Copyright (C) 2004-2016  Alan W. Irwin
!      Copyright (C) 2004  Andrew Ross
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
!      Does several contour plots using different coordinate mappings.

program x22f
    use plplot, double_PI => PL_PI
    use plfortrandemolib
    use iso_c_binding, only: c_ptr, c_loc, c_f_pointer
    implicit none
    real(kind=pl_test_flt), parameter :: PI = double_PI

    integer, parameter :: double = kind(1.0d0)
    real(kind=double) common_max

    integer narr
    integer :: plparseopts_rc
    logical fill
    parameter (narr=6)
    real(kind=pl_test_flt) arrow_x(narr),arrow_y(narr), &
           arrow2_x(narr),arrow2_y(narr)

    type callback_data_type
        ! Only contains data required by the tranform_data callback
        real(kind=double) max
    end type callback_data_type

    data arrow_x/-0.5_pl_test_flt, 0.5_pl_test_flt, 0.3_pl_test_flt, 0.5_pl_test_flt, 0.3_pl_test_flt, 0.5_pl_test_flt/
    data arrow_y/0._pl_test_flt, 0._pl_test_flt, 0.2_pl_test_flt, 0._pl_test_flt, -0.2_pl_test_flt, 0._pl_test_flt/
    data arrow2_x/-0.5_pl_test_flt, 0.3_pl_test_flt, 0.3_pl_test_flt, 0.5_pl_test_flt, 0.3_pl_test_flt, 0.3_pl_test_flt/
    data arrow2_y/0._pl_test_flt, 0._pl_test_flt, 0.2_pl_test_flt, 0._pl_test_flt, -0.2_pl_test_flt, 0._pl_test_flt/

    real(kind=pl_test_flt) :: tr(6), xmin_global, xmax_global, ymin_global, ymax_global
    integer :: nx_global, ny_global
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
    ! Use Fortran callback with no data? (only meaningful
    ! if tr_callback and identity_callback are .false.).
    logical, parameter :: mypltr_callback = .false.

    ! use plstransform callback without data?
    logical, parameter :: ifno_transform_data = .false.

    !      Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    ! I believe tr must be declared and defined globally this way in
    ! order for mypltr in circulation to work properly, and I would otherwise declare
    ! and define tr locally in that routine.

    ! Consistent with nx and ny in circulation.
    nx_global = 20
    ny_global = 20

    ! Must be consistent with actual ranges of xg and yg arrays in circulation
    ! so these values not exactly the same as xmin, xmax, ymin, and ymax there.
    xmin_global = - real(nx_global / 2, kind=pl_test_flt) + 0.5_pl_test_flt
    xmax_global = xmin_global + real(nx_global-1, kind=pl_test_flt)
    ymin_global = - real(ny_global / 2, kind=pl_test_flt) + 0.5_pl_test_flt
    ymax_global = ymin_global + real(ny_global-1, kind=pl_test_flt)
    tr = [(xmax_global-xmin_global)/real(nx_global-1,kind=pl_test_flt), 0.0_pl_test_flt, xmin_global, &
           0.0_pl_test_flt, (ymax_global-ymin_global)/real(ny_global-1,kind=pl_test_flt), ymin_global ]

    call plinit

    call circulation

    fill = .false.

    !     Set arrow style using arrow_x and arrow_y the
    !     plot using these arrows
    call plsvect(arrow_x, arrow_y, fill)
    call constriction( 1 )

    !     Set arrow style using arrow_x and arrow_y the
    !     plot using these arrows
    fill = .true.
    call plsvect(arrow2_x, arrow2_y, fill)
    call constriction( 2 )

    call constriction2

    call plsvect

    call potential

    call plend


contains

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

    !     vector plot of the circulation around the origin
    subroutine circulation()

        integer, parameter :: nx=20, ny=20

        real(kind=pl_test_flt), allocatable :: u(:,:), v(:,:), xg(:), yg(:)

        real(kind=pl_test_flt) :: dx, dy, xmin, xmax, ymin, ymax
        real(kind=pl_test_flt) :: scaling

        dx = 1.0_pl_test_flt
        dy = 1.0_pl_test_flt

        xmin = -real(nx,kind=pl_test_flt)/2.0_pl_test_flt*dx
        xmax = real(nx,kind=pl_test_flt)/2.0_pl_test_flt*dx
        ymin = -real(ny,kind=pl_test_flt)/2.0_pl_test_flt*dy
        ymax = real(ny,kind=pl_test_flt)/2.0_pl_test_flt*dy

        allocate( xg(nx), yg(ny) )
        xg = (arange(nx) + (-nx/2.0_pl_test_flt+0.5_pl_test_flt))*dx
        yg = (arange(ny) + (-ny/2.0_pl_test_flt+0.5_pl_test_flt))*dy

        allocate( u(nx,ny), v(nx,ny) )
        u = spread(yg,1,nx)
        v = -spread(xg,2,ny)

        if(.not. tr_callback .and. identity_callback) then
            ! This scaling the same as if xmin, xmax, ymin, and ymax
            ! replaced in plenv call below by actual minimum and maximum
            ! xg and yg values.  So along with different tick marks you
            ! will also get slightly different scaling with this alternative
            ! compared to all other results.
            call plenv( &
                   real(0,kind=pl_test_flt), real(nx-1,kind=pl_test_flt), &
                   real(0,kind=pl_test_flt), real(ny-1,kind=pl_test_flt), 0, 0)
        else
            call plenv(xmin, xmax, ymin, ymax, 0, 0)
        endif
        call pllab('(x)', '(y)',  &
               '#frPLplot Example 22 - circulation')
        call plcol0(2)
        scaling = 0.0_pl_test_flt
        if(tr_callback) then
            call plvect(u,v,scaling,tr)
        elseif(identity_callback) then
            call plvect(u,v,scaling)
        elseif(mypltr_callback) then
            call plvect(u,v,scaling, mypltr)
        else
            data%tr_data = tr
            call plvect(u,v,scaling, mypltr_data, c_loc(data))
        endif
        call plcol0(1)

    end subroutine circulation

    !     vector plot of the flow through a constricted pipe
    subroutine constriction( astyle )

        integer i, j, nx, ny, astyle
        parameter (nx=20, ny=20)

        character(len=80) :: title

        real(kind=pl_test_flt) :: u(nx, ny), v(nx, ny)
        real(kind=pl_test_flt), allocatable :: xg(:), yg(:)

        real(kind=pl_test_flt) dx, dy, xmin, xmax, ymin, ymax
        real(kind=pl_test_flt) xx, yy, Q, b, dbdx, scaling

        dx = 1.0_pl_test_flt
        dy = 1.0_pl_test_flt

        xmin = -real(nx,kind=pl_test_flt)/2.0_pl_test_flt*dx
        xmax = real(nx,kind=pl_test_flt)/2.0_pl_test_flt*dx
        ymin = -real(ny,kind=pl_test_flt)/2.0_pl_test_flt*dy
        ymax = real(ny,kind=pl_test_flt)/2.0_pl_test_flt*dy

        Q = 2.0_pl_test_flt

        allocate( xg(nx), yg(ny) )

        xg = (arange(nx) + (-nx/2.0_pl_test_flt+0.5_pl_test_flt))*dx
        yg = (arange(ny) + (-ny/2.0_pl_test_flt+0.5_pl_test_flt))*dy
        do i=1,nx
            xx = xg(i)
            do j=1,ny
                yy = yg(j)
                b = ymax/4.0_pl_test_flt*(3.0_pl_test_flt-cos(PI*xx/xmax))
                if (abs(yy).lt.b) then
                    dbdx = ymax/4.0_pl_test_flt*sin(PI*xx/xmax)*PI/xmax*yy/b
                    u(i,j) = Q*ymax/b
                    v(i,j) = u(i,j)*dbdx
                else
                    u(i,j) = 0.0_pl_test_flt
                    v(i,j) = 0.0_pl_test_flt
                endif
            enddo
        enddo

        call plenv(xmin, xmax, ymin, ymax, 0, 0)
        write(title,'(A,I0,A)') '#frPLplot Example 22 - constriction (arrow style ', astyle,')'
        call pllab('(x)', '(y)', title)
        call plcol0(2)
        scaling = -1.0_pl_test_flt
        call plvect(u,v,scaling,xg,yg)
        call plcol0(1)

    end subroutine constriction

    ! Global transform function for a constriction using data passed in
    ! This is the same transformation used in constriction.
    ! Note:
    ! We need to use double precision here, independent of what precision is used
    ! in the rest of the program
    !
    subroutine transform( x, y, xt, yt )

        ! These callback arguments must have exactly these attributes.
        real(kind=double), intent(in) ::  x, y
        real(kind=double), intent(out) :: xt, yt

        xt = x
        yt = y / 4.0_pl_test_flt * ( 3.0_pl_test_flt - cos( PI * x / common_max ) )
    end subroutine transform

    ! Variant of transform that carries a generic data argument.
    subroutine transform_data( x, y, xt, yt, data )

        ! These callback arguments must have exactly these attributes.
        real(kind=double), intent(in) ::  x, y
        real(kind=double), intent(out) :: xt, yt
        type(c_ptr), intent(in) :: data

        type(callback_data_type), pointer :: d
        call c_f_pointer(data, d)

        xt = x
        yt = y / 4.0_pl_test_flt * ( 3.0_pl_test_flt - cos( PI * x / d%max ) )
    end subroutine transform_data

    ! Vector plot of flow through a constricted pipe
    ! with a coordinate transform
    subroutine constriction2()

        integer, parameter :: nx=20, ny=20, nc=11, nseg=20

        real(kind=pl_test_flt) dx, dy
        real(kind=pl_test_flt) xmin, xmax, ymin, ymax
        real(kind=pl_test_flt) Q, scaling
        real(kind=pl_test_flt), allocatable :: xg(:), yg(:), b(:), u(:,:), v(:,:), clev(:)

        type(callback_data_type), target :: data

        dx = 1.0_pl_test_flt
        dy = 1.0_pl_test_flt

        xmin = -real(nx,kind=pl_test_flt)/2.0_pl_test_flt*dx
        xmax = real(nx,kind=pl_test_flt)/2.0_pl_test_flt*dx
        ymin = -real(ny,kind=pl_test_flt)/2.0_pl_test_flt*dy
        ymax = real(ny,kind=pl_test_flt)/2.0_pl_test_flt*dy
        common_max = ymax
        data%max = common_max

        allocate( b(nx) )
        allocate( xg(nx), yg(ny) )
        allocate( u(nx,ny), v(nx,ny) )
        allocate( clev(nc) )

        if(ifno_transform_data) then
            call plstransform( transform )
        else
            call plstransform( transform_data, c_loc(data))
        endif

        Q = 2.0_pl_test_flt
        xg = (arange(nx) + (-nx/2.0_pl_test_flt+0.5_pl_test_flt))*dx
        yg = (arange(ny) + (-ny/2.0_pl_test_flt+0.5_pl_test_flt))*dy
        b = Q*4.0_pl_test_flt/(3.0_pl_test_flt-cos(PI*xg/xmax))
        u = spread(b,2,ny)
        v = 0.0_pl_test_flt*u
        clev = Q + arange(nc) * (Q/real(nc-1,kind=pl_test_flt))

        call plenv(xmin, xmax, ymin, ymax, 0, 0)
        call pllab('(x)', '(y)', &
               '#frPLplot Example 22 - constriction with plstransform')
        call plcol0(2)
        call plshades(u, xmin + dx / 2.0_pl_test_flt, &
               xmax - dx / 2.0_pl_test_flt, &
               ymin + dy / 2.0_pl_test_flt, ymax - dy / 2.0_pl_test_flt, &
               clev, 0.0_pl_test_flt, 1, 1.0_pl_test_flt, .false. )
        scaling = -1.0_pl_test_flt
        call plvect(u,v,scaling,xg,yg)
        call plpath(nseg, xmin, ymax, xmax, ymax)
        call plpath(nseg, xmin, ymin, xmax, ymin)
        call plcol0(1)

        call plstransform()

    end subroutine constriction2

    subroutine potential()

        integer i, j, nr, ntheta, nper, nlevel
        parameter (nr=20, ntheta=20, nper=100, nlevel=10)

        real(kind=pl_test_flt) u(nr, ntheta), v(nr, ntheta), z(nr, ntheta)
        real(kind=pl_test_flt) xg(nr,ntheta), yg(nr,ntheta)
        real(kind=pl_test_flt) clevel(nlevel), px(nper), py(nper)

        real(kind=pl_test_flt) xmin, xmax, ymin, ymax, zmin, zmax, rmax
        real(kind=pl_test_flt) xx, yy, r, theta, scaling, dz

        real(kind=pl_test_flt) eps, q1, d1, q1i, d1i, q2, d2, q2i, d2i
        real(kind=pl_test_flt) div1, div1i, div2, div2i

        rmax = real(nr,kind=pl_test_flt)

        eps = 2.0_pl_test_flt

        q1 = 1.0_pl_test_flt
        d1 = rmax/4.0_pl_test_flt

        q1i = - q1*rmax/d1
        d1i = rmax**2.0_pl_test_flt/d1

        q2 = -1.0_pl_test_flt
        d2 = rmax/4.0_pl_test_flt

        q2i = - q2*rmax/d2
        d2i = rmax**2.0_pl_test_flt/d2

        do i = 1, nr
            r = 0.5 + real(i-1,kind=pl_test_flt)
            do j = 1, ntheta
                theta = 2.*PI/real(ntheta-1,kind=pl_test_flt)*(real(j,kind=pl_test_flt)-0.5)
                xx = r*cos(theta)
                yy = r*sin(theta)
                xg(i,j) = xx
                yg(i,j) = yy
                div1 = sqrt((xg(i,j)-d1)**2 + (yg(i,j)-d1)**2 + eps**2)
                div1i = sqrt((xg(i,j)-d1i)**2 + (yg(i,j)-d1i)**2 + eps**2)

                div2 = sqrt((xg(i,j)-d2)**2 + (yg(i,j)+d2)**2 + eps**2)
                div2i = sqrt((xg(i,j)-d2i)**2 + (yg(i,j)+d2i)**2 + eps**2)

                z(i,j) = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i
                u(i,j) = -q1*(xx-d1)/div1**3 - q1i*(xx-d1i)/div1i**3 - &
                       q2*(xx-d2)/div2**3 - q2i*(xx-d2i)/div2i**3
                v(i,j) = -q1*(yy-d1)/div1**3 - q1i*(yy-d1i)/div1i**3 - &
                       q2*(yy+d2)/div2**3 - q2i*(yy+d2i)/div2i**3
            enddo
        enddo

        call a2mnmx(xg, nr, ntheta, xmin, xmax, nr)
        call a2mnmx(yg, nr, ntheta, ymin, ymax, nr)
        call a2mnmx(z, nr, ntheta, zmin, zmax, nr)

        call plenv(xmin, xmax, ymin, ymax, 0, 0)
        call pllab('(x)', '(y)',  &
               '#frPLplot Example 22 - potential gradient vector plot')

        !     plot contours of the potential
        dz = abs(zmax - zmin)/real(nlevel,kind=pl_test_flt)
        do i = 1, nlevel
            clevel(i) = zmin + (i-0.5_pl_test_flt)*dz
        enddo
        call plcol0(3)
        call pllsty(2)
        call plcont(z,1,nr,1,ntheta,clevel,xg,yg)
        call pllsty(1)
        call plcol0(1)

        call plcol0(2)
        scaling = 25.0_pl_test_flt
        call plvect(u,v,scaling,xg,yg)
        call plcol0(1)

        do i=1,nper
            theta = 2.0_pl_test_flt*PI/real(nper-1,kind=pl_test_flt)*real(i,kind=pl_test_flt)
            px(i) = rmax*cos(theta)
            py(i) = rmax*sin(theta)
        enddo

        call plline(px,py)

    end subroutine potential

    !----------------------------------------------------------------------------
    !      Subroutine a2mnmx
    !      Minimum and the maximum elements of a 2-d array.

    subroutine a2mnmx(f, nx, ny, fmin, fmax, xdim)

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
end program x22f
