!     Demonstrate most pllegend capability including unicode symbols.
!
!     Copyright (C) 2010-2016 Alan W. Irwin
!
!     This file is part of PLplot.
!
!     PLplot is free software; you can redistribute it and/or modify
!     it under the terms of the GNU Library General Public License as published
!     by the Free Software Foundation; either version 2 of the License, or
!     (at your option) any later version.
!
!     PLplot is distributed in the hope that it will be useful,
!     but WITHOUT ANY WARRANTY; without even the implied warranty of
!     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!     GNU Library General Public License for more details.
!
!     You should have received a copy of the GNU Library General Public License
!     along with PLplot; if not, write to the Free Software
!     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
!     This example designed just for devices (e.g., the cairo-related and
!     qt-related devices) where the best choice of glyph is automatically
!     selected by the related libraries (pango/cairo or Qt4) for each
!     unicode character depending on what system fonts are installed.  Of
!     course, you must have the appropriate TrueType fonts installed to
!     have access to all the required glyphs.


!--------------------------------------------------------------------------
!  main
!
!  Demonstrate most pllegend capability including unicode symbols.
! --------------------------------------------------------------------------

program x33f
    use plplot

    implicit none

    integer, parameter :: MAX_NLEGEND = 7

    integer            :: i, k
    integer            :: opt, position
    integer            :: nlegend, nturn
    integer            :: opt_array(MAX_NLEGEND)
    integer            :: text_colors(MAX_NLEGEND)
    integer            :: box_colors(MAX_NLEGEND)
    integer            :: box_patterns(MAX_NLEGEND)
    real(kind=pl_test_flt)   :: box_scales(MAX_NLEGEND)
    real(kind=pl_test_flt)   :: box_line_widths(MAX_NLEGEND)
    integer            :: line_colors(MAX_NLEGEND)
    integer            :: line_styles(MAX_NLEGEND)
    real(kind=pl_test_flt)   :: line_widths(MAX_NLEGEND)
    integer            :: symbol_numbers(MAX_NLEGEND), symbol_colors(MAX_NLEGEND)
    real(kind=pl_test_flt)   :: symbol_scales(MAX_NLEGEND)
    character(len=200) :: text(MAX_NLEGEND)
    character(len=20)  :: symbols(MAX_NLEGEND)
    real(kind=pl_test_flt)   :: legend_width, legend_height, x, y, xstart, ystart
    real(kind=pl_test_flt)   :: max_height, text_scale
    integer            :: opt_base, nrow, ncolumn

    integer            :: position_options(16)
    character(len=3)   :: special_symbols(5)

    real(kind=pl_test_flt)   :: values_small(2)
    real(kind=pl_test_flt)   :: values_uneven(9)
    real(kind=pl_test_flt)   :: values_even(9)

    integer, parameter :: COLORBAR_KINDS = 4
    integer            :: colorbar_option_kinds(COLORBAR_KINDS)
    character(len=100) :: colorbar_option_kind_labels(COLORBAR_KINDS)

    integer, parameter :: COLORBAR_POSITIONS = 4
    integer            :: colorbar_position_options(COLORBAR_POSITIONS)
    character(len=100) :: colorbar_position_option_labels(COLORBAR_POSITIONS)

    integer, parameter :: COLORBAR_LABELS = 4
    integer            :: colorbar_label_options(COLORBAR_LABELS)
    character(len=100) :: colorbar_label_option_labels(COLORBAR_LABELS)

    integer, parameter :: COLORBAR_CAPS = 4
    integer            :: colorbar_cap_options(COLORBAR_CAPS)
    integer :: plparseopts_rc
    character(len=100) :: colorbar_cap_option_labels(COLORBAR_CAPS)

    !     Pick 5 arbitrary UTF-8 symbols useful for plotting points (✠✚✱✪✽✺✰✴✦).
    data special_symbols / &
           '✰',             &
           '✴',             &
           '✱',             &
           '✽',             &
           '✦'              /

    data values_small   / -1.0_pl_test_flt, 1.0_pl_test_flt /
    data values_uneven &
           / -1.0_pl_test_flt, 2.0_pl_test_flt, 2.6_pl_test_flt, 3.4_pl_test_flt, &
           6.0_pl_test_flt, 7.0_pl_test_flt, 8.0_pl_test_flt, 9.0_pl_test_flt, &
           10.0_pl_test_flt /
    data values_even &
           / -2.0_pl_test_flt, -1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt, &
           2.0_pl_test_flt, 3.0_pl_test_flt, 4.0_pl_test_flt, 5.0_pl_test_flt, &
           6.0_pl_test_flt /

    real(kind=pl_test_flt), parameter :: small_factor = 1.e-20_pl_test_flt

    values_small = small_factor*values_small
    values_uneven = small_factor*values_uneven
    values_even = small_factor*values_even

    position_options(1) = PL_POSITION_LEFT + PL_POSITION_TOP + PL_POSITION_OUTSIDE
    position_options(2) = PL_POSITION_TOP + PL_POSITION_OUTSIDE
    position_options(3) = PL_POSITION_RIGHT + PL_POSITION_TOP + PL_POSITION_OUTSIDE
    position_options(4) = PL_POSITION_RIGHT + PL_POSITION_OUTSIDE
    position_options(5) = PL_POSITION_RIGHT + PL_POSITION_BOTTOM + PL_POSITION_OUTSIDE
    position_options(6) = PL_POSITION_BOTTOM + PL_POSITION_OUTSIDE
    position_options(7) = PL_POSITION_LEFT + PL_POSITION_BOTTOM + PL_POSITION_OUTSIDE
    position_options(8) = PL_POSITION_LEFT + PL_POSITION_OUTSIDE
    position_options(9) = PL_POSITION_LEFT + PL_POSITION_TOP + PL_POSITION_INSIDE
    position_options(10) = PL_POSITION_TOP + PL_POSITION_INSIDE
    position_options(11) = PL_POSITION_RIGHT + PL_POSITION_TOP + PL_POSITION_INSIDE
    position_options(12) = PL_POSITION_RIGHT + PL_POSITION_INSIDE
    position_options(13) = PL_POSITION_RIGHT + PL_POSITION_BOTTOM + PL_POSITION_INSIDE
    position_options(14) = PL_POSITION_BOTTOM + PL_POSITION_INSIDE
    position_options(15) = PL_POSITION_LEFT + PL_POSITION_BOTTOM + PL_POSITION_INSIDE
    position_options(16) = PL_POSITION_LEFT + PL_POSITION_INSIDE

    ! plcolorbar options

    ! Colorbar type options
    colorbar_option_kinds(1) = PL_COLORBAR_SHADE
    colorbar_option_kinds(2) = PL_COLORBAR_SHADE + PL_COLORBAR_SHADE_LABEL
    colorbar_option_kinds(3) = PL_COLORBAR_IMAGE
    colorbar_option_kinds(4) = PL_COLORBAR_GRADIENT

    colorbar_option_kind_labels(1) = "Shade colorbars"
    colorbar_option_kind_labels(2) = "Shade colorbars with custom labels"
    colorbar_option_kind_labels(3) = "Image colorbars"
    colorbar_option_kind_labels(4) = "Gradient colorbars"

    ! Which side of the page are we positioned relative to?
    colorbar_position_options(1) = PL_POSITION_LEFT
    colorbar_position_options(2) = PL_POSITION_RIGHT
    colorbar_position_options(3) = PL_POSITION_TOP
    colorbar_position_options(4) = PL_POSITION_BOTTOM

    colorbar_position_option_labels(1) = "Left"
    colorbar_position_option_labels(2) = "Right"
    colorbar_position_option_labels(3) = "Top"
    colorbar_position_option_labels(4) = "Bottom"

    ! Colorbar label positioning options
    colorbar_label_options(1) = PL_COLORBAR_LABEL_LEFT
    colorbar_label_options(2) = PL_COLORBAR_LABEL_RIGHT
    colorbar_label_options(3) = PL_COLORBAR_LABEL_TOP
    colorbar_label_options(4) = PL_COLORBAR_LABEL_BOTTOM

    colorbar_label_option_labels(1) = "Label left"
    colorbar_label_option_labels(2) = "Label right"
    colorbar_label_option_labels(3) = "Label top"
    colorbar_label_option_labels(4) = "Label bottom"

    ! Colorbar cap options
    colorbar_cap_options(1) = PL_COLORBAR_CAP_NONE
    colorbar_cap_options(2) = PL_COLORBAR_CAP_LOW
    colorbar_cap_options(3) = PL_COLORBAR_CAP_HIGH
    colorbar_cap_options(4) = PL_COLORBAR_CAP_LOW + PL_COLORBAR_CAP_HIGH

    colorbar_cap_option_labels(1) = "No caps"
    colorbar_cap_option_labels(2) = "Low cap"
    colorbar_cap_option_labels(3) = "High cap"
    colorbar_cap_option_labels(4) = "Low and high caps"

    !     Parse and process command line arguments

    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !     Initialize plplot
    call plinit

    !     First page illustrating the 16 standard positions.
    call pladv( 0 )
    call plvpor( 0.25_pl_test_flt, 0.75_pl_test_flt, 0.25_pl_test_flt, 0.75_pl_test_flt )
    call plwind( 0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plbox( 'bc', 0.0_pl_test_flt, 0, 'bc', 0.0_pl_test_flt, 0 )
    call plsfont( PL_FCI_SANS, -1, -1 )
    call plmtex( 't', 8.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, &
           'The 16 standard legend positions with' )
    call plmtex( 't', 6.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, &
           'the same (0.05) offset in x and y' )

    nlegend = 1
    !     Only specify legend data that are required according to the
    !     value of opt_array for that entry.
    opt_base          = PL_LEGEND_BACKGROUND + PL_LEGEND_BOUNDING_BOX
    opt_array(1)      = PL_LEGEND_LINE + PL_LEGEND_SYMBOL
    line_styles(1)    = 1
    line_widths(1)    = 1
    symbol_scales(1)  = 1._pl_test_flt
    symbol_numbers(1) = 4
    symbols(1)        = "#(728)"

    !     Use monotype fonts so that all legends are the same size.
    call plsfont( PL_FCI_MONO, -1, -1 )
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    do k = 1,16
        position = position_options(k)
        opt = opt_base
        text_colors(1)   = 1 + mod( k-1, 8 )
        line_colors(1)   = 1 + mod( k-1, 8 )
        symbol_colors(1) = 1 + mod( k-1, 8 )
        write( text(1), '(i2.2)' ) k-1

        call pllegend( legend_width, legend_height, opt, position, &
               0.05_pl_test_flt, 0.05_pl_test_flt,                                &
               0.1_pl_test_flt, 15, 1, 1, 0, 0,                             &
               opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt, &
               1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend),     &
               box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
               line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
               symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend)  )
    enddo

    !     Second page illustrating effect of nrow, ncolumn for the same legend
    !     data.

    call pladv( 0 )
    call plvpor( 0.25_pl_test_flt, 0.75_pl_test_flt, 0.25_pl_test_flt, 0.75_pl_test_flt )
    call plwind( 0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plbox( 'bc', 0.0_pl_test_flt, 0, 'bc', 0.0_pl_test_flt, 0 )
    call plsfont( PL_FCI_SANS, -1, -1 )
    call plmtex( 't', 8.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, &
           'The effect of nrow, ncolumn, PL_LEGEND_ROW_MAJOR,' )
    call plmtex( 't', 6.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, &
           'and position for the same legend data' )

    nlegend = 7

    !     Only specify legend data that are required according to the
    !     value of opt_array for that entry.

    opt_base = PL_LEGEND_BACKGROUND + PL_LEGEND_BOUNDING_BOX
    do k = 1,nlegend
        opt_array(k)      = PL_LEGEND_LINE + PL_LEGEND_SYMBOL
        line_styles(k)    = 1
        line_widths(k)    = 1
        symbol_scales(k)  = 1._pl_test_flt
        symbol_numbers(k) = 2
        symbols(k)        = "#(728)"
        write( text(k), '(i2.2)' ) k-1
        text_colors(k)   = 1 + mod( k-1, 8 )
        line_colors(k)   = 1 + mod( k-1, 8 )
        symbol_colors(k) = 1 + mod( k-1, 8 )
    enddo

    !     Use monotype fonts so that all legends are the same size.

    call plsfont( PL_FCI_MONO, -1, -1 )
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    position = PL_POSITION_TOP + PL_POSITION_OUTSIDE
    opt     = opt_base
    x       = 0._pl_test_flt
    y       = 0.1_pl_test_flt
    nrow    = 1
    ncolumn = nlegend
    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.05_pl_test_flt, 15, 1, 1, nrow, ncolumn,                         &
           opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt,       &
           1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )

    position = PL_POSITION_BOTTOM + PL_POSITION_OUTSIDE
    opt     = opt_base
    x       = 0._pl_test_flt
    y       = 0.1_pl_test_flt
    nrow    = 1
    ncolumn = nlegend
    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.05_pl_test_flt, 15, 1, 1, nrow, ncolumn,                         &
           opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt,       &
           1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )

    position = PL_POSITION_LEFT + PL_POSITION_OUTSIDE
    opt     = opt_base
    x       = 0.1_pl_test_flt
    y       = 0._pl_test_flt
    nrow    = nlegend
    ncolumn = 1
    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.05_pl_test_flt, 15, 1, 1, nrow, ncolumn,                         &
           opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt,       &
           1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )

    position = PL_POSITION_RIGHT + PL_POSITION_OUTSIDE
    opt     = opt_base
    x       = 0.1_pl_test_flt
    y       = 0._pl_test_flt
    nrow    = nlegend
    ncolumn = 1
    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.05_pl_test_flt, 15, 1, 1, nrow, ncolumn,                         &
           opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt,       &
           1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )

    position = PL_POSITION_LEFT + PL_POSITION_TOP + PL_POSITION_INSIDE
    opt     = opt_base
    x       = 0._pl_test_flt
    y       = 0._pl_test_flt
    nrow    = 6
    ncolumn = 2
    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.05_pl_test_flt, 15, 1, 1, nrow, ncolumn,                         &
           opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt,       &
           1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )

    position = PL_POSITION_RIGHT + PL_POSITION_TOP + PL_POSITION_INSIDE
    opt     = opt_base + PL_LEGEND_ROW_MAJOR
    x       = 0._pl_test_flt
    y       = 0._pl_test_flt
    nrow    = 6
    ncolumn = 2
    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.05_pl_test_flt, 15, 1, 1, nrow, ncolumn,                         &
           opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt,       &
           1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )

    position = PL_POSITION_BOTTOM + PL_POSITION_INSIDE
    opt     = opt_base + PL_LEGEND_ROW_MAJOR
    x       = 0._pl_test_flt
    y       = 0._pl_test_flt
    nrow    = 3
    ncolumn = 3
    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.05_pl_test_flt, 15, 1, 1, nrow, ncolumn,                         &
           opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt,       &
           1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )

    !     Third page demonstrating legend alignment

    call pladv( 0 )
    call plvpor( 0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 0.9_pl_test_flt )
    call plwind( 0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plsfont( PL_FCI_SANS, -1, -1 )
    call plmtex( 't', 2.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, &
           'Demonstrate legend alignment' )

    x        = 0.1_pl_test_flt
    y        = 0.1_pl_test_flt
    nturn    = 5
    nlegend  = 0
    position = PL_POSITION_TOP + PL_POSITION_LEFT + PL_POSITION_SUBPAGE
    opt_base = PL_LEGEND_BACKGROUND + PL_LEGEND_BOUNDING_BOX
    opt      = opt_base
    do i = 1,9

        !         Set up legend arrays with the correct size, type.

        if ( i .le. nturn ) then
            nlegend = nlegend + 1
        else
            nlegend = nlegend - 1
        endif
        nlegend = max( 1, nlegend )

        !         nly specify legend data that are required according to the
        !         value of opt_array for that entry.

        do k = 1,nlegend
            opt_array(k)      = PL_LEGEND_LINE + PL_LEGEND_SYMBOL
            line_styles(k)    = 1
            line_widths(k)    = 1
            symbol_scales(k)  = 1._pl_test_flt
            symbol_numbers(k) = 2
            symbols(k)        = "#(728)"
            write( text(k), '(i2.2)' ) k-1
            text_colors(k)   = 1 + mod( k-1, 8 )
            line_colors(k)   = 1 + mod( k-1, 8 )
            symbol_colors(k) = 1 + mod( k-1, 8 )
        enddo

        !         Use monotype fonts so that all legends are the same size.

        call plsfont( PL_FCI_MONO, -1, -1 )
        call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

        nrow    = MIN( 3, nlegend )
        ncolumn = 0

        call pllegend( legend_width, legend_height, opt, position, &
               x, y,                                                  &
               0.025_pl_test_flt, 15, 1, 1, nrow, ncolumn,                  &
               opt_array(1:nlegend), 1.0_pl_test_flt, 1.0_pl_test_flt, 1.5_pl_test_flt, &
               1._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
               box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
               line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
               symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )

        if ( i .eq. nturn ) then
            position = PL_POSITION_TOP + PL_POSITION_RIGHT + PL_POSITION_SUBPAGE
            opt = opt_base
            x   = 1._pl_test_flt - x
            y   = y + legend_height
        else
            x = x + legend_width
            y = y + legend_height
        endif
    enddo

    !     Fourth page illustrating various kinds of legends

    max_height = 0._pl_test_flt
    xstart     = 0.0_pl_test_flt
    ystart     = 0.1_pl_test_flt
    x          = xstart
    y          = ystart
    text_scale = 0.90_pl_test_flt
    call pladv( 0 )
    call plvpor( 0.0_pl_test_flt, 1._pl_test_flt, 0.0_pl_test_flt, 0.90_pl_test_flt )
    call plwind( 0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
    !       call plbox('bc', 0.0, 0, 'bc', 0.0_pl_test_flt, 0)
    call plsfont( PL_FCI_SANS, -1, -1 )
    call plmtex( 't', 2.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, &
           'Demonstrate Various Kinds of Legends' )

    nlegend = 5

    !     Only specify legend data that are required according to the
    !     value of opt_array for that entry.

    position = PL_POSITION_LEFT + PL_POSITION_TOP
    opt_base = PL_LEGEND_BACKGROUND + PL_LEGEND_BOUNDING_BOX + PL_LEGEND_TEXT_LEFT

    !     Set up None, Box, Line, Symbol, and Line & Symbol legend entries.

    opt_array(1) = PL_LEGEND_NONE
    text(1) = 'None'
    text_colors(1) = 1

    opt_array(2) = PL_LEGEND_COLOR_BOX
    text(2) = 'Box'
    text_colors(2)     = 2
    box_colors(2)      = 2
    box_patterns(2)    = 0
    box_scales(2)      = 0.8_pl_test_flt
    box_line_widths(2) = 1

    opt_array(3) = PL_LEGEND_LINE
    text(3) = 'Line'
    text_colors(3) = 3
    line_colors(3) = 3
    line_styles(3) = 1
    line_widths(3) = 1

    opt_array(4) = PL_LEGEND_SYMBOL
    text(4) = 'Symbol'
    text_colors(4)    = 4
    symbol_colors(4)  = 4
    symbol_scales(4)  = text_scale
    symbol_numbers(4) = 4
    symbols(4)        = special_symbols(3)

    opt_array(5) = PL_LEGEND_SYMBOL + PL_LEGEND_LINE
    text(5) = 'L & S'
    text_colors(5)    = 5
    line_colors(5)    = 5
    line_styles(5)    = 1
    line_widths(5)    = 1
    symbol_colors(5)  = 5
    symbol_scales(5)  = text_scale
    symbol_numbers(5) = 4
    symbols(5)        = special_symbols(3)

    opt = opt_base
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Set up symbol legend entries with various symbols.

    do i = 1,nlegend
        opt_array(i) = PL_LEGEND_SYMBOL
        text(i) = 'Symbol ' // special_symbols(i)
        text_colors(i)    = i
        symbol_colors(i)  = i
        symbol_scales(i)  = text_scale
        symbol_numbers(i) = 4
        symbols(i)        = special_symbols(i)
    enddo

    opt = opt_base
    x   = x + legend_width
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Set up symbol legend entries with various numbers of symbols.

    do i = 1,nlegend
        opt_array(i) = PL_LEGEND_SYMBOL
        write( text(i), '(a,i0)' ) 'Symbol Number ', i+1
        text_colors(i)    = i
        symbol_colors(i)  = i
        symbol_scales(i)  = text_scale
        symbol_numbers(i) = i + 1
        symbols(i)        = special_symbols(3)
    enddo

    opt = opt_base
    x   = x + legend_width
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Set up box legend entries with various colours.

    do i = 1,nlegend
        opt_array(i) = PL_LEGEND_COLOR_BOX
        write( text(i), '(a,i0)' ) 'Box Color ', i
        text_colors(i)     = i
        box_colors(i)      = i
        box_patterns(i)    = 0
        box_scales(i)      = 0.8_pl_test_flt
        box_line_widths(i) = 1
    enddo

    opt = opt_base

    !     Use new origin

    x          = xstart
    y          = y + max_height
    max_height = 0._pl_test_flt
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt)

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Set up box legend entries with various patterns.

    do i = 1,nlegend
        opt_array(i) = PL_LEGEND_COLOR_BOX
        write( text(i), '(a,i0)' ) 'Box Pattern ', i-1
        text_colors(i)     = 2
        box_colors(i)      = 2
        box_patterns(i)    = i - 1
        box_scales(i)      = 0.8_pl_test_flt
        box_line_widths(i) = 1
    enddo

    opt = opt_base
    x   = x + legend_width
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Set up box legend entries with various box pattern line widths.

    do i = 1,nlegend
        opt_array(i) = PL_LEGEND_COLOR_BOX
        write( text(i), '(a,i0)' ) 'Box Line Width ', i
        text_colors(i)     = 2
        box_colors(i)      = 2
        box_patterns(i)    = 3
        box_scales(i)      = 0.8_pl_test_flt
        box_line_widths(i) = i
    enddo

    opt = opt_base
    x   = x + legend_width
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Set up line legend entries with various colours.

    do i =1,nlegend
        opt_array(i) = PL_LEGEND_LINE
        write( text(i), '(a,i0)' ) 'Line Color ', i
        text_colors(i) = i
        line_colors(i) = i
        line_styles(i) = 1
        line_widths(i) = 1
    enddo

    opt = opt_base

    !     Use new origin

    x          = xstart
    y          = y + max_height
    max_height = 0._pl_test_flt
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Set up line legend entries with various styles.

    do i = 1,nlegend
        opt_array(i) = PL_LEGEND_LINE
        write( text(i), '(a,i0)' ) 'Line Style ', i
        text_colors(i) = 2
        line_colors(i) = 2
        line_styles(i) = i
        line_widths(i) = 1
    enddo

    opt = opt_base
    x   = x + legend_width
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Set up line legend entries with various widths.

    do i =1,nlegend
        opt_array(i) = PL_LEGEND_LINE
        write( text(i), '(a,i0)' ) 'Line Width ', i
        text_colors(i) = 2
        line_colors(i) = 2
        line_styles(i) = 1
        line_widths(i) = i
    enddo

    opt = opt_base
    x   = x + legend_width
    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )

    call pllegend( legend_width, legend_height, opt, position, x, y, &
           0.1_pl_test_flt, 15, 1, 1, 0, 0,                                   &
           opt_array(1:nlegend), 1.0_pl_test_flt, text_scale, 2.0_pl_test_flt,      &
           0._pl_test_flt, text_colors(1:nlegend), text(1:nlegend), &
           box_colors(1:nlegend), box_patterns(1:nlegend), box_scales(1:nlegend), box_line_widths(1:nlegend), &
           line_colors(1:nlegend), line_styles(1:nlegend), line_widths(1:nlegend), &
           symbol_colors(1:nlegend), symbol_scales(1:nlegend), symbol_numbers(1:nlegend), symbols(1:nlegend) )
    max_height = max( max_height, legend_height )

    !     Color bar examples

    ! Use unsaturated green background colour to contrast with black caps.
    call plscolbg( 70, 185, 70 )
    ! Cut out the greatest and smallest bits of the color spectrum to
    ! leave colors for the end caps.
    call plscmap1_range( 0.01_pl_test_flt, 0.99_pl_test_flt )

    ! We can only test image and gradient colorbars with two element arrays
    do i = 2,COLORBAR_KINDS-1
        call plcolorbar_example( "cmap1_blue_yellow.pal", i, 0, 0._pl_test_flt, 2, values_small )
    enddo
    ! Test shade colorbars with larger arrays
    do i = 0,1
        call plcolorbar_example( "cmap1_blue_yellow.pal", i, 4, 2._pl_test_flt, 9, values_even )
    enddo
    do i = 0,1
        call plcolorbar_example( "cmap1_blue_yellow.pal", i, 0, 0._pl_test_flt, 9, values_uneven )
    enddo

    call plend()

contains

    subroutine plcolorbar_example_page( kind_i, label_i, cap_i, cont_color, cont_width, n_values, values )

        use plplot

        implicit none

        integer  :: kind_i, label_i, cap_i, cont_color, n_values
        real(kind=pl_test_flt)   :: cont_width
        real(kind=pl_test_flt), dimension(:)   :: values

        ! Parameters for the colorbars on this page
        integer            :: position_i, position, opt
        real(kind=pl_test_flt)   :: x, y, x_length, y_length;
        real(kind=pl_test_flt)   :: ticks(1)
        integer            :: sub_ticks(1)
        real(kind=pl_test_flt)   :: low_cap_color, high_cap_color
        logical            :: vertical, ifn
        character(len=20)  :: axis_opts(1)
        integer            :: label_opts(1)
        character(len=200) :: labels(1)
        character(len=200) :: title
        real(kind=pl_test_flt)   :: colorbar_width, colorbar_height
        integer            :: n_values_array(1);
        real(kind=pl_test_flt), allocatable, dimension(:,:) :: values_array

        ticks(1) = 0.0_pl_test_flt
        sub_ticks(1) = 0
        label_opts(1) = 0

        n_values_array(1) = n_values
        allocate(values_array(1,n_values))
        values_array(1,:) = values(:)

        low_cap_color  = 0.0_pl_test_flt;
        high_cap_color = 1.0_pl_test_flt;

        ! Start a new page
        call pladv( 0 )

        ! Draw one colorbar relative to each side of the page
        do position_i = 0,COLORBAR_POSITIONS-1
            position = colorbar_position_options(position_i+1);
            opt      = ior( &
                   colorbar_option_kinds(kind_i+1), &
                   ior(colorbar_label_options(label_i+1), &
                   colorbar_cap_options(cap_i+1) ) )

            vertical = (iand(position, PL_POSITION_LEFT) > 0 .or. iand(position, PL_POSITION_RIGHT) > 0 )
            ifn      = (iand(position, PL_POSITION_LEFT) > 0 .or. iand(position, PL_POSITION_BOTTOM) > 0 )

            ! Set the offset position on the page
            if (vertical .eqv. .true.) then
                x        = 0.0_pl_test_flt
                y        = 0.0_pl_test_flt
                x_length = 0.05_pl_test_flt
                y_length = 0.5_pl_test_flt
            else
                x        = 0.0_pl_test_flt
                y        = 0.0_pl_test_flt
                x_length = 0.5_pl_test_flt
                y_length = 0.05_pl_test_flt
            endif

            ! Set appropriate labelling options.
            if (ifn .eqv. .true.) then
                if ( cont_color .eq. 0 .or. cont_width .eq. 0._pl_test_flt ) then
                    axis_opts(1) = "uwtivn"
                else
                    axis_opts(1) = "uwxvn"
                endif
            else
                if ( cont_color .eq. 0 .or. cont_width .eq. 0._pl_test_flt ) then
                    axis_opts(1) = "uwtivm"
                else
                    axis_opts(1) = "uwxvm"
                endif
            endif

            write(labels(1), '(3A)') trim(colorbar_position_option_labels(position_i+1)), &
                   ', ', trim(colorbar_label_option_labels(label_i+1))

            ! Smaller text
            call plschr( 0.0_pl_test_flt, 0.75_pl_test_flt )
            ! Small ticks on the vertical axis
            call plsmaj( 0.0_pl_test_flt, 0.5_pl_test_flt )
            call plsmin( 0.0_pl_test_flt, 0.5_pl_test_flt )

            call plvpor( 0.20_pl_test_flt, 0.80_pl_test_flt, 0.20_pl_test_flt, 0.80_pl_test_flt )
            call plwind( 0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
            ! Set interesting background colour.
            call plscol0a( 15, 0, 0, 0, 0.20_pl_test_flt )
            call plcolorbar( colorbar_width, colorbar_height, &
                   ior(opt, ior(PL_COLORBAR_BOUNDING_BOX, PL_COLORBAR_BACKGROUND)), position, &
                   x, y, x_length, y_length, &
                   15, 1, 1, &
                   low_cap_color, high_cap_color, &
                   cont_color, cont_width, &
                   label_opts, labels, &
                   axis_opts, ticks, sub_ticks, &
                   n_values_array, values_array )

            ! Reset text and tick sizes
            call plschr( 0.0_pl_test_flt, 1.0_pl_test_flt )
            call plsmaj( 0.0_pl_test_flt, 1.0_pl_test_flt )
            call plsmin( 0.0_pl_test_flt, 1.0_pl_test_flt )
        enddo


        ! Draw a page title
        write(title, '(3A)') trim(colorbar_option_kind_labels(kind_i+1)), ' - ', &
               trim(colorbar_cap_option_labels(cap_i+1))
        call plvpor( 0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
        call plwind( 0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
        call plptex( 0.5_pl_test_flt, 0.5_pl_test_flt, 0.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, title )

        deallocate(values_array)

    end subroutine plcolorbar_example_page

    subroutine plcolorbar_example( palette, kind_i, cont_color, cont_width, n_values, values )
        character(*) :: palette
        integer  :: kind_i, label_i, cap_i, cont_color, n_values
        real(kind=pl_test_flt)   :: cont_width
        real(kind=pl_test_flt), dimension(:)   :: values

        ! Load the color palette
        call plspal1( palette, .true. )

        do label_i = 0,COLORBAR_LABELS-1
            do cap_i = 0,COLORBAR_CAPS-1
                call plcolorbar_example_page( kind_i, label_i, cap_i, &
                       cont_color, cont_width, &
                       n_values, values )
            enddo
        enddo
    end subroutine plcolorbar_example

end program x33f
