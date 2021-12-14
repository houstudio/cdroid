# bindings/octave/PLplot/CMakeLists.txt for PLplot
###
### Process this file with cmake to produce Makefile
###
# Copyright (C) 2006-2017 Alan W. Irwin
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

# Install octave support (in support subdirectory), binding "m", and user "m"
# files.

add_subdirectory(support)

set(binding_m_files
arrow.m
arrows.m
autostyle.m
axis.m
axis_set.m
bgr.m
blue.m
bone.m
bottom_title.m
clip.m
closeallfig.m
closefig.m
colormap.m
comet.m
contour.m
cool.m
copper.m
drawnow.m
fig.m
fig_raise.m
fig_state.m
figure.m
fill.m
free_fig.m
ginput.m
green.m
grid.m
griddata.m
gtext.m
hls2rgb.m
hold.m
hot.m
hsv.m
ishold.m
label_plot.m
legend.m
loglog.m
lp_setup.m
mesh.m
meshc.m
meshz.m
mplot.m
multiplot.m
oneplot.m
peaks.m
pink.m
plclearplot.m
plclg.m
plcolormap.m
pldef.m
plimage.m
plot.m
plot3.m
plot_border.m
plot_margin.m
plrb.m
plsetopt.m
polar.m
prism.m
red.m
rgb2hls.m
rgbplot.m
save_fig.m
semilogx.m
semilogy.m
set_view.m
shade.m
shading.m
shg.m
stopdraw.m
stripc.m
stripc_add.m
stripc_del.m
struct_contains.m
subplot.m
subwindow.m
surf.m
surfc.m
surfl.m
tdeblank.m
text.m
title.m
top_title.m
xlabel.m
xticks.m
ylabel.m
yticks.m
zlabel.m
zoom.m
)
install(FILES ${binding_m_files} DESTINATION ${PLPLOT_OCTAVE_DIR})

configure_file(
${CMAKE_CURRENT_SOURCE_DIR}/plplot_octave_path.m.in
${CMAKE_CURRENT_BINARY_DIR}/plplot_octave_path.m
@ONLY
)
set(user_m_files
use_plplot.m
${CMAKE_CURRENT_BINARY_DIR}/plplot_octave_path.m
toggle_plplot_use.m
)
install(FILES ${user_m_files} DESTINATION ${OCTAVE_M_DIR})
