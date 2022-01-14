#Requires -RunAsAdministrator

Push-Location $PSScriptRoot
Get-ChildItem .\formatter.conf | Copy-Item -Destination 'c:\Program Files (x86)\Arduino\lib\'
Get-ChildItem .\platform.txt | Copy-Item -Destination 'c:\Program Files (x86)\Arduino\hardware\arduino\avr\'
Pop-Location
