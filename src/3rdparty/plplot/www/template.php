<!-- This template can be used to create new pages -->
<?php
  include "corefunctions.php";
?>

<!-- include the html header -->
<?php pageHeader("Template"); ?>

<body>

<div id="pagewrapper">

  <!-- create the menu, template is selected -->
	<?php pageMenu("template"); ?>

	<div id="contentwrapper">
		<div id="leftside">
			<h3>Title</h3>
		  <p>text</p>
		</div>

    <!-- add the sidebar, no news displayed -->
		<?php pageSidebar(); ?>

		<div id="spacer"></div>
	</div>

  <!-- include the page footer -->
	<?php pageFooter(); ?>
</div>

</body>
</html>
