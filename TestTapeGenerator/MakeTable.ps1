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
	

	[System.Globalization.CultureInfo]$saveCurrentCulture = [System.Threading.Thread]::CurrentThread.CurrentCulture
	[System.Globalization.CultureInfo]$saveCurrentUICulture = [System.Threading.Thread]::CurrentThread.CurrentUICulture
		
	[System.Threading.Thread]::CurrentThread.CurrentCulture = [System.Globalization.CultureInfo]::new('en-DK')
	[System.Threading.Thread]::CurrentThread.CurrentUICulture = [System.Globalization.CultureInfo]::new('en-US')


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
	
	[System.Threading.Thread]::CurrentThread.CurrentCulture = [System.Globalization.CultureInfo]::new('en-US')
	[System.Threading.Thread]::CurrentThread.CurrentUICulture = [System.Globalization.CultureInfo]::new('en-US')
	
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
		"uint8_t $($CSVFile.basename)Fit (double dB)",
		"{",
		"     uint8_t rv = 0.00001020475 * pow(dB,6) - 0.000634453 * pow(dB, 5) + 0.012749614 * pow(dB, 4) - 0.058752793 * pow(dB, 3) - 0.352027967 * pow(dB, 2) - 16.60582415 * dB + 254.8871207;",
		"     rv = std::min(rv, (uint8_t)256);",
		"     rv = std::max(rv, (uint8_t)0);",
		"     return rv;",
		"}",
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