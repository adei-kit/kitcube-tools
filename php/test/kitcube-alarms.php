<?php
$DB = DB::Open();
$link = mysql_connect('localhost', 'mysql_user', 'mysql_password');
if (!$link) {
    die('keine Verbindung möglich: ' . mysql_error());
}
echo 'Verbindung erfolgreich';



$sql="SELECT titel, interpret, alter FROM user ORDER BY alter ";
if($_POST['sort'] == 'desc')
{
 $sql .= 'desc';
} else {
 $sql .= 'asc';
}
$db->query($sql." LIMIT 0,10");
?>
<table border="1" align="center">
<tr>
<td align="center">titel
<? echo '<a href="' . $_SERVER['PHP_SELF'] . '?sort=desc">Absteigend</a>';
echo '<a href="' . $_SERVER['PHP_SELF'] . '?sort=asc">Aufsteigend</a>'; ?> </td>
<td align="center">interpret
<? echo '<a href="' . $_SERVER['PHP_SELF'] . '?sort=desc">Absteigend</a>';
echo '<a href="' . $_SERVER['PHP_SELF'] . '?sort=asc">Aufsteigend</a>'; ?> </td>
<td align="center">alter
<? echo '<a href="' . $_SERVER['PHP_SELF'] . '?sort=desc">Absteigend</a>';
echo '<a href="' . $_SERVER['PHP_SELF'] . '?sort=asc">Aufsteigend</a>'; ?> </td>
  </tr>
<? for ($i;$i<$db->num_rows();$i++) {
$result=$db->get_row();
echo "<tr class=main>";
echo "<td align=center>";
  $i += 0;
  echo $i;
echo "</td>";
echo "<td>";
echo $result["titel"]."</td>";
echo "<td align=center>".$result["interpret"]."</td>";
echo "<td align=center>".$result["alter"]."</td>";
echo "</tr>";
} ?>
</table>
