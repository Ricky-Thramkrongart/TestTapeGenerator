#Requires -Version 7

function Create-Table
{
	[CmdletBinding()]
	param
	(
		[Parameter(ValueFromPipeline = $true)]
		[System.IO.FileInfo]$CSVFile
	)
	
	[string]$HeaderBlock = "$($CSVFile.basename.ToUpper())_H"
	
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
	[System.Globalization.CultureInfo]$saveCurrentCulture = [System.Threading.Thread]::CurrentThread.CurrentCulture
	[System.Globalization.CultureInfo]$saveCurrentUICulture = [System.Threading.Thread]::CurrentThread.CurrentUICulture
	
	[System.Threading.Thread]::CurrentThread.CurrentCulture = [System.Globalization.CultureInfo]::new(1033)
	[System.Threading.Thread]::CurrentThread.CurrentUICulture = [System.Globalization.CultureInfo]::new(1033)
	
	[array]$row = @()
	for ([int]$i = 0; $i -ne $TableDouble.Length; $i++)
	{
		$row += "{ $($($TableDouble[$i]) -join ', ') }"
	}
	
	@("#ifndef $HeaderBlock",
		"#define $HeaderBlock",
		"",
		"const PROGMEM double $($CSVFile.basename)[$($OutPutTable.Length)] = $($row -join ', ');",
		"",
		"#endif // $HeaderBlock"
	) | Set-Content -Path ".\$($CSVFile.basename).h"
	
	
	[System.Threading.Thread]::CurrentThread.CurrentCulture = $saveCurrentCulture
	[System.Threading.Thread]::CurrentThread.CurrentUICulture = $saveCurrentUICulture
}

Push-Location $PSScriptRoot

Get-ChildItem .\OutPutTable.csv | Create-Table
Get-ChildItem .\InPutTable.csv | Create-Table

Pop-Location