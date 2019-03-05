<?php
include ("/var/www/htdocs/jpgraph/src/jpgraph.php");
include ("/var/www/htdocs/jpgraph/src/jpgraph_bar.php");

$timestamp_now = date("U");
$start = $timestamp_now - 36000;//36000 = 10 hours
$conn=mysql_connect("localhost","fov","public") or die("Unable to connect to database fov");
mysql_select_db("fov", $conn) or die("Unable to database fov");
$query = "select Timestamp,RA,Declination from Shift where Timestamp >$start and R >= 0.2 and ABS(RA) <= 20 and ABS(Declination) <= 20 order by Timestamp";
$result = mysql_query($query,$conn) or die("query failed");
$k=1;
while( $dd = mysql_fetch_array($result))
{
    $timestamp[$k] = $dd["Timestamp"];
    $ra[$k] = (int)$dd["RA"];
    $dec[$k] = (int)$dd["Declination"];	
    $k++;
}
mysql_free_result($result);
mysql_close($conn);
//Include these 2 lines to plot last data
$ra[$k]=0;
$dec[$k]=0;
$k--;

$night = date("Y/m/d",$timestamp_now);
if($k<2)
{
	$ra=array(0,0,0);
	$dec = array(0,0,0);
	$T="No recent local data for $night";
}
else
{
	$T="Local shifts on RA and DEC for $night";
}
// Create the graph. These two calls are always required
$graph = new Graph(640,200,"auto");	
$graph->SetScale("intint");
$graph->xaxis->Hide();
$graph->SetTickDensity(TICKD_SPARSE);
$graph->img->SetMargin(50,50,10,30);
$graph->SetFrame(False);
$graph->SetMarginColor('white');

// Show the gridlines
$graph->ygrid->Show(true,true);
$graph->xgrid->Show(true,true);

//Create two bar plots
$raplot = new BarPlot($ra);
$raplot->SetFillColor('brown4');
$decplot = new BarPlot($dec);
$decplot->SetFillColor('blue');

//Create the accumulated bar plot
$barplot = new GroupBarPlot(array($raplot,$decplot));
$barplot->SetWidth(0.95);

// Add the plot to the graph
$graph->Add($barplot);

$graph->yaxis->title->Set('Pixel');
$graph->yaxis->title->SetFont(FF_FONT1,FS_BOLD);
$graph->yaxis->title->SetMargin(15);

$graph->title->Set("$T"); 

// Adjust the color of the Y axis
$graph->yaxis->SetColor("black");

// Specify a legend
$raplot->SetLegend("RA");
$decplot->SetLegend("DEC");

// Adjust the position of the legend box
$graph->legend->Pos(0.0,0.15,"right","center");
$graph->legend->SetFillColor('aquamarine3@0.4');

// Display the graph
$graph->Stroke();

?>
