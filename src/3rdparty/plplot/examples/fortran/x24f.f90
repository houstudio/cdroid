!   Unicode Pace Flag
!
!   Copyright (C) 2005 Rafael Laboissiere
!   Copyright (C) 2008 Andrew Ross
!   Copyright (C) 2008-2016 Alan W. Irwin
!
!   This file is part of PLplot.
!
!   PLplot is free software; you can redistribute it and/or modify
!   it under the terms of the GNU Library General Public License as published
!   by the Free Software Foundation; either version 2 of the License, or
!   (at your option) any later version.
!
!   PLplot is distributed in the hope that it will be useful,
!   but WITHOUT ANY WARRANTY; without even the implied warranty of
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!   GNU Library General Public License for more details.
!
!   You should have received a copy of the GNU Library General Public License
!   along with PLplot; if not, write to the Free Software
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
!
!   In Debian, run like this:
!
!   ( TTFDIR=/usr/share/fonts/truetype ; \
!     PLPLOT_FREETYPE_SANS_FONT=$TTFDIR/arphic/bkai00mp.ttf \
!     PLPLOT_FREETYPE_SERIF_FONT=$TTFDIR/freefont/FreeSerif.ttf \
!     PLPLOT_FREETYPE_MONO_FONT=$TTFDIR/ttf-devanagari-fonts/lohit_hi.ttf \
!     PLPLOT_FREETYPE_SCRIPT_FONT=$TTFDIR/unfonts/UnBatang.ttf \
!     PLPLOT_FREETYPE_SYMBOL_FONT=$TTFDIR/ttf-bengali-fonts/JamrulNormal.ttf \
!     ./x24f -dev png -o x24f.png )
!
!   Packages needed:
!
!   ttf-arphic-bkai00mp
!   ttf-freefont
!   ttf-devanagari-fonts
!   ttf-unfonts
!   ttf-bengali-fonts
!
!   For the latest Ubuntu systems lohit_hi.ttf has been moved to the
!   ttf-indic-fonts-core package instead of ttf-devanagari-fonts so you
!   will have to use this package instead and update the font path.


program x24f
    use plplot
    implicit none

    integer i, j
    integer red(7)
    integer green(7)
    integer blue(7)
    integer :: plparseopts_rc

    real(kind=pl_test_flt)  px(4)
    real(kind=pl_test_flt)  py(4)
    real(kind=pl_test_flt)  sx(12)
    real(kind=pl_test_flt)  sy(12)

    character(len=30) peace(12)

    data (red(i) ,i=1,7)   / 240, 204, 204, 204,   0,  39, 125 /
    data (green(i) ,i=1,7) / 240,   0, 125, 204, 204,  80,   0 /
    data (blue(i) ,i=1,7)  / 240,   0,   0,   0,   0, 204, 125 /

    data (px(i) ,i=1,4) / 0.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt, 1.0_pl_test_flt /
    data (py(i) ,i=1,4) / 0.0_pl_test_flt, 0.25_pl_test_flt, 0.25_pl_test_flt, 0.0_pl_test_flt /

    data (sx(i) ,i=1,12) / &
           0.16374_pl_test_flt, &
           0.15844_pl_test_flt, &
           0.15255_pl_test_flt, &
           0.17332_pl_test_flt, &
           0.50436_pl_test_flt, &
           0.51721_pl_test_flt, &
           0.49520_pl_test_flt, &
           0.48713_pl_test_flt, &
           0.83976_pl_test_flt, &
           0.81688_pl_test_flt, &
           0.82231_pl_test_flt, &
           0.82647_pl_test_flt/


    data (sy(i) ,i=1,12) / &
           0.125_pl_test_flt, &
           0.375_pl_test_flt, &
           0.625_pl_test_flt, &
           0.875_pl_test_flt, &
           0.125_pl_test_flt, &
           0.375_pl_test_flt, &
           0.625_pl_test_flt, &
           0.875_pl_test_flt, &
           0.125_pl_test_flt, &
           0.375_pl_test_flt, &
           0.625_pl_test_flt, &
           0.875_pl_test_flt/


    !  Taken from http://www.columbia.edu/~fdc/pace/

    !  Mandarin
    peace(1) = '#<0x00>和平'

    !  Hindi
    peace(2) = '#<0x20>शांति'

    !  English
    peace(3) = '#<0x10>Peace'

    !  Hebrew
    peace(4) = '#<0x10>שלום'

    !  Russian
    peace(5) = '#<0x10>Мир'

    !  German
    peace(6) = '#<0x10>Friede'

    !  Korean
    peace(7) = '#<0x30>평화'

    !  French
    peace(8) = '#<0x10>Paix'

    !  Spanish
    peace(9) = '#<0x10>Paz'

    !  Arabic
    peace(10) = '#<0x10>ﺳﻼم'

    !  Turkish
    peace(11) = '#<0x10>Barış'

    !  Kurdish
    peace(12) = '#<0x10>Hasîtî'

    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    call plinit()

    call pladv(0)
    call plvpor (0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt)
    call plwind (0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt)
    call plcol0 (0)
    call plbox ('', 1.0_pl_test_flt, 0, '', 1.0_pl_test_flt, 0)

    call plscmap0n (7)
    call plscmap0 (red, green, blue)

    call plschr (0.0_pl_test_flt, 4.0_pl_test_flt)
    call plfont (1)

    do i = 1,4

        call plcol0 (i)
        call plfill (px, py)

        do j = 1,4
            py (j) = py (j) + 1.0_pl_test_flt / 4.0_pl_test_flt
        enddo
    enddo

    call plcol0 (0)
    do i = 1,12
        call plptex (sx (i), sy (i), 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, peace (i))
    enddo

    call plend()

end program x24f
