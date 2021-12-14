<?php
  include "corefunctions.php";
?>

<?php pageHeader("Downloads"); ?>

<body>

<div id="pagewrapper">

	<?php pageMenu("downloads"); ?>

	<div id="contentwrapper">
		<div id="leftside">
			<h3><a name="source_code">Source Code</a></h3>
		  <p>All our Windows users and many of our Unix users build PLplot from
source code using our build system that is implemented with <a href="http://cmake.org/">CMake</a>
following the directions in our
<a href="http://sourceforge.net/p/plplot/wiki">wiki</a>.  PLplot source code can be
accessed a number of ways.  You can obtain the latest stable version of PLplot from our
		     <a href="http://sourceforge.net/projects/plplot/files/plplot/">file
		     release site</a>. Alternatively, you can obtain the cutting-edge version of
		     PLplot (but with no promises about stability) using</p>
		  <p><tt>git clone git://git.code.sf.net/p/plplot/plplot plplot.git</tt></p>
		     <p>Finally, you can browse our git repository
		     <a href="http://sourceforge.net/p/plplot/plplot/ci/master/tree">
		     here</a>.</p>
			<h3><a name="Binary_Packages">Binary Packages</a></h3>
			<p>A number of third-party binary packages for PLplot are available.
Binary packages are provided by at least the following Linux distributions:</p>
		  <ul class="arrowlist">
<li><a href="http://packages.ubuntu.com/search?keywords=plplot&amp;searchon=names&amp;suite=all&amp;section=all">Ubuntu</a></li>
<li><a href="http://packages.debian.org/search?searchon=sourcenames&amp;keywords=plplot">Debian</a></li>
<li><a href="https://apps.fedoraproject.org/packages/plplot">Fedora</a></li>
<li><a href="http://software.opensuse.org/package/plplot">openSUSE</a></li>
		  </ul>
<p>Binary packages are provided by at least the following Mac OS X distributions:</p>
		  <ul class="arrowlist">
<li><a href="http://www.finkproject.org/pdb/browse.php?summary=plplot">Fink</a></li>
<li><a href="https://formulae.brew.sh/formula/plplot">Homebrew</a></li>
<li><a href="https://www.macports.org/ports.php?by=name&amp;substr=plplot">MacPorts</a></li>
		  </ul>
<p>To our knowledge no binary packages for PLplot are currently available for
Windows platforms so our Windows users must build PLplot
<a href="#source_code">from source</a>.</p>
		</div>

		<?php pageSidebar(1); ?>

		<div id="spacer"></div>
	</div>

	<?php pageFooter(); ?>
</div>

</body>
</html>
