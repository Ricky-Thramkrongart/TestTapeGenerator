#Requires -Version 7


function Get-RV
{
	[CmdletBinding()]
	param
	(
		[double]$x
	)
	
	$y = 0.00001020475 * [math]::Pow($x, 6) - 0.000634453 * [math]::Pow($x, 5) + 0.012749614 * [math]::Pow($x, 4) - 0.058752793 * [math]::Pow($x, 3) - 0.352027967 * [math]::Pow($x, 2) - 16.60582415 * $x + 254.8871207
	return $y
	
}


[System.Threading.Thread]::CurrentThread.CurrentCulture = [System.Globalization.CultureInfo]::new('en-DK')
[System.Threading.Thread]::CurrentThread.CurrentUICulture = [System.Globalization.CultureInfo]::new('en-US')

$CSVFile = gci .\InPutTable.csv

[array]$Header = @('A00')
$OutPutTable = import-csv $CSVFile -Delimiter ';' -Header $Header
[array]$TableDouble

foreach ($h in $Header)
{
	[array]$array = $OutPutTable.($h)
	[int]$array_size = $array.Length
	for ([int]$i = 0; $i -ne $array_size; $i++)
	{
		$array[$i] = [double]::Parse($array[$i])
	}
	$TableDouble += ,$array
}


for ($i = 0; $i -lt $TableDouble[0].Length; $i++)
{
	[double]$d = $TableDouble[0][$i]
	[int]$(Get-RV -x $d)
}


Pop-Location