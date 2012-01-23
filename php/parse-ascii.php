<?php
function process($f) {
    $lines = array();
    $max = 0;
    while (!feof($f)) {
	$line = fgets($f);
	$line = preg_replace("/(^\s*)|(\s*$)/", "", $line);
	if (!preg_match("/\|/", $line)) continue;
	$columns = preg_split("/\s*\|+\s*/", $line);
	$max = max($max, sizeof($columns));
	array_push($lines, $columns);
    }
    
    $first = true;
    echo "<table>";
    foreach ($lines as $line) {
	if (sizeof($line) < $max) continue;
	echo "<tr>";
	if ($first) {
	    foreach ($line as $col) {
		echo("<th>$col</th>");
	    }
	} else {
	    foreach ($line as $col) {
		echo("<td align='right'>$col</td>");
	    }
	}
	echo "</tr>";
	$first = false;
    }
    echo "</table>";
}

function kitcube_monitor() {    
    $cwd = getcwd();
    chdir("/home/cube");

	echo "<h2>Alarm list</h2>";
    $f = popen("./kitcube-monitor alarm", "r");
    if ($f) {
	process($f);
	pclose($f);
    }

	echo "<h2>List of all devices</h2>";	
    $f = popen("./kitcube-monitor status", "r");
    if ($f) {
	process($f);
	pclose($f);
    }

    chdir($cwd);
}

?>
