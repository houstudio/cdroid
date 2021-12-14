#   Unicode Pace Flag
#
#   Copyright (C) 2005 Rafael Laboissiere
#   Copyright (C) 2008 Andrew Ross
#
#
#   This file is part of PLplot.
#
#   PLplot is free software; you can redistribute it and/or modify
#   it under the terms of the GNU Library General Public License as published
#   by the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   PLplot is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public License
#   along with PLplot; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#
#   In Debian, run like this:
#
#   ( TTFDIR=/usr/share/fonts/truetype ; \
#    PLPLOT_FREETYPE_SANS_FONT=$TTFDIR/arphic/bkai00mp.ttf \
#    PLPLOT_FREETYPE_SERIF_FONT=$TTFDIR/freefont/FreeSerif.ttf \
#    PLPLOT_FREETYPE_MONO_FONT=$TTFDIR/ttf-devanagari-fonts/lohit_hi.ttf \
#    PLPLOT_FREETYPE_SCRIPT_FONT=$TTFDIR/unfonts/UnBatang.ttf \
#    PLPLOT_FREETYPE_SYMBOL_FONT=$TTFDIR/ttf-bengali-fonts/JamrulNormal.ttf \
#    ./x24 -dev png -o x24t.png )
#
#   Packages needed:
#
#   ttf-arphic-bkai00mp
#   ttf-freefont
#   ttf-devanagari-fonts
#   ttf-unfonts
#   ttf-bengali-fonts
#
#   For the latest Ubuntu systems lohit_hi.ttf has been moved to the
#   ttf-indic-fonts-core package instead of ttf-devanagari-fonts so you
#   will have to use this package instead and update the font path.


proc x24 {{w loopback}} {

    matrix red i 7   = {240, 204, 204, 204,   0,  39, 125}
    matrix green i 7 = {240,   0, 125, 204, 204,  80,   0}
    matrix blue i 7  = {240,   0,   0,   0,   0, 204, 125}

    matrix px f 4 = {0.0, 0.0, 1.0, 1.0}
    matrix py f 4 = {0.0, 0.25, 0.25, 0.0}

    matrix sx f 12 = {
	0.16374,
	0.15844,
	0.15255,
	0.17332,
	0.50436,
	0.51721,
	0.49520,
	0.48713,
	0.83976,
	0.81688,
	0.82231,
	0.82647
    }

    matrix sy f 12 = {
	0.125,
	0.375,
	0.625,
	0.875,
	0.125,
	0.375,
	0.625,
	0.875,
	0.125,
	0.375,
	0.625,
	0.875
    }


# Taken from http://www.columbia.edu/~fdc/pace/

    # Mandarin
    # Hindi
    # English
    # Hebrew
    # Russian
    # German
    # Korean
    # French
    # Spanish
    # Arabic
    # Turkish
    # Kurdish
    set peace {
	"#<0x00>和平"
	"#<0x20>शांति"
	"#<0x10>Peace"
	"#<0x10>שלום"
	"#<0x10>Мир"
	"#<0x10>Friede"
	"#<0x30>평화"
	"#<0x10>Paix"
	"#<0x10>Paz"
	"#<0x10>ﺳﻼم"
	"#<0x10>Barış"
	"#<0x10>Hasîtî"
    }

    $w cmd pladv 0
    $w cmd plvpor 0.0 1.0 0.0 1.0
    $w cmd plwind 0.0 1.0 0.0 1.0
    $w cmd plcol0 0
    $w cmd plbox "" 1.0 0 "" 1.0 0

    $w cmd plscmap0n 7
    $w cmd plscmap0 red green blue

    $w cmd plschr 0 4.0
    $w cmd plfont 1

    for {set i 0} {$i < 4} {incr i} {
	$w cmd plcol0 [expr {$i + 1}]
	$w cmd plfill px py

	for {set j 0} {$j < 4} {incr j} {
	    py $j = [expr {[py $j] + 1.0 / 4.0}]

	}
    }

    $w cmd plcol0 0
    set i 0
    foreach pl $peace {
	$w cmd plptex [sx $i] [sy $i] 1.0 0.0 0.5 $pl
	set i [expr {$i+1}]
    }

}
