<?php
	// open plplot rss news feed via simplepie
	include('simplepie.inc');

	// Substring without losing word meaning and
	// tiny words (length 3 by default) are included on the result.
	// "..." is added if result do not reach original string length
	// taken from http://www.php.net/manual/en/function.substr.php#93963
	function _substr($str, $length, $minword = 3)
	{
    $sub = '';
    $len = 0;

    foreach(explode(' ', $str) as $word) {
	    $part = (($sub != '') ? ' ' : '') . $word;
	    $sub .= $part;
	    $len += strlen($part);

	    if(strlen($word) > $minword && strlen($sub) >= $length) {
	    	break;
	    }
    }

    return $sub . (($len < strlen($str)) ? '...' : '');
	}

	//
	// Print standard page header
	//
	function pageHeader($title="Main")
	{
		echo <<<END
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">

<!--
Copyright 2008-2010 Werner Smekal
Copyright 2008-2011 Hazen Babcock
Copyright 2008-2013 Alan W. Irwin
-->

<head>
  <title>PLplot Home Page - $title</title>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <meta name="description" content="Homepage of the open source PLplot project" />
  <meta name="keywords" content="plot,library,c++,python" />
  <link rel="stylesheet" title="liquid" href="css/style_liquid.css" type="text/css" media="screen,projection" />
  <link rel="alternate stylesheet" title="static" href="css/style_static.css" type="text/css" media="screen,projection" />
  <script type="text/javascript" src="js/slimbox.js"></script>
  <link rel="stylesheet" href="css/slimbox.css" type="text/css" media="screen" />
</head>

END;
	}

	//
	// Print menu code with selected element
	// $item might be: index, downloads, examples, development, credits
	//
	function pageMenu($item)#
	{
		echo "	<div id=\"pageheader\">\n";
		echo "	</div>\n";

		echo '	<div id="menubar">';
		echo '		<ul>';
		echo '		  <li><a href="index.php" ' . (($item=='index') ? ('id="selected"') : ('')) .'>Home</a></li>';
		echo '		  <li><a href="http://sourceforge.net/p/plplot/news" ' . (($item=='news') ? ('id="selected"') : ('')) .'>News</a></li>';
		echo '		  <li><a href="downloads.php" ' . (($item=='downloads') ? ('id="selected"') : ('')) .'>Downloads</a></li>';
		echo '		  <li><a href="examples.php" ' . (($item=='examples') ? ('id="selected"') : ('')) .'>Examples</a></li>';
		echo '		  <li><a href="documentation.php" ' . (($item=='documentation') ? ('id="selected"') : ('')) .'>Documentation</a></li>';
//		echo '		  <li><a href="development.php" ' . (($item=='development') ? ('id="selected"') : ('')) .'>Development</a></li>';
		echo '		  <li><a href="credits.php" ' . (($item=='credits') ? ('id="selected"') : ('')) .'>Credits</a></li>';
		echo '		</ul>';
		echo '	</div>';
	}

	//
	// Print a standard page footer
	//
	function pageFooter()
	{
		echo '		<div id="pagefooter">';
		echo '			<p>Original design by <a href="http://DesignsByDarren.com">DesignsByDarren.com</a>, Some Rights Reserved.<br/>Design modifications &copy; 2008-2015 Plplot developer community.<br /><br/>';
		echo '			<a href="http://validator.w3.org/check?uri=referer"><img style="border:0;width:88px;height:31px" src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0 Transitional" /></a>';
		echo '      <a href="http://jigsaw.w3.org/css-validator/check?uri=referer"><img style="border:0;width:88px;height:31px" src="http://jigsaw.w3.org/css-validator/images/vcss" alt="Valid CSS!" /></a>';
		echo '      </p>';
		echo '		</div>';
	}

	//
	// Print News from project page
	// $newscount - number of news itmes to be shown
	// $contentlength - defines the length in characters of the news content
	//
	function pageNews($newscount, $contentlength)
	{
		// Open the PLplot News RSS feed and parse it (rss link taken from the plplot project page)
		$feed = new SimplePie();
		$feed->enable_cache(false);  // disable cache
		$feed->set_feed_url("http://sourceforge.net/p/plplot/news/feed");
		$feed->init();
		$feed->handle_content_type();

		// In case of error show error
		//if($feed->error())
		//	echo $feed->error();

		// show news items
		if($feed->data) {
			$items = $feed->get_items();
			$i = 0;
			foreach($items as $item) {
				echo '<h4><a href="' . $item->get_permalink() . '">' . $item->get_title() . '</a></h4>' . "\n";
				echo '<p>' . _substr(strip_tags($item->get_content()), $contentlength) . ' ';
				echo '<a href="' . $item->get_permalink() . '">Read more</a> (' . $item->get_date('j M Y') . ')</p>' . "\n";

				$i = $i + 1;
				if( $i >= $newscount )
					break;
			}
		} else
			echo '<p>Could not open <a href="' . $url . '">News feed</a>!</p>';
	}

	//
	// Print a standard sidebar
	//
	function pageSidebar($news=0)
	{
		echo <<<END
		<!-- Sidebar -->
		<div id="rightside">
END;
//			Permanently disable and do another way (see above)
//                      because pageNews calls SF php script that returns
//			bad URL's.
		echo "<h3>News</h3>\n";
		pageNews(3, 100);

		echo <<<END
			<h3>Resources</h3>
			<ul class="arrowlist">
				<li><a href="documentation.php">Documentation</a></li>
				<li><a href="http://sourceforge.net/p/plplot/wiki">Wiki</a></li>
				<li><a href="http://sourceforge.net/projects/plplot">SourceForge Project Page</a></li>
				<li><a href="http://sourceforge.net/projects/plplot/support">Project Support Page</a></li>
				<li><a href="http://sourceforge.net/p/plplot/mailman/">Mailing Lists</a></li>
				<li><a href="http://sourceforge.net/p/plplot/bugs/">Bug Tracker</a></li>
			</ul>
			<h3>Source Code</h3>
			<ul class="arrowlist">
				<li><a href="http://sourceforge.net/projects/plplot/files/plplot/">Releases</a></li>
				<li>git repository is git://git.code.sf.net/p/plplot/plplot</li>
				<li><a href="http://sourceforge.net/p/plplot/plplot/ci/master/tree/">Browse git repository</a></li>
			</ul>

		</div>
END;
	}

?>
