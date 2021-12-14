# data/CMakeLists.txt for PLplot
###
### Process this file with cmake to produce Makefile
###
# Copyright (C) 2006 Alan W. Irwin
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

set(data_DATAFILES
  plstnd5.fnt
  plxtnd5.fnt
  cmap0_default.pal
  cmap0_alternate.pal
  cmap0_white_bg.pal
  cmap0_black_on_white.pal
  cmap1_default.pal
  cmap1_blue_yellow.pal
  cmap1_blue_red.pal
  cmap1_gray.pal
  cmap1_radar.pal
  cmap1_highfreq.pal
  cmap1_lowfreq.pal
  )

if(HAVE_SHAPELIB)
  set(data_DOCFILES
    README.shapefiles
    )

  set(data_DATAFILES
    ${data_DATAFILES}
    cglobe.shx
    cglobe.shp
    globe.shx
    globe.shp
    usa.shx
    usa.shp
    usaglobe.shx
    usaglobe.shp
    )


  set(ss_data_DATAFILES
    ss/os_open_conditions.txt
    ss/ss64ne_Building_Area.dbf
    ss/ss64ne_Building_Area.prj
    ss/ss64ne_Building_Area.shp
    ss/ss64ne_Building_Area.shx
    ss/ss64ne_General_Text.dbf
    ss/ss64ne_General_Text.prj
    ss/ss64ne_General_Text.shp
    ss/ss64ne_General_Text.shx
    ss/ss64ne_Height_Contours.dbf
    ss/ss64ne_Height_Contours.prj
    ss/ss64ne_Height_Contours.shp
    ss/ss64ne_Height_Contours.shx
    ss/ss64ne_Landform_Area.dbf
    ss/ss64ne_Landform_Area.prj
    ss/ss64ne_Landform_Area.shp
    ss/ss64ne_Landform_Area.shx
    ss/ss64ne_Landform_Line.dbf
    ss/ss64ne_Landform_Line.prj
    ss/ss64ne_Landform_Line.shp
    ss/ss64ne_Landform_Line.shx
    ss/ss64ne_Road_Centreline.dbf
    ss/ss64ne_Road_Centreline.prj
    ss/ss64ne_Road_Centreline.shp
    ss/ss64ne_Road_Centreline.shx
    ss/ss64ne_Water_Area.dbf
    ss/ss64ne_Water_Area.prj
    ss/ss64ne_Water_Area.shp
    ss/ss64ne_Water_Area.shx
    ss/ss64ne_Water_Line.dbf
    ss/ss64ne_Water_Line.prj
    ss/ss64ne_Water_Line.shp
    ss/ss64ne_Water_Line.shx
    )
endif(HAVE_SHAPELIB)

install(FILES ${data_DATAFILES} DESTINATION ${DATA_DIR})

if(HAVE_SHAPELIB)
  install(FILES ${data_DOCFILES} DESTINATION ${DOC_DIR})
  install(FILES ${ss_data_DATAFILES} DESTINATION ${DATA_DIR}/ss)
endif(HAVE_SHAPELIB)
