//
// A simple GTK application demonstrating the use of the X Drawable
// functionality of plplot's xcairo driver.
//
// Copyright (C) 2008 Jonathan Woithe <jwoithe@physics.adelaide.edu.au>
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
//

// Set to 0 to draw direct to an X Window, 1 to draw via a pixmap
#define TO_PIXMAP    1

#include <stdio.h>
#include <plplot.h>
#include <gtk/gtk.h>

// Needed for GDK_WINDOW_XID
#include <gdk/gdkx.h>

// Main menu structure
static GtkItemFactoryEntry menu_items[] = {
    { "/_File",      NULL,         NULL,          0, "<Branch>", NULL },
    { "/File/_Quit", "<control>Q", gtk_main_quit, 0, NULL,       NULL },
};

#define APP_INITIAL_WIDTH     320
#define APP_INITIAL_HEIGHT    200

typedef struct App
{
    GtkWidget *rootwindow;
    GtkWidget *plotwindow;
    GdkPixmap *plotwindow_pixmap;
} App;

App app;

void setup_plot_drawable( App *a );
static gint ev_plotwindow_conf( GtkWidget *widget, GdkEventConfigure *ev, gpointer *data );
void init_app( App *a );

//--------------------------------------------------------------------------

void setup_plot_drawable( App *a )
{
    struct
    {
        Display  *display;
        Drawable drawable;
    }     xinfo;

    PLFLT x[3] = { 1, 3, 4 };
    PLFLT y[3] = { 3, 2, 5 };

    plsdev( "xcairo" );
    plsetopt( "drvopt", "external_drawable" );
    plinit();

  #if TO_PIXMAP == 1
    // Here we set up to draw to a pixmap
    xinfo.display  = GDK_PIXMAP_XDISPLAY( a->plotwindow_pixmap );
    xinfo.drawable = GDK_PIXMAP_XID( a->plotwindow_pixmap );
  #else
    // Alternatively, we can do direct to a visible X Window
    xinfo.display  = GDK_WINDOW_XDISPLAY( a->plotwindow->window );
    xinfo.drawable = GDK_WINDOW_XID( a->plotwindow->window );
  #endif

    pl_cmd( PLESC_DEVINIT, &xinfo );
    plenv( 0, 5, 0, 5, 0, 0 );

    plline( 3, x, y );
    plend();
}
//--------------------------------------------------------------------------

static gint ev_plotwindow_conf( GtkWidget *widget, GdkEventConfigure * PL_UNUSED( ev ), gpointer * PL_UNUSED( data ) )
{
  #if TO_PIXMAP == 1
    // Allocate pixmap
    if ( app.plotwindow_pixmap )
        gdk_pixmap_unref( app.plotwindow_pixmap );
    app.plotwindow_pixmap = gdk_pixmap_new( widget->window,
        widget->allocation.width, widget->allocation.height, -1 );

    // Clear the pixmap to a sensible background colour
    gdk_draw_rectangle( app.plotwindow_pixmap,
        widget->style->black_gc, TRUE, 0, 0,
        widget->allocation.width, widget->allocation.height );

    // If drawing to a pixmap we can do a plot from the conf handler since
    // the pixmap is now realised (the window widget isn't).
    //
    setup_plot_drawable( &app );
  #else
    (void) widget;     // Cast to void to silence compiler warning about unused parameter
  #endif

    return ( TRUE );
}

static gint ev_plotwindow_expose( GtkWidget *widget, GdkEventExpose *ev, gpointer * PL_UNUSED( data ) )
{
  #if TO_PIXMAP == 1
    // Dump the cached plot (created in the conf handler) to the window from
    // the pixmap.  We don't need to recreate the plot on each expose.
    //
    gdk_draw_pixmap( widget->window,
        widget->style->fg_gc[GTK_WIDGET_STATE( widget )],
        app.plotwindow_pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
        ev->area.width, ev->area.height );
  #else
    (void) widget;     // Cast to void to silence compiler warning about unused parameter
    (void) ev;
    // If drawing direct to an X Window, ensure GTK's double buffering
    // is turned off for that window or else the plot will be overridden
    // when the buffer is dumped to the screen.
    //
    setup_plot_drawable( &app );
  #endif

    return ( TRUE );
}

//--------------------------------------------------------------------------

void init_app( App *a )
{
    GtkWidget      *menubar;
    GtkItemFactory *item_factory;
    GtkAccelGroup  *accel_group = gtk_accel_group_new();
    signed int     nitems       = sizeof ( menu_items ) / sizeof ( menu_items[0] );

    GtkWidget      *vbox, *hbox;

    // Create the top-level root window
    a->rootwindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_signal_connect( GTK_OBJECT( a->rootwindow ), "delete_event", gtk_main_quit,
        NULL );

    // A toplevel box to hold things
    vbox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( a->rootwindow ), vbox );

    // Construct the main menu structure
    item_factory = gtk_item_factory_new( GTK_TYPE_MENU_BAR, "<main>", accel_group );
    gtk_item_factory_create_items( item_factory, (guint) nitems, menu_items, NULL );
    gtk_window_add_accel_group( GTK_WINDOW( a->rootwindow ), accel_group );
    menubar = gtk_item_factory_get_widget( item_factory, "<main>" );
    gtk_box_pack_start( GTK_BOX( vbox ), menubar, FALSE, FALSE, 0 );
    gtk_widget_show( menubar );

    // Fiddle with boxes to effect an indent from the edges of the root window
    hbox = gtk_hbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 10 );
    vbox = gtk_vbox_new( FALSE, 10 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox, TRUE, TRUE, 10 );

    // Add an area to plot into
    a->plotwindow        = gtk_drawing_area_new();
    a->plotwindow_pixmap = NULL;

  #if TO_PIXMAP != 1
    // Turn off double buffering if we are plotting direct to the plotwindow
    // in setup_plot_drawable().
    //
    GTK_WIDGET_UNSET_FLAGS( a->plotwindow, GTK_DOUBLE_BUFFERED );
  #endif

    // By experiment, 3x3 seems to be the smallest size plplot can cope with.
    // Here we utilise the side effect that gtk_widget_set_size_request()
    // effectively sets the minimum size of the widget.
    //
    gtk_widget_set_size_request( a->plotwindow, 3, 3 );
    gtk_box_pack_start( GTK_BOX( vbox ), a->plotwindow, TRUE, TRUE, 0 );

    // Set the initial size of the application
    gtk_window_set_default_size( GTK_WINDOW( a->rootwindow ), APP_INITIAL_WIDTH, APP_INITIAL_HEIGHT );

    g_signal_connect( G_OBJECT( a->plotwindow ), "configure_event",
        G_CALLBACK( ev_plotwindow_conf ), NULL );
    g_signal_connect( G_OBJECT( a->plotwindow ), "expose_event",
        G_CALLBACK( ev_plotwindow_expose ), NULL );

    gtk_widget_show_all( a->rootwindow );
}
//--------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
    gtk_init( &argc, &argv );
    init_app( &app );
    gtk_main();
    return 0;
}
//--------------------------------------------------------------------------
