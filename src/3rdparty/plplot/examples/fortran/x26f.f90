!  Multi-lingual version of the first page of example 4.
!
!  Copyright (C) 2006-2016 Alan W. Irwin
!  Copyright (C) 2006,2008 Andrew Ross
!
!  Thanks to the following for providing translated strings for this example:
!  Valery Pipin (Russian)
!
!  This file is part of PLplot.
!
!  PLplot is free software; you can redistribute it and/or modify
!  it under the terms of the GNU Library General Public License as published
!  by the Free Software Foundation; either version 2 of the License, or
!  (at your option) any later version.
!
!  PLplot is distributed in the hope that it will be useful,
!  but WITHOUT ANY WARRANTY; without even the implied warranty of
!  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!  GNU Library General Public License for more details.
!
!  You should have received a copy of the GNU Library General Public License
!  along with PLplot; if not, write to the Free Software
!  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

!
! This example designed just for devices (e.g., psttfc and the
! cairo-related devices) that utilise the pango and fontconfig libraries. The
! best choice of glyph is selected by fontconfig and automatically rendered
! by pango in way that is sensitive to complex text layout (CTL) language
! issues for each unicode character in this example. Of course, you must
! have the appropriate TrueType fonts installed to have access to all the
! required glyphs.
!
! Translation instructions: The strings to be translated are given by
! x_label, y_label, alty_label, title_label, and line_label below.  The
! encoding used must be UTF-8.
!
! The following strings to be translated involve some scientific/mathematical
! jargon which is now discussed further to help translators.
!
! (1) dB is a decibel unit, see http://en.wikipedia.org/wiki/Decibel .
! (2) degrees is an angular measure, see
!     http://en.wikipedia.org/wiki/Degree_(angle) .
! (3) low-pass filter is one that transmits (passes) low frequencies.
! (4) pole is in the mathematical sense, see
!     http://en.wikipedia.org/wiki/Pole_(complex_analysis) .  "Single Pole"
!     means a particular mathematical transformation of the filter function has
!     a single pole, see
!     http://ccrma.stanford.edu/~jos/filters/Pole_Zero_Analysis_I.html .
!     Furthermore, a single-pole filter must have an inverse square decline
!     (or -20 db/decade). Since the filter plotted here does have that
!     characteristic, it must by definition be a single-pole filter, see also
!     http://www-k.ext.ti.com/SRVS/Data/ti/KnowledgeBases/analog/document/faqs/1p.htm
! (5) decade represents a factor of 10, see
!     http://en.wikipedia.org/wiki/Decade_(log_scale) .
!
!
!

! --------------------------------------------------------------------------
!  main
!
!  Illustration of logarithmic axes, and redefinition of window.
! --------------------------------------------------------------------------

program x26f

    use plplot
    implicit none

    integer, parameter :: nolangs = 2
    integer, parameter :: nlegend = 2

    character(len=80) x_label(nolangs)
    character(len=80) y_label(nolangs)
    character(len=80) legend_text(2,nolangs)
    character(len=80) alty_label(nolangs)
    character(len=80) title_label(nolangs)
    character(len=80) line_label(nolangs)

    integer      i, j
    integer :: plparseopts_rc

    data (x_label(i) ,i=1,nolangs) / &
           'Frequency', &
           'Частота'/

    data (y_label(i) ,i=1,nolangs) / &
           'Amplitude (dB)', &
           'Амплитуда (dB)'/

    data (alty_label(i) ,i=1,nolangs) / &
           'Phase shift (degrees)', &
           'Фазовый сдвиг (градусы)'/

    ! Short rearranged versions of y_label and alty_label.
    data ((legend_text(j,i), j=1, nlegend), i=1,nolangs) / &
           'Amplitude',          'Phase shift'              , &
           'Амплитуда', 'Фазовый сдвиг' /

    data (title_label(i) ,i=1,nolangs) / &
           'Single Pole Low-Pass Filter', &
           'Однополюсный Низко-Частотный Фильтр'/

    data (line_label(i) ,i=1,nolangs) / &
           '-20 dB/decade', &
           '-20 dB/десяток'/

    !  Parse and process command line arguments

    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    !  Initialize plplot

    call plinit()
    call plfont(2)

    !  Make log plots using two different styles.

    do i = 1,nolangs
        call plot1(nlegend, 0, x_label(i), y_label(i), alty_label(i), &
               legend_text(:,i), title_label(i), line_label(i))
    end do
    call plend()

end program x26f

! --------------------------------------------------------------------------
!  plot1
!
!  Log-linear plot.
! --------------------------------------------------------------------------

subroutine plot1( nlegend, type, x_label, y_label, alty_label, &
       legend_text, title_label, line_label )

    use plplot, double_PI => PL_PI
    implicit none

    real(kind=pl_test_flt), parameter :: PI = double_PI
    integer type, nlegend
    character(*) x_label, y_label, alty_label, title_label, line_label

    integer i
    real(kind=pl_test_flt)  freql(101), ampl(101), phase(101)
    real(kind=pl_test_flt)  f0, freq

    character(*) legend_text(nlegend)

    integer          opt_array(nlegend)
    integer          text_colors(nlegend)
    integer          line_colors(nlegend)
    integer          line_styles(nlegend)
    real(kind=pl_test_flt) line_widths(nlegend)
    integer          symbol_numbers(nlegend)
    integer          symbol_colors(nlegend)
    real(kind=pl_test_flt) symbol_scales(nlegend)
    character(len=20) symbols(nlegend)
    integer          box_colors(nlegend), box_patterns(nlegend)
    real(kind=pl_test_flt) box_scales(nlegend), box_line_widths(nlegend)
    real(kind=pl_test_flt) legend_width, legend_height

    call pladv(0)

    !  Set up data for log plot

    f0 = 1.0
    do i = 1,101
        freql(i) = -2.0_pl_test_flt + (i-1) / 20.0_pl_test_flt
        freq     = 10.0_pl_test_flt ** freql(i)
        ampl(i)  = 20.0_pl_test_flt * log10(1.0_pl_test_flt / sqrt(1.0_pl_test_flt + (freq/f0)**2))
        phase(i) = -(180.0_pl_test_flt / PI) * atan(freq / f0)
    end do
    call plvpor(0.15_pl_test_flt, 0.85_pl_test_flt, 0.1_pl_test_flt, 0.9_pl_test_flt)
    call plwind(-2.0_pl_test_flt, 3.0_pl_test_flt, -80.0_pl_test_flt, 0.0_pl_test_flt)

    !  Try different axis and labelling styles.

    call plcol0(1)
    if ( type .eq. 0 ) then
        call plbox("bclnst", 0.0_pl_test_flt, 0, "bnstv", 0.0_pl_test_flt, 0)
    endif
    if ( type .eq. 1 ) then
        call plbox("bcfghlnst", 0.0_pl_test_flt, 0, "bcghnstv", 0.0_pl_test_flt, 0)
    endif

    !  Plot ampl vs freq

    call plcol0(2)
    call plline(freql, ampl)
    call plcol0(2)
    call plptex(1.6_pl_test_flt, -30.0_pl_test_flt, 1.0_pl_test_flt, -20.0_pl_test_flt, 0.5_pl_test_flt, line_label)

    !  Put labels on

    call plcol0(1)
    call plmtex("b", 3.2_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, x_label)
    call plmtex("t", 2.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, title_label)
    call plcol0(2)
    call plmtex("l", 5.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, y_label)

    !  For the gridless case, put phase vs freq on same plot

    if (type .eq. 0) then
        call plcol0(1)
        call plwind(-2.0_pl_test_flt, 3.0_pl_test_flt, -100.0_pl_test_flt, 0.0_pl_test_flt)
        call plbox("", 0.0_pl_test_flt, 0, "cmstv", 30.0_pl_test_flt, 3)
        call plcol0(3)
        call plline(freql, phase)
        call plstring(freql, phase, "#(728)")
        call plcol0(3)
        call plmtex("r", 5.0_pl_test_flt, 0.5_pl_test_flt, 0.5_pl_test_flt, alty_label)
    endif

    !  Draw a legend
    !     First legend entry.

    opt_array(1)   = PL_LEGEND_LINE
    text_colors(1) = 2
    line_colors(1) = 2
    line_styles(1) = 1
    line_widths(1) = 1
    !     defining this makes the Fortran to C transformation of
    !     symbols(1) work on defined values which makes valgrind
    !     happier (even if that transformed string is not used inside the
    !     C version of pllegend)
    symbols(1) = ""

    !     note from the above opt_array the first symbol (and box) indices
    !     do not have to be specified

    !     Second legend entry.

    opt_array(2)      = PL_LEGEND_LINE + PL_LEGEND_SYMBOL
    text_colors(2)    = 3
    line_colors(2)    = 3
    line_styles(2)    = 1
    line_widths(2)    = 1
    symbol_colors(2)  = 3
    symbol_scales(2)  = 1.
    symbol_numbers(2) = 4
    symbols(2)        = "#(728)"

    !     from the above opt_arrays we can completely ignore everything
    !     to do with boxes.
    !     (note: number of legend entries determined from size of
    !     opt_array)

    call plscol0a( 15, 32, 32, 32, 0.70_pl_test_flt )
    call pllegend( legend_width, legend_height, &
           PL_LEGEND_BACKGROUND + PL_LEGEND_BOUNDING_BOX, 0, &
           0.0_pl_test_flt, 0.0_pl_test_flt, 0.10_pl_test_flt, 15, &
           1, 1, 0, 0, &
           opt_array, &
           1.0_pl_test_flt, 1.0_pl_test_flt, 2.0_pl_test_flt, &
           1.0_pl_test_flt, text_colors, legend_text, &
           box_colors, box_patterns, box_scales, box_line_widths, &
           line_colors, line_styles, line_widths, &
           symbol_colors, symbol_scales, symbol_numbers, symbols )

end subroutine plot1
