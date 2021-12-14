//--------------------------------------------------------------------------
//  Unicode Pace Flag
//--------------------------------------------------------------------------
//
//  Copyright (C) 2005 Rafael Laboissiere
//                2006 Andrew Ross
//
//
//  This file is part of PLplot.
//
//  PLplot is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  PLplot is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public License
//  along with PLplot; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// Packages needed (by Debian name):
//
// ttf-arphic-bkai00mp
// ttf-freefont
// ttf-indic-fonts
// ttf-unfonts
// ttf-bangla-fonts
//
// For the latest Ubuntu systems lohit_hi.ttf has been moved to the
//  ttf-indic-fonts-core package instead of ttf-devanagari-fonts so you
//  will have to use this package instead and update the font path.
//

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

class x24 {
    PLStream pls = new PLStream();

    int      red[]   = { 240, 204, 204, 204, 0, 39, 125 };
    int      green[] = { 240, 0, 125, 204, 204, 80, 0 };
    int      blue[]  = { 240, 0, 0, 0, 0, 204, 125 };

    double   px[] = { 0.0, 0.0, 1.0, 1.0 };
    double   py[] = { 0.0, 0.25, 0.25, 0.0 };

    double   sx[] = {
        0.16374,
        0.15844,
        0.15255,
        0.17332,
        0.50436,
        0.51721,
        0.49520,
        0.48713,
        0.83976,
        0.81688,
        0.82231,
        0.82647
    };

    double   sy[] = {
        0.125,
        0.375,
        0.625,
        0.875,
        0.125,
        0.375,
        0.625,
        0.875,
        0.125,
        0.375,
        0.625,
        0.875
    };


    // Taken from http://www.columbia.edu/~fdc/pace/

    String peace[] = {
        // Mandarin
        "#<0x00>和平",
        // Hindi
        "#<0x20>शांति",
        // English
        "#<0x10>Peace",
        // Hebrew
        "#<0x10>שלום",
        // Russian
        "#<0x10>Мир",
        // German
        "#<0x10>Friede",
        // Korean
        "#<0x30>평화",
        // French
        "#<0x10>Paix",
        // Spanish
        "#<0x10>Paz",
        // Arabic
        "#<0x10>ﺳﻼم",
        // Turkish
        "#<0x10>Barış",
        // Kurdish
        "#<0x10>Hasîtî",
    };

    public x24( String[] args )
    {
        int i, j;

        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        pls.init();

        pls.adv( 0 );
        pls.vpor( 0.0, 1.0, 0.0, 1.0 );
        pls.wind( 0.0, 1.0, 0.0, 1.0 );
        pls.col0( 0 );
        pls.box( "", 1.0, 0, "", 1.0, 0 );

        pls.scmap0n( 7 );
        pls.scmap0( red, green, blue );

        pls.schr( 0, 4.0 );
        pls.font( 1 );

        for ( i = 0; i < 4; i++ )
        {
            pls.col0( i + 1 );
            pls.fill( px, py );

            for ( j = 0; j < 4; j++ )
                py [j] += 1.0 / 4.0;
        }

        pls.col0( 0 );
        for ( i = 0; i < 12; i++ )
            pls.ptex( sx [i], sy [i], 1.0, 0.0, 0.5, peace [i] );

        pls.end();
    }

    public static void main( String[] args )
    {
        new x24( args );
    }
}


//--------------------------------------------------------------------------
//                              End of x24.java
//--------------------------------------------------------------------------
