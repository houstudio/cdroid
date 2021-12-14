##  Copyright (C) 2008-2016 Alan W. Irwin
##  Copyright (C) 2008 Andrew Ross
##
##  set/get tester
##
##  This file is part of PLplot.
##
##  PLplot is free software; you can redistribute it and/or modify
##  it under the terms of the GNU Library General Public License as published
##  by the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  PLplot is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU Library General Public License for more details.
##
##  You should have received a copy of the GNU Library General Public License
##  along with PLplot; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
##

1;

function status = ix31c(strm)

  global PL_NOTSET
  # Redirect output to file if required
  if (nargin == 0)
    strm = stdout;
  endif


  ## Clear status flag to begin with
  status = 0;

  r1 = [0, 255];
  g1 = [255, 0];
  b1 = [0, 0];
  a1 = [1.0, 1.0];

  ## Parse and process command line arguments

  ## (void) plparseopts(&argc, argv, PL_PARSE_FULL);

  ## Test setting / getting familying parameters before plinit.
  ## Save values set by plparseopts to be restored later.
  [fam0, num0, bmax0] = plgfam();
  fam1 = 0;
  num1 = 10;
  bmax1 = 1000;
  plsfam(fam1, num1, bmax1);

  ## Retrieve the same values?
  [fam2, num2, bmax2] = plgfam();
  fprintf(strm,"family parameters: fam, num, bmax = %d %d %d\n", fam2, num2, bmax2);
  if (fam2 != fam1 || num2 != num1 || bmax2 != bmax1)
    fputs(stderr, "plgfam test failed\n");
    status = 1;
  endif
  ## Restore values set initially by plparseopts.
  plsfam(fam0, num0, bmax0);

  ## Test setting / getting page parameters before plinit/
  ## Save values set by plparseopts to be restored later.
  [xp0, yp0, xleng0, yleng0, xoff0, yoff0] = plgpage();
  xp1 = 200.;
  yp1 = 200.;
  xleng1 = 400;
  yleng1 = 200;
  xoff1 = 10;
  yoff1 = 20;
  plspage(xp1, yp1, xleng1, yleng1, xoff1, yoff1);

  ## Retrieve the same values?
  [xp2, yp2, xleng2, yleng2, xoff2, yoff2] = plgpage();
  fprintf(strm,"page parameters: xp, yp, xleng, yleng, xoff, yoff = %f %f %d %d %d %d\n", xp2, yp2, xleng2, yleng2, xoff2, yoff2);
  if (xp2 != xp1 || yp2 != yp1 || xleng2 != xleng1 || yleng2 != yleng1 ||
      xoff2 != xoff1 || yoff2 != yoff1 )
    fputs(stderr,"plgpage test failed\n");
    status = 1;
  endif
  ## Restore values set initially by plparseopts.
  plspage(xp0, yp0, xleng0, yleng0, xoff0, yoff0);

  ## Test setting / getting compression parameter across plinit.
  compression1 = 95;
  plscompression(compression1);

  ## Initialize plplot

  plinit();

  ## Test if device initialization screwed around with the preset
  ## compression parameter.
  compression2 = plgcompression();
  fprintf(strm,"Output various PLplot parameters\n");
  fprintf(strm,"compression parameter = %d\n", compression2);
  if (compression2 != compression1)
    fputs(stderr, "plgcompression test failed\n");
    status = 1;
  endif

  ## Exercise plscolor, plscol0, plscmap1, and plscmap1a to make sure
  ## they work without any obvious error messages.

  plscolor(1);

  plscol0(1, 255, 0, 0);

  plscmap1(r1',g1',b1');
  plscmap1a(r1',g1',b1',a1');

  level2 = plglevel();
  fprintf(strm,"level parameter = %d\n", level2);
  if (level2 != 1)
    fprintf(stderr,"plglevel test failed. Level is %d, but 1 expected.\n",level);
    status = 1;
  endif

  pladv(0);
  xmin0 = 0.01;
  xmax0 = 0.99;
  ymin0 = 0.02;
  ymax0 = 0.49;
  plvpor(xmin0, xmax0, ymin0, ymax0);
  [xmin, xmax, ymin, ymax] = plgvpd();
  fprintf(strm,"plvpor: xmin, xmax, ymin, ymax = %f %f %f %f\n", xmin, xmax, ymin, ymax);
  if (xmin != xmin0 || xmax != xmax0 || ymin != ymin0 || ymax != ymax0)
    fputs(stderr,"plgvpd test failed\n");
    status = 1;
  endif
  xmid = 0.5*(xmin+xmax);
  ymid = 0.5*(ymin+ymax);

  xmin0 = 0.2;
  xmax0 = 0.3;
  ymin0 = 0.4;
  ymax0 = 0.5;
  plwind(xmin0, xmax0, ymin0, ymax0);
  [xmin, xmax, ymin, ymax] = plgvpw();
  fprintf(strm,"plwind: xmin, xmax, ymin, ymax = %f %f %f %f\n", xmin, xmax, ymin, ymax);
  if (xmin != xmin0 || xmax != xmax0 || ymin != ymin0 || ymax != ymax0)
    fputs(stderr,"plgvpw test failed\n");
    status = 1;
  endif


  ## Get world coordinates for middle of viewport
  [wx,wy,win] = plcalc_world(xmid,ymid);
  fprintf(strm,"world parameters: wx, wy, win = %f %f %d\n", wx, wy, win);
  if (abs(wx-0.5*(xmin+xmax))>1.0E-5 || abs(wy-0.5*(ymin+ymax))>1.0E-5)
    fputs(stderr,"plcalc_world test failed\n");
    status = 1;
  endif

  ## Retrieve and print the name of the output file (if any)
  ## This goes to stderr not stdout since it will vary between tests and
  ## we want stdout to be identical for compare test.
  fnam = plgfnam();
  if (fnam(1) == "\0")
    fputs(strm,"No output file name is set\n");
  else
    fputs(strm,"Output file name read\n");
  endif
  fprintf(stderr,"Output file name is %s\n",fnam);

  ## Set and get the number of digits used to display axis labels
  ## Note digits is currently ignored in pls[xyz]ax and
  ## therefore it does not make sense to test the returned
  ## value.
  plsxax(3,0);
  [digmax,digits] = plgxax();
  fprintf(strm,"x axis parameters: digmax, digits = %d %d\n", digmax, digits);
  if (digmax != 3)
    fputs(stderr,"plgxax test failed\n");
    status = 1;
    return;
  endif

  plsyax(4,0);
  [digmax,digits] = plgyax();
  fprintf(strm,"y axis parameters: digmax, digits = %d %d\n", digmax, digits);
  if (digmax != 4)
    fputs(stderr,"plgyax test failed\n");
    status = 1;
  endif

  plszax(5,0);
  [digmax,digits] = plgzax();
  fprintf(strm,"z axis parameters: digmax, digits = %d %d\n", digmax, digits);
  if (digmax != 5)
    fputs(stderr,"plgzax test failed\n");
    status = 1;
  endif

  mar0 = 0.05;
  aspect0 = PL_NOTSET;
  jx0 = 0.1;
  jy0 = 0.2;
  plsdidev(mar0, aspect0, jx0, jy0);
  [mar, aspect, jx, jy] = plgdidev();
  fprintf(strm,"device-space window parameters: mar, aspect, jx, jy = %f %f %f %f\n" , mar, aspect, jx, jy);
  if (mar != mar0 || jx != jx0 || jy != jy0)
    fputs(stderr,"plgdidev test failed\n");
    status = 1;
  endif

  ori0 = 1.0;
  plsdiori(ori0);
  ori = plgdiori();
  fprintf(strm,"ori parameter = %f\n", ori);
  if (ori != ori0)
    fputs(stderr,"plgdiori test failed\n");
    status = 1;
  endif

  xmin0 = 0.1;
  ymin0 = 0.2;
  xmax0 = 0.9;
  ymax0 = 0.8;
  plsdiplt(xmin0, ymin0, xmax0, ymax0);
  [xmin, ymin, xmax, ymax] = plgdiplt();
  fprintf(strm,"plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f\n", xmin, ymin, xmax, ymax);
  if (xmin != xmin0 || ymin != ymin0 || xmax != xmax0 || ymax != ymax0)
    fputs(stderr,"plgdiplt test failed\n");
    status = 1;
  endif

  zxmin0 = 0.1;
  zymin0 = 0.1;
  zxmax0 = 0.9;
  zymax0 = 0.9;
  plsdiplz(zxmin0, zymin0, zxmax0, zymax0);
  [zxmin, zymin, zxmax, zymax] = plgdiplt();
  fprintf(strm,"zoomed plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f\n", zxmin, zymin, zxmax, zymax);
  if ( abs(zxmin -(xmin + (xmax-xmin)*zxmin0)) > 1.0E-5 || abs(zymin -(ymin+(ymax-ymin)*zymin0)) > 1.0E-5 || abs(zxmax -(xmin+(xmax-xmin)*zxmax0)) > 1.0E-5 || abs(zymax -(ymin+(ymax-ymin)*zymax0)) > 1.0E-5 )
    fputs(stderr,"plsdiplz test failed\n");
    status = 1;
  endif

  r0 = 10;
  g0 = 20;
  b0 = 30;
  plscolbg(r0, g0, b0);
  [r, g, b] = plgcolbg();
  fprintf(strm,"background colour parameters: r, g, b = %d %d %d\n", r, g, b);
  if (r != r0 || g != g0 || b != b0)
    fputs(stderr,"plgcolbg test failed\n");
    status = 1;
  endif

  r0 = 20;
  g0 = 30;
  b0 = 40;
  a0 = 0.5;
  plscolbga(r0, g0, b0, a0);
  [r, g, b, a] = plgcolbga();
  fprintf(strm,"background/transparency colour parameters: r, g, b, a = %d %d %d %f\n", r, g, b, a);
  if (r != r0 || g != g0 || b != b0 || a != a0)
    fputs(stderr,"plgcolbga test failed\n");
    status = 1;
  endif

  plend();

endfunction

if (exist("strm","var"))
  ix31c(strm);
else
  ix31c();
endif
