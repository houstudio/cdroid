<?php
  include "corefunctions.php";

	# Each element of the structure below contains:
	# [ demo#, number of pages, particular thumbnail for top-level page, credits, logical array indicating which pages to display]
	# if number of pages==0 the entry is ignored (currently none are ignored).
	$demoArray = array(
	  array( 0,  1, 1, "
	<p>Simplest tutorial demo of a 2D line plot.</p>
	\n", array_fill( 0, 1, 1)),
	  array( 1,  1, 1, "
	<p>Simple line plot and multiple windows demo.</p>
	\n", array_fill( 0, 1, 1)),
	  array( 2,  2, 1, "
	<p>Multiple window and color map 0 demo.</p>
	\n", array_fill( 0, 2, 1)),
	  array( 3,  1, 1, "
	<p>Polar plot demo.</p>
	\n", array_fill( 0, 1, 1)),
	  array( 4,  2, 1, "
	<p>Log plot demo.</p>
	\n", array_fill( 0, 2, 1)),
	  array( 5,  1, 1, "
	<p>Histogram demo.</p>
	\n", array_fill( 0, 1, 1)),
	  array( 6,  5, 1, "
	<p>Symbol demo.</p>
	\n", array_fill( 0, 5, 1)),
	  array( 7, 19, 1, "
	<p>Font demo for Hershey encodings.</p>
	\n", array_fill( 0, 19, 1)),
	  array( 8,  10, 1, "
	<p>3-d plot demo.</p>
	\n", array_fill( 0, 10, 1)),
	  array( 9,  5, 1, "
	<p>Contour plot demo.</p>
	\n", array_fill( 0, 5, 1)),
	  array(10,  1, 1, "
	<p>Window positioning demo.</p>
	\n", array_fill( 0, 1, 1)),
	  array(11,  8, 1, "
	<p>Mesh plot demo.</p>
	\n", array_fill( 0, 8, 1)),
	  array(12,  1, 1, "
	<p>Bar chart demo.</p>
	\n", array_fill( 0, 1, 1)),
	  array(13,  1, 1, "
	<p>Pie chart demo.</p>
	\n", array_fill( 0, 1, 1)),
	  array(14,  4, 1, "
	<p>Demo of multiple stream/window capability.  The two pages of the master stream appear as the first two pages here.  The two pages of the slave stream appear as pages 3 and 4 here.</p>
	\n", array_fill( 0, 4, 1)),
	  array(15,  3, 1, "
	<p>Shade plot demo using color map 0 (discrete colors) (plshade).</p>
	\n", array_fill( 0, 3, 1)),
	  array(16,  5, 1, "
	<p>Shade plot demo using color map 1 (continuous colours) (plshades).</p>
	\n", array_fill( 0, 5, 1)),
	  array(17,  1, 1, "
	<p>4-pen strip chart demo.</p>
	\n", array_fill( 0, 1, 1)),
	  array(18,  8, 1, "
	<p>3-d line and point plot demo.</p>
	\n", array_fill( 0, 8, 1)),
	  array(19,  5, 5, "
	<p>Demo of map functions.</p>
	\n", array_fill( 0, 5, 1)),
	  array(20,  6, 3, "
	<p>Demo of gray-scale image manipulation (plimage).</p>
	\n", array_fill( 0, 6, 1)),
	  array(21,  3, 3, "
	<p>Grid data demo.</p>
	\n", array_fill( 0, 3, 1)),
	  array(22,  5, 4, "
	<p>Demo of simple vector plotting.</p>
	\n", array_fill( 0, 5, 1)),
	  array(23, 16, 1, "
	<p>Font demo for unicode encodings which shows Greek letters and math symbols.</p>
	\n", array_fill( 0, 16, 1)),
	  array(24,  1, 1, "
	<p>Demo of unicode Pace Flag.</p>
	<p>Credits:</p>
	<ul class=\"arrowlist\">
	<li><a href=\"http://www.columbia.edu/~fdc/pace/\">Say Peace in all languages!</a></li>
	<li><a href=\"http://www.columbia.edu/~fdc/pace/flag.html\">Pace flag</a></li>
	</ul>
	\n", array_fill( 0, 1, 1)),
	  array(25,  8, 1, "
	<p>Diagnostic demo of filling and clipping polygons.</p>
	\n", array_fill( 0, 8, 1)),
	  array(26,  2, 1, "
	<p>Demo of multi-lingual unicode versions of the first page of example 4.</p>
	\n", array_fill( 0, 2, 1)),
	  array(27, 20, 1, "
	<p>Demo of drawing and filling 'spirograph' curves -
epitrochoids, cycolids, roulettes.  We have specified the general
PLplot option '-eofill' when generating these figures.  That means the
spirographic curves will be filled using the even-odd fill rule rather
than the (default) non-zero winding number fill rule.  The motivation
for this choice is the even-odd rule produces more 'interesting'
looking results than would be produced by the non-zero rule (where a
much larger fraction of the complex spirographic curve would be
filled).
     	</p>
	\n", array_fill( 0, 20, 1)),
	  array(28,  5, 1, "
	<p>Demo of 3-d writing of text (plmtex3 and plptex3).</p>
	\n", array_fill( 0, 5, 1)),
	  array(29, 10, 1, "
	<p>Demo of date / time formatting for axes.</p>
	\n", array_fill( 0, 10, 1)),
	  array(30,  2, 1, "
	<p>Demo of opacity/ transparency (or alpha channel).</p>
	\n", array_fill( 0, 2, 1)),
	  array(31,  1, 1, "
	<p>Diagnostic demo to exercise all our set/get functions.  The result should be a properly-formatted empty plot with black background.</p>
	\n", array_fill( 0, 1, 1)),
	  array(32,  1, 1, "
	<p>Demo of how to draw box-whisker / candlestick plots using PLplot. These are commonly used for financial data.</p>
	\n", array_fill( 0, 1, 1)),
	  array(33,  100, 1, "
	<p>Subset of the results from the demo of legends and colorbars.</p>
	\n", array_fill( 0, 100, 0)));
	# Overwrite the example 33 pages (currently none) that will be displayed
	$demoArray[33][4][0] = 1;
	$demoArray[33][4][1] = 1;
	$demoArray[33][4][2] = 1;
	$demoArray[33][4][3] = 1;
	$demoArray[33][4][4] = 1;
	$demoArray[33][4][20] = 1;
	$demoArray[33][4][36] = 1;
	$demoArray[33][4][52] = 1;
	$demoArray[33][4][68] = 1;
	for($ndx = 84; $ndx <= 99; ++$ndx)
          $demoArray[33][4][$ndx] = 1;
?>

<!-- include the html header -->
<?php pageHeader("Examples"); ?>

<body>

<div id="pagewrapper">

  <!-- create the menu, examples is selected -->
	<?php pageMenu("examples"); ?>

  <!-- the actual content of the page -->
	<div id="contentwrapper">
		<div id="fullside">
<?php

  // if the variable demo was set the source code of this example
  // is displayed
	if(isset($_GET["demo"])) {
		$demoID=$_GET["demo"];  // demo number
		$lbind=$_GET["lbind"];  // language
		$demo_str = sprintf("%02d", $demoArray[$demoID-0][0]);
		$nplots = $demoArray[$demoID-0][1];
		$credits = $demoArray[$demoID-0][3];
	        $demo_dir = "examples-data/demo$demo_str";

    // add buttons for language selection
		echo <<<END
		<h3>Example $demoID</h3>
		<p>Select language to see the source code</p>
		<form class="codeform" action="examples.php" method="get">
		<input type="hidden" name="demo" value="$demo_str" />
		<input type="submit" name="lbind" value="Ada(Standard)" />
		<input type="submit" name="lbind" value="Ada(Traditional)" />
		<input type="submit" name="lbind" value="C" />
		<input type="submit" name="lbind" value="C++" />
		<input type="submit" name="lbind" value="D" />
		<input type="submit" name="lbind" value="Fortran" />
		<input type="submit" name="lbind" value="Java" />
		<input type="submit" name="lbind" value="Lua" />
		<input type="submit" name="lbind" value="Ocaml" />
		<input type="submit" name="lbind" value="Octave" />
		<input type="submit" name="lbind" value="Python" />
		<input type="submit" name="lbind" value="Tcl" />
		</form>
END;

	  switch($lbind) {
	  case ("Ada(Standard)"):    $fname = "xstandard" . $demo_str . "a.adb.txt"; break;
	  case ("Ada(Traditional)"):    $fname = "xtraditional" . $demo_str . "a.adb.txt"; break;
	  case ("C"):      $fname = "x" . $demo_str . "c.c.txt"; break;
	  case ("C++"):$fname = "x" . $demo_str . ".cc.txt"; break;
	  case ("D"):$fname = "x" . $demo_str . "d.d.txt"; break;
	  case ("Fortran"):    $fname = "x" . $demo_str . "f.f90.txt"; break;
	  case ("Java"):   $fname = "x" . $demo_str . ".java.txt"; break;
	  case ("Lua"):    $fname = "x" . $demo_str . ".lua.txt"; break;
	  case ("Ocaml"):  $fname = "x" . $demo_str . ".ml.txt"; break;
	  case ("Octave"): $fname = "x" . $demo_str . "c.m.txt"; break;
	  case ("Python"): $fname = "x" . $demo_str . ".py.txt"; break;
	  case ("Tcl"):    $fname = "x" . $demo_str . ".tcl.txt";  break;
	  default:         $fname = "x" . $demo_str . "c.c.txt";
	  }

    // view code
	  echo "<object id=\"codebox\" type=\"text/plain\" data=\"$demo_dir/$fname\">\n";
		echo "Your browser is not able to display text!\n</object>\n";

    // show plots for chosen example
	  echo "<p>Select to view full-size image</p>\n";
		echo "<p>\n";
	  for($ndx = 1; $ndx <= $nplots; ++$ndx) {
	  if($demoArray[$demoID-0][4][$ndx-1]==1){
	    $ndx_str = sprintf ("%02d", $ndx);
			echo "<a href=\"$demo_dir/x$demo_str.$ndx_str.png\" rel=\"lightbox[plplot]\" title=\"x$demo_str.$ndx_str\">
					<img src=\"$demo_dir/prev-x$demo_str.$ndx_str.png\" border=\"1\" alt=\"x$demo_str.$ndx_str\"/></a>\n";
	  }
	  }
		echo "</p>\n";

    // show credits if any
		if($credits!="")
			echo "$credits";

		echo "<h4>Choose other examples</h4><p>\n";
	  foreach($demoArray as $drec) {
		  $dstr = sprintf ("%02d", $drec[0]);
			if($drec[1]!=0) {
		    if($dstr != $demo_str) {
		      echo "<a href=\"examples.php?demo=$dstr&amp;lbind=$lbind\">x$dstr</a>\n";
		    } else {
		      echo "x$dstr\n";
		    }
			}
	  }
		echo "</p>\n";
	} else {
    // display plots if all examples
		echo <<<END
			<h3>Examples</h3>
			<p>These examples were generated with the pngcairo device and
				 reflect the status of PLplot-5.15.0.</p>
			<p>Select a thumbnail to view full-size image and source code.</p>
			<p>
END;
		foreach($demoArray as $drec) {
		  $demo_str = sprintf ("%02d", $drec[0]);
		  $dir = "demo$demo_str";
		  $thumbnail = sprintf ("%02d", $drec[2]);
		  $npages = $drec[1];

			if($npages!=0) {
				echo "<a href=\"examples.php?demo=$demo_str\"><img border=\"0\" src=\"examples-data/$dir/prev-x$demo_str.$thumbnail.png\" alt=\"x$demo_str.thumbnail\"/></a>\n";
			}
		}
		echo "</p>";
	}
?>
		</div>
	</div>

  <!-- include the page footer -->
	<?php pageFooter(); ?>
</div>

</body>
</html>
