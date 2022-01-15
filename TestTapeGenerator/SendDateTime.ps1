param
(
	[string]$COM = 'COM4'
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'
#Requires -Version 7

do
{
	[System.IO.Ports.SerialPort]$port = $null
	Write-Host "Connect the test tape geneartor to $COM"
	[string]$ports = ''
	[System.IO.Ports.SerialPort]::getportnames() | ForEach-Object { $ports += "$_ " }
	Write-Host "COM ports found: $ports"
	if ([System.IO.Ports.SerialPort]::getportnames() -contains $COM)
	{
		$port = new-Object System.IO.Ports.SerialPort $COM, 115200, None, 8, one
		if (!$port.IsOpen) { $port.open() }
		while ($port.IsOpen)
		{                                                 
			#[string]$datetime = Get-Date -format '2020,12,22,22,22,22.'
			[string]$datetime = Get-Date -format 'yyyy,MM,dd,HH,mm,ss.'
			$port.Write($datetime)
			Write-Host "Sent '$datetime' to $COM"
			Start-Sleep -Seconds 1
		}
	}
	start-Sleep -Seconds 1
}
while ($true)

