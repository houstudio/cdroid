//	plmtex3, plptex3 demo.
//
// Copyright (C) 2007, 2008, 2009  Alan W. Irwin
// Copyright (C) 2007  Andrew Ross
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
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
// Implementation of PLplot example 28 in Java.
//--------------------------------------------------------------------------

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

import java.lang.Math;

class x28 {
    PLStream   pls = new PLStream();

    static int XPTS        = 2;
    static int YPTS        = 2;
    static int NREVOLUTION = 16;
    static int NROTATION   = 8;
    static int NSHEAR      = 8;

    x28( String[] args )
    {
        double xmin     = 0., xmax = 1.0, xmid = 0.5 * ( xmax + xmin ), xrange = xmax - xmin,
               ymin     = 0., ymax = 1.0, ymid = 0.5 * ( ymax + ymin ), yrange = ymax - ymin,
               zmin     = 0., zmax = 1.0, zmid = 0.5 * ( zmax + zmin ), zrange = zmax - zmin,
               ysmin    = ymin + 0.1 * yrange,
               ysmax    = ymax - 0.1 * yrange,
               ysrange  = ysmax - ysmin,
               dysrot   = ysrange / ( NROTATION - 1 ),
               dysshear = ysrange / ( NSHEAR - 1 ),
               zsmin    = zmin + 0.1 * zrange,
               zsmax    = zmax - 0.1 * zrange,
               zsrange  = zsmax - zsmin,
               dzsrot   = zsrange / ( NROTATION - 1 ),
               dzsshear = zsrange / ( NSHEAR - 1 ),
               ys, zs,
               x_inclination, y_inclination, z_inclination,
               x_shear, y_shear, z_shear,
               omega, sin_omega, cos_omega, domega;
        int    i, j;
        double radius, pitch, xpos, ypos, zpos;
        // p1string must be exactly one character + the null termination
        // character.
        String pstring = "The future of our civilization depends on software freedom.";

        // Allocate and define the minimal x, y, and z to insure 3D box
        double[] x   = new double[XPTS];
        double[] y   = new double[YPTS];
        double[][] z = new double[XPTS][YPTS];

        for ( i = 0; i < XPTS; i++ )
        {
            x[i] = xmin + i * ( xmax - xmin ) / ( XPTS - 1 );
        }

        for ( j = 0; j < YPTS; j++ )
            y[j] = ymin + j * ( ymax - ymin ) / ( YPTS - 1 );

        for ( i = 0; i < XPTS; i++ )
        {
            for ( j = 0; j < YPTS; j++ )
            {
                z[i][j] = 0.;
            }
        }

        // Parse and process command line arguments

        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        pls.init();

        // Page 1: Demonstrate inclination and shear capability pattern.

        pls.adv( 0 );
        pls.vpor( -0.15, 1.15, -0.05, 1.05 );
        pls.wind( -1.2, 1.2, -0.8, 1.5 );
        pls.w3d( 1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax,
            20., 45. );

        pls.col0( 2 );
        pls.box3( "b", "", xmax - xmin, 0,
            "b", "", ymax - ymin, 0,
            "bcd", "", zmax - zmin, 0 );

        // z = zmin.
        pls.schr( 0., 1.0 );
        for ( i = 0; i < NREVOLUTION; i++ )
        {
            omega         = 2. * Math.PI * ( (double) i / (double) NREVOLUTION );
            sin_omega     = Math.sin( omega );
            cos_omega     = Math.cos( omega );
            x_inclination = 0.5 * xrange * cos_omega;
            y_inclination = 0.5 * yrange * sin_omega;
            z_inclination = 0.;
            x_shear       = -0.5 * xrange * sin_omega;
            y_shear       = 0.5 * yrange * cos_omega;
            z_shear       = 0.;
            pls.ptex3(
                xmid, ymid, zmin,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.0, "  revolution" );
        }

        // x = xmax.
        pls.schr( 0., 1.0 );
        for ( i = 0; i < NREVOLUTION; i++ )
        {
            omega         = 2. * Math.PI * ( (double) i / (double) NREVOLUTION );
            sin_omega     = Math.sin( omega );
            cos_omega     = Math.cos( omega );
            x_inclination = 0.;
            y_inclination = -0.5 * yrange * cos_omega;
            z_inclination = 0.5 * zrange * sin_omega;
            x_shear       = 0.;
            y_shear       = 0.5 * yrange * sin_omega;
            z_shear       = 0.5 * zrange * cos_omega;
            pls.ptex3(
                xmax, ymid, zmid,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.0, "  revolution" );
        }

        // y = ymax.
        pls.schr( 0., 1.0 );
        for ( i = 0; i < NREVOLUTION; i++ )
        {
            omega         = 2. * Math.PI * ( (double) i / (double) NREVOLUTION );
            sin_omega     = Math.sin( omega );
            cos_omega     = Math.cos( omega );
            x_inclination = 0.5 * xrange * cos_omega;
            y_inclination = 0.;
            z_inclination = 0.5 * zrange * sin_omega;
            x_shear       = -0.5 * xrange * sin_omega;
            y_shear       = 0.;
            z_shear       = 0.5 * zrange * cos_omega;
            pls.ptex3(
                xmid, ymax, zmid,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.0, "  revolution" );
        }
        // Draw minimal 3D grid to finish defining the 3D box.
        pls.mesh( x, y, z, DRAW_LINEXY );

        // Page 2: Demonstrate rotation of string around its axis.
        pls.adv( 0 );
        pls.vpor( -0.15, 1.15, -0.05, 1.05 );
        pls.wind( -1.2, 1.2, -0.8, 1.5 );
        pls.w3d( 1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax,
            20., 45. );

        pls.col0( 2 );
        pls.box3( "b", "", xmax - xmin, 0,
            "b", "", ymax - ymin, 0,
            "bcd", "", zmax - zmin, 0 );

        // y = ymax.
        pls.schr( 0., 1.0 );
        x_inclination = 1.;
        y_inclination = 0.;
        z_inclination = 0.;
        x_shear       = 0.;
        for ( i = 0; i < NROTATION; i++ )
        {
            omega     = 2. * Math.PI * ( (double) i / (double) NROTATION );
            sin_omega = Math.sin( omega );
            cos_omega = Math.cos( omega );
            y_shear   = 0.5 * yrange * sin_omega;
            z_shear   = 0.5 * zrange * cos_omega;
            zs        = zsmax - dzsrot * i;
            pls.ptex3(
                xmid, ymax, zs,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.5, "rotation for y = y#dmax#u" );
        }

        // x = xmax.
        pls.schr( 0., 1.0 );
        x_inclination = 0.;
        y_inclination = -1.;
        z_inclination = 0.;
        y_shear       = 0.;
        for ( i = 0; i < NROTATION; i++ )
        {
            omega     = 2. * Math.PI * ( (double) i / (double) NROTATION );
            sin_omega = Math.sin( omega );
            cos_omega = Math.cos( omega );
            x_shear   = 0.5 * xrange * sin_omega;
            z_shear   = 0.5 * zrange * cos_omega;
            zs        = zsmax - dzsrot * i;
            pls.ptex3(
                xmax, ymid, zs,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.5, "rotation for x = x#dmax#u" );
        }

        // z = zmin.
        pls.schr( 0., 1.0 );
        x_inclination = 1.;
        y_inclination = 0.;
        z_inclination = 0.;
        x_shear       = 0.;
        for ( i = 0; i < NROTATION; i++ )
        {
            omega     = 2. * Math.PI * ( (double) i / (double) NROTATION );
            sin_omega = Math.sin( omega );
            cos_omega = Math.cos( omega );
            y_shear   = 0.5 * yrange * cos_omega;
            z_shear   = 0.5 * zrange * sin_omega;
            ys        = ysmax - dysrot * i;
            pls.ptex3(
                xmid, ys, zmin,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.5, "rotation for z = z#dmin#u" );
        }
        // Draw minimal 3D grid to finish defining the 3D box.
        pls.mesh( x, y, z, DRAW_LINEXY );

        // Page 3: Demonstrate shear of string along its axis.
        // Work around xcairo and pngcairo (but not pscairo) problems for
        // shear vector too close to axis of string. (N.B. no workaround
        // would be domega = 0.)
        domega = 0.05;
        pls.adv( 0 );
        pls.vpor( -0.15, 1.15, -0.05, 1.05 );
        pls.wind( -1.2, 1.2, -0.8, 1.5 );
        pls.w3d( 1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax,
            20., 45. );

        pls.col0( 2 );
        pls.box3( "b", "", xmax - xmin, 0,
            "b", "", ymax - ymin, 0,
            "bcd", "", zmax - zmin, 0 );

        // y = ymax.
        pls.schr( 0., 1.0 );
        x_inclination = 1.;
        y_inclination = 0.;
        z_inclination = 0.;
        y_shear       = 0.;
        for ( i = 0; i < NSHEAR; i++ )
        {
            omega     = domega + 2. * Math.PI * ( (double) i / (double) NSHEAR );
            sin_omega = Math.sin( omega );
            cos_omega = Math.cos( omega );
            x_shear   = 0.5 * xrange * sin_omega;
            z_shear   = 0.5 * zrange * cos_omega;
            zs        = zsmax - dzsshear * i;
            pls.ptex3(
                xmid, ymax, zs,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.5, "shear for y = y#dmax#u" );
        }

        // x = xmax.
        pls.schr( 0., 1.0 );
        x_inclination = 0.;
        y_inclination = -1.;
        z_inclination = 0.;
        x_shear       = 0.;
        for ( i = 0; i < NSHEAR; i++ )
        {
            omega     = domega + 2. * Math.PI * ( (double) i / (double) NSHEAR );
            sin_omega = Math.sin( omega );
            cos_omega = Math.cos( omega );
            y_shear   = -0.5 * yrange * sin_omega;
            z_shear   = 0.5 * zrange * cos_omega;
            zs        = zsmax - dzsshear * i;
            pls.ptex3(
                xmax, ymid, zs,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.5, "shear for x = x#dmax#u" );
        }

        // z = zmin.
        pls.schr( 0., 1.0 );
        x_inclination = 1.;
        y_inclination = 0.;
        z_inclination = 0.;
        z_shear       = 0.;
        for ( i = 0; i < NSHEAR; i++ )
        {
            omega     = domega + 2. * Math.PI * ( (double) i / (double) NSHEAR );
            sin_omega = Math.sin( omega );
            cos_omega = Math.cos( omega );
            y_shear   = 0.5 * yrange * cos_omega;
            x_shear   = 0.5 * xrange * sin_omega;
            ys        = ysmax - dysshear * i;
            pls.ptex3(
                xmid, ys, zmin,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.5, "shear for z = z#dmin#u" );
        }
        // Draw minimal 3D grid to finish defining the 3D box.
        pls.mesh( x, y, z, DRAW_LINEXY );

        // Page 4: Demonstrate drawing a string on a 3D path.
        pls.adv( 0 );
        pls.vpor( -0.15, 1.15, -0.05, 1.05 );
        pls.wind( -1.2, 1.2, -0.8, 1.5 );
        pls.w3d( 1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax,
            40., -30. );

        pls.col0( 2 );
        pls.box3( "b", "", xmax - xmin, 0,
            "b", "", ymax - ymin, 0,
            "bcd", "", zmax - zmin, 0 );

        pls.schr( 0., 1.2 );
        // domega controls the spacing between the various characters of the
        // string and also the maximum value of omega for the given number
        // of characters in *pstring.
        domega = 2. * Math.PI / pstring.length();
        omega  = 0.;
        // 3D function is a helix of the given radius and pitch
        radius = 0.5;
        pitch  = 1. / ( 2. * Math.PI );
        for ( i = 0; i < pstring.length(); i++ )
        {
            sin_omega = Math.sin( omega );
            cos_omega = Math.cos( omega );
            xpos      = xmid + radius * sin_omega;
            ypos      = ymid - radius * cos_omega;
            zpos      = zmin + pitch * omega;
            // In general, the inclination is proportional to the derivative of
            // the position wrt theta.
            x_inclination = radius * cos_omega;;
            y_inclination = radius * sin_omega;
            z_inclination = pitch;
            // The shear vector should be perpendicular to the 3D line with Z
            // component maximized, but for low pitch a good approximation is
            // a constant vector that is parallel to the Z axis.
            x_shear = 0.;
            y_shear = 0.;
            z_shear = 1.;
            pls.ptex3(
                xpos, ypos, zpos,
                x_inclination, y_inclination, z_inclination,
                x_shear, y_shear, z_shear,
                0.5, pstring.substring( i, i + 1 ) );
            omega += domega;
        }
        // Draw minimal 3D grid to finish defining the 3D box.
        pls.mesh( x, y, z, DRAW_LINEXY );

        // Page 5: Demonstrate plmtex3 axis labelling capability
        pls.adv( 0 );
        pls.vpor( -0.15, 1.15, -0.05, 1.05 );
        pls.wind( -1.2, 1.2, -0.8, 1.5 );
        pls.w3d( 1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax,
            20., 45. );

        pls.col0( 2 );
        pls.box3( "b", "", xmax - xmin, 0,
            "b", "", ymax - ymin, 0,
            "bcd", "", zmax - zmin, 0 );

        pls.schr( 0., 1.0 );
        pls.mtex3( "xp", 3.0, 0.5, 0.5, "Arbitrarily displaced" );
        pls.mtex3( "xp", 4.5, 0.5, 0.5, "primary X-axis label" );
        pls.mtex3( "xs", -2.5, 0.5, 0.5, "Arbitrarily displaced" );
        pls.mtex3( "xs", -1.0, 0.5, 0.5, "secondary X-axis label" );
        pls.mtex3( "yp", 3.0, 0.5, 0.5, "Arbitrarily displaced" );
        pls.mtex3( "yp", 4.5, 0.5, 0.5, "primary Y-axis label" );
        pls.mtex3( "ys", -2.5, 0.5, 0.5, "Arbitrarily displaced" );
        pls.mtex3( "ys", -1.0, 0.5, 0.5, "secondary Y-axis label" );
        pls.mtex3( "zp", 4.5, 0.5, 0.5, "Arbitrarily displaced" );
        pls.mtex3( "zp", 3.0, 0.5, 0.5, "primary Z-axis label" );
        pls.mtex3( "zs", -2.5, 0.5, 0.5, "Arbitrarily displaced" );
        pls.mtex3( "zs", -1.0, 0.5, 0.5, "secondary Z-axis label" );
        // Draw minimal 3D grid to finish defining the 3D box.
        pls.mesh( x, y, z, DRAW_LINEXY );

        pls.end();
    }


    public static void main( String[] args )
    {
        new x28( args );
    }
};
