<?php
require("parse-ascii.php");

echo "<html><head>";
echo "<title>KITcube Reader Status Monitor</title>";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"kitcube.css\"/>";
echo "<script type=\"text/javascript\" src=\"kitcube.js\"></script>";
echo "</head><body>";
echo "<div id=\"header_div\" class=\"header\"></div>";
echo "<div id=\"body_div\" class=\"body\">";
//echo "<h1>KITcube Reader Device List</h1>";
kitcube_monitor();
echo "</div></body></html>";
?>
