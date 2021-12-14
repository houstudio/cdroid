<?php
  include "corefunctions.php";
?>

<?php pageHeader("Development"); ?>

<body>

<div id="pagewrapper">

	<?php pageMenu("development"); ?>

	<div id="contentwrapper">
		<div id="leftside">
		  <p>PLplot is an open source library. It's therefore possible to extend
         the library and contribute the code to the community. We introduce some
         information on where new developers should start.</p>

      <h3>Information</h3>
      <p>Apart from this web page there are some other sources where the developer
         can get some information on the PLplot library.
         <ul class="arrowlist">
         <li>The source code itself is well commented and it's always worth have
             a look there to get better understanding.</li>
         <li>The core developers mainly discuss things in the PLplot developer
             mailing list. You can search the archives or ask questions there.</li>
         <li>There is also a wiki where also information for developers can be
             found. Don't forget to extend the wiki if something is missing or wrong.</li>
         <li>The PLplot manual not only provides valuable information for
             users but also for developers.</li>
         </ul>
      </p>

      <h3>Developing drivers</h3>
      <p>If one wants to develop a new driver it's always best to start with the
         code from an old driver. The minimal code needed for a driver can be
         seen in drivers/mem.c or even drivers/null.c. The CMake build system
         needs also be adjusted for the new driver. The necessary information
         is found in drivers/README.drivers. Also, there are several useful
         sections in the DocBook documentation (e.g., "Output Devices", "Driver
         Functions" and "Adding FreeType Library Support to Bitmap Drivers"
         in Chapter 3) that you should look at. If the new driver provides
         the functions needed and is working you can extend the driver e.g.
         by processing the font on your own (drivers/cairo.c or
         drivers/wxwidgets.cpp) and handle key and mouse inputs (drivers/xwin.c).
         </p>
			<h3>Extending the PLplot core</h3>
      <p></p>
		</div>

		<?php pageSidebar(1); ?>

		<div id="spacer"></div>
	</div>

	<?php pageFooter(); ?>
</div>

</body>
</html>
