!      set / get tester.
!
!      Copyright (C) 2008-2016  Alan W. Irwin
!      Copyright (C) 2008  Andrew Ross
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
!--------------------------------------------------------------------------
! main
!
! Demonstrates absolute positioning of graphs on a page.
!--------------------------------------------------------------------------

program x31f
    use plplot, double_PL_NOTSET => PL_NOTSET

    implicit none
    real(kind=pl_test_flt), parameter :: NOTSET = double_PL_NOTSET

    real(kind=pl_test_flt) xmin0, xmax0, ymin0, ymax0, zxmin0, zxmax0, zymin0, zymax0
    real(kind=pl_test_flt) xmin, xmax, ymin, ymax, zxmin, zxmax, zymin, zymax
    real(kind=pl_test_flt) xmid, ymid, wx, wy
    real(kind=pl_test_flt) mar0, aspect0, jx0, jy0, ori0
    real(kind=pl_test_flt) mar, aspect, jx, jy, ori
    integer win, level2, digmax, digits, compression1, compression2
    real(kind=pl_test_flt) xp0, yp0, xp1, yp1, xp2, yp2
    integer xleng0, yleng0, xoff0, yoff0, xleng1, yleng1, xoff1, yoff1
    integer xleng2, yleng2, xoff2, yoff2
    integer fam0, num0, bmax0, fam1, num1, bmax1, fam2, num2, bmax2
    integer r0, g0, b0
    real(kind=pl_test_flt) a0
    integer r, g, b
    real(kind=pl_test_flt) a
    integer r1(2), g1(2), b1(2)
    data r1 /0, 255/
    data g1 /255, 0/
    data b1 /0, 0/
    real(kind=pl_test_flt) a1(2)
    data a1 /1.0_pl_test_flt, 1.0_pl_test_flt/
    character(len=256) fnam
    integer stderr
    integer status
    integer :: plparseopts_rc

    status = 0
    stderr = 0

    !     Parse and process command line arguments

    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !     Test setting / getting familying parameters before plinit
    call plgfam(fam0, num0, bmax0)
    fam1 = 0
    num1 = 10
    bmax1 = 1000
    call plsfam(fam1, num1, bmax1)

    !     Retrieve the same values?
    call plgfam(fam2, num2, bmax2)
    write(*,'(A,I1,I3,I5)') 'family parameters: fam, num, bmax = ', &
           fam2, num2, bmax2
    if (fam2 .ne. fam1 .or. num2 .ne. num1 .or. bmax2 .ne. bmax1) then
        write(stderr,*) 'plgfam test failed\n'
        status = 1
    endif
    !     Restore values set initially by plparseopts.
    call plsfam(fam0, num0, bmax0)

    !     Test setting / getting page parameters before plinit
    !     Save values set by plparseopts to be restored later.
    call plgpage(xp0, yp0, xleng0, yleng0, xoff0, yoff0)
    xp1 = 200._pl_test_flt
    yp1 = 200._pl_test_flt
    xleng1 = 400
    yleng1 = 200
    xoff1 = 10
    yoff1 = 20
    call plspage(xp1, yp1, xleng1, yleng1, xoff1, yoff1)

    !     Retrieve the same values?
    call plgpage(xp2, yp2, xleng2, yleng2, xoff2, yoff2)
    write(*,'(A,2F11.6, 2I4, 2I3)')  &
           'page parameters: xp, yp, xleng, yleng, xoff, yoff =', &
           xp2, yp2, xleng2, yleng2, xoff2, yoff2
    if (xp2 .ne. xp1 .or. yp2 .ne. yp1 .or. xleng2 .ne. xleng1 .or. &
           yleng2 .ne. yleng1 .or. xoff2 .ne. xoff1 .or.  &
           yoff2 .ne. yoff1 ) then
        write(stderr,*) 'plgpage test failed'
        status = 1
    endif
    !     Restore values set initially by plparseopts.
    call plspage(xp0, yp0, xleng0, yleng0, xoff0, yoff0)

    !     Test setting / getting compression parameter across plinit
    compression1 = 95
    call plscompression(compression1)

    !     Initialize plplot

    call plinit()

    !     Test if device initialization screwed around with the preset
    !     compression parameter.
    call plgcompression(compression2)
    write(*,'(A)') 'Output various PLplot parameters'
    write(*,'(A,I2)') 'compression parameter = ', compression2
    if (compression2 .ne. compression1) then
        write(stderr,*) 'plgcompression test failed'
        status = 1
    endif

    !     Exercise plscolor, plscol0, plscmap1, and plscmap1a to make sure
    !     they work without any obvious error messages.
    call plscolor(1)
    call plscol0(1, 255, 0, 0)
    call plscmap1(r1,g1,b1)
    call plscmap1a(r1,g1,b1,a1)

    call plglevel(level2)
    write(*,'(A,I1)') 'level parameter = ', level2
    if (level2 .ne. 1) then
        write(stderr,*) 'plglevel test failed.'
        status = 1
    endif

    call pladv(0)
    xmin0 = 0.01_pl_test_flt
    xmax0 = 0.99_pl_test_flt
    ymin0 = 0.02_pl_test_flt
    ymax0 = 0.49_pl_test_flt
    call plvpor(xmin0, xmax0, ymin0, ymax0)
    call plgvpd(xmin, xmax, ymin, ymax)
    write(*,'(A,4F9.6)') 'plvpor: xmin, xmax, ymin, ymax =',  &
           xmin, xmax, ymin, ymax
    if (xmin .ne. xmin0 .or. xmax .ne. xmax0 .or. ymin .ne. ymin0 .or. ymax .ne. ymax0) then
        write(stderr,*) 'plgvpd test failed'
        status = 1
    endif
    xmid = 0.5*(xmin+xmax)
    ymid = 0.5*(ymin+ymax)

    xmin0 = 0.2_pl_test_flt
    xmax0 = 0.3_pl_test_flt
    ymin0 = 0.4_pl_test_flt
    ymax0 = 0.5_pl_test_flt
    call plwind(xmin0, xmax0, ymin0, ymax0)
    call plgvpw(xmin, xmax, ymin, ymax)
    write(*,'(A,4F9.6)') 'plwind: xmin, xmax, ymin, ymax =', &
           xmin, xmax, ymin, ymax
    if (xmin .ne. xmin0 .or. xmax .ne. xmax0 .or. ymin .ne. ymin0 .or. ymax .ne. ymax0) then
        write(stderr,*) 'plgvpw test failed',xmin,xmax,ymin,ymax
        status = 1
    endif

    !     Get world coordinates for midpoint of viewport
    call plcalc_world(xmid,ymid,wx,wy,win)
    write(*,'(A,2F9.6,I2)') 'world parameters: wx, wy, win =',  &
           wx, wy, win
    if (abs(wx-0.25_pl_test_flt).gt.1.0d-5 .or. abs(wy-0.45_pl_test_flt).gt.1.0d-5) then
        write(stderr,*) 'plcalc_world test failed'
        status = 1
    endif

    !     Retrieve and print the name of the output file (if any)
    call plgfnam(fnam)
    if (len(trim(fnam)) .eq. 0) then
        write(*,'(A)') 'No output file name is set'
    else
        write(*,'(A)') 'Output file name read'
    endif
    write(stderr,'(A,A)') 'Output file name is ',trim(fnam)

    !     Set and get the number of digits used to display axis labels
    !     Note digits is currently ignored in pls[xyz]ax and
    !     therefore it does not make sense to test the returned
    !     value
    call plsxax(3,0)
    call plgxax(digmax,digits)
    write(*,'(A,I2,I2)') 'x axis parameters: digmax, digits =', &
           digmax, digits
    if (digmax .ne. 3) then
        write(stderr,*) 'plgxax test failed'
        status = 1
    endif

    call plsyax(4,0)
    call plgyax(digmax,digits)
    write(*,'(A,I2,I2)') 'y axis parameters: digmax, digits =', &
           digmax, digits
    if (digmax .ne. 4) then
        write(stderr,*) 'plgyax test failed'
        status = 1
    endif

    call plszax(5,0)
    call plgzax(digmax,digits)
    write(*,'(A,I2,I2)') 'z axis parameters: digmax, digits =', &
           digmax, digits
    if (digmax .ne. 5) then
        write(stderr,*) 'plgzax test failed'
        status = 1
    endif

    mar0 = 0.05_pl_test_flt
    aspect0 = NOTSET
    jx0 = 0.1_pl_test_flt
    jy0 = 0.2_pl_test_flt
    call plsdidev(mar0, aspect0, jx0, jy0)
    call plgdidev(mar, aspect, jx, jy)
    write(*,'(A,4F9.6)') 'device-space window parameters: '// &
           'mar, aspect, jx, jy =', mar, aspect, jx, jy
    if (mar .ne. mar0 .or. jx .ne. jx0 .or. jy .ne. jy0) then
        write(stderr,*) 'plgdidev test failed'
        status = 1
    endif

    ori0 = 1.0_pl_test_flt
    call plsdiori(ori0)
    call plgdiori(ori)
    write(*,'(A,F9.6)') 'ori parameter =', ori
    if (ori .ne. ori0) then
        write(stderr,*) 'plgdiori test failed'
        status = 1
    endif

    xmin0 = 0.1_pl_test_flt
    ymin0 = 0.2_pl_test_flt
    xmax0 = 0.9_pl_test_flt
    ymax0 = 0.8_pl_test_flt
    call plsdiplt(xmin0, ymin0, xmax0, ymax0)
    call plgdiplt(xmin, ymin, xmax, ymax)
    write(*,'(A,4F9.6)') 'plot-space window parameters: '// &
           'xmin, ymin, xmax, ymax =', xmin, ymin, xmax, ymax
    if (xmin .ne. xmin0 .or. ymin .ne. ymin0 .or. xmax .ne. xmax0 .or. ymax .ne. ymax0) then
        write(stderr,*) 'plgdiplt test failed'
        status = 1
    endif

    zxmin0 = 0.1_pl_test_flt
    zymin0 = 0.1_pl_test_flt
    zxmax0 = 0.9_pl_test_flt
    zymax0 = 0.9_pl_test_flt
    call plsdiplz(zxmin0, zymin0, zxmax0, zymax0)
    call plgdiplt(zxmin, zymin, zxmax, zymax)
    write(*,'(A,4F9.6)') 'zoomed plot-space window parameters: '// &
           'xmin, ymin, xmax, ymax =', zxmin, zymin, zxmax, zymax
    if ( abs(zxmin -(xmin + (xmax-xmin)*zxmin0)) .gt. 1.0d-5 .or. &
           abs(zymin -(ymin+(ymax-ymin)*zymin0)) .gt. 1.0d-5 .or. &
           abs(zxmax -(xmin+(xmax-xmin)*zxmax0)) .gt. 1.0d-5 .or. &
           abs(zymax -(ymin+(ymax-ymin)*zymax0)) .gt. 1.0d-5 ) then
        write(stderr,*) 'plsdiplz test failed'
        status = 1
    endif

    r0 = 10
    g0 = 20
    b0 = 30
    call plscolbg(r0, g0, b0)
    call plgcolbg(r, g, b)
    write(*,'(A,3I3)') 'background colour parameters: r, g, b =', r, g, b
    if (r .ne. r0 .or. g .ne. g0 .or. b .ne. b0) then
        write(stderr,*) 'plgcolbg test failed'
        status = 1
    endif

    r0 = 20
    g0 = 30
    b0 = 40
    a0 = 0.5_pl_test_flt
    call plscolbga(r0, g0, b0, a0)
    call plgcolbga(r, g, b, a)
    write(*,'(A,3I3,F9.6)') 'background/transparency colour '// &
           'parameters: r, g, b, a =', r, g, b, a
    if (r .ne. r0 .or. g .ne. g0 .or. b .ne. b0 .or. a .ne. a0) then
        write(stderr,*) 'plgcolbga test failed'
        status = 1
    endif

    call plend()
    if (status.ne.0) then
        stop 'Error occured'
    endif
    stop

end program x31f
