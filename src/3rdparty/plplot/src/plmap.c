//      Continental Outline and Political Boundary Backgrounds
//
//      Some plots need a geographical background such as the global
//      surface temperatures or the population density.  The routine
//      plmap() will draw one of the following backgrounds: continental
//      outlines, political boundaries, the United States, and the United
//      States with the continental outlines.  The routine plmeridians()
//      will add the latitudes and longitudes to the background.  After
//      the background has been drawn, one can use a contour routine or a
//      symbol plotter to finish off the plot.
//
//      Copyright (C) 1991, 1993, 1994  Wesley Ebisuzaki
//      Copyright (C) 1994, 2000, 2001  Maurice LeBrun
//      Copyright (C) 1999  Geoffrey Furnish
//      Copyright (C) 2000-2016 Alan W. Irwin
//      Copyright (C) 2001  Andrew Roach
//      Copyright (C) 2001, 2004  Rafael Laboissiere
//      Copyright (C) 2002  Vincent Darley
//      Copyright (C) 2003  Joao Cardoso
//
//      This file is part of PLplot.
//
//      PLplot is free software; you can redistribute it and/or modify
//      it under the terms of the GNU Library General Public License
//      as published by the Free Software Foundation; version 2 of the
//      License.
//
//      PLplot is distributed in the hope that it will be useful, but
//      WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU Library General Public License for more details.
//
//      You should have received a copy of the GNU Library General Public
//      License along with this library; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
//      USA
//
//

#define DEBUG
#define NEED_PLDEBUG

#include "plplotP.h"

#ifdef HAVE_SHAPELIB
#include <shapefil.h>

SHPHandle
OpenShapeFile( PLCHAR_VECTOR fn );

#ifdef HAVE_SAHOOKS
static void
CustomErrors( PLCHAR_VECTOR message );
#endif //HAVE_SAHOOKS

#define OpenMap     OpenShapeFile
#define CloseMap    SHPClose

//redistributes the lon value so that it is within +/-180 deg of midlon
//for wrapping purposes.
void
rebaselon( PLFLT *lon, PLFLT midlon )
{
    if ( *lon > midlon + 180.0 )
        *lon -= floor( ( *lon - midlon - 180.0 ) / 360.0 + 1.0 ) * 360.0;
    else if ( *lon < midlon - 180.0 )
        *lon += floor( ( midlon - 180.0 - *lon ) / 360.0 + 1.0 ) * 360.0;
}

//--------------------------------------------------------------------------
//Actually draw the map lines points and text.
//--------------------------------------------------------------------------
void
drawmapdata( PLMAPFORM_callback mapform, int shapetype, PLINT n, double *x, double *y, PLFLT dx, PLFLT dy, PLFLT just, PLCHAR_VECTOR text )
{
    PLINT i;
    PLFLT *renderX;
    PLFLT *renderY;

    //we need to do something a bit different with filled polygons. The issue is the poles
    //on lat/lon plots. If we draw Antarctica then we expect it to be filled, but this
    //requires us to add in the extra points extending to the pole.
    //We identify a fill that loops round the globe because its start point will not be its
    //end point due to the processing done in drawmap. It should be 360 degrees different to
    //within the rounding error.
    //For a basic plot without any transformation we could just add three points to go down
    //to the pole accross to the starting longitude then up to join the start point, but if
    //we consider a polar plot looking down on the North Pole, then Antarctica gets turned
    //inside out so we actually want to add many points at the pole with different longitudes.

    if ( ( shapetype == SHPT_POLYGON || shapetype == SHPT_POLYGONZ || shapetype == SHPT_POLYGONM ) && x[0] != x[n - 1] )
    {
        //we have a polygon that rings the pole - render it differently to everything else

        if ( x[0] != x[n - 1] )
        {
            //this is a looped round the pole polygon
            PLINT  i;
            double poleLat;
            double lonInterval;
            PLINT  nExtraPoints = MAX( 501, n + 1 );
            double *newX;
            double *newY;

            //The shapefile standard says that as we traverse the vertices, the inside should be
            //on the right
            if ( x[n - 1] > x[0] )
                poleLat = -90.0;
            else
                poleLat = 90.0;

            newX = malloc( ( n + nExtraPoints ) * sizeof ( double ) );
            newY = malloc( ( n + nExtraPoints ) * sizeof ( double ) );
            if ( !newX || !newY )
            {
                free( newX );
                free( newY );
                plabort( "Could not allocate memory for adding pole points to a map polygon." );
                return;
            }
            memcpy( newX, x, n * sizeof ( double ) );
            memcpy( newY, y, n * sizeof ( double ) );

            lonInterval = ( x[0] - x[n - 1] ) / (double) ( nExtraPoints - 2 );

            for ( i = 0; i < nExtraPoints - 1; ++i )
            {
                newX[n + i] = x[n - 1] + i * lonInterval;
                newY[n + i] = poleLat;
            }
            newX[n + nExtraPoints - 1] = x[0];
            newY[n + nExtraPoints - 1] = y[0];
#ifdef PL_DOUBLE
            renderX = newX;
            renderY = newY;
#else
            fltX = malloc( ( n + nExtraPoints ) * sizeof ( PLFLT ) );
            fltX = malloc( ( n + nExtraPoints ) * sizeof ( PLFLT ) );
            if ( !fltX || !fltY )
            {
                free( fltX );
                free( fltY );
                free( newX );
                free( newY );
                plabort( "Could not allocate memory converting map date to floats." );
                return;
            }
            for ( i = 0; i < n + nExtraPoints; ++i )
            {
                fltX[i] = (PLFLT) newX[i];
                fltY[i] = (PLFLT) newY[i];
            }
            renderX = fltX;
            renderY = fltY;
#endif
            if ( mapform != NULL )
                (*mapform)( n + nExtraPoints, renderX, renderY );
            plfill( n + nExtraPoints, renderX, renderY );
            free( newX );
            free( newY );
#ifndef PL_DOUBLE
            free( fltX );
            free( fltY );
#endif

            //rendering is complete, return;
            return;
        }
        else
        {
            //this is just a regular polygon - render it as we would expect
            if ( mapform != NULL )
                (*mapform)( n, x, y );
            plfill( n, x, y );
        }

        //return here as we are done
        return;
    }

    //if we get to here we don't have a polygon wrapping all the way round the globe
    //Just do normal rendering

    //convert to floats if needed
#ifdef PL_DOUBLE
    renderX = x;
    renderY = y;
#else
    fltX = malloc( n * sizeof ( PLFLT ) );
    fltX = malloc( n * sizeof ( PLFLT ) );
    if ( !fltX || !fltY )
    {
        free( fltX );
        free( fltY );
        plabort( "Could not allocate memory converting map date to floats." );
        return;
    }
    for ( i = 0; i < n; ++i )
    {
        fltX[i] = (PLFLT) x[i];
        fltY[i] = (PLFLT) y[i];
    }
    renderX = fltX;
    renderY = fltY;
#endif

    //do the transform if needed
    if ( mapform != NULL )
        ( *mapform )( n, renderX, renderY );

    //deo the rendering
    if ( shapetype == SHPT_ARC )
        plline( n, renderX, renderY );
    else if ( shapetype == SHPT_POINT )
        for ( i = 0; i < n; ++i )
            plptex( renderX[i], renderY[i], dx, dy, just, text );
    else if ( shapetype == SHPT_ARCZ || shapetype == SHPT_ARCM )
        plline( n, renderX, renderY );
    else if ( shapetype == SHPT_POINTM || shapetype == SHPT_POINTZ )
        for ( i = 0; i < n; ++i )
            plptex( renderX[i], renderY[i], dx, dy, just, text );
    else if ( shapetype == SHPT_POLYGON || shapetype == SHPT_POLYGONZ || shapetype == SHPT_POLYGONM )
        plfill( n, renderX, renderY );
    else if ( shapetype == SHPT_NULL )
        ;         //do nothing for a NULL shape
    else
        plabort( "Unknown render type passed in to drawmapdata(). PLplot can render shapefile arcs, points and polygons (including z and m variants)." );

#ifndef PL_DOUBLE
    free( fltX );
    free( fltY );
#endif
}


//--------------------------------------------------------------------------
//This is a function called by the front end map functions to do the map drawing. Its
//parameters are:
//mapform: The transform used to convert the data in raw coordinates to x, y positions
//on the plot
//name: either one of the plplot provided lat/lon maps or the path/file name of a
//shapefile
//dx/dy: the gradient of text/symbols drawn if text is non-null
//shapetype: one of ARC, SHPT_ARCZ, SHPT_ARCM, SHPT_POLYGON, SHPT_POLYGONZ,
//SHPT_POLYGONM, SHPT_POINT, SHPT_POINTM, SHPT_POINTZ. See drawmapdata() for the
//how each type is rendered. But Basically the ARC options are lines, the POLYGON
//options are filled polygons, the POINT options are points/text. Options beginning
//SHPT will only be defined if HAVE_SHAPELIB is true
//text: The text (which can be actual text or a unicode symbol) to be drawn at
//each point
//minx/maxx: The min/max longitude when using a plplot provided map or x value if
//using a shapefile
//miny/maxy: The min/max latitude when using a plplot provided map or y value if
//using a shapefile
//plotentries: used only for shapefiles, as one shapefile contains multiple vectors
//each representing a different item (e.g. multiple boundaries, multiple height
//contours etc. plotentries is an array containing the indices of the
//entries within the shapefile that you wish to plot. if plotentries is null all
//entries are plotted
//nplotentries: the number of elements in plotentries. Ignored if plplot was not built
//with shapefile support or if plotentries is null
//--------------------------------------------------------------------------
void
drawmap( PLMAPFORM_callback mapform, PLCHAR_VECTOR name,
         PLFLT dx, PLFLT dy, int shapetype, PLFLT just, PLCHAR_VECTOR text,
         PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy, PLINT_VECTOR plotentries, PLINT nplotentries )
{
    int    i;
    char   *filename = NULL;                               //filename of the map data
    char   warning[1024];                                  //string used for constructing a sensible warning message
    int    nVertices = 200;                                //number of vertices in a particular part
    double minsectlon, maxsectlon, minsectlat, maxsectlat; //the min/max for each chunk of the data, note double not PLFLT as they are read from the shapefile
    //PLFLT  *bufx   = NULL, *bufy = NULL; //the data that will be plot after dealing with wrapping round the globe
    int    bufsize = 0;                                    //number of elements in bufx and bufy
    size_t filenamelen;                                    //length of the filename
    char   islatlon     = 1;                               //specify if the data is lat-lon or some other projection
    int    appendresult = 0;


    SHPHandle in;
    int       nentries;                            //number of objects in the shapefile
    int       entryindex  = 0;                     //index of plotentries that we are currently rendering
    int       entrynumber = 0;                     //id of the object we are currently rendering
    int       partnumber  = 0;                     //part of the object we are currently rendering (some objects are split into parts)
    SHPObject *object     = NULL;                  //pointer to the object we are currently rendering
    double    *bufxraw;                            //pointer to the raw x data read from the file
    double    *bufyraw;                            //pointer to the raw y data read from the file
    char      *prjfilename = NULL;                 //filename of the projection file
    PDFstrm   *prjfile;                            //projection file
    char      prjtype[] = { 0, 0, 0, 0, 0, 0, 0 }; //string holding the projection type description
    double    fileMins[4];                         //min x, y, z, m for the file
    double    fileMaxs[4];                         //max x, y, z, m for the file
    int       fileShapeType;                       //the shapetype read from the file

    //
    // read map outline
    //

    //strip the .shp extension if a shapefile has been provided
    if ( strstr( name, ".shp" ) )
        filenamelen = ( strlen( name ) - 4 );
    else
        filenamelen = strlen( name );
    filename = (char *) malloc( filenamelen + 1 );
    if ( !filename )
    {
        plabort( "Could not allocate memory for map filename root" );
        return;
    }
    strncpy( filename, name, filenamelen );
    filename[ filenamelen ] = '\0';

    //Open the shp and shx file using shapelib
    if ( ( in = OpenShapeFile( filename ) ) == NULL )
    {
        strcpy( warning, "Could not find " );
        strcat( warning, filename );
        strcat( warning, " file." );
        plabort( warning );
        free( filename );
        return;
    }
    SHPGetInfo( in, &nentries, &fileShapeType, fileMins, fileMaxs );
    if ( shapetype == SHPT_NULL )
    {
        shapetype = fileShapeType;
    }
    //also check for a prj file which will tell us if the data is lat/lon or projected
    //if it is projected then set ncopies to 1 - i.e. don't wrap round longitudes
    prjfilename = (char *) malloc( filenamelen + 5 );
    if ( !prjfilename )
    {
        free( filename );
        plabort( "Could not allocate memory for generating map projection filename" );
        return;
    }
    strncpy( prjfilename, name, filenamelen );
    prjfilename[ filenamelen ] = '\0';
    strcat( prjfilename, ".prj" );
    prjfile = plLibOpenPdfstrm( prjfilename );
    if ( prjfile && prjfile->file )
    {
        fread( prjtype, 1, 6, prjfile->file );
        if ( strcmp( prjtype, "PROJCS" ) == 0 )
            islatlon = 0;
        pdf_close( prjfile );
    }
    free( prjfilename );
    prjfilename = NULL;

    for (;; )
    {
        //each object in the shapefile is split into parts.
        //If we are need to plot the first part of an object then read in a new object
        //and check how many parts it has. Otherwise use the object->panPartStart vector
        //to check the offset of this part and the next part and allocate memory. Copy
        //the data to this memory converting it to PLFLT and draw it.
        //finally increment the part number or if we have finished with the object reset the
        //part numberand increment the object.

        //break condition if we've reached the end of the file
        if ( ( !plotentries && ( entrynumber == nentries ) ) || ( plotentries && ( entryindex == nplotentries ) ) )
            break;

        //if partnumber == 0 then we need to load the next object
        if ( partnumber == 0 )
        {
            if ( plotentries )
                object = SHPReadObject( in, plotentries[entryindex] );
            else
                object = SHPReadObject( in, entrynumber );
        }
        //if the object could not be read, increment the object index to read and
        //return to the top of the loop to try the next object.
        if ( object == NULL )
        {
            entrynumber++;
            entryindex++;
            partnumber = 0;
            continue;
        }

        //work out how many points are in the current part
        if ( object->nParts == 0 )
            nVertices = object->nVertices;                                                       //if object->nParts==0, we can still have 1 vertex. A bit odd but it's the way it goes
        else if ( partnumber == ( object->nParts - 1 ) )
            nVertices = object->nVertices - object->panPartStart[partnumber];                    //panPartStart holds the offset for each part
        else
            nVertices = object->panPartStart[partnumber + 1] - object->panPartStart[partnumber]; //panPartStart holds the offset for each part

        //point the plot buffer to the correct starting vertex
        //and copy it to the PLFLT arrays. If we had object->nParts == 0
        //then panPartStart will be NULL
        if ( object->nParts > 0 )
        {
            bufxraw = object->padfX + object->panPartStart[partnumber];
            bufyraw = object->padfY + object->panPartStart[partnumber];
        }
        else
        {
            bufxraw = object->padfX;
            bufyraw = object->padfY;
        }

        //set the min/max x/y of the object
        minsectlon = object->dfXMin;
        maxsectlon = object->dfXMax;
        minsectlat = object->dfYMin;
        maxsectlat = object->dfYMax;

        if ( nVertices > 0 )
        {
            if ( islatlon )
            {
                PLINT  nExtraPositiveRedraws = 0;
                double unrebasedFirstValue   = bufxraw[0];
                double rebaseAmount;
                //two obvious issues exist here with plotting longitudes:
                //
                //1) wraparound causing lines which go the wrong way round
                //   the globe
                //2) some people plot lon from 0-360 deg, others from -180 - +180
                //   or even more bizzare options - like google earth when zoomed
                //   right out loops round and round.
                //
                //we can cure these problems by conditionally adding/subtracting
                //360 degrees to each longitude point in order to ensure that the
                //distance between adgacent points is always less than 180
                //degrees, then repeatedly shifting the shape by integer multiples
                //of 360 in each longitude direction until the shape has moved out
                //of the plot area.

                //reset our plot longitude limits to avoid crazy loops in case the user has
                //set them to be infinities or similar
                if ( minx == -PLFLT_MAX || minx <= -PLFLT_HUGE_VAL )
                    minx = fileMins[0];
                if ( maxx == -PLFLT_MAX || maxx >= PLFLT_HUGE_VAL )
                    maxx = fileMaxs[0];

                //move the first point by multiples of 360 putting it as close
                //as possible to our plot limits centre point.
                unrebasedFirstValue = bufxraw[0];
                rebaselon( &bufxraw[0], ( minx + maxx ) / 2.0 );
                rebaseAmount = bufxraw[0] - unrebasedFirstValue;
                minsectlon  += rebaseAmount;
                maxsectlon  += rebaseAmount;
                for ( i = 1; i < nVertices; i++ )
                {
                    double difference;
                    //first adjust the point by the same amount as our first point
                    bufxraw[i] += rebaseAmount;

                    //now check it doesn't do any strange wrapping
                    difference = bufxraw[i] - bufxraw[i - 1];
                    if ( difference < -180.0 )
                    {
                        bufxraw[i] += 360.0;
                        maxsectlon  = MAX( maxsectlon, bufxraw[i] );
                    }
                    else if ( difference > 180.0 )
                    {
                        bufxraw[i] -= 360.0;
                        minsectlon  = MIN( minsectlon, bufxraw[i] );
                    }
                }

                //check if the latitude and longitude range means we need to plot this section
                if ( ( maxsectlat > miny ) && ( minsectlat < maxy )
                     && ( maxsectlon > minx ) && ( minsectlon < maxx ) )
                {
                    drawmapdata( mapform, shapetype, nVertices, bufxraw, bufyraw, dx, dy, just, text );
                }
                //now check if adding multiples of 360 to the data still allow it to be plotted
                while ( minsectlon + 360.0 < maxx )
                {
                    for ( i = 0; i < nVertices; ++i )
                        bufxraw[i] += 360.0;
                    drawmapdata( mapform, shapetype, nVertices, bufxraw, bufyraw, dx, dy, just, text );
                    minsectlon += 360.0;
                    ++nExtraPositiveRedraws;
                }
                if ( maxsectlon - 360.0 > minx )
                {
                    for ( i = 0; i < nVertices; ++i )
                        bufxraw[i] -= nExtraPositiveRedraws * 360.0;
                    while ( maxsectlon - 360.0 > minx )
                    {
                        for ( i = 0; i < nVertices; ++i )
                            bufxraw[i] -= 360.0;
                        drawmapdata( mapform, shapetype, nVertices, bufxraw, bufyraw, dx, dy, just, text );
                        maxsectlon -= 360.0;
                    }
                }
            }
            else
            {
                drawmapdata( mapform, shapetype, nVertices, bufxraw, bufyraw, dx, dy, just, text );
            }
        }


        //increment the partnumber or if we've reached the end of
        //an entry increment the entrynumber and set partnumber to 0
        if ( partnumber == object->nParts - 1 || object->nParts == 0 )
        {
            entrynumber++;
            entryindex++;
            partnumber = 0;
            SHPDestroyObject( object );
            object = NULL;
        }
        else
            partnumber++;
    }
    // Close map file
    SHPClose( in );

    //free memory
    free( filename );
}
#endif //HAVE_SHAPELIB


//--------------------------------------------------------------------------
// void plmap(PLMAPFORM_callback mapform, PLCHAR_VECTOR name,
//            PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy);
//
// plot continental outline in world coordinates
//
// v1.4: machine independant version
// v1.3: replaced plcontinent by plmap, added plmeridians
// v1.2: 2 arguments:  mapform, type of plot
//
// mapform(PLINT n, PLFLT *x, PLFLT *y) is a routine to transform the
// coordinate longitudes and latitudes to a plot coordinate system.  By
// using this transform, we can change from a longitude, latitude
// coordinate to a polar stereographic project, for example.  Initially,
// x[0]..[n-1] are the longitudes and y[0]..y[n-1] are the corresponding
// latitudes.  After the call to mapform(), x[] and y[] should be replaced
// by the corresponding plot coordinates.  If no transform is desired,
// mapform can be replaced by NULL.
//
// name is a character string. The value of this parameter determines the
// name of background. The possible values are,
//
//      "globe"		continental outlines
//      "usa"		USA and state boundaries
//      "cglobe"	continental outlines and countries
//      "usaglobe"	USA, state boundaries and continental outlines
// alternatively the filename of a shapefile can be passed if PLplot has
// been compiled with shapelib. In this case either the base name of the
// file can be passed or the filename including the .shp or .shx suffix.
// Only the .shp and .shx files are used.
//
// minx, maxx are the minimum and maximum untransformed x values to be
// plotted. For the built in plots these are longitudes. For other
//shapefiles these are in the units of the shapefile. The value of minx
//must be less than the values of maxx.
//
// miny, maxy are as per minx and maxx except for the built in plots
//the units are latitude.
//--------------------------------------------------------------------------

void
plmap( PLMAPFORM_callback mapform, PLCHAR_VECTOR name,
       PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy )
{
#ifdef HAVE_SHAPELIB
    drawmap( mapform, name, 0.0, 0.0, SHPT_NULL, 0.0, NULL, minx, maxx,
        miny, maxy, NULL, 0 );
#else
    plwarn( "plmap is a no-op because shapelib is not available." );
#endif
}

//--------------------------------------------------------------------------
// void plmapline(PLMAPFORM_callback mapform,
//		PLCHAR_VECTOR name, PLFLT minx, PLFLT maxx, PLFLT miny,
//		PLFLT maxy, PLINT_VECTOR plotentries, PLINT nplotentries);

//New version of plmap which allows us to specify which items in a shapefile
//we want to use. parameters are as above but with the plotentries being an
//array containing the indices of the elements we wish to draw and
//nplotentries being the number of items in plotentries.
//If shapefile access was not built into plplot then plotentries and
//nplotentries are ignored. If plotentries is null than all entries are
//drawn and nplotentries is ignored.
//The name distiguishes it from other functions which plot points/text and
//polygons, but note that the type of element in the shapefile need not
//match the type of element drawn - i.e. arc elements from a shapefile could
//be drawn as points using the plmaptex function.
//--------------------------------------------------------------------------
void
plmapline( PLMAPFORM_callback mapform, PLCHAR_VECTOR name,
           PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
           PLINT_VECTOR plotentries, PLINT nplotentries )
{
#ifdef HAVE_SHAPELIB
    drawmap( mapform, name, 0.0, 0.0, SHPT_ARC, 0.0, "", minx, maxx,
        miny, maxy, plotentries, nplotentries );
#else
    plwarn( "plmapline is a no-op because shapelib is not available." );
#endif
}

//--------------------------------------------------------------------------
// void plmapstring(PLMAPFORM_callback mapform,
//		PLCHAR_VECTOR name, PLFLT just, PLCHAR_VECTOR string,
//		PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
//		PLINT_VECTOR plotentries, PLINT nplotentries);
//
//As per plmapline but plots symbols. The map equivalent of plstring. string
//has the same meaning as in plstring.
//--------------------------------------------------------------------------
void
plmapstring( PLMAPFORM_callback mapform,
             PLCHAR_VECTOR name, PLCHAR_VECTOR string,
             PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
             PLINT_VECTOR plotentries, PLINT nplotentries )
{
#ifdef HAVE_SHAPELIB
    drawmap( mapform, name, 1.0, 0.0, SHPT_POINT, 0.5, string, minx, maxx,
        miny, maxy, plotentries, nplotentries );
#else
    plwarn( "plmapstring is a no-op because shapelib is not available." );
#endif
}

//--------------------------------------------------------------------------
// void plmaptex(PLMAPFORM_callback mapform,
//		PLCHAR_VECTOR name, PLFLT dx, PLFLT dy PLFLT just, PLCHAR_VECTOR text,
//		PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
//		PLINT plotentry);
//
//As per plmapline but plots text. The map equivalent of plptex. dx, dy,
//just and text have the same meaning as in plptex.
//--------------------------------------------------------------------------
void
plmaptex( PLMAPFORM_callback mapform,
          PLCHAR_VECTOR name, PLFLT dx, PLFLT dy, PLFLT just, PLCHAR_VECTOR text,
          PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
          PLINT plotentry )
{
#ifdef HAVE_SHAPELIB
    drawmap( mapform, name, dx, dy, SHPT_POINT, just, text, minx, maxx,
        miny, maxy, &plotentry, 1 );
#else
    plwarn( "plmaptex is a no-op because shapelib is not available." );
#endif
}

//--------------------------------------------------------------------------
// void plmapfill(PLMAPFORM_callback mapform,
//		PLCHAR_VECTOR name, PLFLT minx, PLFLT maxx, PLFLT miny,
//		PLFLT maxy, PLINT_VECTOR plotentries, PLINT nplotentries);
//
//As per plmapline but plots a filled polygon. The map equivalent to
//plfill. Uses the pattern defined by plsty or plpat.
//--------------------------------------------------------------------------
void
plmapfill( PLMAPFORM_callback mapform,
           PLCHAR_VECTOR name, PLFLT minx, PLFLT maxx, PLFLT miny,
           PLFLT maxy, PLINT_VECTOR plotentries, PLINT nplotentries )
{
#ifdef HAVE_SHAPELIB
    drawmap( mapform, name, 0.0, 0.0, SHPT_POLYGON, 0.0, NULL, minx, maxx,
        miny, maxy, plotentries, nplotentries );
#else
    plwarn( "plmapfill is a no-op because shapelib is not available." );
#endif
}

//--------------------------------------------------------------------------
// void plmeridians(PLMAPFORM_callback mapform,
//		    PLFLT dlong, PLFLT dlat, PLFLT minx, PLFLT maxx,
//		    PLFLT miny, PLFLT maxy);
//
// Plot the latitudes and longitudes on the background.  The lines
// are plotted in the current color and line style.
//
// mapform(PLINT n, PLFLT *x, PLFLT *y) is a routine to transform the
// coordinate longitudes and latitudes to a plot coordinate system.  By
// using this transform, we can change from a longitude, latitude
// coordinate to a polar stereographic project, for example.  Initially,
// x[0]..x[n-1] are the longitudes and y[0]..y[n-1] are the corresponding
// latitudes.  After the call to mapform(), x[] and y[] should be replaced
// by the corresponding plot coordinates.  If no transform is desired,
// mapform can be replaced by NULL.
//
// dlat, dlong are the interval in degrees that the latitude and longitude
// lines are to be plotted.
//
// minx, maxx are the values of the longitude on the left and right
// side of the plot, respectively. The value of minx must be less than
// the values of maxx, and the values of maxx-minx must be less
// or equal to 360.
//
// miny, maxy are the minimum and maximum latitudes to be plotted on
// the background.  One can always use -90.0 and 90.0 as the boundary
// outside the plot window will be automatically eliminated.  However, the
// program will be faster if one can reduce the size of the background
// plotted.
//--------------------------------------------------------------------------

#define NSEG    100

void
plmeridians( PLMAPFORM_callback mapform,
             PLFLT dlong, PLFLT dlat,
             PLFLT minlong, PLFLT maxlong, PLFLT minlat, PLFLT maxlat )
{
    PLFLT yy, xx, temp, x[2], y[2], dx, dy;

    if ( minlong > maxlong )
    {
        temp    = minlong;
        minlong = maxlong;
        maxlong = temp;
    }
    if ( minlat > maxlat )
    {
        temp   = minlat;
        minlat = maxlat;
        maxlat = temp;
    }
    dx = ( maxlong - minlong ) / NSEG;
    dy = ( maxlat - minlat ) / NSEG;

    // latitudes

    for ( yy = dlat * ceil( minlat / dlat ); yy <= maxlat; yy += dlat )
    {
        if ( mapform == NULL )
        {
            plpath( NSEG, minlong, yy, maxlong, yy );
        }
        else
        {
            for ( xx = minlong; xx < maxlong; xx += dx )
            {
                y[0] = y[1] = yy;
                x[0] = xx;
                x[1] = xx + dx;
                ( *mapform )( 2, x, y );
                plline( 2, x, y );
            }
        }
    }

    // longitudes

    for ( xx = dlong * ceil( minlong / dlong ); xx <= maxlong; xx += dlong )
    {
        if ( mapform == NULL )
        {
            plpath( NSEG, xx, minlat, xx, maxlat );
        }
        else
        {
            for ( yy = minlat; yy < maxlat; yy += dy )
            {
                x[0] = x[1] = xx;
                y[0] = yy;
                y[1] = yy + dy;
                ( *mapform )( 2, x, y );
                plline( 2, x, y );
            }
        }
    }
}

//--------------------------------------------------------------------------
// SHPHandle OpenShapeFile(fn)
//
//! Returns a handle to a shapefile from the filename fn. Content based on
//! plLibOpenPdfstrm in plctrl.c
//! Locations checked:
//!	PLPLOT_LIB_ENV = $(PLPLOT_LIB)
//!	current directory
//!	PLPLOT_HOME_ENV/lib = $(PLPLOT_HOME)/lib
//!	DATA_DIR
//!	PLLIBDEV
//!
//! @param fn Name of the file.
//!
//! @Return handle to a shapefile on success or NULL if the file cannot be
//! found
//--------------------------------------------------------------------------
#ifdef HAVE_SHAPELIB
#ifdef HAVE_SAHOOKS
// Our thanks to Frank Warmerdam, the developer of shapelib for suggesting
// this approach for quieting shapelib "Unable to open" error messages.
static
void CustomErrors( PLCHAR_VECTOR message )
{
    if ( strstr( message, "Unable to open" ) == NULL )
        fprintf( stderr, "%s\n", message );
}
#endif

SHPHandle
OpenShapeFile( PLCHAR_VECTOR fn )
{
    SHPHandle file;
    char      *fs = NULL, *dn = NULL;
#ifdef HAVE_SAHOOKS
    SAHooks   sHooks;

    SASetupDefaultHooks( &sHooks );
    sHooks.Error = CustomErrors;
#else
    // Using ancient version of shapelib without SAHooks or SHPOpenLL.
    // For this case live with the misleading "Unable to open" error
    // messages.
    // int sHooks;
#define SHPOpenLL( a, b, c )    SHPOpen( a, b )
#endif //HAVE_SAHOOKS

//***   search build tree               ***

    if ( plInBuildTree() == 1 )
    {
        plGetName( SOURCE_DIR, "data", fn, &fs );

        if ( ( file = SHPOpenLL( fs, "rb", &sHooks ) ) != NULL )
            goto done;
    }

//***	search PLPLOT_LIB_ENV = $(PLPLOT_LIB)	***

#if defined ( PLPLOT_LIB_ENV )
    if ( ( dn = getenv( PLPLOT_LIB_ENV ) ) != NULL )
    {
        plGetName( dn, "", fn, &fs );

        if ( ( file = SHPOpenLL( fs, "rb", &sHooks ) ) != NULL )
            goto done;
        fprintf( stderr, PLPLOT_LIB_ENV "=\"%s\"\n", dn ); // what IS set?
    }
#endif  // PLPLOT_LIB_ENV

//***	search current directory	***

    if ( ( file = SHPOpenLL( fn, "rb", &sHooks ) ) != NULL )
    {
        pldebug( "OpenShapeFile", "Found file %s in current directory.\n", fn );
        free_mem( fs );
        return ( file );
    }

//***	search PLPLOT_HOME_ENV/lib = $(PLPLOT_HOME)/lib	***

#if defined ( PLPLOT_HOME_ENV )
    if ( ( dn = getenv( PLPLOT_HOME_ENV ) ) != NULL )
    {
        plGetName( dn, "lib", fn, &fs );

        if ( ( file = SHPOpenLL( fs, "rb", &sHooks ) ) != NULL )
            goto done;
        fprintf( stderr, PLPLOT_HOME_ENV "=\"%s\"\n", dn ); // what IS set?
    }
#endif  // PLPLOT_HOME_ENV/lib

//***   search installed location	***

#if defined ( DATA_DIR )
    plGetName( DATA_DIR, "", fn, &fs );

    if ( ( file = SHPOpenLL( fs, "rb", &sHooks ) ) != NULL )
        goto done;
#endif  // DATA_DIR

//***   search hardwired location	***

#ifdef PLLIBDEV
    plGetName( PLLIBDEV, "", fn, &fs );

    if ( ( file = SHPOpenLL( fs, "rb", &sHooks ) ) != NULL )
        goto done;
#endif  // PLLIBDEV

//***   not found, give up      ***
    pldebug( "OpenShapeFile", "File %s not found.\n", fn );
    free_mem( fs );
    return NULL;

done:
    pldebug( "OpenShapeFile", "SHPOpen successfully opened two files with basename %s\n", fs );
    free_mem( fs );
    return ( file );
}
#endif //ifdef HAVE_SHAPELIB
