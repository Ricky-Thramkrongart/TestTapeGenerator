#Requires -Version 7

[array]$Header = @('A00')
$OutPutTable = import-csv '.\OutPutTable.csv' -Delimiter ';' -Header $Header
[array]$TableDouble

foreach ($h in $Header)
{
	[array] $array = $OutPutTable.($h)
	[int]$array_size = $array.Length
	for ([int]$i = 0; $i -ne $array_size; $i++)
	{
		$array[$i] = [double]::Parse($array[$i])
	}
	$TableDouble += ,$array
}

[System.Threading.Thread]::CurrentThread.CurrentCulture = [System.Globalization.CultureInfo]::new(1033)
[System.Threading.Thread]::CurrentThread.CurrentUICulture = [System.Globalization.CultureInfo]::new(1033)

[array] $row = @()
for ([int] $i = 0; $i -ne $TableDouble.Length; $i++)
{
	$row += "{$($($TableDouble[$i]) -join ', ')}"
}

@("#ifndef OUTPUTTABLE_H",
  "#define OUTPUTTABLE_H",
  "", 	
  "const PROGMEM double OutPutTable[$($OutPutTable.Length)] = $($row -join ', ');",
	"",
  "#endif // OUTPUTTABLE_H"
) | Set-Content -Path .\OutPutTable.h

$InPutTable = import-csv '.\InPutTable.csv' -Delimiter ';' -Header $Header

foreach ($h in $Header)
{
	[array]$array = $InPutTable.($h)
	[int]$array_size = $array.Length
	for ([int]$i = 0; $i -ne $array_size; $i++)
	{
		$array[$i] = [double]::Parse($array[$i])
	}
	$TableDouble += ,$array
}

[System.Threading.Thread]::CurrentThread.CurrentCulture = [System.Globalization.CultureInfo]::new(1033)
[System.Threading.Thread]::CurrentThread.CurrentUICulture = [System.Globalization.CultureInfo]::new(1033)

[array]$row = @()
for ([int]$i = 0; $i -ne $TableDouble.Length; $i++)
{
 	$row += "{$($($TableDouble[$i]) -join ', ')}"
}

@("#ifndef INPUTTABLE_H",
  "#define INPUTTABLE_H",
  "", 	
  "const PROGMEM double InPutTable[$($InPutTable.Length)] = $($row -join ', ');",
	"",
  "#endif // INPUTTABLE_H"
) | Set-Content -Path .\InPutTable.h	

#region RegionName

#endregion RegionName