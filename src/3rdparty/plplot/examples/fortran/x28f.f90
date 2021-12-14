!   Demonstrates plotting text in 3D (plmtex3, plptex3)
!
!   Copyright (C) 2007-2016  Alan W. Irwin
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

program x28f
    use plplot, double_PI => PL_PI

    implicit none

    real(kind=pl_test_flt), parameter :: PI = double_PI
    integer, parameter :: XPTS = 2
    integer, parameter :: YPTS = 2
    integer, parameter :: NREVOLUTION = 16
    integer, parameter :: NROTATION = 8
    integer, parameter :: NSHEAR = 8

    real(PLFLT), dimension(:), allocatable   :: x, y
    real(PLFLT), dimension(:,:), allocatable :: z

    real(PLFLT), parameter :: xmin=0., xmax=1.0,                            &
           xmid = 0.5*(xmax + xmin), xrange = xmax - xmin,                    &
           ymin=0., ymax=1.0, ymid = 0.5*(ymax + ymin), yrange = ymax - ymin, &
           zmin=0., zmax=1.0, zmid = 0.5*(zmax + zmin), zrange = zmax - zmin
    real(PLFLT), parameter :: &
           ysmin    = ymin + 0.1 * yrange, &
           ysmax    = ymax - 0.1 * yrange, &
           ysrange  = ysmax - ysmin, &
           dysrot   = ysrange / ( NROTATION - 1 ), &
           dysshear = ysrange / ( NSHEAR - 1 ), &
           zsmin    = zmin + 0.1 * zrange, &
           zsmax    = zmax - 0.1 * zrange, &
           zsrange  = zsmax - zsmin, &
           dzsrot   = zsrange / ( NROTATION - 1 ), &
           dzsshear = zsrange / ( NSHEAR - 1 )
    real(PLFLT) :: ys, zs, &
           x_inclination, y_inclination, z_inclination,       &
           x_shear, y_shear, z_shear,                         &
           omega, sin_omega, cos_omega, domega
    integer     :: i, j
    integer :: plparseopts_rc
    real(PLFLT) :: radius, pitch, xpos, ypos, zpos

    ! N.B. Need to append PL_END_OF_STRING so spaces get
    ! properly plotted.
    character(len=2)  :: p1string = "O"//PL_END_OF_STRING
    character(len=80) :: pstring  = &
           "The future of our civilization depends on software freedom."

    ! Allocate and define the minimal x, y, and z to insure 3D box
    allocate( x(XPTS) )
    allocate( y(YPTS) )
    allocate( z(XPTS,YPTS) )

    do i = 1,XPTS
        x(i) = xmin + (i-1) * (xmax-xmin)/(XPTS-1)
    enddo

    do j = 1,YPTS
        y(j) = ymin + (j-1) * (ymax-ymin)/(YPTS-1)
    enddo

    z = 0.0

    ! Parse and process command line arguments

    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    call plinit

    ! Page 1: Demonstrate inclination and shear capability pattern.

    call pladv(0)
    call plvpor(-0.15_pl_test_flt, 1.15_pl_test_flt, -0.05_pl_test_flt, 1.05_pl_test_flt)
    call plwind(-1.2_pl_test_flt, 1.2_pl_test_flt, -0.8_pl_test_flt, 1.5_pl_test_flt)
    call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, &
           xmin, xmax, ymin, ymax, zmin, zmax, 20._pl_test_flt, 45._pl_test_flt)

    call plcol0(2)
    call plbox3("b",   "", xmax-xmin, 0, &
           "b",   "", ymax-ymin, 0, &
           "bcd", "", zmax-zmin, 0)

    ! z = zmin.
    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    do i = 1,NREVOLUTION
        omega = 2.*PI*((i-1)/real(NREVOLUTION,kind=pl_test_flt))
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        x_inclination = 0.5*xrange*cos_omega
        y_inclination = 0.5*yrange*sin_omega
        z_inclination = 0.
        x_shear = -0.5*xrange*sin_omega
        y_shear = 0.5*yrange*cos_omega
        z_shear = 0.
        call plptex3( &
               xmid, ymid, zmin, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.0_pl_test_flt, "  revolution")
    enddo

    ! x = xmax.
    call plschr(0._pl_test_flt, 1.0_pl_test_flt)
    do i = 1,NREVOLUTION
        omega = 2.*PI*((i-1)/real(NREVOLUTION,kind=pl_test_flt))
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        x_inclination = 0.
        y_inclination = -0.5*yrange*cos_omega
        z_inclination = 0.5*zrange*sin_omega
        x_shear = 0.
        y_shear = 0.5*yrange*sin_omega
        z_shear = 0.5*zrange*cos_omega
        call plptex3( &
               xmax, ymid, zmid, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.0_pl_test_flt, "  revolution")
    enddo

    ! y = ymax.
    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    do i = 1,NREVOLUTION
        omega = 2.*PI*(i-1)/real(NREVOLUTION,kind=pl_test_flt)
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        x_inclination = 0.5*xrange*cos_omega
        y_inclination = 0.
        z_inclination = 0.5*zrange*sin_omega
        x_shear = -0.5*xrange*sin_omega
        y_shear = 0.
        z_shear = 0.5*zrange*cos_omega
        call plptex3( &
               xmid, ymax, zmid, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.0_pl_test_flt, "  revolution")
    enddo

    ! Draw minimal 3D grid to finish defining the 3D box.
    call plmesh(x, y, z, DRAW_LINEXY)

    ! Page 2: Demonstrate rotation of string around its axis.
    call pladv(0)
    call plvpor(-0.15_pl_test_flt, 1.15_pl_test_flt, -0.05_pl_test_flt, 1.05_pl_test_flt)
    call plwind(-1.2_pl_test_flt, 1.2_pl_test_flt, -0.8_pl_test_flt, 1.5_pl_test_flt)
    call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, xmin, xmax, ymin, ymax, &
           zmin, zmax, 20._pl_test_flt, 45._pl_test_flt)

    call plcol0(2)
    call plbox3("b",   "", xmax-xmin, 0, &
           "b",   "", ymax-ymin, 0, &
           "bcd", "", zmax-zmin, 0)

    ! y = ymax.
    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    x_inclination = 1.
    y_inclination = 0.
    z_inclination = 0.
    x_shear = 0.
    do i = 1,NROTATION
        omega = 2.*PI*(i-1)/real(NROTATION,kind=pl_test_flt)
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        y_shear = 0.5*yrange*sin_omega
        z_shear = 0.5*zrange*cos_omega
        zs        = zsmax - dzsrot * real(i-1,kind=pl_test_flt)
        call plptex3( &
               xmid, ymax, zs, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.5_pl_test_flt, "rotation for y = y#dmax#u")
    enddo

    ! x = xmax.
    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    x_inclination = 0.0
    y_inclination = -1.0
    z_inclination = 0.0
    y_shear = 0.0
    do i = 1,NROTATION
        omega = 2.*PI*((i-1)/real(NROTATION,kind=pl_test_flt))
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        x_shear = 0.5*xrange*sin_omega
        z_shear = 0.5*zrange*cos_omega
        zs        = zsmax - dzsrot * real(i-1,kind=pl_test_flt)
        call plptex3( &
               xmax, ymid, zs, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.5_pl_test_flt, "rotation for x = x#dmax#u")
    enddo

    ! z = zmin.
    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    x_inclination = 1.
    y_inclination = 0.
    z_inclination = 0.
    x_shear = 0.
    do i = 1,NROTATION
        omega = 2.*PI*((i-1)/real(NROTATION,kind=pl_test_flt))
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        y_shear = 0.5*yrange*cos_omega
        z_shear = 0.5*zrange*sin_omega
        ys        = ysmax - dysrot * real(i-1,kind=pl_test_flt)
        call plptex3( &
               xmid, ys, zmin, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.5_pl_test_flt, "rotation for z = z#dmin#u")
    enddo
    ! Draw minimal 3D grid to finish defining the 3D box.
    call plmesh(x, y, z, DRAW_LINEXY)

    ! Page 3: Demonstrate shear of string along its axis.
    ! Work around xcairo and pngcairo (but not pscairo) problems for
    ! shear vector too close to axis of string. (N.B. no workaround
    ! would be domega = 0.)
    domega = 0.05
    call pladv(0)
    call plvpor(-0.15_pl_test_flt, 1.15_pl_test_flt, -0.05_pl_test_flt, 1.05_pl_test_flt)
    call plwind(-1.2_pl_test_flt, 1.2_pl_test_flt, -0.8_pl_test_flt, 1.5_pl_test_flt)
    call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, xmin, xmax, ymin, ymax, &
           zmin, zmax, 20._pl_test_flt, 45._pl_test_flt)

    call plcol0(2)
    call plbox3("b",   "", xmax-xmin, 0, &
           "b",   "", ymax-ymin, 0, &
           "bcd", "", zmax-zmin, 0)

    ! y = ymax.
    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    x_inclination = 1.
    y_inclination = 0.
    z_inclination = 0.
    y_shear = 0.
    do i = 1,NSHEAR
        omega = domega + 2.*PI*((i-1)/real(NSHEAR,kind=pl_test_flt))
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        x_shear = 0.5*xrange*sin_omega
        z_shear = 0.5*zrange*cos_omega
        zs        = zsmax - dzsshear * real(i-1,kind=pl_test_flt)
        call plptex3( &
               xmid, ymax, zs, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.5_pl_test_flt, "shear for y = y#dmax#u")
    enddo

    ! x = xmax.
    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    x_inclination = 0.
    y_inclination = -1.
    z_inclination = 0.
    x_shear = 0.
    do i = 1,NSHEAR
        omega = domega + 2.*PI*((i-1)/real(NSHEAR,kind=pl_test_flt))
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        y_shear = -0.5*yrange*sin_omega
        z_shear = 0.5*zrange*cos_omega
        zs        = zsmax - dzsshear * real(i-1,kind=pl_test_flt)
        call plptex3( &
               xmax, ymid, zs, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.5_pl_test_flt, "shear for x = x#dmax#u")
    enddo

    ! z = zmin.
    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    x_inclination = 1.
    y_inclination = 0.
    z_inclination = 0.
    z_shear = 0.
    do i = 1,NSHEAR
        omega = domega + 2.*PI*((i-1)/real(NSHEAR,kind=pl_test_flt))
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        y_shear = 0.5*yrange*cos_omega
        x_shear = 0.5*xrange*sin_omega
        ys        = ysmax - dysshear * real(i-1,kind=pl_test_flt)
        call plptex3( &
               xmid, ys, zmin, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.5_pl_test_flt, "shear for z = z#dmin#u")
    enddo
    ! Draw minimal 3D grid to finish defining the 3D box.
    call plmesh(x, y, z, DRAW_LINEXY)

    ! Page 4: Demonstrate drawing a string on a 3D path.
    call pladv(0)
    call plvpor(-0.15_pl_test_flt, 1.15_pl_test_flt, -0.05_pl_test_flt, 1.05_pl_test_flt)
    call plwind(-1.2_pl_test_flt, 1.2_pl_test_flt, -0.8_pl_test_flt, 1.5_pl_test_flt)
    call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, &
           xmin, xmax, ymin, ymax, zmin, zmax, 40._pl_test_flt, -30._pl_test_flt)

    call plcol0(2)
    call plbox3("b",   "", xmax-xmin, 0, &
           "b",   "", ymax-ymin, 0, &
           "bcd", "", zmax-zmin, 0)

    call plschr(0.0_pl_test_flt, 1.2_pl_test_flt)
    ! domega controls the spacing between the various characters of the
    ! string and also the maximum value of omega for the given number
    ! of characters in pstring.
    domega = 2.*PI/len_trim(pstring)
    omega = 0.
    ! 3D function is a helix of the given radius and pitch
    radius = 0.5
    pitch = 1./(2.*PI)
    do i = 1,len_trim(pstring)
        sin_omega = sin(omega)
        cos_omega = cos(omega)
        xpos = xmid + radius*sin_omega
        ypos = ymid - radius*cos_omega
        zpos = zmin + pitch*omega
        ! In general, the inclination is proportional to the derivative of
        ! the position wrt theta.
        x_inclination = radius*cos_omega
        y_inclination = radius*sin_omega
        z_inclination = pitch
        ! The shear vector should be perpendicular to the 3D line with Z
        ! component maximized, but for low pitch a good approximation is
        ! a constant vector that is parallel to the Z axis.
        x_shear = 0.
        y_shear = 0.
        z_shear = 1.
        p1string(1:1) = pstring(i:i)
        call plptex3( &
               xpos, ypos, zpos, &
               x_inclination, y_inclination, z_inclination, &
               x_shear, y_shear, z_shear, &
               0.5_pl_test_flt, p1string)
        omega = omega + domega
    enddo
    ! Draw minimal 3D grid to finish defining the 3D box.
    call plmesh(x, y, z, DRAW_LINEXY)

    ! Page 5: Demonstrate plmtex3 axis labelling capability
    call pladv(0)
    call plvpor(-0.15_pl_test_flt, 1.15_pl_test_flt, -0.05_pl_test_flt, 1.05_pl_test_flt)
    call plwind(-1.2_pl_test_flt, 1.2_pl_test_flt, -0.8_pl_test_flt, 1.5_pl_test_flt)
    call plw3d(1.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt, &
           xmin, xmax, ymin, ymax, zmin, zmax, 20._pl_test_flt, 45._pl_test_flt)

    call plcol0(2)
    call plbox3("b",   "", xmax-xmin, 0, &
           "b",   "", ymax-ymin, 0, &
           "bcd", "", zmax-zmin, 0)

    call plschr(0.0_pl_test_flt, 1.0_pl_test_flt)
    call plmtex3("xp", 3.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "Arbitrarily displaced")
    call plmtex3("xp", 4.5_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "primary X-axis label")
    call plmtex3("xs", -2.5_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "Arbitrarily displaced")
    call plmtex3("xs", -1.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "secondary X-axis label")
    call plmtex3("yp", 3.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "Arbitrarily displaced")
    call plmtex3("yp", 4.5_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "primary Y-axis label")
    call plmtex3("ys", -2.5_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "Arbitrarily displaced")
    call plmtex3("ys", -1.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "secondary Y-axis label")
    call plmtex3("zp", 4.5_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "Arbitrarily displaced")
    call plmtex3("zp", 3.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "primary Z-axis label")
    call plmtex3("zs", -2.5_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "Arbitrarily displaced")
    call plmtex3("zs", -1.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, "secondary Z-axis label")
    ! Draw minimal 3D grid to finish defining the 3D box.
    call plmesh(x, y, z, DRAW_LINEXY)

    ! Clean up.
    deallocate( x, y, z )
    call plend
    stop
end program x28f
