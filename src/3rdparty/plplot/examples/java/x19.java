//--------------------------------------------------------------------------
// Copyright (C) 2001  Geoffrey Furnish
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

//--------------------------------------------------------------------------
// Implementation of PLplot example 19 in Java.
//--------------------------------------------------------------------------

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

import java.lang.Math;

class MapTransform implements PLCallbackCT {
    public void coordTransform( double x, double y, double[] xt, double[] yt, Object data )
    {
        double radius;

        radius = 90.0 - y;
        xt[0]  = radius * Math.cos( x * Math.PI / 180.0 );
        yt[0]  = radius * Math.sin( x * Math.PI / 180.0 );
    }
}

class Mapform19 implements PLCallbackMapform {
    public void mapform( double[] x, double[] y )
    {
        int    i;
        double xp, yp, radius;
        for ( i = 0; i < x.length; i++ )
        {
            radius = 90.0 - y[i];
            xp     = radius * Math.cos( x[i] * Math.PI / 180.0 );
            yp     = radius * Math.sin( x[i] * Math.PI / 180.0 );
            x[i]   = xp;
            y[i]   = yp;
        }
    }
}

class LabelFunc19 implements PLCallbackLabel {
    // A custom axis labeling function for longitudes and latitudes.
    public String label( int axis, double value )
    {
        String label           = "";
        String direction_label = "";
        double label_val       = 0.0;

        if ( axis == PL_Y_AXIS )
        {
            label_val = value;
            if ( label_val > 0.0 )
            {
                direction_label = " N";
            }
            else if ( label_val < 0.0 )
            {
                direction_label = " S";
            }
            else
            {
                direction_label = "Eq";
            }
        }
        else if ( axis == PL_X_AXIS )
        {
            label_val = normalize_longitude( value );
            if ( label_val > 0.0 )
            {
                direction_label = " E";
            }
            else if ( label_val < 0.0 )
            {
                direction_label = " W";
            }
            else
            {
                direction_label = "";
            }
        }
        if ( axis == PL_Y_AXIS && value == 0.0 )
        {
            // A special case for the equator
            label = direction_label;
        }
        else
        {
            label = "" + ( (int) Math.abs( label_val ) ) + direction_label;
        }
        return label;
    }

    // "Normalize" longitude values so that they always fall between -180.0
    // and 180.0
    double normalize_longitude( double lon )
    {
        double times;

        if ( lon >= -180.0 && lon <= 180.0 )
        {
            return ( lon );
        }
        else
        {
            times = Math.floor( ( Math.abs( lon ) + 180.0 ) / 360.0 );
            if ( lon < 0.0 )
            {
                return ( lon + 360.0 * times );
            }
            else
            {
                return ( lon - 360.0 * times );
            }
        }
    }
}

class x19 {
    PLStream pls = new PLStream();

    public static void main( String[] args )
    {
        new x19( args );
    }

    public x19 ( String[] args )
    {
        double            minx, maxx, miny, maxy;
        PLCallbackMapform nullCallback        = null;
        PLCallbackLabel   nullLabelCallback   = null;
        PLCallbackCT      nullCTCallback      = null;
        LabelFunc19       geolocation_labeler = new LabelFunc19();
        MapTransform      map_transform       = new MapTransform();

        double[] x = new double[1];
        double[] y = new double[1];
        int i;

        // Parse and process command line arguments.
        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        // Longitude (x) and latitude (y)

        miny = -70;
        maxy = 80;

        // Initialize PLplot.
        pls.init();
        // Cartesian plots
        // Most of world

        minx = -170;
        maxx = minx + 360;

        // Setup a custom latitude and longitude-based scaling function.
        pls.slabelfunc( geolocation_labeler );

        pls.col0( 1 );
        pls.env( minx, maxx, miny, maxy, 1, 70 );
        pls.map( nullCallback, "usaglobe", minx, maxx, miny, maxy );

        // The Americas

        minx = 190;
        maxx = 340;

        pls.col0( 1 );
        pls.env( minx, maxx, miny, maxy, 1, 70 );
        pls.map( nullCallback, "usaglobe", minx, maxx, miny, maxy );

        // Clear the labelling function.
        pls.slabelfunc( nullLabelCallback );

        // Polar, Northern hemisphere

        // Create callback object containing mapform function
        Mapform19 mapform19 = new Mapform19();

        minx = 0;
        maxx = 360;

        pls.env( -75., 75., -75., 75., 1, -1 );
        pls.map( mapform19, "globe", minx, maxx, miny, maxy );

        pls.lsty( 2 );
        pls.meridians( mapform19, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 );


        // Polar, Northern hemisphere, this time with a PLplot-wide transform

        minx = 0;
        maxx = 360;

        pls.stransform( map_transform, null );

        pls.lsty( 1 );
        pls.env( -75., 75., -75., 75., 1, -1 );
        // No need to set the map transform here as the global transform
        // will be used.
        pls.map( nullCallback, "globe", minx, maxx, miny, maxy );

        pls.lsty( 2 );
        pls.meridians( nullCallback, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 );

        // Show Baltimore, MD on the map
        pls.col0( 2 );
        pls.ssym( 0.0, 2.0 );
        x[0] = -76.6125;
        y[0] = 39.2902778;
        pls.poin( x, y, 18 );
        pls.ssym( 0.0, 1.0 );
        pls.ptex( -76.6125, 43.0, 0.0, 0.0, 0.0, "Baltimore, MD" );

        // For Java, this is how the global transform is cleared
        pls.stransform( nullCTCallback, null );

        // An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
        // These were chosen because they provide shapefiles for small grid boxes which
        // are easilly manageable for this demo.

        pls.lsty( 1 );

        minx = 240570;
        maxx = 621109;
        miny = 87822;
        maxy = 722770;
        pls.scol0( 0, 255, 255, 255 );
        pls.scol0( 1, 0, 0, 0 );
        pls.scol0( 2, 150, 150, 150 );
        pls.scol0( 3, 0, 50, 200 );
        pls.scol0( 4, 50, 50, 50 );
        pls.scol0( 5, 150, 0, 0 );
        pls.scol0( 6, 100, 100, 255 );

        minx = 265000;
        maxx = 270000;
        miny = 145000;
        maxy = 150000;
        pls.scol0( 0, 255, 255, 255 ); //white
        pls.scol0( 1, 0, 0, 0 );       //black
        pls.scol0( 2, 255, 200, 0 );   //yellow for sand
        pls.scol0( 3, 60, 230, 60 );   //green for woodland
        pls.scol0( 4, 210, 120, 60 );  //brown for contours
        pls.scol0( 5, 150, 0, 0 );     //red for major roads
        pls.scol0( 6, 180, 180, 255 ); //pale blue for water
        pls.scol0( 7, 100, 100, 100 ); //pale grey for shingle or boulders
        pls.scol0( 8, 100, 100, 100 ); //dark grey for custom polygons - generally crags

        pls.col0( 1 );
        pls.env( minx, maxx, miny, maxy, 1, -1 );
        pls.lab( "", "", "Martinhoe CP, Exmoor National Park, UK (shapelib only)" );

        //Beach
        pls.col0( 2 );
        int beachareas[] = { 23, 24 };
        pls.mapfill( nullCallback, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, beachareas );

        //woodland
        pls.col0( 3 );
        int nwoodlandareas = 94;
        int[] woodlandareas = new int[nwoodlandareas];
        for ( i = 0; i < nwoodlandareas; ++i )
            woodlandareas[i] = i + 218;
        pls.mapfill( nullCallback, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, woodlandareas );

        //shingle or boulders
        pls.col0( 7 );
        int shingleareas[] = { 0, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 217, 2424, 2425, 2426, 2427, 2428, 2491, 2577 };
        pls.mapfill( nullCallback, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, shingleareas );

        //crags
        pls.col0( 8 );
        int ncragareas = 2024;
        int [] cragareas = new int[ncragareas];
        for ( i = 0; i < ncragareas; ++i )
            cragareas[i] = i + 325;
        pls.mapfill( nullCallback, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, cragareas );

        //draw contours, we need to separate contours from high/low coastline
        //draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy );
        pls.col0( 4 );
        pls.mapline( nullCallback, "ss/ss64ne_Height_Contours", minx, maxx, miny, maxy, null );

        //draw the sea and surface water
        pls.width( 0.0 );
        pls.col0( 6 );
        pls.mapfill( nullCallback, "ss/ss64ne_Water_Area", minx, maxx, miny, maxy, null );
        pls.width( 2.0 );
        pls.mapline( nullCallback, "ss/ss64ne_Water_Line", minx, maxx, miny, maxy, null );

        //draw the roads, first with black and then thinner with colour to give an
        //an outlined appearance
        pls.width( 5.0 );
        pls.col0( 1 );
        pls.mapline( nullCallback, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, null );
        pls.width( 3.0 );
        pls.col0( 0 );
        pls.mapline( nullCallback, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, null );
        pls.col0( 5 );
        int majorroads[] = { 33, 48, 71, 83, 89, 90, 101, 102, 111 };
        pls.mapline( nullCallback, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, majorroads );

        //draw buildings
        pls.width( 1.0 );
        pls.col0( 1 );
        pls.mapfill( nullCallback, "ss/ss64ne_Building_Area", minx, maxx, miny, maxy, null );

        //labels
        pls.sfci( 0x80000100 );
        pls.schr( 0, 0.8 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "MARTINHOE CP", minx, maxx, miny, maxy, 202 );
        pls.schr( 0, 0.7 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale\nDown", minx, maxx, miny, maxy, 13 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "South\nDown", minx, maxx, miny, maxy, 34 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Martinhoe\nCommon", minx, maxx, miny, maxy, 42 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woody Bay", minx, maxx, miny, maxy, 211 );
        pls.schr( 0, 0.6 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Mill Wood", minx, maxx, miny, maxy, 16 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale Wood", minx, maxx, miny, maxy, 17 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 1.0, "Bodley", minx, maxx, miny, maxy, 31 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.0, "Martinhoe", minx, maxx, miny, maxy, 37 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woolhanger\nCommon", minx, maxx, miny, maxy, 60 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "West Ilkerton\nCommon", minx, maxx, miny, maxy, 61 );
        pls.maptex( nullCallback, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Caffyns\nHeanton\nDown", minx, maxx, miny, maxy, 62 );

        pls.end();
    }
}

//--------------------------------------------------------------------------
//                              End of x19.java
//--------------------------------------------------------------------------
