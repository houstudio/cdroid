# www/css/CMakeLists.txt to help generate the PLplot project website.

# Copyright (C) 2008-2018 Alan W. Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# Variables to control configuration of CSS files.

# Colour scheme for menu section is mostly done by fixed gradient
# background images.  Thus configuring the non-menu colours is tricky since
# the result must be reasonably consistent with menu.  Because of this
# limitation I (AWI) ultimately decided to configure just one colour
# (the dark text colour).

# Make dark text absolutely black for best visibility.
set(dark_text_color "#000000")

# Widths and heights:

# N.B. all configured widths and heights are expressed in percentages
# appropriate for the liquid style sheet.  To convert to static style sheet
# numbers simply multiply all these numbers primarily by 800/100 but
# sometimes by other factors when percentages are nested.
set(per_cent_width_pagewrapper 95)
set(per_cent_width_rightside 23)
set(per_cent_width_fullside 100)
set(per_cent_width_leftside 75)
set(per_cent_width_featurebox 31)
set(per_cent_height_pagefooter 8)
set(per_cent_width_codebox 92)
set(per_cent_width_searchbar 14)

# Liquid style:
set(width_pagewrapper "${per_cent_width_pagewrapper}%")
set(width_rightside "${per_cent_width_rightside}%")
set(width_fullside "${per_cent_width_fullside}%")
set(width_leftside "${per_cent_width_leftside}%")
set(width_featurebox "${per_cent_width_featurebox}%")
set(height_pagefooter "${per_cent_height_pagefooter}%")
set(width_codebox "${per_cent_width_codebox}%")
set(width_searchbar "${per_cent_width_searchbar}%")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/style.css.in
  ${CMAKE_CURRENT_BINARY_DIR}/style_liquid.css @ONLY
  )

# static style:
math(EXPR static_width_pagewrapper "${per_cent_width_pagewrapper}*(800/100)")
math(EXPR static_width_rightside "${per_cent_width_rightside}*(700/100)")
math(EXPR static_width_fullside "${per_cent_width_fullside}*(700/100)")
math(EXPR static_width_leftside "${per_cent_width_leftside}*(700/100)")
math(EXPR static_width_featurebox "${per_cent_width_featurebox}*(800/100)")
math(EXPR static_height_pagefooter "${per_cent_height_pagefooter}*(800/100)")
math(EXPR static_width_codebox "${per_cent_width_codebox}*(700/100)")
math(EXPR static_width_searchbar "${per_cent_width_searchbar}*(800/100)")

set(width_pagewrapper "${static_width_pagewrapper}px")
set(width_rightside "${static_width_rightside}px")
set(width_fullside "${static_width_fullside}px")
set(width_leftside "${static_width_leftside}px")
set(width_featurebox "${static_width_featurebox}px")
set(height_pagefooter "${static_height_pagefooter}px")
set(width_codebox "${static_width_codebox}px")
set(width_searchbar "${static_width_searchbar}px")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/style.css.in
  ${CMAKE_CURRENT_BINARY_DIR}/style_static.css @ONLY
  )

