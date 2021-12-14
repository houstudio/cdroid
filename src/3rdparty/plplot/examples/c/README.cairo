Some examples demonstrating how to use the PLplot xcairo and extcairo
devices to draw into an externally supplied XDrawable or Cairo context.

EXTERNAL X DRAWABLE EXAMPLES
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The external X drawable example is extXdrawable_demo.c.  It is designed to
demonstrate the use of the ability of plplot's xcairo driver to use an X
drawable supplied by an application.  The example itself uses GTK but the
concept applies to any graphical framework, or even programs using libX11
directly.

To build this demo, install PLplot with the "make install" command and then
run 'make extXdrawable_demo' in the installed .../examples/c directory.

EXTERNAL CAIRO CONTEXT EXAMPLES
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The C program ext-cairo-test.c demonstrates how to pass in an external
Cairo Context that PLplot can use for plotting using the extcairo device.

To build this demo, install PLplot with the "make install" command and then
run 'make ext-cairo-test' in the installed .../examples/c directory.

