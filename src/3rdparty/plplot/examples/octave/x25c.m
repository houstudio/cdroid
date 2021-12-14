##	Bar chart demo.
##

##--------------------------------------------------------------------------
## main
##
## Test program for filling polygons and proper clipping
##--------------------------------------------------------------------------

1;

function ix25c
  npts = 0;

## Parse and process command line arguments

## plparseopts(&argc, argv, PL_PARSE_FULL);

## Initialize plplot

  plssub(3,3);
  plinit;

  xextreme = [-120,120;-120,120;-120,120;-80,80;-220,-120;-20,20;-20,20;-80,80;20,120];
  yextreme = [-120,120;20,120;-20,120;-20,120;-120,120;-120,120;-20,20;-80,80;-120,120];

  for k=0:1
    for j=0:3
      if ( j == 0 )
	## Polygon 1: a diamond
	x0 = [0;-100;0;100];
	y0 = [-100;0;100;0];
	npts = 4 ;
      endif
      if ( j == 1 )
	## Polygon 1: a diamond - reverse direction
	x0 = [100;0;-100;0];
	y0 = [0;100;0;-100];
	npts = 4 ;
      endif
      if ( j == 2 )
	## Polygon 2: a square with punctures
	x0 = [-100;-100;80;-100;-100;-80;0;80;100;100];
	y0 = [-100;-80;0;80;100;100;80;100;100;-100];
	npts = 10 ;
      endif
      if ( j == 3 )
	## Polygon 2: a square with punctures - reversed direction
	x0 = [100;100;80;0;-80;-100;-100;80;-100;-100];
	y0 = [-100;100;100;80;100;100;80;0;-80;-100];
	npts = 10 ;
      endif
      for i = 1:9
	pladv(0);
	plvsta;
	plwind(xextreme(i,1), xextreme(i,2), yextreme(i,1), yextreme(i,2));

	plcol0(2) ;
	plbox("bc", 1.0, 0, "bcnv", 10.0, 0);
	plcol0(1) ;
	plpsty(0) ;
	if (k == 0)
	  plfill(x0,y0) ;
	else
	  plgradient(x0,y0,45.) ;
	endif
	plcol0(2) ;
	pllsty(1) ;
	plline(x0,y0) ;
      endfor
    endfor
  endfor

## Don't forget to call plend() to finish off!

    plend1;

end

ix25c
