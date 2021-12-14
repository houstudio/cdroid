!      plshades demo, using color fill
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
program x16f
    use plplot, double_PI => PL_PI
    use iso_c_binding, only: c_ptr, c_loc, c_f_pointer
    implicit none
    real(kind=pl_test_flt), parameter :: PI = double_PI
    real(kind=pl_test_flt) xx, yy, argx, argy, distort, r, t
    integer NLEVEL, NX, NY, PERIMETERPTS, xdim, ydim
    parameter(NLEVEL = 20)
    !      xdim and ydim are the static dimensions of the 2D arrays while
    !      NX and NY are the defined area.
    parameter(xdim=99, NX = 35)
    parameter(ydim=100, NY = 46)
    parameter(PERIMETERPTS = 100)
    real(kind=pl_test_flt) clevel(NLEVEL), shedge(NLEVEL+1), &
           z(xdim,ydim), w(xdim,ydim), xg1(xdim), yg1(ydim), &
           xg2(xdim,ydim), yg2(xdim,ydim), zmin, zmax, &
           px(PERIMETERPTS), py(PERIMETERPTS)
    integer cont_color
    real(kind=pl_test_flt) fill_width, cont_width
    real(kind=pl_test_flt) colorbar_width, colorbar_height
    integer NUM_AXES, NUM_LABELS
    parameter(NUM_AXES=1, NUM_LABELS=1)
    character(len=20) :: axis_opts(NUM_AXES)
    integer num_values(NUM_AXES)
    real(kind=pl_test_flt) values(NUM_AXES,NLEVEL+1)
    real(kind=pl_test_flt) axis_ticks(NUM_AXES)
    integer axis_subticks(NUM_AXES)
    character(len=100) :: labels(NUM_LABELS)
    integer label_opts(NUM_LABELS)

    integer i, j
    integer :: plparseopts_rc
    real(kind=pl_test_flt) tr(6)

    ! Global parameters to be used in mypltr callback
    real(kind=pl_test_flt), parameter :: xmin = -1.0_pl_test_flt
    real(kind=pl_test_flt), parameter :: xmax =  1.0_pl_test_flt
    real(kind=pl_test_flt), parameter :: ymin = -1.0_pl_test_flt
    real(kind=pl_test_flt), parameter :: ymax =  1.0_pl_test_flt

    ! Use no callback?
    logical, parameter :: no_callback = .false.

    ! Use tr callback? (only meaningful
    ! if no_callback is .false.).
    logical, parameter :: tr_callback = .false.

    ! Use Fortran callback with no data? (only meaningful
    ! if no_callback and tr_callback are .false.).
    logical, parameter :: mypltr_callback = .false.

    type mypltr_data_type
        ! Only contains data required by the mypltr_data callback
        integer :: xpts_data, ypts_data
        real(kind=pl_test_flt) :: xmin_data, xmax_data, ymin_data, ymax_data
    end type mypltr_data_type

    ! Global data type to be used in mypltr_data callback
    type(mypltr_data_type), target :: data

    data%xpts_data = NX
    data%ypts_data = NY
    data%xmin_data = xmin
    data%xmax_data = xmax
    data%ymin_data = ymin
    data%ymax_data = ymax

    tr = (/ (xmax-xmin)/real(NX-1,kind=pl_test_flt), 0.0_pl_test_flt, xmin, &
           0.0_pl_test_flt, (ymax-ymin)/real(NY-1,kind=pl_test_flt), ymin /)

    !      Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !      Load color palettes
    call plspal0('cmap0_black_on_white.pal')
    call plspal1('cmap1_gray.pal', .true.)

    !      Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    call plscmap0n(3)

    !      Initialize plplot

    call plinit()

    !      Calculate the data matrices.
    do i=1,NX
        xx = real(i-1-(NX/2),kind=pl_test_flt)/real(NX/2,kind=pl_test_flt)
        do j=1,NY
            yy = real(j-1-(NY/2),kind=pl_test_flt)/real(NY/2,kind=pl_test_flt) - 1.0_pl_test_flt
            z(i,j) = - sin(7._pl_test_flt*xx) * cos(7._pl_test_flt*yy) + xx*xx - yy*yy
            w(i,j) = - cos(7._pl_test_flt*xx) * sin(7._pl_test_flt*yy) + 2._pl_test_flt*xx*yy
        enddo
    enddo

    call a2mnmx(z, NX, NY, zmin, zmax, xdim)
    do i = 1, NLEVEL
        clevel(i) = zmin + (zmax - zmin) * (i - 0.5_pl_test_flt) / real(NLEVEL,kind=pl_test_flt)
    enddo
    do i = 1, NLEVEL+1
        shedge(i) = zmin + (zmax - zmin) * real(i-1,kind=pl_test_flt) / real(NLEVEL,kind=pl_test_flt)
    enddo

    !      Build the 1-d coord arrays.
    distort = 0.4_pl_test_flt
    do i=1,NX
        xx = -1._pl_test_flt + real(i-1,kind=pl_test_flt)*2._pl_test_flt/real(NX-1,kind=pl_test_flt)
        xg1(i) = xx + distort*cos(0.5_pl_test_flt*PI*xx)
    enddo

    do j=1,NY
        yy = -1._pl_test_flt + real(j-1,kind=pl_test_flt)*2._pl_test_flt/real(NY-1,kind=pl_test_flt)
        yg1(j) = yy - distort*cos(0.5_pl_test_flt*PI*yy)
    enddo

    !      Build the 2-d coord arrays.
    do i=1,NX
        xx = -1._pl_test_flt + real(i-1,kind=pl_test_flt)*2._pl_test_flt/real(NX-1,kind=pl_test_flt)
        argx = 0.5_pl_test_flt*PI*xx
        do j=1,NY
            yy = -1._pl_test_flt + real(j-1,kind=pl_test_flt)*2._pl_test_flt/real(NY-1,kind=pl_test_flt)
            argy = 0.5_pl_test_flt*PI*yy
            xg2(i,j) = xx + distort*cos(argx)*cos(argy)
            yg2(i,j) = yy - distort*cos(argx)*cos(argy)
        enddo
    enddo

    !      Plot using transform of index range to xmin, xmax, ymin, ymax
    call pladv(0)
    call plvpor(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
    call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt)

    call plpsty(0)

    fill_width = 2
    cont_color = 0
    cont_width = 0
    axis_opts(1) = 'bcvtm'
    axis_ticks(1) = 0.0_pl_test_flt
    axis_subticks(1) = 0
    label_opts(1) = PL_COLORBAR_LABEL_BOTTOM
    labels(1) = 'Magnitude'

    if(no_callback) then
        call plshades(z(:NX,:NY), -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, &
               1._pl_test_flt, &
               shedge, fill_width, &
               cont_color, cont_width, .true. )
    elseif(tr_callback) then
        call plshades(z(:NX,:NY), -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, &
               1._pl_test_flt, &
               shedge, fill_width, &
               cont_color, cont_width, .true., tr )
    elseif(mypltr_callback) then
        call plshades(z(:NX,:NY), -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, &
               1._pl_test_flt, &
               shedge, fill_width, &
               cont_color, cont_width, .true., mypltr )
    else
        call plshades(z(:NX,:NY), -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, &
               1._pl_test_flt, &
               shedge, fill_width, &
               cont_color, cont_width, .true., mypltr_data, c_loc(data) )
    endif

    ! Smaller text
    call  plschr( 0.0_pl_test_flt, 0.75_pl_test_flt )
    ! Small ticks on the vertical axis
    call plsmaj( 0.0_pl_test_flt, 0.5_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 0.5_pl_test_flt )

    num_values(1) = NLEVEL + 1;
    values(1,:)   = shedge;
    call plcolorbar( colorbar_width, colorbar_height, &
           ior(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0, &
           0.005_pl_test_flt, 0.0_pl_test_flt, 0.0375_pl_test_flt, 0.875_pl_test_flt, 0, 1, 1, &
           0.0_pl_test_flt, 0.0_pl_test_flt, &
           cont_color, cont_width, &
           label_opts, labels, &
           axis_opts, &
           axis_ticks, axis_subticks, &
           num_values, values )

    ! Reset text and tick sizes
    call plschr( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmaj( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 1.0_pl_test_flt )

    call plcol0(1)
    call plbox('bcnst', 0.0_pl_test_flt, 0, 'bcnstv', 0.0_pl_test_flt, 0)
    call plcol0(2)
    call pllab('distance', 'altitude', 'Bogon density')

    !      Plot using 1d coordinate transform

    call plspal0('cmap0_black_on_white.pal')
    call plspal1('cmap1_blue_yellow.pal', .true.)
    call plscmap0n(3)

    call pladv(0)
    call plvpor(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
    call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt)

    call plpsty(0)

    fill_width = 2
    cont_color = 0
    cont_width = 0
    call plshades(z(:NX,:NY), -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, &
           1._pl_test_flt, &
           shedge, fill_width, &
           cont_color, cont_width, .true., xg1(:NX), yg1(:NY))

    ! Smaller text
    call  plschr( 0.0_pl_test_flt, 0.75_pl_test_flt )
    ! Small ticks on the vertical axis
    call plsmaj( 0.0_pl_test_flt, 0.5_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 0.5_pl_test_flt )

    num_values(1) = NLEVEL + 1;
    values(1,:)   = shedge;
    call plcolorbar( colorbar_width, colorbar_height, &
           ior(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0, &
           0.005_pl_test_flt, 0.0_pl_test_flt, 0.0375_pl_test_flt, 0.875_pl_test_flt, 0, 1, 1, &
           0.0_pl_test_flt, 0.0_pl_test_flt, &
           cont_color, cont_width, &
           label_opts, labels, &
           axis_opts, &
           axis_ticks, axis_subticks, &
           num_values, values )

    ! Reset text and tick sizes
    call plschr( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmaj( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 1.0_pl_test_flt )

    call plcol0(1)
    call plbox('bcnst', 0.0_pl_test_flt, 0, 'bcnstv', 0.0_pl_test_flt, 0)
    call plcol0(2)
    call pllab('distance', 'altitude', 'Bogon density')

    !      Plot using 2d coordinate transform

    call plspal0('cmap0_black_on_white.pal')
    call plspal1('cmap1_blue_red.pal', .true.)
    call plscmap0n(3)

    call pladv(0)
    call plvpor(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
    call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt)

    call plpsty(0)

    fill_width = 2
    cont_color = 0
    cont_width = 0
    call plshades(z(:NX,:NY), -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, &
           1._pl_test_flt, &
           shedge, fill_width, &
           cont_color, cont_width, .false., xg2(:NX,:NY), yg2(:NX,:NY) )

    ! Smaller text
    call  plschr( 0.0_pl_test_flt, 0.75_pl_test_flt )
    ! Small ticks on the vertical axis
    call plsmaj( 0.0_pl_test_flt, 0.5_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 0.5_pl_test_flt )

    num_values(1) = NLEVEL + 1;
    values(1,:)   = shedge;
    call plcolorbar( colorbar_width, colorbar_height, &
           ior(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0, &
           0.005_pl_test_flt, 0.0_pl_test_flt, 0.0375_pl_test_flt, 0.875_pl_test_flt, 0, 1, 1, &
           0.0_pl_test_flt, 0.0_pl_test_flt, &
           cont_color, cont_width, &
           label_opts, labels, &
           axis_opts, &
           axis_ticks, axis_subticks, &
           num_values, values )

    ! Reset text and tick sizes
    call plschr( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmaj( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 1.0_pl_test_flt )

    call plcol0(1)
    call plbox('bcnst', 0.0_pl_test_flt, 0, 'bcnstv', 0.0_pl_test_flt, 0)
    call plcol0(2)
    call plcont(w, 1, nx, 1, ny, clevel, xg2, yg2)
    call pllab('distance', 'altitude', &
           'Bogon density, with streamlines')

    !      Plot using 2d coordinate transform and plshades contours.
    call plspal0('')
    call plspal1('', .true. )
    call plscmap0n(3)

    call pladv(0)
    call plvpor(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
    call plwind(-1.0_pl_test_flt, 1.0_pl_test_flt, -1.0_pl_test_flt, 1.0_pl_test_flt)

    call plpsty(0)

    fill_width = 2
    cont_color = 2
    cont_width = 3
    call plshades(z(:NX,:NY), -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, &
           1._pl_test_flt, &
           shedge, fill_width, &
           cont_color, cont_width, .false., xg2(:NX,:NY), yg2(:NX,:NY) )

    ! Smaller text
    call  plschr( 0.0_pl_test_flt, 0.75_pl_test_flt )
    ! Small ticks on the vertical axis
    call plsmaj( 0.0_pl_test_flt, 0.5_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 0.5_pl_test_flt )

    num_values(1) = NLEVEL + 1;
    values(1,:)   = shedge;
    call plcolorbar( colorbar_width, colorbar_height, &
           ior(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0, &
           0.005_pl_test_flt, 0.0_pl_test_flt, 0.0375_pl_test_flt, 0.875_pl_test_flt, 0, 1, 1, &
           0.0_pl_test_flt, 0.0_pl_test_flt, &
           2, 3._pl_test_flt, &
           label_opts, labels, &
           axis_opts, &
           axis_ticks, axis_subticks, &
           num_values, values )

    ! Reset text and tick sizes
    call plschr( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmaj( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 1.0_pl_test_flt )

    call plcol0(1)
    call plbox('bcnst', 0.0_pl_test_flt, 0, 'bcnstv', 0.0_pl_test_flt, 0)
    call plcol0(2)
    call pllab('distance', 'altitude', 'Bogon density')

    !      Example with polar coordinates.
    call plspal0('cmap0_black_on_white.pal')
    call plspal1('cmap1_gray.pal', .true. )
    call plscmap0n(3)

    call pladv(0)
    call plvpor(0.1_pl_test_flt, 0.9_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
    call plwind(-1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, 1._pl_test_flt)

    call plpsty(0)

    !      Build new coordinate matrices.
    do i = 1, NX
        r = real(i-1,kind=pl_test_flt)/real(NX-1,kind=pl_test_flt)
        do j = 1, NY
            t = (2._pl_test_flt*PI/real(NY-1,kind=pl_test_flt))*real(j-1,kind=pl_test_flt)
            xg2(i,j) = r*cos(t)
            yg2(i,j) = r*sin(t)
            z(i,j) = exp(-r*r)*cos(5._pl_test_flt*PI*r)*cos(5._pl_test_flt*t)
        enddo
    enddo

    !      Need a new shedge to go along with the new data set.
    call a2mnmx(z, NX, NY, zmin, zmax, xdim)
    do i = 1, NLEVEL+1
        shedge(i) = zmin + (zmax - zmin) * real(i-1,kind=pl_test_flt) / real(NLEVEL,kind=pl_test_flt)
    enddo

    !      Now we can shade the interior region.
    fill_width = 2
    cont_color = 0
    cont_width = 0
    call plshades(z(:NX,:NY), -1._pl_test_flt, 1._pl_test_flt, -1._pl_test_flt, &
           1._pl_test_flt, &
           shedge, fill_width, &
           cont_color, cont_width, .false., xg2(:NX,:NY), yg2(:NX,:NY) )

    ! Smaller text
    call  plschr( 0.0_pl_test_flt, 0.75_pl_test_flt )
    ! Small ticks on the vertical axis
    call plsmaj( 0.0_pl_test_flt, 0.5_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 0.5_pl_test_flt )

    num_values(1) = NLEVEL + 1;
    values(1,:)   = shedge;
    call plcolorbar( colorbar_width, colorbar_height, &
           ior(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0, &
           0.005_pl_test_flt, 0.0_pl_test_flt, 0.0375_pl_test_flt, 0.875_pl_test_flt, 0, 1, 1, &
           0.0_pl_test_flt, 0.0_pl_test_flt, &
           cont_color, cont_width, &
           label_opts, labels, &
           axis_opts, &
           axis_ticks, axis_subticks, &
           num_values, values )

    ! Reset text and tick sizes
    call plschr( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmaj( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsmin( 0.0_pl_test_flt, 1.0_pl_test_flt )

    !      Now we can draw the perimeter.  (If do before, shade stuff may overlap.)
    do i = 1, PERIMETERPTS
        t = (2._pl_test_flt*PI/real(PERIMETERPTS-1,kind=pl_test_flt))*real(i-1,kind=pl_test_flt)
        px(i) = cos(t)
        py(i) = sin(t)
    enddo

    call plcol0(1)
    call plline(px, py)

    !      And label the plot.
    call plcol0(2)
    call pllab( '', '',  'Tokamak Bogon Instability' )

    call plend

contains

    ! Callback function that relies on global NX, NY, xmin, xmax, ymin, ymax
    subroutine mypltr( x, y, xt, yt )

        ! These callback arguments must have exactly these attributes.
        real(kind=pl_test_flt), intent(in) ::  x, y
        real(kind=pl_test_flt), intent(out) :: xt, yt

        xt = xmin + ((xmax-xmin)/real(NX-1,kind=pl_test_flt))*x
        yt = ymin + ((ymax-ymin)/real(NY-1,kind=pl_test_flt))*y

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
end program x16f
