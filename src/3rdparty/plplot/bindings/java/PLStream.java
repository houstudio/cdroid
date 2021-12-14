//--------------------------------------------------------------------------
//
// Copyright (C) 2004  Andrew Ross
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// This class provides a more object orientated wrapper to the PLplot library
// for java. It is currently very similar to the C++ plstream class.
// Each instance of the class corresponds to a plplot stream. Calling a
// method in the class will ensure the stream is correctly set before
// calling the underlying API function.
//

package plplot.core;

import java.io.*;

public class PLStream implements plplotjavacConstants {
// Class data.
    int        stream_id = -1;

    static int active_streams = 0;

// Constructor
    public PLStream()
    {
        int[] strm = new int[1];

        // If this is the first instance of the class we
        // need to load the C part of the bindings
        if ( active_streams == 0 )
        {
            openlib();
        }

        // Create stream and check it worked ok.
        plplotjavac.plmkstrm( strm );
        if ( strm[0] != -1 )
        {
            stream_id = strm[0];
            active_streams++;
        }
        else
        {
            System.err.println( "Error creating plplot stream" );
            stream_id = -1;
        }
    }

// Ensure this is the current stream
    public int set_stream()
    {
        if ( ( stream_id == -1 ) || ( active_streams == 0 ) )
        {
            System.err.println( "Error: This stream is not active" );
            return -1;
        }
        plplotjavac.plsstrm( stream_id );
        return 0;
    }

// Method to load the native C part of the java wrapper
    public void openlib()
    {
        File libname = null;

        try {
            String libdir = System.getProperty( "plplot.libdir" );
            libname = new File( libdir + File.separatorChar + plplot.core.config.libname );
            if ( !libname.exists() )
            {
                libname = null;
            }
        } catch ( Exception e ) {
        }
        if ( libname == null )
        {
            libname = new File( plplot.core.config.libdir + File.separatorChar + plplot.core.config.libname );
            if ( !libname.exists() )
            {
                libname = null;
            }
        }
        if ( libname != null )
        {
            try {
                System.load( libname.getAbsolutePath() );
            } catch ( UnsatisfiedLinkError e ) {
                System.err.println( "Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e );
                System.exit( 1 );
            }
        }
        else
        {
            System.err.println( "Unable to find native code library.\n" );
            System.exit( 1 );
        }
    }

// The following are wrappers to the C API methods, or their derivatives

    public void setcontlabelformat( int lexp, int sigdig )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pl_setcontlabelformat( lexp, sigdig );
    }

    public void setcontlabelparam( double offset, double size, double spacing, int active )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pl_setcontlabelparam( offset, size, spacing, active );
    }

    public void adv( int page )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pladv( page );
    }

    public void arc( double x, double y, double a, double b, double angle1, double angle2, double rotate, boolean fill )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plarc( x, y, a, b, angle1, angle2, rotate, fill );
    }

    public void axes( double x0, double y0, String xopt, double xtick, int nxsub,
                      String yopt, double ytick, int nysub )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plaxes( x0, y0, xopt, xtick, nxsub, yopt, ytick, nysub );
    }

    public void bin( double[] x, double[] y, int center )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plbin( x, y, center );
    }

    public void bop()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plbop();
    }

    public void box( String xopt, double xtick, int nxsub,
                     String yopt, double ytick, int nysub )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plbox( xopt, xtick, nxsub, yopt, ytick, nysub );
    }

    public void box3( String xopt, String xlabel, double xtick, int nsubx,
                      String yopt, String ylabel, double ytick, int nsuby,
                      String zopt, String zlabel, double ztick, int nsubz )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plbox3( xopt, xlabel, xtick, nsubx, yopt, ylabel, ytick, nsuby,
            zopt, zlabel, ztick, nsubz );
    }

    public void btime( int year[], int month[], int day[], int hour[], int min[], double sec[], double ctime )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plbtime( year, month, day, hour, min, sec, ctime );
    }

    public void calc_world( double rx, double ry, double[] wx, double[] wy, int[] window )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plcalc_world( rx, ry, wx, wy, window );
    }

    public void clear()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plclear();
    }

    public void col0( int icol0 )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plcol0( icol0 );
    }

    public void col1( double col1 )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plcol1( col1 );
    }

    public void configtime( double scale, double offset1, double offset2,
                            int ccontrol, boolean ifbtime_offset, int year,
                            int month, int day, int hour, int min,
                            double sec )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plconfigtime( scale, offset1, offset2, ccontrol, ifbtime_offset,
            year, month, day, hour, min, sec );
    }

    public void cont( double[][] f, int kx, int lx, int ky, int ly,
                      double[] clevel, double[][] pltr, double[][] OBJECT_DATA )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plcont( f, kx, lx, ky, ly, clevel, pltr, OBJECT_DATA );
    }

    public void cpstrm( PLStream pls, boolean flags )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plcpstrm( pls.stream_id, flags );
    }

    public void ctime( int year, int month, int day, int hour, int min, double sec, double ctime[] )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plctime( year, month, day, hour, min, sec, ctime );
    }

// The end / end1 functions have extra code in to keep track of the
// stream references in the class.
    public void end()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plend();
        active_streams = 0;
        stream_id      = -1;
    }

    public void end1()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plend1();

        active_streams--;
        stream_id = -1;
    }

    public void env( double xmin, double xmax, double ymin, double ymax, int just, int axis )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plenv( xmin, xmax, ymin, ymax, just, axis );
    }

    public void env0( double xmin, double xmax, double ymin, double ymax, int just, int axis )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plenv0( xmin, xmax, ymin, ymax, just, axis );
    }

    public void eop()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pleop();
    }

    public void errx( double[] xmin, double[] xmax, double[] y )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plerrx( xmin, xmax, y );
    }

    public void erry( double[] x, double[] ymin, double[] ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plerry( x, ymin, ymax );
    }

    public void famadv()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plfamadv();
    }

    public void fill( double[] x, double[] y )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plfill( x, y );
    }

    public void fill3( double[] x, double[] y, double[] z )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plfill3( x, y, z );
    }

    public void flush()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plflush();
    }

    public void font( int ifont )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plfont( ifont );
    }

    public void fontld( int fnt )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plfontld( fnt );
    }

    public void gchr( double[] p_def, double[] p_ht )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgchr( p_def, p_ht );
    }

    public void gcol0( int icol0, int[] r, int[] g, int[] b )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgcol0( icol0, r, g, b );
    }

    public void gcol0a( int icol0, int[] r, int[] g, int[] b, double[] a )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgcol0a( icol0, r, g, b, a );
    }

    public void gcolbg( int[] r, int[] g, int[] b )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgcolbg( r, g, b );
    }

    public void gcolbga( int[] r, int[] g, int[] b, double[] a )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgcolbga( r, g, b, a );
    }

    public void gcompression( int[] compression )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgcompression( compression );
    }

    public void gdev( StringBuffer dev )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgdev( dev );
    }

    public void gdidev( double[] mar, double[] aspect, double[] jx, double[] jy )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgdidev( mar, aspect, jx, jy );
    }

    public void gdiori( double[] rot )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgdiori( rot );
    }

    public void gdiplt( double[] xmin, double[] xmax, double[] ymin, double[] ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgdiplt( xmin, xmax, ymin, ymax );
    }

    public int getCursor( PLGraphicsIn gin )
    {
        if ( set_stream() == -1 ) return 0;
        return plplotjavac.plGetCursor( gin );
    }

    public void gfam( int[] fam, int[] num, int[]  bmax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgfam( fam, num, bmax );
    }

    public void gfci( long[] pfci )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgfci( pfci );
    }

    public void gfnam( StringBuffer fnam )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgfnam( fnam );
    }

    public void gfont( int[] family, int[] style, int[] weight )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgfont( family, style, weight );
    }

    public void glevel( int[] p_level )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plglevel( p_level );
    }

    public void gpage( double[] xp, double[] yp, int[] xleng, int[] yleng, int[] xoff, int[] yoff )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgpage( xp, yp, xleng, yleng, xoff, yoff );
    }

    public void gra()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgra();
    }

    public void gradient( double[] x, double[] y, double angle )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgradient( x, y, angle );
    }

    public void griddata( double[] x, double[] y, double[] z, double[] xg,
                          double[] yg, double[][] zg, int type, double data )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgriddata( x, y, z, xg, yg, zg, type, data );
    }

    public void gspa( double[] xmin, double[] xmax, double[] ymin, double[] ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgspa( xmin, xmax, ymin, ymax );
    }

// Note: The user should never need this in with this class
// since the stream is encapsulated in the class.
//public void gstrm(int[] p_strm) {
//    if (set_stream() == -1) return;
//    plplotjavac.plgstrm(p_strm);
//}

    public void gver( StringBuffer ver )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgver( ver );
    }

    public void gvpd( double[] xmin, double[] xmax, double[] ymin, double[] ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgvpd( xmin, xmax, ymin, ymax );
    }

    public void gvpw( double[] xmin, double[] xmax, double[] ymin, double[] ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgvpw( xmin, xmax, ymin, ymax );
    }

    public void gxax( int[] digmax, int[] digits )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgxax( digmax, digits );
    }

    public void gyax( int[] digmax, int[] digits )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgyax( digmax, digits );
    }

    public void gzax( int[] digmax, int[] digits )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgzax( digmax, digits );
    }

    public void hist( double[] data, double datmin, double datmax, int nbin, int oldwin )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plhist( data, datmin, datmax, nbin, oldwin );
    }

    public void image( double[][] data, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, double Dxmin, double Dxmax, double Dymin, double Dymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plimage( data, xmin, xmax, ymin, ymax, zmin, zmax, Dxmin, Dxmax, Dymin, Dymax );
    }

    public void imagefr( double[][] data, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, double valuemin, double valuemax, double[][] pltr_im, double[][] OBJECT_DATA_im )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plimagefr( data, xmin, xmax, ymin, ymax, zmin, zmax, valuemin, valuemax, pltr_im, OBJECT_DATA_im );
    }

    public void init()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plinit();
    }

    public void join( double x1, double y1, double x2, double y2 )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pljoin( x1, y1, x2, y2 );
    }

    public void lab( String xlabel, String ylabel, String tlabel )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pllab( xlabel, ylabel, tlabel );
    }

    public void legend( double[] p_legend_width, double[] p_legend_height,
                        int opt, int position, double x, double y, double plot_width,
                        int bg_color, int bb_color, int bb_style,
                        int nrow, int ncolumn, int[] opt_array,
                        double text_offset, double text_scale,
                        double text_spacing, double text_justification,
                        int[] text_colors, String[] text,
                        int[] box_colors, int[] box_patterns,
                        double[] box_scales, double[] box_line_widths,
                        int[] line_colors, int[] line_styles,
                        double[] line_widths,
                        int[] symbol_colors, double[] symbol_scales,
                        int[] symbol_numbers, String[] symbols )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pllegend( p_legend_width, p_legend_height,
            opt, position, x, y, plot_width, bg_color, bb_color,
            bb_style, nrow, ncolumn, opt_array,
            text_offset, text_scale, text_spacing,
            text_justification, text_colors, text,
            box_colors, box_patterns, box_scales,
            box_line_widths, line_colors, line_styles,
            line_widths, symbol_colors, symbol_scales,
            symbol_numbers, symbols );
    }


    public void colorbar( double[] p_colorbar_width,
                          double[] p_colorbar_height,
                          int opt, int position, double x, double y,
                          double x_length, double y_length,
                          int bg_color, int bb_color, int bb_style,
                          double low_cap_color, double high_cap_color,
                          int cont_color, double cont_width,
                          int[] label_opts, String[] labels,
                          String[] axis_opts,
                          double[] ticks, int[] sub_ticks,
                          int[] n_values, double[][] values )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plcolorbar( p_colorbar_width, p_colorbar_height,
            opt, position, x, y, x_length, y_length,
            bg_color, bb_color, bb_style,
            low_cap_color, high_cap_color,
            cont_color, cont_width,
            label_opts, labels, axis_opts,
            ticks, sub_ticks,
            n_values, values );
    }


    public void lightsource( double x, double y, double z )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pllightsource( x, y, z );
    }

    public void line( double[] x, double[] y )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plline( x, y );
    }

    public void line3( double[] x, double[] y, double[] z )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plline3( x, y, z );
    }

    public void lsty( int lin )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pllsty( lin );
    }

    public void map( PLCallbackMapform mapform, String type, double minlong, double maxlong, double minlat, double maxlat )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmap( mapform, type, minlong, maxlong, minlat, maxlat );
    }

    public void mapline( PLCallbackMapform mapform, String type, double minlong, double maxlong, double minlat, double maxlat, int[] plotentries )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmapline( mapform, type, minlong, maxlong, minlat, maxlat, plotentries );
    }

    public void mapstring( PLCallbackMapform mapform, String type, String string, double minlong, double maxlong, double minlat, double maxlat, int[] plotentries )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmapstring( mapform, type, string, minlong, maxlong, minlat, maxlat, plotentries );
    }

    public void maptex( PLCallbackMapform mapform, String type, double dx, double dy, double just, String text, double minlong, double maxlong, double minlat, double maxlat, int plotentry )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmaptex( mapform, type, dx, dy, just, text, minlong, maxlong, minlat, maxlat, plotentry );
    }

    public void mapfill( PLCallbackMapform mapform, String type, double minlong, double maxlong, double minlat, double maxlat, int[] plotentries )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmapfill( mapform, type, minlong, maxlong, minlat, maxlat, plotentries );
    }

    public void meridians( PLCallbackMapform mapform, double dlong, double dlat, double minlong, double maxlong, double minlat, double maxlat )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmeridians( mapform, dlong, dlat, minlong, maxlong, minlat, maxlat );
    }

    public void minMax2dGrid( double[][] f, double[] fmax, double[] fmin )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plMinMax2dGrid( f, fmax, fmin );
    }

    public void mesh( double[] x, double[] y, double[][] z, int opt )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmesh( x, y, z, opt );
    }

    public void meshc( double[] x, double[] y, double[][] z, int opt, double[] clevel )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmeshc( x, y, z, opt, clevel );
    }

// Don't need this in the OO approach - create a new object instead.
//public void mkstrm(int[] OUTPUT) {
//    if (set_stream() == -1) return;
//    plplotjavac.plmkstrm(int[] OUTPUT);
//}

    public void mtex( String side, double disp, double pos, double just, String text )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmtex( side, disp, pos, just, text );
    }

    public void mtex3( String side, double disp, double pos, double just, String text )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plmtex3( side, disp, pos, just, text );
    }

    public void plot3d( double[] x, double[] y, double[][] z, int opt, boolean side )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plot3d( x, y, z, opt, side );
    }

    public void plot3dc( double[] x, double[] y, double[][] z, int opt, double[] clevel )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plot3dc( x, y, z, opt, clevel );
    }

    public void plot3dcl( double[] x, double[] y, double[][] z, int opt,
                          double[] clevel, int ixstart, int[] indexymin, int[] indexymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plot3dcl( x, y, z, opt, clevel, ixstart, indexymin, indexymax );
    }

    public void surf3d( double[] x, double[] y, double[][] z, int opt, double[] clevel )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsurf3d( x, y, z, opt, clevel );
    }

    public void surf3dl( double[] x, double[] y, double[][] z, int opt,
                         double[] clevel, int ixstart, int[] indexymin, int[] indexymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsurf3dl( x, y, z, opt, clevel, ixstart, indexymin, indexymax );
    }

    public int parseopts( String[] argv, int mode )
    {
        if ( set_stream() == -1 ) return 0;
        return plplotjavac.plparseopts( argv, mode );
    }

    public void pat( int[] inc, int[] del )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plpat( inc, del );
    }

    public void path( int n, double x1, double y1, double x2, double y2 )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plpath( n, x1, y1, x2, y2 );
    }

    public void poin( double[] x, double[] y, int code )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plpoin( x, y, code );
    }

    public void poin3( double[] x, double[] y, double[] z, int code )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plpoin3( x, y, z, code );
    }

    public void poly3( double[] x, double[] y, double[] z, boolean[] draw, boolean ifcc )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plpoly3( x, y, z, draw, ifcc );
    }

    public void prec( int setp, int prec )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plprec( setp, prec );
    }

    public void psty( int patt )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plpsty( patt );
    }

    public void ptex( double x, double y, double dx, double dy, double just, String text )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plptex( x, y, dx, dy, just, text );
    }

    public void ptex3( double x, double y, double z, double dx, double dy, double dz, double sx, double sy, double sz, double just, String text )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plptex3( x, y, z, dx, dy, dz, sx, sy, sz, just, text );
    }

    public double randd()
    {
        if ( set_stream() == -1 ) return 0.0;
        return plplotjavac.plrandd();
    }

    public void replot()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plreplot();
    }

    public void schr( double def, double scale )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plschr( def, scale );
    }

    public void scmap0( int[] r, int[] g, int[] b )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap0( r, g, b );
    }

    public void scmap0a( int[] r, int[] g, int[] b, double[] a )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap0a( r, g, b, a );
    }

    public void scmap0n( int ncol0 )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap0n( ncol0 );
    }

    public void scmap1( int[] r, int[] g, int[] b )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap1( r, g, b );
    }

    public void scmap1a( int[] r, int[] g, int[] b, double[] a )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap1a( r, g, b, a );
    }

    public void scmap1l( boolean itype, double[] intensity, double[] coord1,
                         double[] coord2, double[] coord3, boolean[] alt_hue_path )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap1l( itype, intensity, coord1, coord2, coord3, alt_hue_path );
    }

    public void scmap1l( boolean itype, double[] intensity, double[] coord1,
                         double[] coord2, double[] coord3 )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap1l( itype, intensity, coord1, coord2, coord3, null );
    }

    public void scmap1la( boolean itype, double[] intensity, double[] coord1,
                          double[] coord2, double[] coord3, double[] a, boolean[] alt_hue_path )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap1la( itype, intensity, coord1, coord2, coord3, a, alt_hue_path );
    }

    public void scmap1la( boolean itype, double[] intensity, double[] coord1,
                          double[] coord2, double[] coord3, double[] a )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap1la( itype, intensity, coord1, coord2, coord3, a, null );
    }

    public void scmap1n( int ncol1 )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap1n( ncol1 );
    }

    public void scmap1_range( double min_color, double max_color )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscmap1_range( min_color, max_color );
    }

    public void gcmap1_range( double[] min_color, double[] max_color )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plgcmap1_range( min_color, max_color );
    }

    public void scol0( int icol0, int r, int g, int b )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscol0( icol0, r, g, b );
    }

    public void scol0a( int icol0, int r, int g, int b, double a )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscol0a( icol0, r, g, b, a );
    }

    public void scolbg( int r, int g, int b )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscolbg( r, g, b );
    }

    public void scolbga( int r, int g, int b, double a )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscolbga( r, g, b, a );
    }

    public void scolor( int color )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscolor( color );
    }

    public void scompression( int compression )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plscompression( compression );
    }

    public void sdev( String devname )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsdev( devname );
    }

    public void sdidev( double mar, double aspect, double jx, double jy )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsdidev( mar, aspect, jx, jy );
    }

    public void sdimap( int dimxmin, int dimxmax, int dimymin, int dimymax,
                        double dimxpmm, double dimypmm )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsdimap( dimxmin, dimxmax, dimymin, dimymax, dimxpmm, dimypmm );
    }

    public void sdiori( double rot )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsdiori( rot );
    }

    public void sdiplt( double xmin, double ymin, double xmax, double ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsdiplt( xmin, ymin, xmax, ymax );
    }

    public void sdiplz( double xmin, double ymin, double xmax, double ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsdiplz( xmin, ymin, xmax, ymax );
    }

    public void seed( long s )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plseed( s );
    }

    public void sesc( char esc )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsesc( esc );
    }

    public int setopt( String opt, String optarg )
    {
        if ( set_stream() == -1 ) return 0;
        return plplotjavac.plsetopt( opt, optarg );
    }

    public void sfam( int fam, int num, int bmax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsfam( fam, num, bmax );
    }

    public void sfci( long fci )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsfci( fci );
    }

    public void sfnam( String fnam )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsfnam( fnam );
    }

    public void sfont( int family, int style, int weight )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsfont( family, style, weight );
    }

    public void shades( double[][] a, double xmin, double xmax, double ymin,
                        double ymax, double[] clevel, double fill_width, int cont_color,
                        double cont_width, boolean rectangular,
                        double[][] pltr, double[][] OBJECT_DATA )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plshades( a, xmin, xmax, ymin, ymax, clevel, fill_width,
            cont_color, cont_width, rectangular, pltr, OBJECT_DATA );
    }

    public void shade( double[][] a, double left, double right, double bottom,
                       double top, double shade_min, double shade_max, int sh_cmap,
                       double sh_color, double sh_width, int min_color, double min_width,
                       int max_color, double max_width, boolean rectangular,
                       double[][] pltr, double[][] OBJECT_DATA )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plshade( a, left, right, bottom, top, shade_min, shade_max,
            sh_cmap, sh_color, sh_width, min_color, min_width,
            max_color, max_width, rectangular, pltr, OBJECT_DATA );
    }

    public void slabelfunc( PLCallbackLabel label, Object obj )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plslabelfunc( label, obj );
    }

    public void slabelfunc( PLCallbackLabel label )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plslabelfunc( label, null );
    }

    public void smaj( double def, double scale )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsmaj( def, scale );
    }

    public void smin( double def, double scale )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsmin( def, scale );
    }

    public void sori( int ori )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsori( ori );
    }

    public void spage( double xp, double yp, int xleng, int yleng, int xoff, int yoff )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plspage( xp, yp, xleng, yleng, xoff, yoff );
    }

    public void spal0( String filename )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plspal0( filename );
    }

    public void spal1( String filename, boolean interpolate )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plspal1( filename, interpolate );
    }

    public void spause( boolean pause )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plspause( pause );
    }

    public void sstrm( int strm )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsstrm( strm );
    }

    public void ssub( int nx, int ny )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plssub( nx, ny );
    }

    public void ssym( double def, double scale )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plssym( def, scale );
    }

    public void star( int nx, int ny )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstar( nx, ny );
    }

    public void start( String devname, int nx, int ny )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstart( devname, nx, ny );
    }

    public void stransform( PLCallbackCT coordTrans, Object data )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstransform( coordTrans, data );
    }

    public void string( double[] x, double[] y, String string )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstring( x, y, string );
    }

    public void string3( double[] x, double[] y, double[] z, String string )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstring3( x, y, z, string );
    }

    public void stripa( int id, int pen, double x, double y )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstripa( id, pen, x, y );
    }

    public void stripc( int[] id, String xspec, String yspec,
                        double xmin, double xmax, double xjump,
                        double ymin, double ymax, double xlpos, double ylpos,
                        boolean y_ascl, boolean acc, int colbox, int collab,
                        int[] colline, int[] styline, String[] legline,
                        String labx, String laby, String labtop )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstripc( id, xspec, yspec, xmin, xmax, xjump, ymin, ymax,
            xlpos, ylpos, y_ascl, acc, colbox, collab, colline,
            styline, legline, labx, laby, labtop );
    }

    public void stripd( int id )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstripd( id );
    }

    public void styl( int[] mark, int[] space )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstyl( mark, space );
    }

    public void svect( double[] arrow_x, double[] arrow_y, boolean fill )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsvect( arrow_x, arrow_y, fill );
    }

    public void svpa( double xmin, double xmax, double ymin, double ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsvpa( xmin, xmax, ymin, ymax );
    }

    public void sxax( int digmax, int digits )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsxax( digmax, digits );
    }

    public void syax( int digmax, int digits )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsyax( digmax, digits );
    }

    public void sym( double[] x, double[] y, int code )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsym( x, y, code );
    }

    public void szax( int digmax, int digits )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plszax( digmax, digits );
    }

    public void text()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pltext();
    }

    public void timefmt( String fmt )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.pltimefmt( fmt );
    }

    public void vasp( double aspect )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plvasp( aspect );
    }

    public void vect( double[][] u, double[][] v, double scale, double[][] pltr, double[][] OBJECT_DATA )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plvect( u, v, scale, pltr, OBJECT_DATA );
    }

    public void vpas( double xmin, double xmax, double ymin, double ymax, double aspect )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plvpas( xmin, xmax, ymin, ymax, aspect );
    }

    public void vpor( double xmin, double xmax, double ymin, double ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plvpor( xmin, xmax, ymin, ymax );
    }

    public void vsta()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plvsta();
    }

    public void w3d( double basex, double basey, double height, double xmin0,
                     double xmax0, double ymin0, double ymax0, double zmin0,
                     double zmax0, double alt, double az )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plw3d( basex, basey, height, xmin0, xmax0, ymin0, ymax0,
            zmin0, zmax0, alt, az );
    }

    public void width( double width )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plwidth( width );
    }

    // Overloaded version with integer width.
    public void width( int width )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plwidth( width );
    }

    public void wind( double xmin, double xmax, double ymin, double ymax )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plwind( xmin, xmax, ymin, ymax );
    }

    public void xormod( boolean mode, boolean[] status )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plxormod( mode, status );
    }

    public void ClearOpts()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plClearOpts();
    }

    public void ResetOpts()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plResetOpts();
    }

    public void SetUsage( String program_string, String usage_string )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plSetUsage( program_string, usage_string );
    }

    public void OptUsage()
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plOptUsage();
    }

    public void hlsrgb( double h, double l, double s, double[] r, double[] g, double[] b )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plhlsrgb( h, l, s, r, g, b );
    }

    public void rgbhls( double r, double g, double b, double[] h, double[] l, double[] s )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plrgbhls( r, g, b, h, l, s );
    }

// Deprecated versions of methods which use int for a flag instead of
// boolean.
    public void cpstrm( int iplsr, int flags )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plcpstrm( iplsr, flags != 0 );
    }

    public void plot3d( double[] x, double[] y, double[][] z, int opt, int side )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plot3d( x, y, z, opt, side != 0 );
    }

    public void poly3( double[] x, double[] y, double[] z, int[] draw, int ifcc )
    {
        if ( set_stream() == -1 ) return;
        boolean [] loc_draw = new boolean[draw.length];
        for ( int i = 0; i < draw.length; i++ )
        {
            loc_draw[i] = ( draw[i] != 0 );
        }
        plplotjavac.plpoly3( x, y, z, loc_draw, ifcc != 0 );
    }

    public void scmap1l( int itype, double[] intensity, double[] coord1,
                         double[] coord2, double[] coord3, int[] alt_hue_path )
    {
        if ( set_stream() == -1 ) return;
        boolean [] loc_alt_hue_path = null;
        if ( alt_hue_path != null )
        {
            loc_alt_hue_path = new boolean[alt_hue_path.length];
            for ( int i = 0; i < alt_hue_path.length; i++ )
            {
                loc_alt_hue_path[i] = ( alt_hue_path[i] != 0 );
            }
        }
        plplotjavac.plscmap1l( itype != 0, intensity, coord1, coord2, coord3, loc_alt_hue_path );
    }

    public void shades( double[][] a, double xmin, double xmax, double ymin,
                        double ymax, double[] clevel, double fill_width, int cont_color,
                        double cont_width, int rectangular,
                        double[][] pltr, double[][] OBJECT_DATA )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plshades( a, xmin, xmax, ymin, ymax, clevel, fill_width,
            cont_color, cont_width, rectangular != 0, pltr, OBJECT_DATA );
    }

    public void shade( double[][] a, double left, double right, double bottom,
                       double top, double shade_min, double shade_max, int sh_cmap,
                       double sh_color, double sh_width, int min_color, double min_width,
                       int max_color, double max_width, int rectangular,
                       double[][] pltr, double[][] OBJECT_DATA )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plshade( a, left, right, bottom, top, shade_min, shade_max,
            sh_cmap, sh_color, sh_width, min_color, min_width,
            max_color, max_width, rectangular != 0, pltr, OBJECT_DATA );
    }

    public void spause( int pause )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plspause( pause != 0 );
    }

    public void stripc( int[] id, String xspec, String yspec,
                        double xmin, double xmax, double xjump,
                        double ymin, double ymax, double xlpos, double ylpos,
                        int y_ascl, int acc, int colbox, int collab,
                        int[] colline, int[] styline, String[] legline,
                        String labx, String laby, String labtop )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plstripc( id, xspec, yspec, xmin, xmax, xjump, ymin, ymax,
            xlpos, ylpos, y_ascl != 0, acc != 0, colbox, collab,
            colline, styline, legline, labx, laby, labtop );
    }

    public void svect( double[] arrow_x, double[] arrow_y, int fill )
    {
        if ( set_stream() == -1 ) return;
        plplotjavac.plsvect( arrow_x, arrow_y, fill != 0 );
    }

    public void xormod( int mode, int[] status )
    {
        if ( set_stream() == -1 ) return;
        boolean [] loc_status = new boolean[1];
        plplotjavac.plxormod( mode != 0, loc_status );
        status[0] = loc_status[0] ? 1 : 0;
    }
}
