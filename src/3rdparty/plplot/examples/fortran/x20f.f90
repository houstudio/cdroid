!      Copyright (C) 2004-2016  Alan W. Irwin
!      Copyright (C) 2008  Andrew Ross
!
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
!
!      plimage demo
!
!

!static PLOptionTable options[] = {
!{
!    "dbg",			/* extra debugging plot */
!    NULL,
!    NULL,
!    &dbg,
!    PL_OPT_BOOL,
!    "-dbg",
!    "Extra debugging plot" },
!{
!    "nosombrero",			/* Turns on test of xor function */
!    NULL,
!    NULL,
!    &nosombrero,
!    PL_OPT_BOOL,
!    "-nosombrero",
!    "No sombrero plot" },
!{
!    "nointeractive",			/* Turns on test of xor function */
!    NULL,
!    NULL,
!    &nointeractive,
!    PL_OPT_BOOL,
!    "-nointeractive",
!    "No interactive selection" },
!{
!    "save",			/* For saving in postscript */
!    NULL,
!    NULL,
!    &f_name,
!    PL_OPT_STRING,
!    "-save filename",
!      "Save sombrero plot in color postscript `filename'" },
!{
!    NULL,			/* option */
!    NULL,			/* handler */
!    NULL,			/* client data */
!    NULL,			/* address of variable to set */
!    0,				/* mode flag */
!    NULL,			/* short syntax */
!    NULL }			/* long syntax */
!};

program x20f
    use plplot, double_PI => PL_PI
    use iso_c_binding, only: c_ptr, c_loc, c_f_pointer

    implicit none

    real(kind=pl_test_flt), parameter :: PI = double_PI
    integer, parameter          :: XDIM = 260, YDIM = 220
    real(kind=pl_test_flt), parameter :: XDIMR = XDIM, YDIMR = YDIM

    real(kind=pl_test_flt) ::  x(XDIM), y(YDIM), z(XDIM,YDIM), r(XDIM,YDIM)
    real(kind=pl_test_flt) ::  xi, yi, xe, ye
    integer i, j
    real(kind=pl_test_flt)  width_r, height_r

    !
    !     Dimensions taken from "Chloe.pgm"
    !
    integer width, height, num_col
    integer :: plparseopts_rc
    real(kind=pl_test_flt), dimension(:,:), pointer :: img_f
    real(kind=pl_test_flt), dimension(:,:), pointer :: xg, yg
    real(kind=pl_test_flt), dimension(:), pointer :: xg1, yg1
    real(kind=pl_test_flt) :: img_max, img_min

    real(kind=pl_test_flt) :: x0, y0, dy, stretch

    !
    !     Parameters from command-line
    !
    logical dbg
    logical nosombrero
    logical nointeractive
    character(len=80) f_name
    integer :: plgetcursor_rc
    real(kind=pl_test_flt) tr(6)

    ! Global parameters to be used in mypltr callback
    real(kind=pl_test_flt) :: xmin, xmax, ymin, ymax

    ! Use no callback?
    logical, parameter :: no_callback = .false.

    ! Use tr callback? (Only meaningful if no_callback is false.)
    logical, parameter :: tr_callback = .false.

    ! Use Fortran callback with no data? (Only meaningful
    ! if no_callback and tr_callback are both .false.).
    logical, parameter :: mypltr_callback = .false.

    type mypltr_data_type
        ! Only contains data required by the mypltr_data callback
        integer :: xpts_data, ypts_data
        real(kind=pl_test_flt) :: xmin_data, xmax_data, ymin_data, ymax_data
    end type mypltr_data_type

    ! Global data type to be used in mypltr_data callback
    type(mypltr_data_type), target :: data

    !     Parse and process command line arguments
    !
    !      call plMergeOpts(options, 'x20c options', NULL)

    dbg            = .false.
    nosombrero     = .false.
    nointeractive  = .false.
    f_name         = ' '
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !     Initialize plplot

    call plinit

    !     View image border pixels
    if (dbg) then
        call plenv(1._pl_test_flt, XDIMR, 1._pl_test_flt, YDIMR, 1, 1)

        z = 0.0_pl_test_flt

        !         Build a one pixel square border, for diagnostics
        do i = 1,XDIM
            !             Right
            z(i,YDIM) = 1._pl_test_flt
            !             Left
            z(i,1)    = 1._pl_test_flt
        enddo

        do i = 1,YDIM
            !             Top
            z(1,i)    = 1._pl_test_flt
            !             Bottom
            z(XDIM,i) = 1._pl_test_flt
        enddo

        call pllab('...around a blue square.',' '//PL_END_OF_STRING, &
               'A red border should appear...')

        call plimage(z, 1._pl_test_flt, XDIMR, 1._pl_test_flt, YDIMR, 0._pl_test_flt, 0._pl_test_flt, &
               1._pl_test_flt, XDIMR, 1._pl_test_flt, YDIMR)
    endif

    !     Sombrero-like demo
    if (.not. nosombrero) then
        !         draw a yellow plot box, useful for diagnostics! :(
        call plcol0(2)
        call plenv(0._pl_test_flt, 2._pl_test_flt*PI, 0.0_pl_test_flt, 3._pl_test_flt*PI, 1, -1)

        do i=1,XDIM
            x(i) = real(i-1,kind=pl_test_flt)*2._pl_test_flt*PI/real(XDIM-1,kind=pl_test_flt)
        enddo
        do i=1,YDIM
            y(i) = real(i-1,kind=pl_test_flt)*3._pl_test_flt*PI/real(YDIM-1,kind=pl_test_flt)
        enddo

        do i=1,XDIM
            do j=1,YDIM
                r(i,j) = sqrt(x(i)*x(i)+y(j)*y(j))+0.001_pl_test_flt
                z(i,j) = sin(r(i,j)) / (r(i,j))
            enddo
        enddo

        call pllab('No, an amplitude clipped "sombrero"', '', &
               'Saturn?')
        call plptex(2._pl_test_flt, 2._pl_test_flt, 3._pl_test_flt, 4._pl_test_flt, 0._pl_test_flt, 'Transparent image')
        call plimage(z, 0._pl_test_flt, 2._pl_test_flt*PI, 0.0_pl_test_flt, 3._pl_test_flt*PI, &
               0.05_pl_test_flt, 1._pl_test_flt, 0._pl_test_flt, 2._pl_test_flt*PI, 0._pl_test_flt, 3._pl_test_flt*PI)

        !         Save the plot
        if (f_name .ne. ' ') then
            call save_plot(f_name)
        endif
    endif

    !
    !     Read Chloe image
    !     Note we try two different locations to cover the case where this
    !     example is being run from the test_c.sh script
    !
    if (.not. read_img('Chloe.pgm', img_f, width, height, num_col)) then
        if (.not. read_img('../Chloe.pgm', img_f, width, height, num_col)) then
            !C            call plabort('No such file')
            write(*,*) 'Image could not be read'
            call plend()
            stop
        endif
    endif

    width_r  = real(width,kind=pl_test_flt)
    height_r = real(height,kind=pl_test_flt)

    ! Initialize callback data
    xmin = 0.0_pl_test_flt
    xmax = width_r
    ymin = 0.0_pl_test_flt
    ymax = height_r

    data%xpts_data = width
    data%ypts_data = height
    data%xmin_data = xmin
    data%xmax_data = xmax
    data%ymin_data = ymin
    data%ymax_data = ymax

    tr = (/ (xmax-xmin)/real(width-1,kind=pl_test_flt), 0.0_pl_test_flt, xmin, &
           0.0_pl_test_flt, (ymax-ymin)/real(height-1,kind=pl_test_flt), ymin /)

    !     Set gray colormap
    call gray_cmap(num_col)

    !     Display Chloe
    call plenv(1._pl_test_flt, width_r, 1._pl_test_flt, height_r, 1, -1)

    if (.not. nointeractive) then
        call pllab('Set and drag Button 1 to (re)set selection, Butto'// &
               'n 2 to finish.',' '//PL_END_OF_STRING,'Chloe...')
    else
        call pllab('',' '//PL_END_OF_STRING,'Chloe...')
    endif

    call plimage(img_f, 1._pl_test_flt, width_r, 1._pl_test_flt, &
           height_r, 0._pl_test_flt, 0._pl_test_flt, 1._pl_test_flt, width_r, 1._pl_test_flt, height_r)

    !     Selection/expansion demo
    if (.not. nointeractive) then
        xi = 25.0_pl_test_flt
        xe = 130.0_pl_test_flt
        yi = 235.0_pl_test_flt
        ye = 125.0_pl_test_flt

        if (get_clip(xi, xe, yi, ye)) then
            call plend()
            stop
        endif

        !
        !         I'm unable to continue, clearing the plot and advancing to the next
        !         one, without hiting the enter key, or pressing the button... help!
        !         Forcing the xwin driver to leave locate mode and destroying the
        !         xhairs (in GetCursorCmd()) solves some problems, but I still have
        !         to press the enter key or press Button-2 to go to next plot, even
        !         if a pladv() is not present!  Using plbop() solves the problem, but
        !         it shouldn't be needed!
        !
        !         plspause(0), pladv(0), plspause(1), also works,
        !         but the above question remains.
        !         With this approach, the previous pause state is lost,
        !         as there is no API call to get its current state.
        !

        call plspause(.false.)
        call pladv(0)

        !         Display selection only
        call plimage(img_f, 1._pl_test_flt, width_r, 1._pl_test_flt, &
               height_r, 0._pl_test_flt, 0._pl_test_flt, xi, xe, ye, yi)

        call plspause(.true.)

        !         Zoom in selection
        call plenv(xi, xe, ye, yi, 1, -1)
        call plimage(img_f, 1._pl_test_flt, width_r, 1._pl_test_flt, &
               height_r, 0._pl_test_flt, 0._pl_test_flt, xi, xe, ye, yi)
    endif

    !     Base the dynamic range on the image contents.

    call a2mnmx(img_f,width,height,img_min,img_max,width)

    call plcol0(2)
    call plenv(0._pl_test_flt, width_r, 0._pl_test_flt, height_r, 1, -1)
    call pllab("", "", "Reduced dynamic range image example")
    if(no_callback) then
        call plimagefr(img_f, 0._pl_test_flt, width_r, 0._pl_test_flt, &
               height_r, 0._pl_test_flt, 0._pl_test_flt, img_min + img_max * 0.25_pl_test_flt, &
               img_max - img_max * 0.25_pl_test_flt)
    elseif(tr_callback) then
        call plimagefr(img_f, 0._pl_test_flt, width_r, 0._pl_test_flt, &
               height_r, 0._pl_test_flt, 0._pl_test_flt, img_min + img_max * 0.25_pl_test_flt, &
               img_max - img_max * 0.25_pl_test_flt, tr)
    elseif(mypltr_callback) then
        call plimagefr(img_f, 0._pl_test_flt, width_r, 0._pl_test_flt, &
               height_r, 0._pl_test_flt, 0._pl_test_flt, img_min + img_max * 0.25_pl_test_flt, &
               img_max - img_max * 0.25_pl_test_flt, mypltr)
    else
        call plimagefr(img_f, 0._pl_test_flt, width_r, 0._pl_test_flt, &
               height_r, 0._pl_test_flt, 0._pl_test_flt, img_min + img_max * 0.25_pl_test_flt, &
               img_max - img_max * 0.25_pl_test_flt, mypltr_data, c_loc(data))
    endif

    !     Draw a distorted version of the original image, showing its
    !     full dynamic range.
    call plenv(0._pl_test_flt, width_r, 0._pl_test_flt, height_r, 1, -1)
    call pllab("", "", "Distorted image example")

    !     Populate the 2-d grids used for the distortion
    !     NB grids must be 1 larger in each dimension than the image
    !     since the coordinates are for the corner of each pixel.
    allocate( xg(width+1,height+1) )
    allocate( yg(width+1,height+1) )
    x0 = 0.5_pl_test_flt*width_r
    y0 = 0.5_pl_test_flt*height_r
    dy = 0.5_pl_test_flt*height_r
    stretch = 0.5_pl_test_flt
    do i=1,width+1
        do j=1,height+1
            xg(i,j) = x0 + (x0-real(i-1,kind=pl_test_flt))*(1.0_pl_test_flt - stretch* &
                   cos((real(j-1,kind=pl_test_flt)-y0)/dy*PI*0.5_pl_test_flt))
            yg(i,j) = real(j-1,kind=pl_test_flt)
        enddo
    enddo
    call plimagefr(img_f, 0._pl_test_flt, width_r, 0._pl_test_flt, &
           height_r, 0._pl_test_flt, 0._pl_test_flt, img_min, img_max, xg, yg)

    if(.false.) then
        ! Two test pages to compare undistorted results plotted with
        ! doubly dimensioned xg, yg, versus singly dimensioned xg1, yg1
        call plenv(0._pl_test_flt, width_r, 0._pl_test_flt, height_r, 1, -1)
        call pllab("", "", "Undistorted image example")

        do i=1,width+1
            do j=1,height+1
                xg(i,j) = real(i-1,kind=pl_test_flt)
                yg(i,j) = real(j-1,kind=pl_test_flt)
            enddo
        enddo
        call plimagefr(img_f, 0._pl_test_flt, width_r, 0._pl_test_flt, &
               height_r, 0._pl_test_flt, 0._pl_test_flt, img_min, img_max, xg, yg)

        call plenv(0._pl_test_flt, width_r, 0._pl_test_flt, height_r, 1, -1)
        call pllab("", "", "Undistorted image example")

        allocate( xg1(width+1) )
        allocate( yg1(height+1) )
        do i=1,width+1
            xg1(i) = real(i-1,kind=pl_test_flt)
        enddo
        do j=1,height+1
            yg1(j) = real(j-1,kind=pl_test_flt)
        enddo

        call plimagefr(img_f, 0._pl_test_flt, width_r, 0._pl_test_flt, &
               height_r, 0._pl_test_flt, 0._pl_test_flt, img_min, img_max, xg1, yg1)

        deallocate(xg1, yg1)
    end if

    deallocate( img_f, xg, yg )

    call plend()
    stop

contains

    ! Callback function that relies on global width, height, xmin, xmax, ymin, ymax
    subroutine mypltr( x, y, xt, yt )

        ! These callback arguments must have exactly these attributes.
        real(kind=pl_test_flt), intent(in) ::  x, y
        real(kind=pl_test_flt), intent(out) :: xt, yt

        xt = xmin + ((xmax-xmin)/real(width-1,kind=pl_test_flt))*x
        yt = ymin + ((ymax-ymin)/real(height-1,kind=pl_test_flt))*y

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

!     Determine the unit of length (of 8 or less) for direct-access files
    subroutine bytes_in_rec( bytes )
        implicit none
        integer     bytes

        character(len=8) string
        integer     ierr

        open( 10, file = '_x20f_.bin', access = 'direct', recl = 1 )

        string = "12345678"
        bytes = 0
        ierr = 0
        do while(ierr == 0 .and. bytes < 8)
            bytes = bytes + 1
            write( 10, rec = 1, iostat = ierr ) string(1:bytes)
        enddo
        if(ierr /= 0) bytes = bytes -1

        close( 10, status = 'delete' )

    end subroutine bytes_in_rec


    !     Read image from file in binary ppm format
    logical function read_img(fname, img_f, width, height, num_col)

        character(*), intent(in)                  :: fname
        integer, intent(out)                      :: width, height
        real(kind=pl_test_flt), dimension(:,:), pointer :: img_f
        integer      num_col

        character, dimension(8) :: img
        character(len=80), dimension(2) :: ver
        integer ::  i, j, w, h, b

        integer :: ierr
        integer ::  count
        integer ::  record

        integer ::  bytes = 0
        integer ::  lastlf = 0
        integer ::  first
        integer ::  last
        integer ::  pixel

        !     Naive grayscale binary ppm reading. If you know how to, improve it

        open( 10, file = fname, status = 'old', iostat = ierr )

        if (ierr .ne. 0 ) then
            read_img = .false.
            return
        endif

        !
        !     Read the first lines (count them for later re-reading)
        !
        count = 1
        read( 10, '(a)', iostat = ierr ) ver(1)

        !     I only understand "P5"!
        if (ver(1) .ne. 'P5' .or. ierr .ne. 0) then
            read_img = .false.
            return
        endif

        do
            count = count + 1
            read( 10, '(a)', iostat = ierr ) ver(1)

            if (ierr .ne. 0) then
                read_img = .false.
                write(*,*) 'Error!'
                return
            endif

            if (ver(1)(1:1) .ne. '#' ) then
                exit
            endif
        enddo

        !     Found the line with the sizes, copy this one and the next

        count = count + 1
        read( 10, '(a)', iostat = ierr ) ver(2)

        read( ver(1), * ) w, h
        read( ver(2), * ) num_col

        allocate( img_f(w,h) )

        close( 10 )

        !
        !     Read the second part - we need to do it the hard way :(
        !
        !     Note:
        !     The algorithm works if the unit of record length is a byte or
        !     if it is a multiple therefore (up to 8 bytes are allowed).
        !     Background: Some compilers use a word (4 bytes) as the unit of
        !     length instead, but often provide a compile switch to use bytes,
        !     we can not rely on this though.
        !
        call bytes_in_rec( bytes )

        open( 10, file = fname, access = 'direct', recl = 1 )

        record = 0
        do while ( count > 0 )
            !
            !     Look for the end of the line with sizes
            !
            record = record + 1
            read( 10, rec = record, iostat = ierr ) (img(i), i = 1,bytes )
            if ( ierr .ne. 0 ) then
                exit
            endif

            do i = 1,bytes
                if ( img(i) == char(10) ) then
                    count  = count - 1
                    lastlf = i
                endif
            enddo
        enddo

        !
        !     We have found the picture bytes!
        !     The picture needs to be flipped vertically.
        !     So do that rightaway
        !
        first = lastlf + 1
        last  = bytes
        pixel = 0

        do
            do b = first, last
                pixel = pixel + 1
                if ( pixel <= h*w ) then
                    i     = 1 + mod(pixel-1,w)
                    j     = 1 + (pixel-1)/w
                    img_f(i,h-j+1) = real(ichar(img(b),kind=pl_test_flt))
                endif
            enddo
            if ( pixel < h*w ) then
                record = record + 1
                read( 10, rec = record ) (img(b), b = 1,bytes )
                first  = 1
            else
                exit
            endif
        enddo

        width  = w
        height = h
        read_img = .true.

    end function read_img

    !     Save plot
    subroutine save_plot(fname)

        character*(*) fname

        integer cur_strm, new_strm

        !     Get current stream
        call plgstrm(cur_strm)

        !     Create a new one
        call plmkstrm(new_strm)

        !     New device type. Use a known existing driver
        call plsdev('psc')
        call plsfnam(fname)

        !     Copy old stream parameters to new stream
        call plcpstrm(cur_strm, .false.)
        call plreplot()
        call plend1()

        !     Return to previous one
        call plsstrm(cur_strm)

    end subroutine save_plot

    !     Get selection square interactively
    logical function get_clip(xi, xe, yi, ye)

        real(kind=pl_test_flt) xi, xe, yi, ye

        type(PLGraphicsIn) :: gin
        real(kind=pl_test_flt) :: xxi, yyi, xxe, yye, t
        logical st, start
        real(kind=pl_test_flt) sx(5), sy(5)

        integer PLK_Return
        data PLK_Return / Z'0D' /
        integer hex100
        data hex100 / Z'100' /

        xxi = xi
        yyi = yi
        xxe = xe
        yye = ye
        start = .false.

        !     Enter xor mode to draw a selection rectangle
        call plxormod(.true., st)

        !     Driver has xormod capability, continue
        if (st) then
            do while (.true.)

                call plxormod(.false., st)
                plgetcursor_rc = plgetcursor(gin)
                call plxormod(.true., st)

                if (gin%button .eq. 1) then
                    xxi = gin%wX
                    yyi = gin%wY
                    if (start) then
                        !                 Clear previous rectangle
                        call plline(sx, sy)
                    endif

                    start = .false.

                    sx(1) = xxi
                    sy(1) = yyi
                    sx(5) = xxi
                    sy(5) = yyi
                endif

                if (iand(gin%state,hex100).ne.0) then
                    xxe = gin%wX
                    yye = gin%wY
                    if (start) then
                        !                 Clear previous rectangle
                        call plline(sx, sy)
                    endif
                    start = .true.

                    sx(3) = xxe
                    sy(3) = yye
                    sx(2) = xxe
                    sy(2) = yyi
                    sx(4) = xxi
                    sy(4) = yye
                    !              Draw new rectangle
                    call plline(sx, sy)
                endif

                if ((gin%button .eq. 3).or.(gin%keysym .eq. PLK_Return).or.(gin%keysym .eq. ichar('Q'))) then
                    if (start) then
                        !                 Clear previous rectangle
                        call plline(sx, sy)
                        exit
                    endif
                endif
            enddo

            !         Leave xor mode
            call plxormod(.false., st)

            if (xxe .lt. xxi) then
                t   = xxi
                xxi = xxe
                xxe = t
            endif

            if (yyi .lt. yye) then
                t   = yyi
                yyi = yye
                yye = t
            endif

            xe = xxe
            xi = xxi
            ye = yye
            yi = yyi

            get_clip = (gin%keysym .eq. ichar('Q'))
            !          get_clip = .false.
        else
            !         driver has no xormod capability, just do nothing
            get_clip = .false.
            return
        endif

    end function get_clip

    !     Set gray colormap
    subroutine gray_cmap(num_col)

        integer num_col
        real(kind=pl_test_flt) r(2), g(2), b(2), pos(2)
        logical rev(1)

        r(1) = 0.0
        g(1) = 0.0
        b(1) = 0.0
        r(2) = 1.0
        g(2) = 1.0
        b(2) = 1.0

        pos(1) = 0.0
        pos(2) = 1.0
        rev(1) = .false.

        call plscmap1n(num_col)
        call plscmap1l(.true., pos, r, g, b, rev)

    end subroutine gray_cmap


    !----------------------------------------------------------------------------
    !      Subroutine a2mmx
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

end program x20f
