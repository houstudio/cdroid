<?php
  include "corefunctions.php";
?>

<!-- include the html header -->
<?php pageHeader("Main"); ?>

<body>

<div id="pagewrapper">

  <!-- create the menu, index is selected -->
	<?php pageMenu("index"); ?>

  <!-- the actual content of the page -->
  	<div id="contentwrapper">
	<div id="leftside">
	<h3>Introduction</h3>
        <p>PLplot is a <a href="#cross_platform">cross-platform</a> software package for creating scientific plots whose (<a href="http://www.utf-8.com/">UTF-8</a>) plot symbols and text are limited in practice only by what <a href="http://www.unicode.org/">Unicode</a>-aware system fonts are installed on a user's computer.  The PLplot software, which is primarily licensed under <a href="http://www.gnu.org/licenses/lgpl.html">the LGPL</a>, has a clean architecture that is organized as a core C library, separate language <a href="#bindings">bindings</a> for that library, and separate device drivers that are dynamically loaded by the core library which control how the plots are presented in <a href="#files">noninteractive</a> and <a href="#interactive">interactive</a> plotting contexts.</p>
	<p>The PLplot core library can be used to create standard x-y plots, semi-log plots, log-log plots, contour plots, 3D surface plots, mesh plots, bar charts and pie charts. Multiple graphs (of the same or different sizes) may be placed on a single page, and multiple pages are allowed for those device formats that support them.</p>
	<p>PLplot has core library support for plot symbols and text specified by
the user in the UTF-8 encoding of Unicode. This means for our many
Unicode-aware devices that plot symbols and text are only limited by
the collection of glyphs normally available via installed system
fonts.  Furthermore, a large subset of our Unicode-aware devices also
support complex text layout (CTL) languages such as Arabic, Hebrew,
and Indic and Indic-derived CTL scripts such as Devanagari, Thai, Lao,
and Tibetan.  Thus, for these PLplot devices essentially any language
that is supported by Unicode and installed system fonts can be used to
label plots.</p>
			<h3>Feature Summary</h3>
			<div class="splitcontentleft">
				<h4><a name="cross_platform">Cross Platform</a></h4>
				 <p>PLplot is currently known to work on the following platforms:</p>
				 <ul class="arrowlist">
				   <li>Linux, Mac OS X, and other Unices</li>
				   <li>MSVC IDE on the Microsoft version of Windows (Windows 2000 and later)</li>
				   <li>Cygwin on the Microsoft version of Windows</li>
				   <li>MinGW-w64/MSYS2 on the Microsoft version of Windows</li>
				 </ul>

				 <p>For each of the above platforms, PLplot can be built from <a href="downloads.php">source</a>, and for the Linux and Mac OS X platforms third-party binary packages for PLplot <a href="downloads.php#Binary_Packages">are available</a>.</p>
				<h4><a name="bindings">Language Bindings</a></h4>
				<p>The language bindings of the C PLplot library are currently the following:</p>
				<ul class="arrowlist">
				  <li>Ada</li>
				  <li>C++</li>
				  <li>D</li>
				  <li>Fortran</li>
				  <li>Java</li>
				  <li><a href="http://common-lisp.net/project/cl-plplot/">Lisp</a></li>
				  <li>Lua</li>
				  <li>OCaml</li>
				  <li>Octave</li>
				  <li><a href="http://search.cpan.org/~dhunt/PDL-Graphics-PLplot">Perl/PDL</a></li>
				  <li>Python</li>
				  <li>Tcl/Tk</li>
				</ul>
			</div>
			<div class="splitcontentright">
				<h4><a name="files">Output File Formats</a></h4>
				<p>PLplot device drivers support a number of plotting file formats.</p>
				<ul class="arrowlist">
				  <li>CGM</li>
				  <li>GIF</li>
				  <li>JPEG</li>
				  <li>PBM</li>
				  <li>PDF</li>
				  <li>PNG</li>
				  <li>PostScript</li>
				  <li>SVG</li>
				  <li>Xfig</li>
				</ul>
				<h4><a name="interactive">Interactive Platforms</a></h4>
				<p>PLplot device drivers support a number of platforms that are suitable for interactive plotting.</p>
				<ul class="arrowlist">
				  <li>GDI</li>
				  <li>GTK+</li>
				  <li>PyQt</li>
				  <li>Qt</li>
				  <li>Tcl/Tk</li>
				  <li>wxWidgets</li>
				  <li>X</li>
				</ul>
			</div>
			<div class="clear"></div>


      <!-- show some plots using the lightbox js script -->
			<h3>Screenshots</h3>
			<p>
			<a href="examples-data/demo03/x03.01.png" rel="lightbox[plplot]" title="Polar plot">
				<img src="examples-data/demo03/x03.01.png" width="172" height="129" border="1" alt="Polar plot"/></a>
			<a href="examples-data/demo08/x08.03.png" rel="lightbox[plplot]" title="Shade 3D plots">
				<img src="examples-data/demo08/x08.03.png" width="172" height="129" border="1" alt="Shade 3D plots"/></a>
			<a href="examples-data/demo09/x09.01.png" rel="lightbox[plplot]" title="Contouring plot">
				<img src="examples-data/demo09/x09.01.png" width="172" height="129" border="1" alt="Contouring plot"/></a>
			<a href="examples-data/demo16/x16.01.png" rel="lightbox[plplot]" title="False colour plot">
				<img src="examples-data/demo16/x16.01.png" width="172" height="129" border="1" alt="False colour plot"/></a>
			<a href="examples-data/demo19/x19.05.png" rel="lightbox[plplot]" title="Map plot">
				<img src="examples-data/demo19/x19.05.png" width="172" height="129" border="1" alt="False colour plot"/></a>
			<a href="examples-data/demo24/x24.01.png" rel="lightbox[plplot]" title="Unicode example">
				<img src="examples-data/demo24/x24.01.png" width="172" height="129" border="1" alt="Unicode example"/></a>
			<a href="examples-data/demo30/x30.01.png" rel="lightbox[plplot]" title="Alpha color support">
				<img src="examples-data/demo30/x30.01.png" width="172" height="129" border="1" alt="Alpha color support"/></a>
			</p>
			<p>You can find the code for these and other examples at the <a href="examples.php">examples page</a></p>
		</div>

    <!-- add the sidebar, news displayed -->
    <?php pageSidebar(1); ?>

		<div id="spacer"></div>
	</div>

  <!-- include the page footer -->
	<?php pageFooter(); ?>
</div>

</body>
</html>
