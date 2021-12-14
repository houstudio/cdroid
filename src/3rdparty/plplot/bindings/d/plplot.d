// Converted to D from plplot_d.h by htod
module plplot;

private import std.string;
private import std.array;
private import std.algorithm;
private import std.stdio;
private import std.conv;

// improved D interface

// certain functions must be declared as C functions so that PLplot
// can handle them
extern ( C ) {
alias PLINT function( PLFLT, PLFLT ) def_func;
alias void function( PLINT, PLFLT*, PLFLT* ) fill_func;
alias void function( PLFLT, PLFLT, PLFLT*, PLFLT*, PLPointer ) pltr_func;
alias void function( PLINT, PLFLT*, PLFLT* ) mapform_func;
alias void function( PLFLT, PLFLT, PLFLT*, PLFLT*, PLPointer ) ct_func;
}

// D definition of PLcGrid and PLcGrid2
struct PLcGrid
{
    PLFLT[] xg;
    PLFLT[] yg;
    PLFLT[] zg;
}
struct PLcGrid2
{
    PLFLT[][] xg;
    PLFLT[][] yg;
    PLFLT[][] zg;
}

// helper function to convert D dynamic arrays in C dynamic arrays
private PLFLT** convert_array( PLFLT[][] a )
{
    if ( !a )
        return null;

    size_t nx = a.length;
    size_t ny = a[0].length;

    PLFLT  ** c_a = ( new PLFLT *[nx] ).ptr;
    for ( size_t i = 0; i < nx; i++ )
    {
        assert( ny == a[i].length, "convert_array(): Array must be 2 dimensional!" );
        c_a[i] = a[i].ptr;
    }

    return c_a;
}

// Process options list using current options info.
PLINT plparseopts( char[][] args, PLINT mode )
{
    char*[] c_args = new char*[args.length];
    foreach ( size_t i, char[] arg; args )
        c_args[i] = cast(char *) toStringz( arg );
    int argc = cast(int) c_args.length;
    return c_plparseopts( &argc, cast(char**) c_args, mode );
}

// simple arrow plotter.
void plvect( PLFLT[][] u, PLFLT[][] v, PLFLT scale, pltr_func pltr = null, PLPointer pltr_data = null )
{
    PLINT nx = cast(PLINT) u.length;
    PLINT ny = cast(PLINT) u[0].length;
    assert( nx == v.length, "plvect(): Arrays must be of same length!" );
    assert( ny == v[0].length, "plvect(): Arrays must be of same length!" );

    c_plvect( convert_array( u ), convert_array( v ), nx, ny, scale, pltr, pltr_data );
}

void plvect( PLFLT[][] u, PLFLT[][] v, PLFLT scale, ref PLcGrid cgrid )
{
    PLINT nx = cast(PLINT) u.length;
    PLINT ny = cast(PLINT) u[0].length;
    assert( nx == v.length, "plvect(): Arrays must be of same length!" );
    assert( ny == v[0].length, "plvect(): Arrays must be of same length!" );

    c_PLcGrid c;
    c.xg = cgrid.xg.ptr;
    c.nx = cast(PLINT) cgrid.xg.length;
    c.yg = cgrid.yg.ptr;
    c.ny = cast(PLINT) cgrid.yg.length;
    c.zg = cgrid.zg.ptr;
    c.nz = cast(PLINT) cgrid.zg.length;

    c_plvect( convert_array( u ), convert_array( v ), nx, ny, scale, &pltr1, &c );
}

void plvect( PLFLT[][] u, PLFLT[][] v, PLFLT scale, ref PLcGrid2 cgrid2 )
{
    PLINT nx = cast(PLINT) u.length;
    PLINT ny = cast(PLINT) u[0].length;
    assert( nx == v.length, "plvect(): Arrays must be of same length!" );
    assert( ny == v[0].length, "plvect(): Arrays must be of same length!" );

    c_PLcGrid2 c2;
    c2.xg = convert_array( cgrid2.xg );
    c2.yg = convert_array( cgrid2.yg );
    c2.zg = convert_array( cgrid2.zg );
    c2.nx = cast(PLINT) cgrid2.xg.length;
    c2.ny = cast(PLINT) cgrid2.xg[0].length;
    if ( cgrid2.yg )
    {
        assert( c2.nx == cgrid2.yg.length, "plvect(): Arrays must be of same length!" );
        assert( c2.ny == cgrid2.yg[0].length, "plvect(): Arrays must be of same length!" );
    }
    if ( cgrid2.zg )
    {
        assert( c2.nx == cgrid2.zg.length, "plvect(): Arrays must be of same length!" );
        assert( c2.ny == cgrid2.zg[0].length, "plvect(): Arrays must be of same length!" );
    }

    c_plvect( convert_array( u ), convert_array( v ), nx, ny, scale, &pltr2, &c2 );
}

void plsvect( PLFLT[] arrowx, PLFLT[] arrowy, PLBOOL fill )
{
    PLINT npts = cast(PLINT) arrowx.length;
    assert( npts == arrowy.length, "plsvect(): Arrays must be of same length!" );
    c_plsvect( arrowx.ptr, arrowy.ptr, npts, fill );
}

// This functions similarly to plbox() except that the origin of the axes
// is placed at the user-specified point (x0, y0).
void plaxes( PLFLT x0, PLFLT y0, string xopt, PLFLT xtick, PLINT nxsub,
             string yopt, PLFLT ytick, PLINT nysub )
{
    c_plaxes( x0, y0, toStringz( xopt ), xtick, nxsub, toStringz( yopt ), ytick, nysub );
}

// Plot a histogram using x to store data values and y to store frequencies
void plbin( PLFLT[] x, PLFLT[] y, PLINT opt )
{
    PLINT nbin = cast(PLINT) x.length;
    assert( nbin == y.length, "plbin(): Arrays must be of same length!" );
    c_plbin( nbin, x.ptr, y.ptr, opt );
}

// This draws a box around the current viewport.
void plbox( string xopt, PLFLT xtick, PLINT nxsub, string yopt, PLFLT ytick, PLINT nysub )
{
    c_plbox( toStringz( xopt ), xtick, nxsub, toStringz( yopt ), ytick, nysub );
}

// This is the 3-d analogue of plbox().
void plbox3( string xopt, string xlabel, PLFLT xtick, PLINT nsubx,
             string yopt, string ylabel, PLFLT ytick, PLINT nsuby,
             string zopt, string zlabel, PLFLT ztick, PLINT nsubz )
{
    c_plbox3( toStringz( xopt ), toStringz( xlabel ), xtick, nsubx,
        toStringz( yopt ), toStringz( ylabel ), ytick, nsuby,
        toStringz( zopt ), toStringz( zlabel ), ztick, nsubz );
}

// Routine for drawing continuous colour legends
void plcolorbar( PLFLT *p_colorbar_width, PLFLT *p_colorbar_height,
                 PLINT opt, PLINT position, PLFLT x, PLFLT y,
                 PLFLT x_length, PLFLT y_length,
                 PLINT bg_color, PLINT bb_color, PLINT bb_style,
                 PLFLT low_cap_color, PLFLT high_cap_color,
                 PLINT cont_color, PLFLT cont_width,
                 PLINT[] label_opts, string[] label,
                 string[] axis_opts,
                 PLFLT[] ticks, PLINT[] sub_ticks,
                 PLFLT[][] values )
{
    PLINT   n_labels = cast(PLINT) label_opts.length;
    PLINT   n_axes   = cast(PLINT) axis_opts.length;
    PLINT[] n_values = new PLINT[values.length];
    for ( size_t i = 0; i < values.length; i++ )
    {
        n_values[i] = cast(PLINT) values[i].length;
    }
    immutable( char ) * *labelz     = array( map!toStringz( label ) ).ptr;
    immutable( char ) * *axis_optsz = array( map!toStringz( axis_opts ) ).ptr;
    assert( n_labels == label.length, "plcolorbar(): Arrays must be of same length!" );
    assert( n_labels == label_opts.length, "plcolorbar(): Arrays must be of same length!" );
    assert( n_axes == axis_opts.length, "plcolorbar(): Arrays must be of same length!" );
    assert( n_axes == ticks.length, "plcolorbar(): Arrays must be of same length!" );
    assert( n_axes == sub_ticks.length, "plcolorbar(): Arrays must be of same length!" );

    c_plcolorbar( p_colorbar_width, p_colorbar_height,
        opt, position, x, y,
        x_length, y_length,
        bg_color, bb_color, bb_style,
        low_cap_color, high_cap_color,
        cont_color, cont_width,
        n_labels, label_opts.ptr, labelz,
        n_axes, axis_optsz,
        ticks.ptr, sub_ticks.ptr,
        n_values.ptr, convert_array( values ) );
}

// Draws a contour plot from data in f(nx,ny).  Is just a front-end to
// plfcont, with a particular choice for f2eval and f2eval_data.
//
void plcont( PLFLT[][] f, PLINT kx, PLINT lx, PLINT ky, PLINT ly, PLFLT[] clevel,
             pltr_func pltr, PLPointer pltr_data = null )
{
    PLINT nx = cast(PLINT) f.length;
    PLINT ny = cast(PLINT) f[0].length;

    c_plcont( convert_array( f ), nx, ny, kx, lx, ky, ly, clevel.ptr, cast(PLINT) clevel.length,
        pltr, pltr_data );
}

void plcont( PLFLT[][] f, PLINT kx, PLINT lx, PLINT ky, PLINT ly, PLFLT[] clevel,
             ref PLcGrid cgrid )
{
    PLINT     nx = cast(PLINT) f.length;
    PLINT     ny = cast(PLINT) f[0].length;

    c_PLcGrid c;
    c.xg = cgrid.xg.ptr;
    c.nx = cast(PLINT) cgrid.xg.length;
    c.yg = cgrid.yg.ptr;
    c.ny = cast(PLINT) cgrid.yg.length;
    c.zg = cgrid.zg.ptr;
    c.nz = cast(PLINT) cgrid.zg.length;

    c_plcont( convert_array( f ), nx, ny, kx, lx, ky, ly, clevel.ptr, cast(PLINT) clevel.length,
        &pltr1, &c );
}

void plcont( PLFLT[][] f, PLINT kx, PLINT lx, PLINT ky, PLINT ly, PLFLT[] clevel,
             ref PLcGrid2 cgrid2 )
{
    PLINT      nx = cast(PLINT) f.length;
    PLINT      ny = cast(PLINT) f[0].length;

    c_PLcGrid2 c2;
    c2.xg = convert_array( cgrid2.xg );
    c2.yg = convert_array( cgrid2.yg );
    c2.zg = convert_array( cgrid2.zg );
    c2.nx = cast(PLINT) cgrid2.xg.length;
    c2.ny = cast(PLINT) cgrid2.xg[0].length;
    if ( cgrid2.yg )
    {
        assert( c2.nx == cgrid2.yg.length, "plcont(): Arrays must be of same length!" );
        assert( c2.ny == cgrid2.yg[0].length, "plcont(): Arrays must be of same length!" );
    }
    if ( cgrid2.zg )
    {
        assert( c2.nx == cgrid2.zg.length, "plcont(): Arrays must be of same length!" );
        assert( c2.ny == cgrid2.zg[0].length, "plcont(): Arrays must be of same length!" );
    }

    c_plcont( convert_array( f ), nx, ny, kx, lx, ky, ly, clevel.ptr, cast(PLINT) clevel.length,
        &pltr2, &c2 );
}

// Draws a contour plot using the function evaluator f2eval and data stored
// by way of the f2eval_data pointer.  This allows arbitrary organizations
// of 2d array data to be used.
//
//void  plfcont(PLFLT  function(PLINT , PLINT , PLPointer )f2eval, PLPointer f2eval_data, PLINT nx, PLINT ny, PLINT kx, PLINT lx, PLINT ky, PLINT ly, PLFLT *clevel, PLINT nlevel, void  function(PLFLT , PLFLT , PLFLT *, PLFLT *, PLPointer )pltr, PLPointer pltr_data);

// Plot horizontal error bars (xmin(i),y(i)) to (xmax(i),y(i))
void plerrx( PLFLT[] xmin, PLFLT[] xmax, PLFLT[] y )
{
    PLINT n = cast(PLINT) y.length;
    assert( n == xmin.length, "plerrx(): Arrays must be of same length!" );
    assert( n == xmax.length, "plerrx(): Arrays must be of same length!" );
    c_plerrx( n, xmin.ptr, xmax.ptr, y.ptr );
}

// Plot vertical error bars (x,ymin(i)) to (x(i),ymax(i))
void plerry( PLFLT[] x, PLFLT[] ymin, PLFLT[] ymax )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == ymin.length, "plerry(): Arrays must be of same length!" );
    assert( n == ymax.length, "plerry(): Arrays must be of same length!" );
    c_plerry( n, x.ptr, ymin.ptr, ymax.ptr );
}

// Pattern fills the polygon bounded by the input points.
void plfill( PLFLT[] x, PLFLT[] y )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plfill(): Arrays must be of same length!" );
    c_plfill( n, x.ptr, y.ptr );
}

// Pattern fills the 3d polygon bounded by the input points.
void plfill3( PLFLT[] x, PLFLT[] y, PLFLT[] z )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plfill3(): Arrays must be of same length!" );
    assert( n == z.length, "plfill3(): Arrays must be of same length!" );
    c_plfill3( n, x.ptr, y.ptr, z.ptr );
}

// Get the current device (keyword) name
void plgdev( out string p_dev )
{
    char cdev[1024];
    c_plgdev( cdev.ptr );
    p_dev = to!string( cdev.ptr );
}

// Get the (current) output file name.  Must be preallocated to >80 bytes
void plgfnam( out string fnam )
{
    char cfnam[1024];
    c_plgfnam( cfnam.ptr );
    fnam = to!string( cfnam.ptr );
}

// Draw gradient in polygon.
void plgradient( PLFLT[] x, PLFLT[] y, PLFLT angle )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plgradient(): Arrays must be of same length!" );
    c_plgradient( n, x.ptr, y.ptr, angle );
}

// grid irregularly sampled data
void  plgriddata( PLFLT[] x, PLFLT[] y, PLFLT[] z, PLFLT[] xg, PLFLT[] yg, PLFLT[][] zg, PLINT type, PLFLT data )
{
    PLINT npts = cast(PLINT) x.length;
    assert( npts == y.length, "plgriddata(): Arrays must be of same length!" );
    assert( npts == z.length, "plgriddata(): Arrays must be of same length!" );

    PLINT nxg = cast(PLINT) xg.length;
    PLINT nyg = cast(PLINT) yg.length;
    assert( nxg == zg.length, "plgriddata(): Arrays must be of same length!" );
    assert( nyg == zg[0].length, "plgriddata(): Arrays must be of same length!" );

    c_plgriddata( x.ptr, y.ptr, z.ptr, npts, xg.ptr, nxg, yg.ptr, nyg, convert_array( zg ), type, data );
}

// Get the current library version number
void plgver( out string p_ver )
{
    char cver[1024];
    c_plgver( cver.ptr );
    p_ver = to!string( cver.ptr );
}

// Draws a histogram of n values of a variable in array data[0..n-1]
void plhist( PLFLT[] data, PLFLT datmin, PLFLT datmax, PLINT nbin, PLINT opt )
{
    c_plhist( cast(PLINT) data.length, data.ptr, datmin, datmax, nbin, opt );
}

// Simple routine for labelling graphs.
void pllab( string xlabel, string ylabel, string tlabel )
{
    c_pllab( toStringz( xlabel ), toStringz( ylabel ), toStringz( tlabel ) );
}

// Routine for drawing discrete line, symbol, or cmap0 legends
void pllegend( PLFLT *p_legend_width, PLFLT *p_legend_height,
               PLINT opt, PLINT position, PLFLT x, PLFLT y, PLFLT plot_width,
               PLINT bg_color, PLINT bb_color, PLINT bb_style,
               PLINT nrow, PLINT ncolumn,
               PLINT[] opt_array,
               PLFLT text_offset, PLFLT text_scale, PLFLT text_spacing,
               PLFLT text_justification,
               PLINT[] text_colors, string[] text,
               PLINT[] box_colors, PLINT[] box_patterns,
               PLFLT[] box_scales, PLFLT[] box_line_widths,
               PLINT[] line_colors, PLINT[] line_styles,
               PLFLT[] line_widths,
               PLINT[] symbol_colors, PLFLT[] symbol_scales,
               PLINT[] symbol_numbers, string[] symbols )
{
    PLINT nlegend = cast(PLINT) opt_array.length;
    immutable( char ) * *textz    = array( map!toStringz( text ) ).ptr;
    immutable( char ) * *symbolsz = array( map!toStringz( symbols ) ).ptr;
    assert( nlegend == text_colors.length, "pllegend(): Arrays must be of same length!" );
    assert( nlegend == text.length, "pllegend(): Arrays must be of same length!" );
    assert( box_colors == null || nlegend == box_colors.length, "pllegend(): Arrays must be of same length!" );
    assert( box_patterns == null || nlegend == box_patterns.length, "pllegend(): Arrays must be of same length!" );
    assert( box_scales == null || nlegend == box_scales.length, "pllegend(): Arrays must be of same length!" );
    assert( box_line_widths == null || nlegend == box_line_widths.length, "pllegend(): Arrays must be of same length!" );
    assert( line_colors == null || nlegend == line_colors.length, "pllegend(): Arrays must be of same length!" );
    assert( line_styles == null || nlegend == line_styles.length, "pllegend(): Arrays must be of same length!" );
    assert( line_widths == null || nlegend == line_widths.length, "pllegend(): Arrays must be of same length!" );
    assert( symbol_colors == null || nlegend == symbol_colors.length, "pllegend(): Arrays must be of same length!" );
    assert( symbol_scales == null || nlegend == symbol_scales.length, "pllegend(): Arrays must be of same length!" );
    assert( symbol_numbers == null || nlegend == symbol_numbers.length, "pllegend(): Arrays must be of same length!" );
    assert( symbols == null || nlegend == symbols.length, "pllegend(): Arrays must be of same length!" );
    c_pllegend( p_legend_width, p_legend_height,
        opt, position, x, y, plot_width,
        bg_color, bb_color, bb_style,
        nrow, ncolumn,
        nlegend, opt_array.ptr,
        text_offset, text_scale, text_spacing,
        text_justification,
        text_colors.ptr, textz,
        box_colors.ptr, box_patterns.ptr,
        box_scales.ptr, box_line_widths.ptr,
        line_colors.ptr, line_styles.ptr,
        line_widths.ptr,
        symbol_colors.ptr, symbol_scales.ptr,
        symbol_numbers.ptr, symbolsz );
}

// Draws line segments connecting a series of points.
void plline( PLFLT[] x, PLFLT[] y )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plline(): Arrays must be of same length!" );
    c_plline( n, x.ptr, y.ptr );
}

// Draws a line in 3 space.
void plline3( PLFLT[] x, PLFLT[] y, PLFLT[] z )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plline3(): Arrays must be of same length!" );
    assert( n == z.length, "plline3(): Arrays must be of same length!" );
    c_plline3( n, x.ptr, y.ptr, z.ptr );
}

// plot continental outline in world coordinates
void plmap( mapform_func mapform, string type, PLFLT minlong, PLFLT maxlong,
            PLFLT minlat, PLFLT maxlat )
{
    c_plmap( mapform, toStringz( type ), minlong, maxlong, minlat, maxlat );
}


// Plot map outlines

void plmapline( mapform_func mapform, string name, PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                const PLINT[] plotentries )
{
    PLINT n = cast(PLINT) plotentries.length;
    c_plmapline( mapform, toStringz( name ), minx, maxx, miny, maxy, plotentries.ptr, n );
}

// Plot map points

void plmapstring( mapform_func mapform, string name, string string,
                  PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy, const PLINT[] plotentries )
{
    PLINT n = cast(PLINT) plotentries.length;
    c_plmapstring( mapform, toStringz( name ), toStringz( string ), minx, maxx, miny, maxy, plotentries.ptr, n );
}

// Plot map text

void plmaptex( mapform_func mapform, string name, PLFLT dx, PLFLT dy, PLFLT just, string text,
               PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy, PLINT plotentry )
{
    c_plmaptex( mapform, toStringz( name ), dx, dy, just, toStringz( text ), minx, maxx, miny, maxy, plotentry );
}

// Plot map fills
void plmapfill( mapform_func mapform, string name, PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                const PLINT[] plotentries ) // Plots a mesh representation of the function z[x][y].
{
    PLINT n = cast(PLINT) plotentries.length;
    c_plmapfill( mapform, toStringz( name ), minx, maxx, miny, maxy, plotentries.ptr, n );
}

void plmesh( PLFLT[] x, PLFLT[] y, PLFLT[][] z, PLINT opt )
{
    PLINT nx = cast(PLINT) z.length;
    PLINT ny = cast(PLINT) z[0].length;

    assert( nx == x.length, "plmesh(): Arrays must be of same length!" );
    assert( ny == y.length, "plmesh(): Arrays must be of same length!" );

    c_plmesh( x.ptr, y.ptr, convert_array( z ), nx, ny, opt );
}

// Plots a mesh representation of the function z[x][y] with contour
void plmeshc( PLFLT[] x, PLFLT[] y, PLFLT[][] z, PLINT opt, PLFLT[] clevel )
{
    PLINT nx = cast(PLINT) z.length;
    PLINT ny = cast(PLINT) z[0].length;

    assert( nx == x.length, "plmeshc(): Arrays must be of same length!" );
    assert( ny == y.length, "plmeshc(): Arrays must be of same length!" );

    c_plmeshc( x.ptr, y.ptr, convert_array( z ), nx, ny, opt, clevel.ptr, cast(PLINT) clevel.length );
}

// Prints out "text" at specified position relative to viewport
void plmtex( string side, PLFLT disp, PLFLT pos, PLFLT just, string text )
{
    c_plmtex( toStringz( side ), disp, pos, just, toStringz( text ) );
}

// Prints out "text" at specified position relative to viewport (3D)
void plmtex3( string side, PLFLT disp, PLFLT pos, PLFLT just, string text )
{
    c_plmtex3( toStringz( side ), disp, pos, just, toStringz( text ) );
}

// Plots a 3-d representation of the function z[x][y].
void plot3d( PLFLT[] x, PLFLT[] y, PLFLT[][] z, PLINT opt, PLBOOL side )
{
    PLINT nx = cast(PLINT) z.length;
    PLINT ny = cast(PLINT) z[0].length;

    assert( nx == x.length, "plot3d(): Arrays must be of same length!" );
    assert( ny == y.length, "plot3d(): Arrays must be of same length!" );

    c_plot3d( x.ptr, y.ptr, convert_array( z ), nx, ny, opt, side );
}

// Plots a 3-d representation of the function z[x][y] with contour.
void plot3dc( PLFLT[] x, PLFLT[] y, PLFLT[][] z, PLINT opt, PLFLT[] clevel )
{
    PLINT nx = cast(PLINT) z.length;
    PLINT ny = cast(PLINT) z[0].length;

    assert( nx == x.length, "plot3dc(): Arrays must be of same length!" );
    assert( ny == y.length, "plot3dc(): Arrays must be of same length!" );

    c_plot3dc( x.ptr, y.ptr, convert_array( z ), nx, ny, opt, clevel.ptr, cast(PLINT) clevel.length );
}

// Plots a 3-d representation of the function z[x][y] with contour and
// y index limits.
void plot3dcl( PLFLT[] x, PLFLT[] y, PLFLT[][] z, PLINT opt, PLFLT[] clevel,
               PLINT ixstart, PLINT ixn, PLINT[] indexymin, PLINT[] indexymax )
{
    PLINT nx = cast(PLINT) z.length;
    PLINT ny = cast(PLINT) z[0].length;

    assert( nx == x.length, "plot3dcl(): Arrays must be of same length!" );
    assert( ny == y.length, "plot3dcl(): Arrays must be of same length!" );

    c_plot3dcl( x.ptr, y.ptr, convert_array( z ), nx, ny, opt, clevel.ptr, cast(PLINT) clevel.length,
        ixstart, ixn, indexymin.ptr, indexymax.ptr );
}

// Set fill pattern directly.
void plpat( PLINT[] inc, PLINT[] del )
{
    PLINT nlin = cast(PLINT) inc.length;
    assert( nlin == del.length, "plpat(): Arrays must be of same length!" );
    c_plpat( nlin, inc.ptr, del.ptr );
}

// Plots array y against x for n points using ASCII code "code".
void plpoin( PLFLT[] x, PLFLT[] y, PLINT code )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plpoin(): Arrays must be of same length!" );
    c_plpoin( n, x.ptr, y.ptr, code );
}

// Draws a series of points in 3 space.
void plpoin3( PLFLT[] x, PLFLT[] y, PLFLT[] z, PLINT code )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plpoin3(): Arrays must be of same length!" );
    assert( n == z.length, "plpoin3(): Arrays must be of same length!" );
    c_plpoin3( n, x.ptr, y.ptr, z.ptr, code );
}

// Plots array y against x for n points using (UTF-8) text string
void plstring( PLFLT[] x, PLFLT[] y, string text )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plstring(): Arrays must be of same length!" );
    c_plstring( n, x.ptr, y.ptr, toStringz( text ) );
}

// Draws a series of points (described by [UTF8] text string) in 3 space.
void plstring3( PLFLT[] x, PLFLT[] y, PLFLT[] z, string text )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plstring3(): Arrays must be of same length!" );
    assert( n == z.length, "plstring3(): Arrays must be of same length!" );
    c_plstring3( n, x.ptr, y.ptr, z.ptr, toStringz( text ) );
}

// Draws a polygon in 3 space.
void plpoly3( PLFLT[] x, PLFLT[] y, PLFLT[] z, PLBOOL[] draw, PLBOOL ifcc )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plpoly3(): Arrays must be of same length!" );
    assert( n == z.length, "plpoly3(): Arrays must be of same length!" );
    assert( n - 1 == draw.length, "plpoly3(): Array draw must be of same length then other arrays minus 1!" );
    c_plpoly3( n, x.ptr, y.ptr, z.ptr, draw.ptr, ifcc );
}

// Prints out "text" at world cooordinate (x,y).
void plptex( PLFLT x, PLFLT y, PLFLT dx, PLFLT dy, PLFLT just, string text )
{
    c_plptex( x, y, dx, dy, just, toStringz( text ) );
}

// Prints out "text" at world cooordinate (x,y,z).
void plptex3( PLFLT wx, PLFLT wy, PLFLT wz, PLFLT dx, PLFLT dy, PLFLT dz,
              PLFLT sx, PLFLT sy, PLFLT sz, PLFLT just, string text )
{
    c_plptex3( wx, wy, wz, dx, dy, dz, sx, sy, sz, just, toStringz( text ) );
}

// Set the colors for color table 0 from a cmap0 file
void plspal0( string filename )
{
    c_plspal0( toStringz( filename ) );
}

// Set the colors for color table 1 from a cmap1 file
void plspal1( string filename, PLBOOL interpolate )
{
    c_plspal1( toStringz( filename ), interpolate );
}

// Set color map 0 colors by 8 bit RGB values
void plscmap0( PLINT[] r, PLINT[] g, PLINT[] b )
{
    PLINT ncol0 = cast(PLINT) r.length;
    assert( ncol0 == g.length, "plscmap0(): Arrays must be of same length!" );
    assert( ncol0 == b.length, "plscmap0(): Arrays must be of same length!" );
    c_plscmap0( r.ptr, g.ptr, b.ptr, ncol0 );
}

// Set color map 0 colors by 8 bit RGB values and alpha values
void plscmap0a( PLINT[] r, PLINT[] g, PLINT[] b, PLFLT[] a )
{
    PLINT ncol0 = cast(PLINT) r.length;
    assert( ncol0 == g.length, "plscmap0a(): Arrays must be of same length!" );
    assert( ncol0 == b.length, "plscmap0a(): Arrays must be of same length!" );
    assert( ncol0 == a.length, "plscmap0a(): Arrays must be of same length!" );
    c_plscmap0a( r.ptr, g.ptr, b.ptr, a.ptr, ncol0 );
}

// Set color map 1 colors by 8 bit RGB values
void plscmap1( PLINT[] r, PLINT[] g, PLINT[] b )
{
    PLINT ncol1 = cast(PLINT) r.length;
    assert( ncol1 == g.length, "plscmap1(): Arrays must be of same length!" );
    assert( ncol1 == b.length, "plscmap1(): Arrays must be of same length!" );
    c_plscmap1( r.ptr, g.ptr, b.ptr, ncol1 );
}

// Set color map 1 colors by 8 bit RGB and alpha values
void plscmap1a( PLINT[] r, PLINT[] g, PLINT[] b, PLFLT[] a )
{
    PLINT ncol1 = cast(PLINT) r.length;
    assert( ncol1 == g.length, "plscmap1a(): Arrays must be of same length!" );
    assert( ncol1 == b.length, "plscmap1a(): Arrays must be of same length!" );
    assert( ncol1 == a.length, "plscmap1a(): Arrays must be of same length!" );
    c_plscmap1a( r.ptr, g.ptr, b.ptr, a.ptr, ncol1 );
}

// Set color map 1 colors using a piece-wise linear relationship between
// intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.
void plscmap1l( PLBOOL itype, PLFLT[] intensity, PLFLT[] coord1,
                PLFLT[] coord2, PLFLT[] coord3, PLBOOL[] alt_hue_path = null )
{
    PLINT npts = cast(PLINT) intensity.length;
    assert( npts == coord1.length, "plscmap1l(): Arrays must be of same length!" );
    assert( npts == coord2.length, "plscmap1l(): Arrays must be of same length!" );
    assert( npts == coord3.length, "plscmap1l(): Arrays must be of same length!" );
    if ( alt_hue_path != null )
    {
        assert( npts - 1 == alt_hue_path.length, "plscmap1l(): Array alt_hue_path must be of same length then other arrays minus 1!" );
        c_plscmap1l( itype, npts, intensity.ptr, coord1.ptr, coord2.ptr, coord3.ptr, alt_hue_path.ptr );
    }
    else
        c_plscmap1l( itype, npts, intensity.ptr, coord1.ptr, coord2.ptr, coord3.ptr, null );
}


// Set color map 1 colors using a piece-wise linear relationship between
// intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.
// Will also linear interpolate alpha values.
void plscmap1la( PLBOOL itype, PLFLT[] intensity, PLFLT[] coord1,
                 PLFLT[] coord2, PLFLT[] coord3, PLFLT[] a, PLBOOL[] alt_hue_path = null )
{
    PLINT npts = cast(PLINT) intensity.length;
    assert( npts == coord1.length, "plscmap1la(): Arrays must be of same length!" );
    assert( npts == coord2.length, "plscmap1la(): Arrays must be of same length!" );
    assert( npts == coord3.length, "plscmap1la(): Arrays must be of same length!" );
    assert( npts == a.length, "plscmap1la(): Arrays must be of same length!" );
    if ( alt_hue_path != null )
    {
        assert( npts - 1 == alt_hue_path.length, "plscmap1la(): Array alt_hue_path must be of same length then other arrays minus 1!" );
        c_plscmap1la( itype, npts, intensity.ptr, coord1.ptr, coord2.ptr, coord3.ptr, a.ptr, alt_hue_path.ptr );
    }
    else
        c_plscmap1la( itype, npts, intensity.ptr, coord1.ptr, coord2.ptr, coord3.ptr, a.ptr, null );
}

// Set the device (keyword) name
void plsdev( string devname )
{
    c_plsdev( toStringz( devname ) );
}

// Set the output file name.
void plsfnam( string fnam )
{
    c_plsfnam( toStringz( fnam ) );
}

// Shade region.
void plshade( PLFLT[][] a, def_func defined, PLFLT left, PLFLT right,
              PLFLT bottom, PLFLT top, PLFLT shade_min, PLFLT shade_max, PLINT sh_cmap,
              PLFLT sh_color, PLFLT sh_width, PLINT min_color, PLFLT min_width, PLINT max_color,
              PLFLT max_width, PLBOOL rectangular,
              pltr_func pltr = null, PLPointer pltr_data = null )
{
    PLINT nx = cast(PLINT) a.length;
    PLINT ny = cast(PLINT) a[0].length;

    c_plshade( convert_array( a ), nx, ny, defined, left, right, bottom, top, shade_min, shade_max, sh_cmap,
        sh_color, sh_width, min_color, min_width, max_color, max_width, &c_plfill,
        rectangular, pltr, pltr_data );
}

void plshades( PLFLT[][] a, def_func defined, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
               PLFLT[] clevel, PLFLT fill_width, PLINT cont_color, PLFLT cont_width,
               PLBOOL rectangular, pltr_func pltr = null, PLPointer pltr_data = null )
{
    PLINT nx = cast(PLINT) a.length;
    PLINT ny = cast(PLINT) a[0].length;

    c_plshades( convert_array( a ), nx, ny, defined, xmin, xmax, ymin, ymax, clevel.ptr, cast(PLINT) clevel.length,
        fill_width, cont_color, cont_width, &c_plfill, rectangular, pltr, pltr_data );
}

void plshades( PLFLT[][] a, def_func defined, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
               PLFLT[] clevel, PLFLT fill_width, PLINT cont_color, PLFLT cont_width,
               PLBOOL rectangular, ref PLcGrid cgrid )
{
    PLINT     nx = cast(PLINT) a.length;
    PLINT     ny = cast(PLINT) a[0].length;

    c_PLcGrid c;
    c.xg = cgrid.xg.ptr;
    c.nx = cast(PLINT) cgrid.xg.length;
    c.yg = cgrid.yg.ptr;
    c.ny = cast(PLINT) cgrid.yg.length;
    c.zg = cgrid.zg.ptr;
    c.nz = cast(PLINT) cgrid.zg.length;

    c_plshades( convert_array( a ), nx, ny, defined, xmin, xmax, ymin, ymax, clevel.ptr, cast(PLINT) clevel.length,
        fill_width, cont_color, cont_width, &c_plfill, rectangular, &pltr1, &c );
}

void plshades( PLFLT[][] a, def_func defined, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
               PLFLT[] clevel, PLFLT fill_width, PLINT cont_color, PLFLT cont_width,
               PLBOOL rectangular, ref PLcGrid2 cgrid2 )
{
    PLINT      nx = cast(PLINT) a.length;
    PLINT      ny = cast(PLINT) a[0].length;

    c_PLcGrid2 c2;
    c2.xg = convert_array( cgrid2.xg );
    c2.yg = convert_array( cgrid2.yg );
    c2.zg = convert_array( cgrid2.zg );
    c2.nx = cast(PLINT) cgrid2.xg.length;
    c2.ny = cast(PLINT) cgrid2.xg[0].length;
    if ( cgrid2.yg )
    {
        assert( c2.nx == cgrid2.yg.length, "plcont(): Arrays must be of same length!" );
        assert( c2.ny == cgrid2.yg[0].length, "plcont(): Arrays must be of same length!" );
    }
    if ( cgrid2.zg )
    {
        assert( c2.nx == cgrid2.zg.length, "plcont(): Arrays must be of same length!" );
        assert( c2.ny == cgrid2.zg[0].length, "plcont(): Arrays must be of same length!" );
    }

    c_plshades( convert_array( a ), nx, ny, defined, xmin, xmax, ymin, ymax, clevel.ptr, cast(PLINT) clevel.length,
        fill_width, cont_color, cont_width, &c_plfill, rectangular, &pltr2, &c2 );
}

// Initialize PLplot, passing the device name and windows/page settings.
void plstart( string devname, PLINT nx, PLINT ny )
{
    c_plstart( toStringz( devname ), nx, ny );
}

// Create 1d stripchart
void plstripc( PLINT* id, string xspec, string yspec, PLFLT xmin, PLFLT xmax, PLFLT xjump,
               PLFLT ymin, PLFLT ymax, PLFLT xlpos, PLFLT ylpos, PLBOOL y_ascl, PLBOOL acc,
               PLINT colbox, PLINT collab, PLINT[] colline, PLINT[] styline, string[] legline,
               string labx, string laby, string labtop )
{
    assert( 4 == colline.length, "plstripc(): Arrays must be of length 4!" );
    assert( 4 == styline.length, "plstripc(): Arrays must be of length 4!" );
    assert( 4 == legline.length, "plstripc(): Arrays must be of length 4!" );

    immutable( char ) * *leglinez = array( map!toStringz( legline ) ).ptr;
    //for ( int i = 0; i < 4; i++ )
    //{
    //    leglinez[i] = toStringz( legline[i] );
    //}

    c_plstripc( id, toStringz( xspec ), toStringz( yspec ), xmin, xmax, xjump, ymin, ymax,
        xlpos, ylpos, y_ascl, acc, colbox, collab, colline.ptr, styline.ptr, leglinez,
        toStringz( labx ), toStringz( laby ), toStringz( labtop ) );
}

// plots a 2d image (or a matrix too large for plshade() )
void plimagefr( PLFLT[][] idata, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
                PLFLT zmin, PLFLT zmax, PLFLT valuemin, PLFLT valuemax,
                pltr_func pltr = null, PLPointer pltr_data = null )
{
    PLINT nx = cast(PLINT) idata.length;
    PLINT ny = cast(PLINT) idata[0].length;

    c_plimagefr( convert_array( idata ), nx, ny, xmin, xmax, ymin, ymax, zmin, zmax,
        valuemin, valuemax, pltr, pltr_data );
}

// plots a 2d image (or a matrix too large for plshade() )
void plimagefr( PLFLT[][] idata, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
                PLFLT zmin, PLFLT zmax, PLFLT valuemin, PLFLT valuemax, PLcGrid cgrid )
{
    PLINT     nx = cast(PLINT) idata.length;
    PLINT     ny = cast(PLINT) idata[0].length;

    c_PLcGrid c;
    c.xg = cgrid.xg.ptr;
    c.nx = cast(PLINT) cgrid.xg.length;
    c.yg = cgrid.yg.ptr;
    c.ny = cast(PLINT) cgrid.yg.length;
    c.zg = cgrid.zg.ptr;
    c.nz = cast(PLINT) cgrid.zg.length;

    c_plimagefr( convert_array( idata ), nx, ny, xmin, xmax, ymin, ymax, zmin, zmax,
        valuemin, valuemax, &pltr1, &c );
}

// plots a 2d image (or a matrix too large for plshade() )
void plimagefr( PLFLT[][] idata, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
                PLFLT zmin, PLFLT zmax, PLFLT valuemin, PLFLT valuemax, PLcGrid2 cgrid2 )
{
    PLINT      nx = cast(PLINT) idata.length;
    PLINT      ny = cast(PLINT) idata[0].length;

    c_PLcGrid2 c2;
    c2.xg = convert_array( cgrid2.xg );
    c2.yg = convert_array( cgrid2.yg );
    c2.zg = convert_array( cgrid2.zg );
    c2.nx = cast(PLINT) cgrid2.xg.length;
    c2.ny = cast(PLINT) cgrid2.xg[0].length;
    if ( cgrid2.yg )
    {
        assert( c2.nx == cgrid2.yg.length, "plcont(): Arrays must be of same length!" );
        assert( c2.ny == cgrid2.yg[0].length, "plcont(): Arrays must be of same length!" );
    }
    if ( cgrid2.zg )
    {
        assert( c2.nx == cgrid2.zg.length, "plcont(): Arrays must be of same length!" );
        assert( c2.ny == cgrid2.zg[0].length, "plcont(): Arrays must be of same length!" );
    }

    c_plimagefr( convert_array( idata ), nx, ny, xmin, xmax, ymin, ymax, zmin, zmax,
        valuemin, valuemax, &pltr2, &c2 );
}

// plots a 2d image (or a matrix too large for plshade() ) - colors
// automatically scaled
void plimage( PLFLT[][] idata, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
              PLFLT zmin, PLFLT zmax, PLFLT Dxmin, PLFLT Dxmax, PLFLT Dymin, PLFLT Dymax )
{
    PLINT nx = cast(PLINT) idata.length;
    PLINT ny = cast(PLINT) idata[0].length;

    c_plimage( convert_array( idata ), nx, ny, xmin, xmax, ymin, ymax, zmin, zmax, Dxmin, Dxmax,
        Dymin, Dymax );
}

// Set up a new line style
void plstyl( PLINT[] mark, PLINT[] space )
{
    PLINT nms = cast(PLINT) mark.length;
    assert( nms == space.length, "plstyl(): Arrays must be of same length!" );
    c_plstyl( nms, mark.ptr, space.ptr );
}

// Plots the 3d surface representation of the function z[x][y].
void plsurf3d( PLFLT[] x, PLFLT[] y, PLFLT[][] z, PLINT opt, PLFLT[] clevel = null )
{
    PLINT nx = cast(PLINT) z.length;
    PLINT ny = cast(PLINT) z[0].length;
    assert( nx == x.length, "plsurf3d(): Arrays must be of same length!" );
    assert( ny == y.length, "plsurf3d(): Arrays must be of same length!" );

    if ( clevel )
        c_plsurf3d( x.ptr, y.ptr, convert_array( z ), nx, ny, opt, clevel.ptr, cast(PLINT) clevel.length );
    else
        c_plsurf3d( x.ptr, y.ptr, convert_array( z ), nx, ny, opt, null, 0 );
}

// Plots the 3d surface representation of the function z[x][y] with y
// index limits.
void plsurf3dl( PLFLT[] x, PLFLT[] y, PLFLT[][] z, PLINT opt, PLFLT[] clevel,
                PLINT ixstart, PLINT ixn, PLINT[] indexymin, PLINT[] indexymax )
{
    PLINT nx = cast(PLINT) z.length;
    PLINT ny = cast(PLINT) z[0].length;
    assert( nx == x.length, "plsurf3d(): Arrays must be of same length!" );
    assert( ny == y.length, "plsurf3d(): Arrays must be of same length!" );

    c_plsurf3dl( x.ptr, y.ptr, convert_array( z ), nx, ny, opt, clevel.ptr, cast(PLINT) clevel.length,
        ixstart, ixn, indexymin.ptr, indexymax.ptr );
}

// Plots array y against x for n points using Hershey symbol "code"
void plsym( PLFLT[] x, PLFLT[] y, PLINT code )
{
    PLINT n = cast(PLINT) x.length;
    assert( n == y.length, "plsym(): Arrays must be of same length!" );
    c_plsym( n, x.ptr, y.ptr, code );
}

// Set the format for date / time labels
void pltimefmt( string fmt )
{
    c_pltimefmt( toStringz( fmt ) );
}

//--------------------------------------------------------------------------
//		Functions for use from C or C++ only
//--------------------------------------------------------------------------

// Returns a list of file-oriented device names and their menu strings
//void  plgFileDevs(char ***p_menustr, char ***p_devname, int *p_ndev);

// Returns a list of all device names and their menu strings
//void  plgDevs(char ***p_menustr, char ***p_devname, int *p_ndev);

// Set the function pointer for the keyboard event handler
//void  plsKeyEH(void  function(PLGraphicsIn *, void *, int *)KeyEH, void *KeyEH_data);

// Set the function pointer for the (mouse) button event handler
//void  plsButtonEH(void  function(PLGraphicsIn *, void *, int *)ButtonEH, void *ButtonEH_data);

// Sets an optional user bop handler
//void  plsbopH(void  function(void *, int *)handler, void *handler_data);

// Sets an optional user eop handler
//void  plseopH(void  function(void *, int *)handler, void *handler_data);

// Set the variables to be used for storing error info
//void plsError(PLINT *errcode, char *errmsg)
//{
//}

// Sets an optional user exit handler.
//void  plsexit(int  function(char *)handler);

// Sets an optional user abort handler.
//void  plsabort(void  function(char *)handler);

// Function evaluators

// Does a lookup from a 2d function array.  Array is of type (PLFLT **),
// and is column dominant (normal C ordering).
//PLFLT  plf2eval2(PLINT ix, PLINT iy, PLPointer plf2eval_data);

// Does a lookup from a 2d function array.  Array is of type (PLFLT *),
// and is column dominant (normal C ordering).
//PLFLT  plf2eval(PLINT ix, PLINT iy, PLPointer plf2eval_data);

// Does a lookup from a 2d function array.  Array is of type (PLFLT *),
// and is row dominant (Fortran ordering).
//PLFLT  plf2evalr(PLINT ix, PLINT iy, PLPointer plf2eval_data);

// Command line parsing utilities

// Merge user option table into internal info structure.
//PLINT  plMergeOpts(PLOptionTable *options, char *name, char **notes);

// Set the strings used in usage and syntax messages.
//void  plSetUsage(char *program_string, char *usage_string);

// Process input strings, treating them as an option and argument pair.
PLINT plsetopt( string opt, string optarg )
{
    return c_plsetopt( toStringz( opt ), toStringz( optarg ) );
}

// Miscellaneous

// Get the escape character for text strings.
//void  plgesc(char *p_esc);

// Front-end to driver escape function.
//void  pl_cmd(PLINT op, void *ptr);

// Return full pathname for given file if executable
//PLINT  plFindName(char *p);

// Looks for the specified executable file according to usual search path.
//char * plFindCommand(char *fn);

// Gets search name for file by concatenating the dir, subdir, and file
// name, allocating memory as needed.
//void  plGetName(char *dir, char *subdir, char *filename, char **filespec);

// Prompts human to input an integer in response to given message.
//PLINT  plGetInt(char *s);

// Prompts human to input a float in response to given message.
//PLFLT  plGetFlt(char *s);

// Find the maximum and minimum of a 2d matrix allocated with plAllc2dGrid().
void plMinMax2dGrid( PLFLT[][] f, out PLFLT fmax, out PLFLT fmin )
{
    plMinMax2dGrid( convert_array( f ), cast(PLINT) f.length, cast(PLINT) f[0].length, &fmax, &fmin );
}

// Wait for graphics input event and translate to world coordinates
//PLINT  plGetCursor(PLGraphicsIn *gin);

// Translates relative device coordinates to world coordinates.
//PLINT  plTranslateCursor(PLGraphicsIn *gin);

extern ( C ) :

alias double PLFLT;

// This is apparently portable if stdint.h exists.
// A reasonable back-up in case stdint.h does not exist on the platform.
alias uint PLUNICODE;
alias int  PLINT;

// For identifying logical (boolean) arguments
alias PLINT PLBOOL;

// For passing user data, as with X's XtPointer
alias void* PLPointer;

//--------------------------------------------------------------------------
// Complex data types and other good stuff
//--------------------------------------------------------------------------


// Define constants that are accessible from the API
// MAINTENANCE 2017-12
// These are taken from the "#define" section of bindings/swig-support/plplotcapi.i
// and transformed as follows:
// grep '^#define' bindings/swig-support/plplotcapi.i |sed -e '/#define / s?#define ?const ?' -e 's? \([(0-9]\)? = \1?' -e 's? * //.*$??' -e 's? *$?;?' | grep '=' >|/tmp/software
// and then that generated file was inserted here.

const PLESC_SET_RGB                = 1;
const PLESC_ALLOC_NCOL             = 2;
const PLESC_SET_LPB                = 3;
const PLESC_EXPOSE                 = 4;
const PLESC_RESIZE                 = 5;
const PLESC_REDRAW                 = 6;
const PLESC_TEXT                   = 7;
const PLESC_GRAPH                  = 8;
const PLESC_FILL                   = 9;
const PLESC_DI                     = 10;
const PLESC_FLUSH                  = 11;
const PLESC_EH                     = 12;
const PLESC_GETC                   = 13;
const PLESC_SWIN                   = 14;
const PLESC_DOUBLEBUFFERING        = 15;
const PLESC_XORMOD                 = 16;
const PLESC_SET_COMPRESSION        = 17;
const PLESC_CLEAR                  = 18;
const PLESC_DASH                   = 19;
const PLESC_HAS_TEXT               = 20;
const PLESC_IMAGE                  = 21;
const PLESC_IMAGEOPS               = 22;
const PLESC_PL2DEVCOL              = 23;
const PLESC_DEV2PLCOL              = 24;
const PLESC_SETBGFG                = 25;
const PLESC_DEVINIT                = 26;
const PLESC_GETBACKEND             = 27;
const PLESC_BEGIN_TEXT             = 28;
const PLESC_TEXT_CHAR              = 29;
const PLESC_CONTROL_CHAR           = 30;
const PLESC_END_TEXT               = 31;
const PLESC_START_RASTERIZE        = 32;
const PLESC_END_RASTERIZE          = 33;
const PLESC_ARC                    = 34;
const PLESC_GRADIENT               = 35;
const PLESC_MODESET                = 36;
const PLESC_MODEGET                = 37;
const PLESC_FIXASPECT              = 38;
const PLESC_IMPORT_BUFFER          = 39;
const PLESC_APPEND_BUFFER          = 40;
const PLESC_FLUSH_REMAINING_BUFFER = 41;
const PLTEXT_FONTCHANGE            = 0;
const PLTEXT_SUPERSCRIPT           = 1;
const PLTEXT_SUBSCRIPT             = 2;
const PLTEXT_BACKCHAR              = 3;
const PLTEXT_OVERLINE              = 4;
const PLTEXT_UNDERLINE             = 5;
const ZEROW2B                       = 1;
const ZEROW2D                       = 2;
const ONEW2B                        = 3;
const ONEW2D                        = 4;
const PLSWIN_DEVICE                 = 1;
const PLSWIN_WORLD                  = 2;
const PL_X_AXIS                     = 1;
const PL_Y_AXIS                     = 2;
const PL_Z_AXIS                     = 3;
const PL_OPT_ENABLED                = 0x0001;
const PL_OPT_ARG                    = 0x0002;
const PL_OPT_NODELETE               = 0x0004;
const PL_OPT_INVISIBLE              = 0x0008;
const PL_OPT_DISABLED               = 0x0010;
const PL_OPT_FUNC                   = 0x0100;
const PL_OPT_BOOL                   = 0x0200;
const PL_OPT_INT                    = 0x0400;
const PL_OPT_FLOAT                  = 0x0800;
const PL_OPT_STRING                 = 0x1000;
const PL_PARSE_PARTIAL              = 0x0000;
const PL_PARSE_FULL                 = 0x0001;
const PL_PARSE_QUIET                = 0x0002;
const PL_PARSE_NODELETE             = 0x0004;
const PL_PARSE_SHOWALL              = 0x0008;
const PL_PARSE_OVERRIDE             = 0x0010;
const PL_PARSE_NOPROGRAM            = 0x0020;
const PL_PARSE_NODASH               = 0x0040;
const PL_PARSE_SKIP                 = 0x0080;
const PL_FCI_MARK                   = 0x80000000;
const PL_FCI_IMPOSSIBLE             = 0x00000000;
const PL_FCI_HEXDIGIT_MASK          = 0xf;
const PL_FCI_HEXPOWER_MASK          = 0x7;
const PL_FCI_HEXPOWER_IMPOSSIBLE    = 0xf;
const PL_FCI_FAMILY                 = 0x0;
const PL_FCI_STYLE                  = 0x1;
const PL_FCI_WEIGHT                 = 0x2;
const PL_FCI_SANS                   = 0x0;
const PL_FCI_SERIF                  = 0x1;
const PL_FCI_MONO                   = 0x2;
const PL_FCI_SCRIPT                 = 0x3;
const PL_FCI_SYMBOL                 = 0x4;
const PL_FCI_UPRIGHT                = 0x0;
const PL_FCI_ITALIC                 = 0x1;
const PL_FCI_OBLIQUE                = 0x2;
const PL_FCI_MEDIUM                 = 0x0;
const PL_FCI_BOLD                   = 0x1;
const PL_MAXKEY                     = 16;
const PL_MASK_SHIFT                 = 0x1;
const PL_MASK_CAPS                  = 0x2;
const PL_MASK_CONTROL               = 0x4;
const PL_MASK_ALT                   = 0x8;
const PL_MASK_NUM                   = 0x10;
const PL_MASK_ALTGR                 = 0x20;
const PL_MASK_WIN                   = 0x40;
const PL_MASK_SCROLL                = 0x80;
const PL_MASK_BUTTON1               = 0x100;
const PL_MASK_BUTTON2               = 0x200;
const PL_MASK_BUTTON3               = 0x400;
const PL_MASK_BUTTON4               = 0x800;
const PL_MASK_BUTTON5               = 0x1000;
const PL_MAXWINDOWS                 = 64;
const PL_NOTSET                     = ( -42 );
const PLESC_DOUBLEBUFFERING_ENABLE  = 1;
const PLESC_DOUBLEBUFFERING_DISABLE = 2;
const PLESC_DOUBLEBUFFERING_QUERY   = 3;
const PL_BIN_DEFAULT                = 0x0;
const PL_BIN_CENTRED                = 0x1;
const PL_BIN_NOEXPAND               = 0x2;
const PL_BIN_NOEMPTY                = 0x4;
const GRID_CSA                      = 1;
const GRID_DTLI                     = 2;
const GRID_NNI                      = 3;
const GRID_NNIDW                    = 4;
const GRID_NNLI                     = 5;
const GRID_NNAIDW                   = 6;
const PL_HIST_DEFAULT               = 0x00;
const PL_HIST_NOSCALING             = 0x01;
const PL_HIST_IGNORE_OUTLIERS       = 0x02;
const PL_HIST_NOEXPAND              = 0x08;
const PL_HIST_NOEMPTY               = 0x10;
const PL_POSITION_NULL              = 0x0;
const PL_POSITION_LEFT              = 0x1;
const PL_POSITION_RIGHT             = 0x2;
const PL_POSITION_TOP               = 0x4;
const PL_POSITION_BOTTOM            = 0x8;
const PL_POSITION_INSIDE            = 0x10;
const PL_POSITION_OUTSIDE           = 0x20;
const PL_POSITION_VIEWPORT          = 0x40;
const PL_POSITION_SUBPAGE           = 0x80;
const PL_LEGEND_NULL                = 0x0;
const PL_LEGEND_NONE                = 0x1;
const PL_LEGEND_COLOR_BOX           = 0x2;
const PL_LEGEND_LINE                = 0x4;
const PL_LEGEND_SYMBOL              = 0x8;
const PL_LEGEND_TEXT_LEFT           = 0x10;
const PL_LEGEND_BACKGROUND          = 0x20;
const PL_LEGEND_BOUNDING_BOX        = 0x40;
const PL_LEGEND_ROW_MAJOR           = 0x80;
const PL_COLORBAR_NULL              = 0x0;
const PL_COLORBAR_LABEL_LEFT        = 0x1;
const PL_COLORBAR_LABEL_RIGHT       = 0x2;
const PL_COLORBAR_LABEL_TOP         = 0x4;
const PL_COLORBAR_LABEL_BOTTOM      = 0x8;
const PL_COLORBAR_IMAGE             = 0x10;
const PL_COLORBAR_SHADE             = 0x20;
const PL_COLORBAR_GRADIENT          = 0x40;
const PL_COLORBAR_CAP_NONE          = 0x80;
const PL_COLORBAR_CAP_LOW           = 0x100;
const PL_COLORBAR_CAP_HIGH          = 0x200;
const PL_COLORBAR_SHADE_LABEL       = 0x400;
const PL_COLORBAR_ORIENT_RIGHT      = 0x800;
const PL_COLORBAR_ORIENT_TOP        = 0x1000;
const PL_COLORBAR_ORIENT_LEFT       = 0x2000;
const PL_COLORBAR_ORIENT_BOTTOM     = 0x4000;
const PL_COLORBAR_BACKGROUND        = 0x8000;
const PL_COLORBAR_BOUNDING_BOX      = 0x10000;
const PL_DRAWMODE_UNKNOWN           = 0x0;
const PL_DRAWMODE_DEFAULT           = 0x1;
const PL_DRAWMODE_REPLACE           = 0x2;
const PL_DRAWMODE_XOR               = 0x4;
const DRAW_LINEX                    = 0x001;
const DRAW_LINEY                    = 0x002;
const DRAW_LINEXY                   = 0x003;
const MAG_COLOR                     = 0x004;
const BASE_CONT                     = 0x008;
const TOP_CONT                      = 0x010;
const SURF_CONT                     = 0x020;
const DRAW_SIDES                    = 0x040;
const FACETED                       = 0x080;
const MESH = 0x100;
// End of constants.

// Obsolete names

// Option table definition

struct _N1
{
    string opt;
    int  function( char *, char *, void * ) handler;
    void   *client_data;
    void   *var;
    int    mode;
    string syntax;
    string desc;
}
alias _N1 PLOptionTable;

// PLplot Graphics Input structure


struct _N2
{
    int   type;
    uint  state;
    uint  keysym;
    uint  button;
    PLINT subwindow;
    char [16] string;
    int   pX;
    int   pY;
    PLFLT dX;
    PLFLT dY;
    PLFLT wX;
    PLFLT wY;
}
alias _N2 PLGraphicsIn;

// Structure for describing the plot window


struct _N3
{
    PLFLT dxmi;
    PLFLT dxma;
    PLFLT dymi;
    PLFLT dyma;
    PLFLT wxmi;
    PLFLT wxma;
    PLFLT wymi;
    PLFLT wyma;
}
alias _N3 PLWindow;

// Structure for doing display-oriented operations via escape commands
// May add other attributes in time

struct _N4
{
    uint x;
    uint y;
    uint width;
    uint height;
}
alias _N4 PLDisplay;

// See plcont.c for examples of the following

//
// PLfGrid is for passing (as a pointer to the first element) an arbitrarily
// dimensioned array.  The grid dimensions MUST be stored, with a maximum of 3
// dimensions assumed for now.
//

struct _N5
{
    PLFLT *f;
    PLINT nx;
    PLINT ny;
    PLINT nz;
}
alias _N5 PLfGrid;

//
// PLfGrid2 is for passing (as an array of pointers) a 2d function array.  The
// grid dimensions are passed for possible bounds checking.
//

struct _N6
{
    PLFLT **f;
    PLINT nx;
    PLINT ny;
}
alias _N6 PLfGrid2;

//
// NOTE: a PLfGrid3 is a good idea here but there is no way to exploit it yet
// so I'll leave it out for now.
//

//
// PLcGrid is for passing (as a pointer to the first element) arbitrarily
// dimensioned coordinate transformation arrays.  The grid dimensions MUST be
// stored, with a maximum of 3 dimensions assumed for now.
//

struct _N7
{
    PLFLT *xg;
    PLFLT *yg;
    PLFLT *zg;
    PLINT nx;
    PLINT ny;
    PLINT nz;
}
alias _N7 c_PLcGrid;

//
// PLcGrid2 is for passing (as arrays of pointers) 2d coordinate
// transformation arrays.  The grid dimensions are passed for possible bounds
// checking.
//

struct _N8
{
    PLFLT **xg;
    PLFLT **yg;
    PLFLT **zg;
    PLINT nx;
    PLINT ny;
}
alias _N8 c_PLcGrid2;

//
// NOTE: a PLcGrid3 is a good idea here but there is no way to exploit it yet
// so I'll leave it out for now.
//

// PLColor is the usual way to pass an rgb color value.

struct _N9
{
    ubyte r;
    ubyte g;
    ubyte b;
    PLFLT a;
    char  *name;
}
alias _N9 PLColor;

// PLControlPt is how cmap1 control points are represented.

struct _N10
{
    PLFLT h;
    PLFLT l;
    PLFLT s;
    PLFLT p;
    PLFLT a;
    int   alt_hue_path;
}
alias _N10 PLControlPt;

// A PLBufferingCB is a control block for interacting with devices
// that support double buffering.

struct _N11
{
    PLINT cmd;
    PLINT result;
}
alias _N11 PLBufferingCB;

//--------------------------------------------------------------------------* *		BRAINDEAD-ness
//
// Some systems allow the Fortran & C namespaces to clobber each other.
// For PLplot to work from Fortran on these systems, we must name the the
// externally callable C functions something other than their Fortran entry
// names.  In order to make this as easy as possible for the casual user,
// yet reversible to those who abhor my solution, I have done the
// following:
//
//	The C-language bindings are actually different from those
//	described in the manual.  Macros are used to convert the
//	documented names to the names used in this package.  The
//	user MUST include plplot.h in order to get the name
//	redefinition correct.
//
// Sorry to have to resort to such an ugly kludge, but it is really the
// best way to handle the situation at present.  If all available
// compilers offer a way to correct this stupidity, then perhaps we can
// eventually reverse it.
//
// If you feel like screaming at someone (I sure do), please
// direct it at your nearest system vendor who has a braindead shared
// C/Fortran namespace.  Some vendors do offer compiler switches that
// change the object names, but then everybody who wants to use the
// package must throw these same switches, leading to no end of trouble.
//
// Note that this definition should not cause any noticable effects except
// when debugging PLplot calls, in which case you will need to remember
// the real function names (same as before but with a 'c_' prepended).
//
// Also, to avoid macro conflicts, the BRAINDEAD part must not be expanded
// in the stub routines.
//
// Aside: the reason why a shared Fortran/C namespace is deserving of the
// BRAINDEAD characterization is that it completely precludes the the kind
// of universal API that is attempted (more or less) with PLplot, without
// Herculean efforts (e.g. remapping all of the C bindings by macros as
// done here).  The vendors of such a scheme, in order to allow a SINGLE
// type of argument to be passed transparently between C and Fortran,
// namely, a pointer to a conformable data type, have slammed the door on
// insertion of stub routines to handle the conversions needed for other
// data types.  Intelligent linkers could solve this problem, but these are
// not anywhere close to becoming universal.  So meanwhile, one must live
// with either stub routines for the inevitable data conversions, or a
// different API.  The former is what is used here, but is made far more
// difficult in a braindead shared Fortran/C namespace.
//--------------------------------------------------------------------------




alias c_pl_setcontlabelformat pl_setcontlabelformat;
alias c_pl_setcontlabelparam  pl_setcontlabelparam;
alias c_pladv pladv;
//alias c_plaxes plaxes;
//alias c_plbin plbin;
alias c_plbop plbop;
//alias c_plbox plbox;
//alias c_plbox3 plbox3;
alias c_plbtime      plbtime;
alias c_plslabelfunc plslabelfunc;
alias c_plcalc_world plcalc_world;
alias c_plarc        plarc;
alias c_plclear      plclear;
alias c_plcol0       plcol0;
alias c_plcol1       plcol1;
//alias c_plcolorbar   plcolorbar;
alias c_plconfigtime plconfigtime;
//alias c_plcont plcont;
alias c_plcpstrm     plcpstrm;
alias c_plctime      plctime;
alias c_plend        plend;
alias c_plend1       plend1;
alias c_plenv        plenv;
alias c_plenv0       plenv0;
alias c_pleop        pleop;
//alias c_plerrx plerrx;
//alias c_plerry plerry;
alias c_plfamadv plfamadv;
//alias c_plfill plfill;
//alias c_plfill3 plfill3;
alias c_plflush        plflush;
alias c_plfont         plfont;
alias c_plfontld       plfontld;
alias c_plgchr         plgchr;
alias c_plgcol0        plgcol0;
alias c_plgcolbg       plgcolbg;
alias c_plgcol0a       plgcol0a;
alias c_plgcolbga      plgcolbga;
alias c_plgcompression plgcompression;
//alias c_plgdev plgdev;
alias c_plgdidev       plgdidev;
alias c_plgdiori       plgdiori;
alias c_plgdiplt       plgdiplt;
alias c_plgfam         plgfam;
alias c_plgfci         plgfci;
//alias c_plgfnam plgfnam;
alias c_plgfont        plgfont;
alias c_plglevel       plglevel;
alias c_plgpage        plgpage;
alias c_plgradient     plgrdient;
alias c_plgra          plgra;
//alias c_plgriddata plgriddata;
alias c_plgspa         plgspa;
alias c_plgstrm        plgstrm;
//alias c_plgver plgver;
alias c_plgvpd         plgvpd;
alias c_plgvpw         plgvpw;
alias c_plgxax         plgxax;
alias c_plgyax         plgyax;
alias c_plgzax         plgzax;
//alias c_plhist plhist;
alias c_plhlsrgb       plhlsrgb;
//alias c_plimage plimage;
//alias c_plimagefr plimagefr;
alias c_plinit plinit;
alias c_pljoin pljoin;
//alias c_pllab pllab;
//alias c_pllegend pllegend;
alias c_pllightsource pllightsource;
//alias c_plline plline;
alias c_plpath        plpath;
//alias c_plline3 plline3;
alias c_pllsty        pllsty;
//alias c_plmap plmap;
alias c_plmeridians   plmeridians;
//alias c_plmesh plmesh;
//alias c_plmeshc plmeshc;
alias c_plmkstrm plmkstrm;
//alias c_plmtex plmtex;
//alias c_plmtex3 plmtex3;
//alias c_plot3d plot3d;
//alias c_plot3dc plot3dc;
//alias c_plot3dcl plot3dcl;
//alias c_plparseopts plparseopts;
//alias c_plpat plpat;
//alias c_plpoin plpoin;
//alias c_plpoin3 plpoin3;
//alias c_plpoly3 plpoly3;
alias c_plprec plprec;
alias c_plpsty plpsty;
//alias c_plptex plptex;
//alias c_plptex3 plptex3;
alias c_plrandd  plrandd;
alias c_plreplot plreplot;
alias c_plrgbhls plrgbhls;
alias c_plschr   plschr;
//alias c_plscmap0 plscmap0;
//alias c_plscmap0a plscmap0a;
alias c_plscmap0n plscmap0n;
//alias c_plscmap1 plscmap1;
//alias c_plscmap1a plscmap1a;
//alias c_plscmap1l plscmap1l;
//alias c_plscmap1la plscmap1la;
alias c_plscmap1n      plscmap1n;
alias c_plscmap1_range plscmap1_range;
alias c_plscol0        plscol0;
alias c_plscol0a       plscol0a;
alias c_plscolbg       plscolbg;
alias c_plscolbga      plscolbga;
alias c_plscolor       plscolor;
alias c_plscompression plscompression;
// alias c_plsdev plsdev;
alias c_plsdidev       plsdidev;
alias c_plsdimap       plsdimap;
alias c_plsdiori       plsdiori;
alias c_plsdiplt       plsdiplt;
alias c_plsdiplz       plsdiplz;
alias c_plseed         plseed;
alias c_plsesc         plsesc;
//alias c_plsetopt plsetopt;
alias c_plsfam         plsfam;
alias c_plsfci         plsfci;
// alias c_plsfnam plsfnam;
alias c_plsfont        plsfont;
//alias c_plshade plshade;
//alias c_plshades plshades;
alias c_plsmaj  plsmaj;
alias c_plsmem  plsmem;
alias c_plsmin  plsmin;
alias c_plsori  plsori;
alias c_plspage plspage;
// alias c_plspal0 plspal0;
// alias c_plspal1 plspal1;
alias c_plspause     plspause;
alias c_plsstrm      plsstrm;
alias c_plssub       plssub;
alias c_plssym       plssym;
alias c_plstar       plstar;
//alias c_plstart plstart;
alias c_plstransform plstransform;
alias c_plstripa     plstripa;
//alias c_plstripc plstripc;
alias c_plstripd     plstripd;
//alias c_plstring     plstring;
//alias c_plstring3     plstring3;
//alias c_plstyl plstyl;
//alias c_plsurf3d plsurf3d;
//alias c_plsurf3dl plsurf3dl;
//alias c_plsvect plsvect;
alias c_plsvpa   plsvpa;
alias c_plsxax   plsxax;
alias c_plsyax   plsyax;
//alias c_plsym plsym;
alias c_plszax   plszax;
alias c_pltext   pltext;
//alias c_pltimefmt pltimefmt;
alias c_plvasp   plvasp;
//alias c_plvect plvect;
alias c_plvpas   plvpas;
alias c_plvpor   plvpor;
alias c_plvsta   plvsta;
alias c_plw3d    plw3d;
alias c_plwidth  plwidth;
alias c_plwind   plwind;

alias c_plxormod plxormod;


//--------------------------------------------------------------------------* *		Function Prototypes
//--------------------------------------------------------------------------


// All void types
// C routines callable from stub routines come first

// set the format of the contour labels
void c_pl_setcontlabelformat( PLINT lexp, PLINT sigdig );

// set offset and spacing of contour labels
void c_pl_setcontlabelparam( PLFLT offset, PLFLT size, PLFLT spacing, PLINT active );

// Advance to subpage "page", or to the next one if "page" = 0.
void c_pladv( PLINT page );

// simple arrow plotter.
void c_plvect( PLFLT **u, PLFLT **v, PLINT nx, PLINT ny, PLFLT scale,
               void function( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer ) pltr, PLPointer pltr_data );
void c_plsvect( PLFLT *arrowx, PLFLT *arrowy, PLINT npts, PLBOOL fill );

// This functions similarly to plbox() except that the origin of the axes
// is placed at the user-specified point (x0, y0).
void c_plaxes( PLFLT x0, PLFLT y0, const char *xopt, PLFLT xtick, PLINT nxsub,
               const char *yopt, PLFLT ytick, PLINT nysub );

// Plot a histogram using x to store data values and y to store frequencies
void c_plbin( PLINT nbin, PLFLT *x, PLFLT *y, PLINT opt );

// Start new page.  Should only be used with pleop().
void c_plbop();

// This draws a box around the current viewport.
void c_plbox( const char *xopt, PLFLT xtick, PLINT nxsub, const char *yopt, PLFLT ytick, PLINT nysub );

// This is the 3-d analogue of plbox().
void c_plbox3( const char *xopt, const char *xlabel, PLFLT xtick, PLINT nsubx, const char *yopt,
               const char *ylabel, PLFLT ytick, PLINT nsuby, const char *zopt, const char *zlabel,
               PLFLT ztick, PLINT nsubz );

// Calculate broken-down time from continuous time for current stream.
void c_plbtime( PLINT *year, PLINT *month, PLINT *day, PLINT *hour, PLINT *min, PLFLT *sec, PLFLT ctime );

// Setup a user-provided custom labeling function
void c_plslabelfunc( void function( PLINT, PLFLT, char*, PLINT, PLPointer ) labelfunc,
                     PLPointer label_data );

// Calculate world coordinates and subpage from relative device coordinates.
void c_plcalc_world( PLFLT rx, PLFLT ry, PLFLT *wx, PLFLT *wy, PLINT *window );

// Plot an arc
void c_plarc( PLFLT x, PLFLT y, PLFLT a, PLFLT b, PLFLT angle1, PLFLT angle2,
              PLFLT rotate, PLBOOL fill );

// Clear current subpage.
void c_plclear();

// Set color, map 0.  Argument is integer between 0 and 15.
void c_plcol0( PLINT icol0 );

// Set color, map 1.  Argument is a float between 0. and 1.
void c_plcol1( PLFLT col1 );


// Configure transformation between continuous and broken-down time (and
// vice versa) for current stream.
void c_plconfigtime( PLFLT scale, PLFLT offset1, PLFLT offset2, PLINT ccontrol, PLBOOL ifbtime_offset,
                     PLINT year, PLINT month, PLINT day, PLINT hour, PLINT min, PLFLT sec );

// Draws a contour plot from data in f(nx,ny).  Is just a front-end to
// plfcont, with a particular choice for f2eval and f2eval_data.
//
void c_plcont( PLFLT **f, PLINT nx, PLINT ny, PLINT kx, PLINT lx, PLINT ky, PLINT ly,
               PLFLT *clevel, PLINT nlevel,
               void function( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer ) pltr, PLPointer pltr_data );

// Draws a contour plot using the function evaluator f2eval and data stored
// by way of the f2eval_data pointer.  This allows arbitrary organizations
// of 2d array data to be used.
//
void plfcont( PLFLT function( PLINT, PLINT, PLPointer ) f2eval, PLPointer f2eval_data, PLINT nx, PLINT ny,
              PLINT kx, PLINT lx, PLINT ky, PLINT ly, PLFLT *clevel, PLINT nlevel,
              void function( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer ) pltr, PLPointer pltr_data );

// Copies state parameters from the reference stream to the current stream.
void c_plcpstrm( PLINT iplsr, PLBOOL flags );

// Calculate continuous time from broken-down time for current stream.
void c_plctime( PLINT year, PLINT month, PLINT day, PLINT hour, PLINT min, PLFLT sec, PLFLT *ctime );

// Converts input values from relative device coordinates to relative plot
// coordinates.
void pldid2pc( PLFLT *xmin, PLFLT *ymin, PLFLT *xmax, PLFLT *ymax );

// Converts input values from relative plot coordinates to relative
// device coordinates.
void pldip2dc( PLFLT *xmin, PLFLT *ymin, PLFLT *xmax, PLFLT *ymax );

// End a plotting session for all open streams.
void c_plend();

// End a plotting session for the current stream only.
void c_plend1();

// Simple interface for defining viewport and window.
void c_plenv( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax, PLINT just, PLINT axis );

// similar to plenv() above, but in multiplot mode does not advance the subpage,
// instead the current subpage is cleared
void c_plenv0( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax, PLINT just, PLINT axis );

// End current page.  Should only be used with plbop().
void c_pleop();

// Plot horizontal error bars (xmin(i),y(i)) to (xmax(i),y(i))
void c_plerrx( PLINT n, PLFLT *xmin, PLFLT *xmax, PLFLT *y );

// Plot vertical error bars (x,ymin(i)) to (x(i),ymax(i))
void c_plerry( PLINT n, PLFLT *x, PLFLT *ymin, PLFLT *ymax );

// Advance to the next family file on the next new page
void c_plfamadv();

// Pattern fills the polygon bounded by the input points.
void c_plfill( PLINT n, PLFLT *x, PLFLT *y );

// Pattern fills the 3d polygon bounded by the input points.
void c_plfill3( PLINT n, PLFLT *x, PLFLT *y, PLFLT *z );

// Flushes the output stream.  Use sparingly, if at all.
void c_plflush();

// Sets the global font flag to 'ifont'.
void c_plfont( PLINT ifont );

// Load specified font set.
void c_plfontld( PLINT fnt );

// Get character default height and current (scaled) height
void c_plgchr( PLFLT *p_def, PLFLT *p_ht );

// Returns 8 bit RGB values for given color from color map 0
void c_plgcol0( PLINT icol0, PLINT *r, PLINT *g, PLINT *b );

// Returns 8 bit RGB values for given color from color map 0 and alpha value
void c_plgcol0a( PLINT icol0, PLINT *r, PLINT *g, PLINT *b, PLFLT *a );

// Returns the background color by 8 bit RGB value
void c_plgcolbg( PLINT *r, PLINT *g, PLINT *b );

// Returns the background color by 8 bit RGB value and alpha value
void c_plgcolbga( PLINT *r, PLINT *g, PLINT *b, PLFLT *a );

// Returns the current compression setting
void c_plgcompression( PLINT *compression );

// Get the current device (keyword) name
void c_plgdev( char *p_dev );

// Retrieve current window into device space
void c_plgdidev( PLFLT *p_mar, PLFLT *p_aspect, PLFLT *p_jx, PLFLT *p_jy );

// Get plot orientation
void c_plgdiori( PLFLT *p_rot );

// Retrieve current window into plot space
void c_plgdiplt( PLFLT *p_xmin, PLFLT *p_ymin, PLFLT *p_xmax, PLFLT *p_ymax );

// Get FCI (font characterization integer)

void  c_plgfci( PLUNICODE *pfci );

// Get family file parameters

void  c_plgfam( PLINT *p_fam, PLINT *p_num, PLINT *p_bmax );

// Get the (current) output file name.  Must be preallocated to >80 bytes
void c_plgfnam( char *fnam );

// Get the current font family, style and weight
void c_plgfont( PLINT *p_family, PLINT *p_style, PLINT *p_weight );

// Get the (current) run level.
void c_plglevel( PLINT *p_level );

// Get output device parameters.
void c_plgpage( PLFLT *p_xp, PLFLT *p_yp, PLINT *p_xleng, PLINT *p_yleng,
                PLINT *p_xoff, PLINT *p_yoff );

// Switches to graphics screen.
void c_plgra();

// Draw gradient in polygon.
void c_plgradient( PLINT n, PLFLT *x, PLFLT *y, PLFLT angle );

// grid irregularly sampled data
void c_plgriddata( PLFLT *x, PLFLT *y, PLFLT *z, PLINT npts, PLFLT *xg, PLINT nptsx,
                   PLFLT *yg, PLINT nptsy, PLFLT **zg, PLINT type, PLFLT data );

// Get subpage boundaries in absolute coordinates
void c_plgspa( PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax );

// Get current stream number.
void c_plgstrm( PLINT *p_strm );

// Get the current library version number
void c_plgver( char *p_ver );

// Get viewport boundaries in normalized device coordinates
void c_plgvpd( PLFLT *p_xmin, PLFLT *p_xmax, PLFLT *p_ymin, PLFLT *p_ymax );

// Get viewport boundaries in world coordinates
void c_plgvpw( PLFLT *p_xmin, PLFLT *p_xmax, PLFLT *p_ymin, PLFLT *p_ymax );

// Get x axis labeling parameters
void c_plgxax( PLINT *p_digmax, PLINT *p_digits );

// Get y axis labeling parameters
void c_plgyax( PLINT *p_digmax, PLINT *p_digits );

// Get z axis labeling parameters
void c_plgzax( PLINT *p_digmax, PLINT *p_digits );

// Draws a histogram of n values of a variable in array data[0..n-1]
void c_plhist( PLINT n, PLFLT *data, PLFLT datmin, PLFLT datmax, PLINT nbin, PLINT opt );

// Functions for converting between HLS and RGB color space
void c_plhlsrgb( PLFLT h, PLFLT l, PLFLT s, PLFLT *p_r, PLFLT *p_g, PLFLT *p_b );

// Initializes PLplot, using preset or default options
void c_plinit();

// Draws a line segment from (x1, y1) to (x2, y2).
void c_pljoin( PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2 );

// Simple routine for labelling graphs.
void c_pllab( const char *xlabel, const char *ylabel, const char *tlabel );

// Routine for drawing discrete line, symbol, or cmap0 legends
void c_pllegend( PLFLT *p_legend_width, PLFLT *p_legend_height,
                 PLINT opt, PLINT position, PLFLT x, PLFLT y, PLFLT plot_width,
                 PLINT bg_color, PLINT bb_color, PLINT bb_style,
                 PLINT nrow, PLINT ncolumn,
                 PLINT nlegend, PLINT *opt_array,
                 PLFLT text_offset, PLFLT text_scale, PLFLT text_spacing,
                 PLFLT text_justification,
                 PLINT *text_colors, const char **text,
                 PLINT *box_colors, PLINT *box_patterns,
                 PLFLT *box_scales, PLFLT *box_line_widths,
                 PLINT *line_colors, PLINT *line_styles,
                 PLFLT *line_widths,
                 PLINT *symbol_colors, PLFLT *symbol_scales,
                 PLINT *symbol_numbers, const char **symbols );

// Routine for drawing continuous colour legends
void c_plcolorbar( PLFLT *p_colorbar_width, PLFLT *p_colorbar_height,
                   PLINT opt, PLINT position, PLFLT x, PLFLT y,
                   PLFLT x_length, PLFLT y_length,
                   PLINT bg_color, PLINT bb_color, PLINT bb_style,
                   PLFLT low_cap_color, PLFLT high_cap_color,
                   PLINT cont_color, PLFLT cont_width,
                   PLINT n_labels, const PLINT *label_opts, const char **label,
                   PLINT n_axes, const char ** axis_opts,
                   const PLFLT *ticks, const PLINT *sub_ticks,
                   const PLINT *n_values, const PLFLT **values );

// Sets position of the light source
void c_pllightsource( PLFLT x, PLFLT y, PLFLT z );

// Draws line segments connecting a series of points.
void c_plline( PLINT n, PLFLT *x, PLFLT *y );

// Draws a line in 3 space.
void c_plline3( PLINT n, PLFLT *x, PLFLT *y, PLFLT *z );

// Set line style.
void c_pllsty( PLINT lin );

// plot continental outline in world coordinates
void c_plmap( void function( PLINT, PLFLT *, PLFLT* ) mapform, const char *type, PLFLT minlong,
              PLFLT maxlong, PLFLT minlat, PLFLT maxlat );

// Plot map outlines
void c_plmapline( void function( PLINT, PLFLT *, PLFLT * ) mapform, const char *name,
                  PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                  const PLINT *plotentries, PLINT nplotentries );

// Plot map points
void c_plmapstring( void function( PLINT, PLFLT *, PLFLT * ) mapform,
                    const char *name, const char *string,
                    PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                    const PLINT *plotentries, PLINT nplotentries );

// Plot map text
void c_plmaptex( void function( PLINT, PLFLT *, PLFLT * ) mapform,
                 const char *name, PLFLT dx, PLFLT dy, PLFLT just, const char *text,
                 PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                 PLINT plotentry );

// Plot map fills
void c_plmapfill( void function( PLINT, PLFLT *, PLFLT * ) mapform,
                  const char *name, PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
                  const PLINT *plotentries, PLINT nplotentries );

// Plot the latitudes and longitudes on the background.
void c_plmeridians( void function( PLINT, PLFLT *, PLFLT* ) mapform, PLFLT dlong, PLFLT dlat,
                    PLFLT minlong, PLFLT maxlong, PLFLT minlat, PLFLT maxlat );

// Plots a mesh representation of the function z[x][y].
void c_plmesh( PLFLT *x, PLFLT *y, PLFLT **z, PLINT nx, PLINT ny, PLINT opt );

// Plots a mesh representation of the function z[x][y] with contour
void c_plmeshc( PLFLT *x, PLFLT *y, PLFLT **z, PLINT nx, PLINT ny, PLINT opt,
                PLFLT *clevel, PLINT nlevel );

// Creates a new stream and makes it the default.
void c_plmkstrm( PLINT *p_strm );

// Prints out "text" at specified position relative to viewport
void c_plmtex( const char *side, PLFLT disp, PLFLT pos, PLFLT just, const char *text );

// Prints out "text" at specified position relative to viewport (3D)
void c_plmtex3( const char *side, PLFLT disp, PLFLT pos, PLFLT just, const char *text );

// Plots a 3-d representation of the function z[x][y].
void c_plot3d( PLFLT *x, PLFLT *y, PLFLT **z, PLINT nx, PLINT ny, PLINT opt, PLBOOL side );

// Plots a 3-d representation of the function z[x][y] with contour.
void c_plot3dc( PLFLT *x, PLFLT *y, PLFLT **z, PLINT nx, PLINT ny, PLINT opt,
                PLFLT *clevel, PLINT nlevel );

// Plots a 3-d representation of the function z[x][y] with contour and
// y index limits.
void c_plot3dcl( PLFLT *x, PLFLT *y, PLFLT **z, PLINT nx, PLINT ny, PLINT opt,
                 PLFLT *clevel, PLINT nlevel, PLINT ixstart, PLINT ixn,
                 PLINT *indexymin, PLINT *indexymax );

//
//  valid options for plot3dc():
//
//  DRAW_SIDES, BASE_CONT, TOP_CONT (not yet),
//  MAG_COLOR, DRAW_LINEX, DRAW_LINEY, DRAW_LINEXY.
//
//  valid options for plsurf3d():
//
//  MAG_COLOR, BASE_CONT, SURF_CONT, FACETED, DRAW_SIDES.
//

// Set fill pattern directly.
void c_plpat( PLINT nlin, PLINT *inc, PLINT *del );

// Draw a line connecting two points, accounting for coordinate transforms
void c_plpath( PLINT n, PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2 );

// Plots array y against x for n points using ASCII code "code".
void c_plpoin( PLINT n, PLFLT *x, PLFLT *y, PLINT code );

// Draws a series of points in 3 space.
void c_plpoin3( PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, PLINT code );

// Draws a polygon in 3 space.
void c_plpoly3( PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, PLBOOL *draw, PLBOOL ifcc );

// Plots array y against x for n points using (UTF-8) text string
void c_plstring( PLINT n, PLFLT *x, PLFLT *y, const char *text );

// Draws a series of points (described by [UTF8] text string) in 3 space.
void c_plstring3( PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, const char * text );

// Set the floating point precision (in number of places) in numeric labels.
void c_plprec( PLINT setp, PLINT prec );

// Set fill pattern, using one of the predefined patterns.
void c_plpsty( PLINT patt );

// Prints out "text" at world cooordinate (x,y).
void c_plptex( PLFLT x, PLFLT y, PLFLT dx, PLFLT dy, PLFLT just, const char *text );

// Prints out "text" at world cooordinate (x,y,z).
void c_plptex3( PLFLT wx, PLFLT wy, PLFLT wz, PLFLT dx, PLFLT dy, PLFLT dz, PLFLT sx, PLFLT sy, PLFLT sz, PLFLT just, const char *text );

// Random number generator based on Mersenne Twister.
// Obtain real random number in range [0,1].
PLFLT c_plrandd();

// Replays contents of plot buffer to current device/file.
void c_plreplot();

// Functions for converting between HLS and RGB color space

void  c_plrgbhls( PLFLT r, PLFLT g, PLFLT b, PLFLT *p_h, PLFLT *p_l, PLFLT *p_s );

// Set character height.

void  c_plschr( PLFLT def, PLFLT scale );

// Set color map 0 colors by 8 bit RGB values
void c_plscmap0( PLINT *r, PLINT *g, PLINT *b, PLINT ncol0 );

// Set color map 0 colors by 8 bit RGB values and alpha values
void c_plscmap0a( PLINT *r, PLINT *g, PLINT *b, PLFLT *a, PLINT ncol0 );

// Set number of colors in cmap 0
void c_plscmap0n( PLINT ncol0 );

// Set color map 1 colors by 8 bit RGB values
void c_plscmap1( PLINT *r, PLINT *g, PLINT *b, PLINT ncol1 );

// Set color map 1 colors by 8 bit RGB and alpha values
void c_plscmap1a( PLINT *r, PLINT *g, PLINT *b, PLFLT *a, PLINT ncol1 );

// Set color map 1 colors using a piece-wise linear relationship between
// intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.
void c_plscmap1l( PLBOOL itype, PLINT npts, PLFLT *intensity, PLFLT *coord1, PLFLT *coord2, PLFLT *coord3, PLBOOL *alt_hue_path );

// Set color map 1 colors using a piece-wise linear relationship between
// intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.
// Will also linear interpolate alpha values.
void c_plscmap1la( PLBOOL itype, PLINT npts, PLFLT *intensity, PLFLT *coord1, PLFLT *coord2, PLFLT *coord3, PLFLT *a, PLBOOL *alt_hue_path );

// Set number of colors in cmap 1
void c_plscmap1n( PLINT ncol1 );

// Set the color map 1 range used in continuous plots
void c_plscmap1_range( PLFLT min_color, PLFLT max_color );

// Get the color map 1 range used in continuous plots
void c_plgcmap1_range( PLFLT *min_color, PLFLT *max_color );

// Set a given color from color map 0 by 8 bit RGB value
void c_plscol0( PLINT icol0, PLINT r, PLINT g, PLINT b );

// Set a given color from color map 0 by 8 bit RGB value
void c_plscol0a( PLINT icol0, PLINT r, PLINT g, PLINT b, PLFLT a );

// Set the background color by 8 bit RGB value
void c_plscolbg( PLINT r, PLINT g, PLINT b );

// Set the background color by 8 bit RGB value and alpha value
void c_plscolbga( PLINT r, PLINT g, PLINT b, PLFLT a );

// Used to globally turn color output on/off
void c_plscolor( PLINT color );

// Set the compression level

void  c_plscompression( PLINT compression );

// Set the device (keyword) name
void c_plsdev( const char *devname );

// Set window into device space using margin, aspect ratio, and
// justification

void  c_plsdidev( PLFLT mar, PLFLT aspect, PLFLT jx, PLFLT jy );

// Set up transformation from metafile coordinates.

void  c_plsdimap( PLINT dimxmin, PLINT dimxmax, PLINT dimymin, PLINT dimymax, PLFLT dimxpmm, PLFLT dimypmm );

// Set plot orientation, specifying rotation in units of pi/2.

void  c_plsdiori( PLFLT rot );

// Set window into plot space

void  c_plsdiplt( PLFLT xmin, PLFLT ymin, PLFLT xmax, PLFLT ymax );

// Set window into plot space incrementally (zoom)
void c_plsdiplz( PLFLT xmin, PLFLT ymin, PLFLT xmax, PLFLT ymax );

// Set seed for internal random number generator
void c_plseed( uint s );

// Set the escape character for text strings.
void c_plsesc( char esc );

// Set family file parameters

void  c_plsfam( PLINT fam, PLINT num, PLINT bmax );

// Set FCI (font characterization integer)

void  c_plsfci( PLUNICODE fci );

// Set the output file name.
void c_plsfnam( const char *fnam );

// Set the current font family, style and weight

void  c_plsfont( PLINT family, PLINT style, PLINT weight );

// Shade region.
void c_plshade( PLFLT **a, PLINT nx, PLINT ny, PLINT function( PLFLT, PLFLT ) defined, PLFLT left,
                PLFLT right, PLFLT bottom, PLFLT top, PLFLT shade_min, PLFLT shade_max, PLINT sh_cmap,
                PLFLT sh_color, PLFLT sh_width, PLINT min_color, PLFLT min_width, PLINT max_color,
                PLFLT max_width, void function( PLINT, PLFLT *, PLFLT* ) fill, PLBOOL rectangular,
                void function( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer ) pltr, PLPointer pltr_data );

void c_plshades( PLFLT **a, PLINT nx, PLINT ny, PLINT function( PLFLT, PLFLT ) defined, PLFLT xmin,
                 PLFLT xmax, PLFLT ymin, PLFLT ymax, PLFLT *clevel, PLINT nlevel, PLFLT fill_width,
                 PLINT cont_color, PLFLT cont_width, void function( PLINT, PLFLT *, PLFLT* ) fill,
                 PLBOOL rectangular, void function( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer ) pltr,
                 PLPointer pltr_data );

void  plfshade( PLFLT function( PLINT, PLINT, PLPointer ) f2eval, PLPointer f2eval_data, PLFLT function( PLINT, PLINT, PLPointer ) c2eval, PLPointer c2eval_data, PLINT nx, PLINT ny, PLFLT left, PLFLT right, PLFLT bottom, PLFLT top, PLFLT shade_min, PLFLT shade_max, PLINT sh_cmap, PLFLT sh_color, PLFLT sh_width, PLINT min_color, PLFLT min_width, PLINT max_color, PLFLT max_width, void function( PLINT, PLFLT *, PLFLT * ) fill, PLBOOL rectangular, void function( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer ) pltr, PLPointer pltr_data );

// Set up lengths of major tick marks.

void  c_plsmaj( PLFLT def, PLFLT scale );

// Set the memory area to be plotted (with the 'mem' driver)

void  c_plsmem( PLINT maxx, PLINT maxy, void *plotmem );

// Set up lengths of minor tick marks.

void  c_plsmin( PLFLT def, PLFLT scale );

// Set orientation.  Must be done before calling plinit.

void  c_plsori( PLINT ori );

// Set output device parameters.  Usually ignored by the driver.
void c_plspage( PLFLT xp, PLFLT yp, PLINT xleng, PLINT yleng, PLINT xoff, PLINT yoff );

// Set the colors for color table 0 from a cmap0 file
void c_plspal0( const char* filename );

// Set the colors for color table 1 from a cmap1 file
void c_plspal1( const char *filename, PLBOOL interpolate );

// Set the pause (on end-of-page) status
void c_plspause( PLBOOL pause );

// Set stream number.

void  c_plsstrm( PLINT strm );

// Set the number of subwindows in x and y

void  c_plssub( PLINT nx, PLINT ny );

// Set symbol height.

void  c_plssym( PLFLT def, PLFLT scale );

// Initialize PLplot, passing in the windows/page settings.
void c_plstar( PLINT nx, PLINT ny );

// Initialize PLplot, passing the device name and windows/page settings.
void c_plstart( const char *devname, PLINT nx, PLINT ny );

// Set the coordinate transform
void c_plstransform( ct_func coordinate_transform = null, PLPointer coordinate_transform_data = null );

// Add a point to a stripchart.
void c_plstripa( PLINT id, PLINT pen, PLFLT x, PLFLT y );

// Create 1d stripchart
void c_plstripc( PLINT *id, const char *xspec, const char *yspec, PLFLT xmin, PLFLT xmax, PLFLT xjump, PLFLT ymin, PLFLT ymax, PLFLT xlpos, PLFLT ylpos, PLBOOL y_ascl, PLBOOL acc, PLINT colbox, PLINT collab, PLINT *colline, PLINT *styline, const char **legline, const char *labx, const char *laby, const char *labtop );

// Deletes and releases memory used by a stripchart.
void c_plstripd( PLINT id );

// plots a 2d image (or a matrix too large for plshade() )
void c_plimagefr( PLFLT **idata, PLINT nx, PLINT ny, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
                  PLFLT zmin, PLFLT zmax, PLFLT valuemin, PLFLT valuemax,
                  void function( PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer ), PLPointer pltr_data );

// plots a 2d image (or a matrix too large for plshade() ) - colors
// automatically scaled
void c_plimage( PLFLT **idata, PLINT nx, PLINT ny, PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
                PLFLT zmin, PLFLT zmax, PLFLT Dxmin, PLFLT Dxmax, PLFLT Dymin, PLFLT Dymax );

// Set up a new line style
void c_plstyl( PLINT nms, PLINT *mark, PLINT *space );

// Plots the 3d surface representation of the function z[x][y].
void c_plsurf3d( PLFLT *x, PLFLT *y, PLFLT **z, PLINT nx, PLINT ny, PLINT opt,
                 PLFLT *clevel, PLINT nlevel );

// Plots the 3d surface representation of the function z[x][y] with y
// index limits.
void c_plsurf3dl( PLFLT *x, PLFLT *y, PLFLT **z, PLINT nx, PLINT ny, PLINT opt, PLFLT *clevel,
                  PLINT nlevel, PLINT ixstart, PLINT ixn, PLINT *indexymin, PLINT *indexymax );

// Sets the edges of the viewport to the specified absolute coordinates
void c_plsvpa( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax );

// Set x axis labeling parameters
void c_plsxax( PLINT digmax, PLINT digits );

// Set inferior X window
void plsxwin( PLINT window_id );

// Set y axis labeling parameters
void c_plsyax( PLINT digmax, PLINT digits );

// Plots array y against x for n points using Hershey symbol "code"
void c_plsym( PLINT n, PLFLT *x, PLFLT *y, PLINT code );

// Set z axis labeling parameters

void  c_plszax( PLINT digmax, PLINT digits );

// Switches to text screen.

void  c_pltext();

// Set the format for date / time labels
void c_pltimefmt( const char *fmt );

// Sets the edges of the viewport with the given aspect ratio, leaving
// room for labels.

void  c_plvasp( PLFLT aspect );

// Creates the largest viewport of the specified aspect ratio that fits
// within the specified normalized subpage coordinates.

void  c_plvpas( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax, PLFLT aspect );

// Creates a viewport with the specified normalized subpage coordinates.

void  c_plvpor( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax );

// Defines a "standard" viewport with seven character heights for
// the left margin and four character heights everywhere else.

void  c_plvsta();

// Set up a window for three-dimensional plotting.

void  c_plw3d( PLFLT basex, PLFLT basey, PLFLT height, PLFLT xmin0, PLFLT xmax0, PLFLT ymin0, PLFLT ymax0, PLFLT zmin0, PLFLT zmax0, PLFLT alt, PLFLT az );

// Set pen width.

void  c_plwidth( PLFLT width );

// Set up world coordinates of the viewport boundaries (2d plots).

void  c_plwind( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax );

//  set xor mode; mode = 1-enter, 0-leave, status = 0 if not interactive device

void  c_plxormod( PLBOOL mode, PLBOOL *status );

//--------------------------------------------------------------------------* *		Functions for use from C or C++ only
//--------------------------------------------------------------------------

// Returns a list of file-oriented device names and their menu strings

void  plgFileDevs( char ***p_menustr, char ***p_devname, int *p_ndev );

// Returns a list of all device names and their menu strings

void  plgDevs( char ***p_menustr, char ***p_devname, int *p_ndev );

// Set the function pointer for the keyboard event handler

void  plsKeyEH( void function( PLGraphicsIn *, void *, int * ) KeyEH, void *KeyEH_data );

// Set the function pointer for the (mouse) button event handler

void  plsButtonEH( void function( PLGraphicsIn *, void *, int * ) ButtonEH, void *ButtonEH_data );

// Sets an optional user bop handler

void  plsbopH( void function( void *, int * ) handler, void *handler_data );

// Sets an optional user eop handler

void  plseopH( void function( void *, int * ) handler, void *handler_data );

// Set the variables to be used for storing error info

void  plsError( PLINT *errcode, const char *errmsg );

// Sets an optional user exit handler.

void  plsexit( int function( const char * ) handler );

// Sets an optional user abort handler.

void  plsabort( void function( const char * ) handler );

// Transformation routines

// Identity transformation.
void pltr0( PLFLT x, PLFLT y, PLFLT* tx, PLFLT* ty, PLPointer pltr_data );

// Does linear interpolation from singly dimensioned coord arrays.
void pltr1( PLFLT x, PLFLT y, PLFLT* tx, PLFLT* ty, PLPointer pltr_data );

// Does linear interpolation from doubly dimensioned coord arrays
// (column dominant, as per normal C 2d arrays).
void pltr2( PLFLT x, PLFLT y, PLFLT* tx, PLFLT* ty, PLPointer pltr_data );

// Just like pltr2() but uses pointer arithmetic to get coordinates from
// 2d grid tables.
void pltr2p( PLFLT x, PLFLT y, PLFLT* tx, PLFLT* ty, PLPointer pltr_data );

// Identity transformation for plots from Fortran.
void pltr0f( PLFLT x, PLFLT y, PLFLT* tx, PLFLT* ty, void* pltr_data );

// Does linear interpolation from doubly dimensioned coord arrays
// (row dominant, i.e. Fortran ordering).
void pltr2f( PLFLT x, PLFLT y, PLFLT* tx, PLFLT* ty, void* pltr_data );

// Function evaluators

// Does a lookup from a 2d function array.  Array is of type (PLFLT **),
// and is column dominant (normal C ordering).

PLFLT  plf2eval2( PLINT ix, PLINT iy, PLPointer plf2eval_data );

// Does a lookup from a 2d function array.  Array is of type (PLFLT *),
// and is column dominant (normal C ordering).

PLFLT  plf2eval( PLINT ix, PLINT iy, PLPointer plf2eval_data );

// Does a lookup from a 2d function array.  Array is of type (PLFLT *),
// and is row dominant (Fortran ordering).

PLFLT  plf2evalr( PLINT ix, PLINT iy, PLPointer plf2eval_data );

// Command line parsing utilities
// Clear internal option table info structure.
void plClearOpts();

// Reset internal option table info structure.
void plResetOpts();

// Merge user option table into internal info structure.

PLINT  plMergeOpts( PLOptionTable *options, const char *name, const char **notes );

// Set the strings used in usage and syntax messages.

void  plSetUsage( const char *program_string, const char *usage_string );

// Process input strings, treating them as an option and argument pair.
PLINT c_plsetopt( const char *opt, const char *optarg );

// Process options list using current options info.
PLINT c_plparseopts( int *p_argc, char **argv, PLINT mode );

// Print usage & syntax message.

void  plOptUsage();

// Miscellaneous

// Get the escape character for text strings.

void  plgesc( char *p_esc );

// Front-end to driver escape function.

void  pl_cmd( PLINT op, void *ptr );

// Return full pathname for given file if executable

PLINT  plFindName( char *p );

// Looks for the specified executable file according to usual search path.

char * plFindCommand( const char *fn );

// Gets search name for file by concatenating the dir, subdir, and file
// name, allocating memory as needed.

void  plGetName( const char *dir, const char *subdir, const char *filename, char **filespec );

// Prompts human to input an integer in response to given message.

PLINT  plGetInt( const char *s );

// Prompts human to input a float in response to given message.

PLFLT  plGetFlt( const char *s );

// Nice way to allocate space for a vectored 2d grid

// Allocates a block of memory for use as a 2-d grid of PLFLT's.

void  plAlloc2dGrid( PLFLT ***f, PLINT nx, PLINT ny );

// Frees a block of memory allocated with plAlloc2dGrid().

void  plFree2dGrid( PLFLT **f, PLINT nx, PLINT ny );

// Find the maximum and minimum of a 2d matrix allocated with plAllc2dGrid().
void plMinMax2dGrid( PLFLT **f, PLINT nx, PLINT ny, PLFLT *fmax, PLFLT *fmin );

// Wait for graphics input event and translate to world coordinates

PLINT  plGetCursor( PLGraphicsIn *gin );

// Translates relative device coordinates to world coordinates.

PLINT  plTranslateCursor( PLGraphicsIn *gin );

