Instructions to create the plplot module
========================================

* Download the D compiler (version 1.028) from http://www.digitalmars.com/d/download.html
* Download the digitalmars C/C++ compiler from http://www.digitalmars.com/download/freecompiler.html
* Unzip them and set the PATH variable accordingly
* Download the htod tool from http://www.digitalmars.com/d/2.0/htod.html and put it in the dm/bin directory
* cd into the plplot/include directory
* edit the plplot.h file:
  - remove the first 136 lines
  - add
      #define PLDLLIMPEXP
      #define PLDLLIMPEXP_DATA(type) type
    before the rest of the header file
  - remove the plgfile(FILE **p_file); and plsfile(FILE *file); declarations (around line 1600)
* run "htod plplot.h plplot.d -hc -DDOUBLE"
* remove
    alias DBL_MAX PLFLT_MAX;
    alias DBL_MIN PLFLT_MIN;
  from the plplot.d file (right at the top).
* needed to add the following lines at about line 880, since they were not converted
    const DRAW_LINEX=1<<0; /* draw lines parallel to the X axis */
    const DRAW_LINEY=1<<1; /* draw lines parallel to the Y axis */
    const DRAW_LINEXY=DRAW_LINEX|DRAW_LINEY; /* draw lines parallel to both the X and Y axis */
    const MAG_COLOR=1<<2; /* draw the mesh with a color dependent of the magnitude */
    const BASE_CONT=1<<3; /* draw contour plot at bottom xy plane */
    const TOP_CONT=1<<4; /* draw contour plot at top xy plane */
    const SURF_CONT=1<<5; /* draw contour plot at surface */
    const DRAW_SIDES=1<<6; /* draw sides */
    const FACETED=1<<7; /* draw outline for each square that makes up the surface */
    const MESH=1<<8; /* draw mesh */

Compile examples
================
* Download and install the gdc compiler (only tested with this compiler)
  - Linux: http://dgcc.sourceforge.net/
  - Ubuntu: sudo apt-get install gdc
  - Windows: http://gdcwin.sourceforge.net/ (You need the MinGW compiler 3.4.5 installed as well)
  - Mac OS X: http://gdcmac.sourceforge.net/
* cd into the plplot/examples/d directory
* MinGW: gdc x07d.d -I ..\..\bindings\d path_to_build_directory\dll\libplplotd.dll.a -o x07d
* Linux: gdc x06d.d -I../../bindings/d path_to_build_directory/src/libplplotd.so -o x06d
