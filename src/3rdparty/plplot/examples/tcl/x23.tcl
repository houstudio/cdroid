#   Displays Greek letters and mathematically interesting Unicode ranges
#   Copyright (C) 2005,2008 Alan Irwin
#   Copyright (C) 2005,2008 Andrew Ross
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
# Displays Greek letters and mathematically interesting Unicode ranges
#

proc x23 {{w loopback}} {

    set Greek0 "#gA"
    set Greek1 "#gB"
    set Greek2 "#gG"
    set Greek3 "#gD"
    set Greek4 "#gE"
    set Greek5 "#gZ"
    set Greek6 "#gY"
    set Greek7 "#gH"
    set Greek8 "#gI"
    set Greek9 "#gK"
    set Greek10 "#gL"
    set Greek11 "#gM"
    set Greek12 "#gN"
    set Greek13 "#gC"
    set Greek14 "#gO"
    set Greek15 "#gP"
    set Greek16 "#gR"
    set Greek17 "#gS"
    set Greek18 "#gT"
    set Greek19 "#gU"
    set Greek20 "#gF"
    set Greek21 "#gX"
    set Greek22 "#gQ"
    set Greek23 "#gW"
    set Greek24 "#ga"
    set Greek25 "#gb"
    set Greek26 "#gg"
    set Greek27 "#gd"
    set Greek28 "#ge"
    set Greek29 "#gz"
    set Greek30 "#gy"
    set Greek31 "#gh"
    set Greek32 "#gi"
    set Greek33 "#gk"
    set Greek34 "#gl"
    set Greek35 "#gm"
    set Greek36 "#gn"
    set Greek37 "#gc"
    set Greek38 "#go"
    set Greek39 "#gp"
    set Greek40 "#gr"
    set Greek41 "#gs"
    set Greek42 "#gt"
    set Greek43 "#gu"
    set Greek44 "#gf"
    set Greek45 "#gx"
    set Greek46 "#gq"
    set Greek47 "#gw"


    matrix Type1 i 166 = {
	0x0020, 0x0021, 0x0023, 0x0025, 0x0026,
	0x0028, 0x0029, 0x002b, 0x002c, 0x002e,
	0x002f, 0x0030, 0x0031, 0x0032, 0x0033,
	0x0034, 0x0035, 0x0036, 0x0037, 0x0038,
	0x0039, 0x003a, 0x003b, 0x003c, 0x003d,
	0x003e, 0x003f, 0x005b, 0x005d, 0x005f,
	0x007b, 0x007c, 0x007d, 0x00a9, 0x00ac,
	0x00ae, 0x00b0, 0x00b1, 0x00d7, 0x00f7,
	0x0192, 0x0391, 0x0392, 0x0393, 0x0394,
	0x0395, 0x0396, 0x0397, 0x0398, 0x0399,
	0x039a, 0x039b, 0x039c, 0x039d, 0x039e,
	0x039f, 0x03a0, 0x03a1, 0x03a3, 0x03a4,
	0x03a5, 0x03a6, 0x03a7, 0x03a8, 0x03a9,
	0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5,
	0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba,
	0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
	0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4,
	0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9,
	0x03d1, 0x03d2, 0x03d5, 0x03d6, 0x2022,
	0x2026, 0x2032, 0x2033, 0x203e, 0x2044,
	0x2111, 0x2118, 0x211c, 0x2122, 0x2126,
	0x2135, 0x2190, 0x2191, 0x2192, 0x2193,
	0x2194, 0x21b5, 0x21d0, 0x21d1, 0x21d2,
	0x21d3, 0x21d4, 0x2200, 0x2202, 0x2203,
	0x2205, 0x2206, 0x2207, 0x2208, 0x2209,
	0x220b, 0x220f, 0x2211, 0x2212, 0x2215,
	0x2217, 0x221a, 0x221d, 0x221e, 0x2220,
	0x2227, 0x2228, 0x2229, 0x222a, 0x222b,
	0x2234, 0x223c, 0x2245, 0x2248, 0x2260,
	0x2261, 0x2264, 0x2265, 0x2282, 0x2283,
	0x2284, 0x2286, 0x2287, 0x2295, 0x2297,
	0x22a5, 0x22c5, 0x2320, 0x2321, 0x2329,
	0x232a, 0x25ca, 0x2660, 0x2663, 0x2665,
	0x2666
    }

    set title0 "#<0x10>PLplot Example 23 - Greek Letters"
    set title1 "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (a)"
    set title2 "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (b)"
    set title3 "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (c)"
    set title4 "#<0x10>PLplot Example 23 - Number Forms Unicode Block"
    set title5 "#<0x10>PLplot Example 23 - Arrows Unicode Block (a)"
    set title6 "#<0x10>PLplot Example 23 - Arrows Unicode Block (b)"
    set title7 "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (a)"
    set title8 "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (b)"
    set title9 "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (c)"
    set title10 "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (d)"


    matrix lo i 11 = {
	0x0,
	0x0,
	0x40,
	0x80,
	0x2153,
	0x2190,
	0x21d0,
	0x2200,
	0x2240,
	0x2280,
	0x22c0
    }

    matrix hi i 11 = {
	0x30,
	0x40,
	0x80,
	0xA6,
	0x2184,
	0x21d0,
	0x2200,
	0x2240,
	0x2280,
	0x22c0,
	0x2300
    }

    matrix nxcells i 11 = {
	12,
	8,
	8,
	8,
	8,
	8,
	8,
	8,
	8,
	8,
	8
    }

    matrix nycells i 11 = {
	8,
	8,
	8,
	8,
	8,
	8,
	8,
	8,
	8,
	8,
	8
    }

    # non-zero values Must be consistent with nxcells and nycells.
    matrix offset i 11 =  {
 	0,
	0,
	64,
	128,
	0,
	0,
	0,
	0,
	0,
	0,
	0
    }

# 30 possible FCI values.
    matrix fci i 30 = {
	0x80000000,
	0x80000001,
	0x80000002,
	0x80000003,
	0x80000004,
	0x80000010,
	0x80000011,
	0x80000012,
	0x80000013,
	0x80000014,
	0x80000020,
	0x80000021,
	0x80000022,
	0x80000023,
	0x80000024,
	0x80000100,
	0x80000101,
	0x80000102,
	0x80000103,
	0x80000104,
	0x80000110,
	0x80000111,
	0x80000112,
	0x80000113,
	0x80000114,
	0x80000120,
	0x80000121,
	0x80000122,
	0x80000123,
	0x80000124,
    }

    set family(0) "sans-serif"
    set family(1) "serif"
    set family(2) "monospace"
    set family(3) "script"
    set family(4) "symbol"

    set style(0) "upright"
    set style(1) "italic"
    set style(2) "oblique"

    set weight(0) "medium"
    set weight(1) "bold"

    for {set page 0} {$page<11} {incr page} {

	$w cmd pladv 0

	# Set up viewport and window

	$w cmd plvpor 0.02 0.98 0.02 0.90
	$w cmd plwind 0.0 1.0 0.0 1.0
	$w cmd plgspa xmin xmax ymin ymax
	$w cmd plschr 0.0 0.8
	set ycharacter_scale [ expr {1.0/($ymax-$ymin)} ]

	# Factor should be 0.5, but heuristically it turns out to be larger.
	$w cmd plgchr chardef charht
	set yoffset [ expr {1.0*$charht*$ycharacter_scale} ]

	# Draw the grid using plbox

	$w cmd plcol0 2
	set deltax [ expr {1.0/[nxcells $page]} ]
	set deltay [ expr {1.0/[nycells $page]} ]
	$w cmd plbox "bcg" $deltax 0 "bcg" $deltay 0
	$w cmd plcol0 15
	set length [ expr {[hi $page]-[lo $page]} ]
	set slice 0
	for {set j [ expr {[nycells $page]-1} ]} {$j>=-1} {incr j -1} {
	    set y [ expr {(0.5+$j)*$deltay} ]
	    for {set i 0} {$i<[nxcells $page]} {incr i} {
	        set x [ expr {(0.5+$i)*$deltax} ]
		if {$slice < $length} {
		    if {$page == 0} {
		        set cmdString [format "#%s" [set Greek$slice]]
	            } elseif {$page <= 3} {
			set cmdString [format "##\[0x%.4x\]" [ Type1 [expr {[offset $page]+$slice}] ] ]
		    } else {
			set cmdString [format "##\[0x%.4x\]" [expr {[lo $page]+$slice}]]
	            }
		    $w cmd plptex $x [ expr {$y+$yoffset}] 1. 0. 0.5 [ string range $cmdString 1 end ]
		    $w cmd plptex $x [ expr {$y-$yoffset}] 1. 0. 0.5 $cmdString
		}
		incr slice
            }
	}

	$w cmd plschr 0. 1.0
	# Page title
	$w cmd plmtex "t" 1.5 0.5 0.5 [set title$page]
    }

    # Demonstrate methods of getting the current fonts
    $w cmd plgfci fci_old
    $w cmd plgfont ifamily istyle iweight
    puts [ format "For example 23 prior to page 12 the FCI is 0x%x" $fci_old ]
    puts [ format "For example 23 prior to page 12 the font family, style and weight are %s %s %s" \
	       $family($ifamily) $style($istyle) $weight($iweight) ]

    for {set page 11} {$page<16} {incr page} {
	set dy 0.030

	$w cmd pladv 0
	$w cmd plvpor 0.02 0.98 0.02 0.90
	$w cmd plwind 0.0 1.0 0.0 1.0
	$w cmd plsfci 0
	if {$page == 11} {
	    $w cmd plmtex "t" 1.5 0.5 0.5 "#<0x10>PLplot Example 23 - Set Font with plsfci"
	} elseif {$page == 12} {
	    $w cmd plmtex "t" 1.5 0.5 0.5 "#<0x10>PLplot Example 23 - Set Font with plsfont"
	} elseif {$page == 13} {
	    $w cmd plmtex "t" 1.5 0.5 0.5 "#<0x10>PLplot Example 23 - Set Font with ##<0x8nnnnnnn> construct"
	} elseif {$page == 14} {
	    $w cmd plmtex "t" 1.5 0.5 0.5 "#<0x10>PLplot Example 23 - Set Font with ##<0xmn> constructs"
	} elseif {$page == 15} {
	    $w cmd plmtex "t" 1.5 0.5 0.5 "#<0x10>PLplot Example 23 - Set Font with ##<FCI COMMAND STRING/> constructs"
	}
	$w cmd plschr 0. 0.75
	for {set i 0} {$i< 30} {incr i} {
	    set family_index [ expr {$i % 5}]
	    set style_index [ expr {($i/5) % 3}]
	    set weight_index [ expr {(($i/5)/3) % 2}]
	    if {$page == 11} {
		$w cmd plsfci [fci $i]
		set string [format "Page 12, %s, %s, %s:  The quick brown fox jumps over the lazy dog" \
				$family($family_index) \
				$style($style_index) \
				$weight($weight_index) \
			   ]
	    } elseif {$page == 12} {
		$w cmd plsfont $family_index $style_index $weight_index
		set string [format \
				"Page 13, %s, %s, %s:  The quick brown fox jumps over the lazy dog" \
				$family($family_index) \
				$style($style_index) \
				$weight($weight_index) \
			   ]
	    } elseif {$page == 13} {
		set string [format \
				"Page 14, %s, %s, %s:  #<0x%x>The quick brown fox jumps over the lazy dog" \
				$family($family_index) \
				$style($style_index) \
				$weight($weight_index) \
				[fci $i] \
			       ]
	    } elseif {$page == 14} {
		set string [format \
				"Page 15, %s, %s, %s:  #<0x%1x0>#<0x%1x1>#<0x%1x2>The quick brown fox jumps over the lazy dog" \
				$family($family_index) \
				$style($style_index) \
				$weight($weight_index) \
				$family_index \
				$style_index \
				$weight_index \
			       ]
	    } elseif {$page == 15} {
		set string [format \
				"Page 16, %s, %s, %s:  #<%s/>#<%s/>#<%s/>The quick brown fox jumps over the lazy dog" \
				$family($family_index) \
				$style($style_index) \
				$weight($weight_index) \
				$family($family_index) \
				$style($style_index) \
				$weight($weight_index) \
			       ]
	    }
	    $w cmd plptex 0. [ expr {1. - ($i+0.5)*$dy}] 1. 0. 0. $string
       }

       $w cmd plschr 0. 1.0
    }

    # Restore defaults
    $w cmd plcol0 1
}
