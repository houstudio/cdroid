!      Shade plot demo.
!      Does a variety of shade plots.
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
program x15f
    use plplot
    use plfortrandemolib
    use iso_c_binding, only: c_ptr, c_loc, c_f_pointer
    implicit none

    integer  xdim, ydim, XPTS, YPTS
    !      xdim and ydim are the static dimensions of the 2D arrays while
    !      NX and NY are the defined area.
    parameter( xdim = 99, XPTS = 35, ydim = 100, YPTS = 46 )

    integer i,  j
    integer :: plparseopts_rc
    real(kind=pl_test_flt)  xx, yy
    real(kind=pl_test_flt)  z(xdim, ydim), zmin, zmax, tr(6)

    ! Global parameters to be used in mypltr callback
    real(kind=pl_test_flt), parameter :: xmin = -1.0_pl_test_flt
    real(kind=pl_test_flt), parameter :: xmax =  1.0_pl_test_flt
    real(kind=pl_test_flt), parameter :: ymin = -1.0_pl_test_flt
    real(kind=pl_test_flt), parameter :: ymax =  1.0_pl_test_flt

    ! Use tr callback?
    logical, parameter :: tr_callback = .false.

    ! Use C pltr1 callback? (only meaningful
    ! if tr_callback is .false.).
    logical, parameter :: pltr1_callback = .false.

    ! Use Fortran callback with no data? (only meaningful
    ! if tr_callback and pltr1_callback are .false.).
    logical, parameter :: mypltr_callback = .false.

    type mypltr_data_type
        ! Only contains data required by the mypltr_data callback
        integer :: xpts_data, ypts_data
        real(kind=pl_test_flt) :: xmin_data, xmax_data, ymin_data, ymax_data
    end type mypltr_data_type

    ! Global data type to be used in mypltr_data callback
    type(mypltr_data_type), target :: data

    data%xpts_data = XPTS
    data%ypts_data = YPTS
    data%xmin_data = xmin
    data%xmax_data = xmax
    data%ymin_data = ymin
    data%ymax_data = ymax

    tr = (/ (xmax-xmin)/real(XPTS-1,kind=pl_test_flt), 0.0_pl_test_flt, xmin, &
           0.0_pl_test_flt, (ymax-ymin)/real(YPTS-1,kind=pl_test_flt), ymin /)
    !      Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !      Set up color map 1

          call cmap1_init2()

    !      Initialize plplot

    call plinit()

    !      Set up data array

    do i = 1,XPTS
        xx = real(i-1 - (XPTS / 2),kind=pl_test_flt) / real(XPTS / 2,kind=pl_test_flt)
        do j = 1,YPTS
            yy = real(j-1 - (YPTS / 2),kind=pl_test_flt) / real(YPTS / 2,kind=pl_test_flt) - 1.0_pl_test_flt
            z(i,j) = xx*xx - yy*yy + (xx - yy)/(xx*xx+yy*yy + 0.1_pl_test_flt)
        enddo
    enddo

    call a2mnmx(z, XPTS, YPTS, zmin, zmax, xdim)

    call plot1(z, XPTS, YPTS, zmin, zmax, xdim)
    call plot2(z, XPTS, YPTS, zmin, zmax, xdim)
    call plot3()

    call plend()

contains

    ! Callback function that relies on global XPTS, YPTS, xmin, xmax, ymin, ymax
    subroutine mypltr( x, y, xt, yt )

        ! These callback arguments must have exactly these attributes.
        real(kind=pl_test_flt), intent(in) ::  x, y
        real(kind=pl_test_flt), intent(out) :: xt, yt

        xt = xmin + ((xmax-xmin)/real(XPTS-1,kind=pl_test_flt))*x
        yt = ymin + ((ymax-ymin)/real(YPTS-1,kind=pl_test_flt))*y

    end subroutine mypltr

    ! Callback function that uses data argument to pass required data.
    subroutine mypltr_data( x, y, xt, yt, data )

        ! These callback arguments must have exactly these attributes.
        real(kind=pl_test_flt), intent(in) ::  x, y
        real(kind=pl_test_flt), intent(out) :: xt, yt
        type(c_ptr), intent(in) :: data

        type(mypltr_data_type), pointer :: d
        call c_f_pointer(data, d)

        xt = d%xmin_data + ((d%xmax_data-d%xmin_data)/real(d%xpts_data-1,kind=pl_test_flt))*x
        yt = d%ymin_data + ((d%ymax_data-d%ymin_data)/real(d%ypts_data-1,kind=pl_test_flt))*y

    end subroutine mypltr_data

    ! -------------------------------------------------------------------------
    !      cmap1_init1
    !
    !      Initializes color map 1 in HLS space.
    ! -------------------------------------------------------------------------

!    subroutine cmap1_init1()
!        use plplot
!        implicit none
!        real(kind=pl_test_flt)   i(4), h(4), l(4), s(4)
!
!        i(1) = 0.0_pl_test_flt    ! left boundary
!        i(2) = 0.45_pl_test_flt   ! just before center
!        i(3) = 0.55_pl_test_flt   ! just after center
!        i(4) = 1.0_pl_test_flt    ! right boundary
!
!        h(1) = 260.0_pl_test_flt  ! hue -- low: blue-violet
!        h(2) = 260.0_pl_test_flt  ! only change as we go over vertex
!        h(3) = 20.0_pl_test_flt   ! hue -- high: red
!        h(4) = 20.0_pl_test_flt   ! keep fixed
!
!
!        l(1) = 0.5_pl_test_flt    ! lightness -- low
!        l(2) = 0.0_pl_test_flt    ! lightness -- center
!        l(3) = 0.0_pl_test_flt    ! lightness -- center
!        l(4) = 0.5_pl_test_flt    ! lightness -- high
!
!        !     call plscolbg(255,255,255)
!        !     l(1) = 0.5_pl_test_flt    ! lightness -- low
!        !     l(2) = 1.0_pl_test_flt    ! lightness -- center
!        !     l(3) = 1.0_pl_test_flt    ! lightness -- center
!        !     l(4) = 0.5_pl_test_flt    ! lightness -- high
!
!        s(1) = 1.0_pl_test_flt    ! maximum saturation
!        s(2) = 1.0_pl_test_flt    ! maximum saturation
!        s(3) = 1.0_pl_test_flt    ! maximum saturation
!        s(4) = 1.0_pl_test_flt    ! maximum saturation
!
!        call plscmap1l(.false., i, h, l, s)
!    end subroutine cmap1_init1

    ! -------------------------------------------------------------------------
    !      cmap1_init2
    !
    !      Initializes color map 1 in HLS space.
    ! -------------------------------------------------------------------------

    subroutine cmap1_init2()
        use plplot
        implicit none
        real(kind=pl_test_flt) i(4), h(4), l(4), s(4)

        i(1) = 0.0_pl_test_flt    ! left boundary
        i(2) = 0.45_pl_test_flt   ! just before center
        i(3) = 0.55_pl_test_flt   ! just after center
        i(4) = 1.0_pl_test_flt    ! right boundary

        h(1) = 260.0_pl_test_flt  ! hue -- low: blue-violet
        h(2) = 260.0_pl_test_flt  ! only change as we go over vertex
        h(3) = 20.0_pl_test_flt   ! hue -- high: red
        h(4) = 20.0_pl_test_flt   ! keep fixed


        l(1) = 0.6_pl_test_flt    ! lightness -- low
        l(2) = 0.0_pl_test_flt    ! lightness -- center
        l(3) = 0.0_pl_test_flt    ! lightness -- center
        l(4) = 0.6_pl_test_flt    ! lightness -- high

        !     call plscolbg(255,255,255)
        !     l(1) = 0.5_pl_test_flt    ! lightness -- low
        !     l(2) = 1.0_pl_test_flt    ! lightness -- center
        !     l(3) = 1.0_pl_test_flt    ! lightness -- center
        !     l(4) = 0.5_pl_test_flt    ! lightness -- high

        s(1) = 1.0_pl_test_flt    ! maximum saturation
        s(2) = 0.5_pl_test_flt    ! maximum saturation
        s(3) = 0.5_pl_test_flt    ! maximum saturation
        s(4) = 1.0_pl_test_flt    ! maximum saturation

        call plscmap1l(.false., i, h, l, s)
    end subroutine cmap1_init2

    ! -------------------------------------------------------------------------
    !      plot1
    !
    !      Illustrates a single shaded region.
    ! -------------------------------------------------------------------------

    subroutine plot1(z, XPTS, YPTS, zmin, zmax, xdim)
        use plplot
        implicit none

        integer xdim, XPTS, YPTS
        real(kind=pl_test_flt)  z(xdim,YPTS), zmin, zmax

        real(kind=pl_test_flt)  shade_min, shade_max, sh_color
        integer sh_cmap
        integer min_color, max_color
        real(kind=pl_test_flt) sh_width, min_width, max_width
        real(kind=pl_test_flt), dimension(:), allocatable :: xg, yg

        sh_cmap   = 0
        min_color = 0
        min_width = 0
        max_color = 0
        max_width = 0

        call pladv(0)
        call plvpor( 0.1_pl_test_flt, 0.9_pl_test_flt,  0.1_pl_test_flt, 0.9_pl_test_flt)
        call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt)

        !      Plot using identity transform

        shade_min = zmin + (zmax-zmin)*0.4_pl_test_flt
        shade_max = zmin + (zmax-zmin)*0.6_pl_test_flt
        sh_color  = 7
        sh_width  = 2
        min_color = 9
        max_color = 2
        min_width = 2
        max_width = 2

        call plpsty(8)

        if(tr_callback) then
            call plshade(z(:XPTS,:YPTS), &
                   -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, 1._pl_test_flt, &
                   shade_min, shade_max, &
                   sh_cmap, sh_color, sh_width, &
                   min_color, min_width, max_color, max_width, .true., tr )
        elseif(pltr1_callback) then
            allocate( xg(XPTS), yg(YPTS) )
            xg = (2.0_pl_test_flt/real(XPTS-1,kind=pl_test_flt))*arange(XPTS) - 1.0_pl_test_flt
            yg = (2.0_pl_test_flt/real(YPTS-1,kind=pl_test_flt))*arange(YPTS) - 1.0_pl_test_flt
            call plshade(z(:XPTS,:YPTS), &
                   -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, 1._pl_test_flt, &
                   shade_min, shade_max, &
                   sh_cmap, sh_color, sh_width, &
                   min_color, min_width, max_color, max_width, .true., xg, yg )
        elseif(mypltr_callback) then
            call plshade(z(:XPTS,:YPTS), &
                   -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, 1._pl_test_flt, &
                   shade_min, shade_max, &
                   sh_cmap, sh_color, sh_width, &
                   min_color, min_width, max_color, max_width, .true., mypltr )
        else
            call plshade(z(:XPTS,:YPTS), &
                   -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, 1._pl_test_flt, &
                   shade_min, shade_max, &
                   sh_cmap, sh_color, sh_width, &
                   min_color, min_width, max_color, max_width, .true., mypltr_data, c_loc(data))
        endif

        call plcol0(1)
        call plbox('bcnst', 0.0_pl_test_flt, 0, 'bcnstv', 0.0_pl_test_flt, 0)
        call plcol0(2)
        call pllab('distance', 'altitude', 'Bogon flux')

    end subroutine plot1

    ! -------------------------------------------------------------------------
    !      plot2
    !
    !      Illustrates multiple adjacent shaded regions, using different fill
    !      patterns for each region.
    ! -------------------------------------------------------------------------

    subroutine plot2(z, XPTS, YPTS, zmin, zmax, xdim)
        use plplot
        implicit none

        integer xdim, XPTS, YPTS
        real(kind=pl_test_flt)  z(xdim,YPTS), zmin, zmax

        real(kind=pl_test_flt)  shade_min, shade_max, sh_color
        integer sh_cmap
        integer min_color, max_color
        real(kind=pl_test_flt) sh_width, min_width, max_width
        integer i, j

        integer nlin(10), inc(2,10), del(2,10)
        data nlin /1, 1, 1, 1, 1, 2, 2, 2, 2, 2/
        data ( (inc(i,j), i=1,2), j=1,10) / &
               450, 0, -450, 0, 0, 0, 900, 0, &
               300, 0, 450,-450, 0, 900, 0, 450, &
               450, -450, 0, 900/
        data ( (del(i,j), i=1,2), j=1,10) / &
               2000, 2000, 2000, 2000, 2000, 2000, &
               2000, 2000, 2000, 2000, 2000, 2000, &
               2000, 2000, 2000, 2000, 4000, 4000, &
               4000, 2000/

        sh_cmap   = 0
        min_color = 0
        min_width = 0
        max_color = 0
        max_width = 0
        sh_width  = 2

        call pladv(0)
        call plvpor( 0.1_pl_test_flt, 0.9_pl_test_flt,  0.1_pl_test_flt, 0.9_pl_test_flt)
        call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt)

        do  i = 1,10
            shade_min = zmin + (zmax - zmin) * (i-1) / 10.0_pl_test_flt
            shade_max = zmin + (zmax - zmin) * i / 10.0_pl_test_flt
            sh_color = i+5
            call plpat( inc(1:nlin(i),i),del(1:nlin(i),i))
            call plshade(z(:XPTS,:YPTS), &
                   -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, 1._pl_test_flt, &
                   shade_min, shade_max, &
                   sh_cmap, sh_color, sh_width, &
                   min_color, min_width, max_color, max_width, .true. )
        enddo

        call plcol0(1)
        call plbox('bcnst', 0.0_pl_test_flt, 0, 'bcnstv', 0.0_pl_test_flt, 0)
        call plcol0(2)
        call pllab('distance', 'altitude', 'Bogon flux')

    end subroutine plot2
    !--------------------------------------------------------------------------
    !     plot3
    !
    !     Illustrates shaded regions in 3d, using a different fill pattern for
    !     each region.
    !--------------------------------------------------------------------------

    subroutine plot3
        use plplot
        implicit none

        real(kind=pl_test_flt) xx1(5), xx2(5), yy1(5), yy2(5), zz1(5), zz2(5)
        data xx1 / -1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, -1.0_pl_test_flt/
        data xx2 / -1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, -1.0_pl_test_flt/
        data yy1 /1.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt/
        data yy2 / -1.0_pl_test_flt, -1.0_pl_test_flt, 0.0_pl_test_flt, 0.0_pl_test_flt, -1.0_pl_test_flt/
        data zz1 / 0.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt/
        data zz2 / 0.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt/

        call pladv(0)
        call plvpor(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
        call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt)
        call plw3d(1._pl_test_flt, 1._pl_test_flt, 1._pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt, &
               -1.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt,1.5_pl_test_flt, 30._pl_test_flt, -40._pl_test_flt)

        !     Plot using identity transform

        call plcol0(1)
        call plbox3("bntu", "X", 0.0_pl_test_flt, 0, "bntu", "Y", 0.0_pl_test_flt, 0, &
               "bcdfntu", "Z", 0.5_pl_test_flt, 0)
        call plcol0(2)
        call pllab("","","3-d polygon filling")

        call plcol0(3)
        call plpsty(1)
        call plline3(xx1, yy1, zz1)
        call plfill3(xx1(1:4), yy1(1:4), zz1(1:4))
        call plpsty(2)
        call plline3(xx2, yy2, zz2)
        call plfill3(xx2(1:4), yy2(1:4), zz2(1:4))

    end subroutine plot3

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
end program x15f
