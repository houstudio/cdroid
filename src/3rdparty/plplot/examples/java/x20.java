//--------------------------------------------------------------------------
// Copyright (C) 2004,2006  Andrew Ross
// Copyright (C) 2004-2014  Alan W. Irwin
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
// Implementation of PLplot example 20 in Java.
//--------------------------------------------------------------------------

//
// Current user defined command line options are not supported in
// the Java bindings
//

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

import java.io.*;
import java.util.*;

class x20 {
    // Class data
    PLStream       pls = new PLStream();

    static int     XDIM          = 260;
    static int     YDIM          = 220;
    static boolean dbg           = false;
    static boolean nosombrero    = false;
    static boolean nointeractive = false;
    static String  f_name        = null;

    //static PLOptionTable options[];

    // PLOptionTable options[] = {
    // {
    //     "dbg",			/* extra debugging plot */
    //     NULL,
    //     NULL,
    //     &dbg,
    //     PL_OPT_BOOL,
    //     "-dbg",
    //     "Extra debugging plot" },
    // {
    //     "nosombrero",			/* Turns on test of xor function */
    //     NULL,
    //     NULL,
    //     &nosombrero,
    //     PL_OPT_BOOL,
    //     "-nosombrero",
    //     "No sombrero plot" },
    // {
    //     "nointeractive",			/* Turns on test of xor function */
    //     NULL,
    //     NULL,
    //     &nointeractive,
    //     PL_OPT_BOOL,
    //     "-nointeractive",
    //     "No interactive selection" },
    // {
    //     "save",			/* For saving in postscript */
    //     NULL,
    //     NULL,
    //     &f_name,
    //     PL_OPT_STRING,
    //     "-save filename",
    //       "Save sombrero plot in color postscript `filename'" },
    // {
    //     NULL,			/* option */
    //     NULL,			/* handler */
    //     NULL,			/* client data */
    //     NULL,			/* address of variable to set */
    //     0,				/* mode flag */
    //     NULL,			/* short syntax */
    //     NULL }			/* long syntax */
    // };


    x20( String[] args )
    {
        double x[]   = new double[XDIM];
        double y[]   = new double[YDIM];
        double z[][] = new double[XDIM][YDIM];
        double r[][];
        double xi[] = new double[1];
        double yi[] = new double[1];
        double xe[] = new double[1];
        double ye[] = new double[1];
        int    i, j, width, height, num_col;
        int    n[] = new int[1];
        double img_f[][];
        double img_min;
        double img_max;
        double maxmin[] = new double[2];
        double x0, y0, dy, stretch;
        double deltax, deltay, xg[][], yg[][];


        // plplot initialization

        // Parse and process command line arguments.
        //pls.MergeOpts(options, "x20c options", NULL);
        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        // Initialize PLplot.
        pls.init();

        // view image border pixels
        if ( dbg )
        {
            pls.env( 1., XDIM, 1., YDIM, 1, 1 ); // no plot box

            // build a one pixel square border, for diagnostics
            for ( i = 0; i < XDIM; i++ )
                z[i][YDIM - 1] = 1.; // right
            for ( i = 0; i < XDIM; i++ )
                z[i][0] = 1.;        // left

            for ( i = 0; i < YDIM; i++ )
                z[0][i] = 1.;        // top
            for ( i = 0; i < YDIM; i++ )
                z[XDIM - 1][i] = 1.; // botton

            pls.lab( "...around a blue square.", " ", "A red border should appear..." );

            pls.image( z, 1., XDIM, 1., YDIM, 0., 0.,
                1., XDIM, 1., YDIM );
        }

        // sombrero-like demo
        if ( !nosombrero )
        {
            r = new double[XDIM][YDIM];
            pls.col0( 2 ); // draw a yellow plot box, useful for diagnostics! :(
            pls.env( 0., 2. * Math.PI, 0, 3. * Math.PI, 1, -1 );

            for ( i = 0; i < XDIM; i++ )
                x[i] = i * 2. * Math.PI / ( XDIM - 1 );
            for ( i = 0; i < YDIM; i++ )
                y[i] = i * 3. * Math.PI / ( YDIM - 1 );

            for ( i = 0; i < XDIM; i++ )
                for ( j = 0; j < YDIM; j++ )
                {
                    r[i][j] = Math.sqrt( x[i] * x[i] + y[j] * y[j] ) + 1e-3;
                    z[i][j] = Math.sin( r[i][j] ) / ( r[i][j] );
                }

            pls.lab( "No, an amplitude clipped \"sombrero\"", "", "Saturn?" );
            pls.ptex( 2., 2., 3., 4., 0., "Transparent image" );
            pls.image( z, 0., 2. * Math.PI, 0., 3. * Math.PI, 0.05, 1.,
                0., 2. * Math.PI, 0., 3. * Math.PI );

            // save the plot
            if ( f_name != null )
                save_plot( f_name );
        }


        // read Chloe image
        if ( ( img_f = read_img( "Chloe.pgm", n ) ) == null )
        {
            if ( ( img_f = read_img( "../Chloe.pgm", n ) ) == null )
            {
                System.out.println( "File error - aborting" );
                pls.end();
                System.exit( 1 );
            }
        }
        num_col = n[0];
        width   = img_f.length;
        height  = img_f[0].length;

        // set gray colormap
        gray_cmap( num_col );

        // display Chloe
        pls.env( 1., width, 1., height, 1, -1 );

        if ( !nointeractive )
            pls.lab( "Set and drag Button 1 to (re)set selection, Button 2 to finish.", " ", "Chloe..." );
        else
            pls.lab( "", " ", "Chloe..." );

        pls.image( img_f, 1., width, 1., height, 0., 0.,
            1., width, 1., height );

        // selection/expansion demo
        if ( !nointeractive )
        {
            xi[0] = 25.; xe[0] = 130.;
            yi[0] = 235.; ye[0] = 125.;

            if ( get_clip( xi, xe, yi, ye ) )   // get selection rectangle
            {
                pls.end();
                System.exit( 0 );
            }

            //
            // I'm unable to continue, clearing the plot and advancing to the next
            // one, without hiting the enter key, or pressing the button... help!
            //
            // Forcing the xwin driver to leave locate mode and destroying the
            // xhairs (in GetCursorCmd()) solves some problems, but I still have
            // to press the enter key or press Button-2 to go to next plot, even
            // if a pladv() is not present!  Using plbop() solves the problem, but
            // it shouldn't be needed!
            //

            // pls.bop();

            //
            // spause(false), adv(0), spause(true), also works,
            // but the above question remains.
            // With this approach, the previous pause state is lost,
            // as there is no API call to get its current state.
            //

            pls.spause( false );
            pls.adv( 0 );

            // display selection only
            pls.image( img_f, 1., width, 1., height, 0., 0., xi[0], xe[0], ye[0], yi[0] );

            pls.spause( true );

            // zoom in selection
            pls.env( xi[0], xe[0], ye[0], yi[0], 1, -1 );
            pls.image( img_f, 1., width, 1., height, 0., 0., xi[0], xe[0], ye[0], yi[0] );
        }

        // Base the dynamic range on the image contents.
        f2mnmx( img_f, width, height, maxmin );
        img_max = maxmin[0];
        img_min = maxmin[1];

        // For java we use 2-d arrays to replace the pltr function
        // even for the NULL case.
        xg = new double[width + 1][height + 1];
        yg = new double[width + 1][height + 1];

        // In order to mimic the NULL case, the following must be true.
        // xg[i] = i*deltax; yg[j] = j*deltay, where
        deltax = (double) width / (double) ( width - 1 );
        deltay = (double) height / (double) ( height - 1 );
        for ( i = 0; i <= width; i++ )
        {
            for ( j = 0; j <= height; j++ )
            {
                xg[i][j] = i * deltax;
                yg[i][j] = j * deltay;
            }
        }
        // Draw a saturated version of the original image.  Only use
        // the middle 50% of the image's full dynamic range.
        pls.col0( 2 );
        pls.env( 0, width, 0, height, 1, -1 );
        pls.lab( "", "", "Reduced dynamic range image example" );
        pls.imagefr( img_f, 0., width, 0., height, 0., 0.,
            img_min + img_max * 0.25,
            img_max - img_max * 0.25,
            xg, yg );

        // Draw a distorted version of the original image, showing its full dynamic range.
        pls.env( 0, width, 0, height, 1, -1 );
        pls.lab( "", "", "Distorted image example" );

        x0      = 0.5 * width;
        y0      = 0.5 * height;
        dy      = 0.5 * height;
        stretch = 0.5;

        // In C / C++ the following would work, with plimagefr directly calling
        // mypltr. For compatibilty with other language bindings the same effect
        // can be achieved by generating the transformed grid first and then
        // using pltr2.
        // plimagefr(img_f, width, height, 0., width, 0., height, 0., 0., img_min, img_max, mypltr, (PLPointer) &stretch);

        for ( i = 0; i <= width; i++ )
        {
            for ( j = 0; j <= height; j++ )
            {
                xg[i][j] = x0 + ( x0 - i ) * ( 1.0 - stretch *
                                               Math.cos( ( j - y0 ) / dy * Math.PI * 0.5 ) );
                yg[i][j] = j;
            }
        }

        pls.imagefr( img_f, 0., width, 0., height, 0., 0., img_min, img_max, xg, yg );
        pls.end();
    }

    // read image from file in binary ppm format
    double [][] read_img( String fname, int [] num_col )
    {
        BufferedReader  in;
        DataInputStream in2;
        double[][] img;
        String          line;
        StringTokenizer st;
        int             i, j, w, h;

        // naive grayscale binary ppm reading. If you know how to, improve it
        try {
            in  = new BufferedReader( new FileReader( fname ) );
            in2 = new DataInputStream( new DataInputStream( new BufferedInputStream( new FileInputStream( fname ) ) ) );
        }
        catch ( FileNotFoundException e ) {
            System.out.println( "File " + fname + " not found" );
            return null;
        }

        try {
            line = in.readLine();

            if ( line.compareTo( "P5\n" ) == 0 )   // I only understand this!
            {
                System.out.println( line );
                System.out.println( "unknown file format " + fname );
                return null;
            }
            in2.skip( line.getBytes().length + 1 );

            do
            {
                line = in.readLine();
                in2.skip( line.getBytes().length + 1 );
            } while ( line.charAt( 0 ) == '#' );

            st = new StringTokenizer( line );
            w  = Integer.parseInt( st.nextToken() );
            h  = Integer.parseInt( st.nextToken() );

            line = in.readLine();
            in2.skip( line.getBytes().length + 1 );
            st         = new StringTokenizer( line );
            num_col[0] = Integer.parseInt( st.nextToken() );

            img = new double[w][h];

            for ( j = 0; j < h; j++ )
            {
                for ( i = 0; i < w; i++ )
                {
                    img[i][h - j - 1] = in2.readUnsignedByte();
                }
            }
        }
        catch ( IOException e ) {
            System.out.println( "Error reading " + fname );
            return null;
        }

        return img;
    }

    // save plot
    void save_plot( String fname )
    {
        PLStream pls2 = new PLStream(); // create a new one

        pls2.sdev( "psc" );             // new device type. Use a known existing driver
        pls2.sfnam( fname );            // file name

        pls2.cpstrm( pls, false );      // copy old stream parameters to new stream
        pls2.replot();                  // do the save
    }

    //  get selection square interactively
    boolean get_clip( double[] xi, double[] xe, double[] yi, double[] ye )
    {
        PLGraphicsIn gin   = new PLGraphicsIn();
        double       xxi   = xi[0], yyi = yi[0], xxe = xe[0], yye = ye[0], t;
        boolean      start = false;
        boolean[] st = new boolean[1];

        pls.xormod( true, st ); // enter xor mode to draw a selection rectangle

        if ( st[0] )            // driver has xormod capability, continue
        {
            double sx[] = new double[5];
            double sy[] = new double[5];
            while ( true )
            {
                pls.xormod( false, st );
                pls.getCursor( gin );
                pls.xormod( true, st );

                if ( gin.getButton() == 1 )
                {
                    xxi = gin.getWX(); yyi = gin.getWY();
                    if ( start )
                        pls.line( sx, sy ); // clear previous rectangle

                    start = false;

                    sx[0] = xxi; sy[0] = yyi;
                    sx[4] = xxi; sy[4] = yyi;
                }

                if ( ( gin.getState() & 0x100 ) != 0 )
                {
                    xxe = gin.getWX(); yye = gin.getWY();
                    if ( start )
                        pls.line( sx, sy ); // clear previous rectangle

                    start = true;

                    sx[2] = xxe; sy[2] = yye;
                    sx[1] = xxe; sy[1] = yyi;
                    sx[3] = xxi; sy[3] = yye;
                    pls.line( sx, sy ); // draw new rectangle
                }

                if ( gin.getButton() == 3 || gin.getKeysym() == 0x0D || gin.getKeysym() == 'Q' )
                {
                    if ( start )
                        pls.line( sx, sy ); // clear previous rectangle
                    break;
                }
            }
            pls.xormod( false, st ); // leave xor mod
            if ( xxe < xxi )
            {
                t = xxi; xxi = xxe; xxe = t;
            }

            if ( yyi < yye )
            {
                t = yyi; yyi = yye; yye = t;
            }

            xe[0] = xxe; xi[0] = xxi;
            ye[0] = yye; yi[0] = yyi;

            return ( gin.getKeysym() == 'Q' );
        }

        return false;
    }

    // set gray colormap
    void gray_cmap( int num_col )
    {
        double r[]   = new double[2];
        double g[]   = new double[2];
        double b[]   = new double[2];
        double pos[] = new double[2];

        r[0] = g[0] = b[0] = 0.0;
        r[1] = g[1] = b[1] = 1.0;

        pos[0] = 0.0;
        pos[1] = 1.0;

        pls.scmap1n( num_col );
        pls.scmap1l( true, pos, r, g, b );
    }

    // Calculate the minimum and maximum of a 2-d array
    void f2mnmx( double [][] f, int nx, int ny, double[] fmaxmin )
    {
        int i, j;

        fmaxmin[0] = f[0][0];
        fmaxmin[1] = fmaxmin[0];

        for ( i = 0; i < nx; i++ )
        {
            for ( j = 0; j < ny; j++ )
            {
                fmaxmin[0] = Math.max( fmaxmin[0], f[i][j] );
                fmaxmin[1] = Math.min( fmaxmin[1], f[i][j] );
            }
        }
    }

    public static void main( String[] args )
    {
        new x20( args );
    }
}


//--------------------------------------------------------------------------
//                              End of x20.cc
//--------------------------------------------------------------------------
