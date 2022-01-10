#Requires -Version 7

Set-StrictMode -Version 2.0

class RecordStep
{
    [uint64]$Frequency
    [uint64]$Time
    [int64]$Level
    [string]$Comment
    
    RecordStep([uint64]$Frequency, [uint64]$Time, [int64]$Level)
    {
        $this.Frequency = $Frequency
        $this.Time = $Time
        $this.Level = $Level
        $this.Comment = $null
    }
    
    RecordStep([uint64]$Frequency, [uint64]$Time, [int64]$Level, [string]$Comment)
    {
        $this.Frequency = $Frequency
        $this.Time = $Time
        $this.Level = $Level
        $this.Comment = $Comment
    }
    
    [string] ToString ()
    {
        return "$($this.Frequency) Hz $($this.Level) dB $($this.Time) Sec"
    }
}

class TapeInfo
{
    [string]$Description
    [uint16]$Tracks
    [uint16]$Length
    [uint16]$Flux
    [string]$Format
    [double]$Target
    [RecordStep[]]$RecordSteps
    
    TapeInfo(
        [string]$Description,
        [uint16]$Tracks,
        [uint16]$Flux,
        [string]$Format,
        [double]$Target,
        [RecordStep[]]$RecordSteps
    )
    {
        $this.Description = $Description
        $this.Tracks = $Tracks
        $this.Length = ($RecordSteps | Measure-Object -Property Time -Sum).Sum
        $this.Flux = $Flux
        $this.Format = $Format
        $this.Target = $Target
        $this.RecordSteps = $RecordSteps
    }
    
    [string] ToString ()
    {
        return "$($this.Description)`n$($this.Tracks) Track - $($this.RecordSteps.Length) Tracks - $([math]::ceiling($this.Length/60)) Min - $($this.Flux) nW/m - $($this.Format)"
    }
}

[TapeInfo[]]$TapeList = @(
    #################################################################
    [TapeInfo]::new('Revox B77 Test Tape 1', 2, 257, 'Reeltape', 0.0, @(
            [RecordStep]::new(1000, 120, 0, 'Level'),
            [RecordStep]::new(1000, 60, -20, 'Azimuth'),
            [RecordStep]::new(10000, 120, -20, 'Azimuth'),
            [RecordStep]::new(10000, 120, 0, 'Azimuth'))),
    #################################################################	
    [TapeInfo]::new('Revox B77 Test Tape 2', 4, 257, 'Reeltape', -5.0, @(
            [RecordStep]::new(1000, 120, 0, 'Level'),
            [RecordStep]::new(1000, 60, -20, 'Azimuth'),
            [RecordStep]::new(10000, 120, -20, 'Azimuth'),
            [RecordStep]::new(10000, 120, 0, 'Azimuth'))),
    #################################################################
    [TapeInfo]::new('Universal Test Tape', 2, 200, 'Cassette', 0.0, @(
            [RecordStep]::new(1000, 120, 0, 'Level'),
            [RecordStep]::new(31.5, 60, 0, 'Azimuth'),
            [RecordStep]::new(31.5, 60, -3),
            [RecordStep]::new(31.5, 60, -6),
            [RecordStep]::new(31.5, 60, -10),
            [RecordStep]::new(31.5, 60, -15),
            [RecordStep]::new(31.5, 60, -20),
            [RecordStep]::new(63, 60, 0),
            [RecordStep]::new(63, 60, -3),
            [RecordStep]::new(63, 60, -6),
            [RecordStep]::new(63, 60, -10),
            [RecordStep]::new(63, 60, -15),
            [RecordStep]::new(63, 60, -20),
            [RecordStep]::new(125, 60, 0),
            [RecordStep]::new(125, 60, -3),
            [RecordStep]::new(125, 60, -6),
            [RecordStep]::new(125, 60, -10),
            [RecordStep]::new(125, 60, -15),
            [RecordStep]::new(125, 60, -20),
            [RecordStep]::new(250, 60, 0),
            [RecordStep]::new(250, 60, -3),
            [RecordStep]::new(250, 60, -6),
            [RecordStep]::new(250, 60, -10),
            [RecordStep]::new(250, 60, -15),
            [RecordStep]::new(250, 60, -20),
            [RecordStep]::new(500, 60, 0),
            [RecordStep]::new(500, 60, -3),
            [RecordStep]::new(500, 60, -6),
            [RecordStep]::new(500, 60, -10),
            [RecordStep]::new(500, 60, -15),
            [RecordStep]::new(500, 60, -20),
            [RecordStep]::new(1000, 60, 0),
            [RecordStep]::new(1000, 60, -3),
            [RecordStep]::new(1000, 60, -6),
            [RecordStep]::new(1000, 60, -10),
            [RecordStep]::new(1000, 60, -15),
            [RecordStep]::new(1000, 60, -20),
            [RecordStep]::new(2000, 60, 0),
            [RecordStep]::new(2000, 60, -3),
            [RecordStep]::new(2000, 60, -6),
            [RecordStep]::new(2000, 60, -10),
            [RecordStep]::new(2000, 60, -15),
            [RecordStep]::new(2000, 60, -20),
            [RecordStep]::new(4000, 60, 0),
            [RecordStep]::new(4000, 60, -3),
            [RecordStep]::new(4000, 60, -6),
            [RecordStep]::new(4000, 60, -10),
            [RecordStep]::new(4000, 60, -15),
            [RecordStep]::new(4000, 60, -20),
            [RecordStep]::new(6300, 60, 0),
            [RecordStep]::new(6300, 60, -3),
            [RecordStep]::new(6300, 60, -6),
            [RecordStep]::new(6300, 60, -10),
            [RecordStep]::new(6300, 60, -15),
            [RecordStep]::new(6300, 60, -20),
            [RecordStep]::new(8000, 60, 0),
            [RecordStep]::new(8000, 60, -3),
            [RecordStep]::new(8000, 60, -6),
            [RecordStep]::new(8000, 60, -10),
            [RecordStep]::new(8000, 60, -15),
            [RecordStep]::new(8000, 60, -20),
            [RecordStep]::new(10000, 60, -10),
            [RecordStep]::new(10000, 60, -15),
            [RecordStep]::new(10000, 60, -20),
            [RecordStep]::new(12500, 60, -10),
            [RecordStep]::new(12500, 60, -15),
            [RecordStep]::new(12500, 60, -20),
            [RecordStep]::new(15000, 60, -10),
            [RecordStep]::new(15000, 60, -15),
            [RecordStep]::new(15000, 60, -20),
            [RecordStep]::new(1000, 120, 0))),
    #################################################################
    [TapeInfo]::new('Wow and Flutter Test Tape 1', 2, 257, 'Reeltape', 0.0, @(
            [RecordStep]::new(3000, 120, 0, 'DIN Standard'),
            [RecordStep]::new(3150, 120, 0, 'JIS Standard'))),
    #################################################################
    [TapeInfo]::new('Wow and Flutter Test Tape 2', 4, 257, 'Reeltape', 0.0, @(
            [RecordStep]::new(3000, 120, 0, 'DIN Standard'),
            [RecordStep]::new(3150, 120, 0, 'JIS Standard'))),
    #################################################################
    [TapeInfo]::new('Wow and Flutter Test Tape 3', 4, 185, 'Reeltape', -5.0, @(
            [RecordStep]::new(3000, 120, 0, 'DIN Standard'),
            [RecordStep]::new(3150, 120, 0, 'JIS Standard'))),
    #################################################################
    [TapeInfo]::new('AKAI GX 75 95 Test Tape', 2, 200, 'Cassette', 0, @(
            [RecordStep]::new(315, 180, 0, 'Level'),
            [RecordStep]::new(1000, 180, 0, 'Azimuth'),
            [RecordStep]::new(3150, 180, 0),
            [RecordStep]::new(10000, 180, -15))),
    #################################################################
    [TapeInfo]::new('Nakamichi Test Tape', 2, 200, 'Cassette', 0, @(
            [RecordStep]::new(20000, 180, -20, 'DA09001A 20 kHz Frequency Response'),
            [RecordStep]::new(15000, 180, -20, 'DA09002A 15 kHz Frequency Response'),
            [RecordStep]::new(10000, 180, -20, 'DA09003A 10 kHz Frequency Response'),
            [RecordStep]::new(15000, 180, 0, 'DA09004A 15 kHz Azimuth'),
            [RecordStep]::new(400, 180, -10, 'DA09005A 400 Hz Playback Level'),
            [RecordStep]::new(3000, 180, 0, 'DA09006A 3 kHz Speed and Wow & Flutter'))),
    #################################################################
    [TapeInfo]::new('Revox B215 Test Tape', 2, 200, 'Cassette', 0, @(
            [RecordStep]::new(1000, 180, + 2),
            [RecordStep]::new(1000, 180, 0),
            [RecordStep]::new(3150, 180, 0),
            [RecordStep]::new(10000, 180, -20),
            [RecordStep]::new(16000, 180, -20))),
    #################################################################
    [TapeInfo]::new('Wow and Flutter Test Tape 4', 2, 200, 'Cassette', 0, @(
            [RecordStep]::new(3000, 480, 0, 'DIN Standard'),
            [RecordStep]::new(3150, 480, 0, 'JIS standard'))),
    #################################################################
    [TapeInfo]::new('Panasonic QZZCFM Test Tape', 2, 200, 'Cassette', 0, @(
            [RecordStep]::new(315, 180, -2, 'Level'),
            [RecordStep]::new(3000, 180, -10, 'Azimuth'),
            [RecordStep]::new(8000, 180, -20),
            [RecordStep]::new(63, 20, -20),
            [RecordStep]::new(125, 20, -20),
            [RecordStep]::new(250, 20, -20),
            [RecordStep]::new(1000, 20, -20),
            [RecordStep]::new(2000, 20, -20),
            [RecordStep]::new(8000, 20, -20),
            [RecordStep]::new(10000, 20, -20),
            [RecordStep]::new(12500, 20, -20))),
    #################################################################
    [TapeInfo]::new('Studer A710 Test Tape', 2, 200, 'Cassette', 0, @(
            [RecordStep]::new(315, 460, 0, 'Level'),
            [RecordStep]::new(10000, 460, -20, 'Azimuth'))),
    #################################################################
    [TapeInfo]::new('Tandberg 24 Test Tape', 2, 250, 'Cassette', 0, @(
            [RecordStep]::new(1000, 4800, + 2, 'Flux: 250nW/m'))),
    #################################################################
    [TapeInfo]::new('Playback EQ Test Tape', 2, 200, 'Cassette', 0, @(
            [RecordStep]::new(1000, 120, 0, '1 kHz Reference Level'),
            [RecordStep]::new(10000, 5, -20, 'Alternating 1kHz/10kHz'),
            [RecordStep]::new(1000, 5, -20, 'Azimuth'),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 5, -20),
            [RecordStep]::new(1000, 5, -20),
            [RecordStep]::new(10000, 120, -20))),
    #################################################################
    [TapeInfo]::new('TEAC MTT Multi Test Tape', 2, 200, 'Cassette', 0, @(
            [RecordStep]::new(315, 120, 0, 'MTT-212N'),
            [RecordStep]::new(315, 120, -4, 'MTT-212CN')
            [RecordStep]::new(1000, 120, -4, 'MTT-212EN')
            [RecordStep]::new(1000, 120, -10, 'MTT-118N')
            [RecordStep]::new(3000, 120, -10, 'MTT-111N')
            [RecordStep]::new(6300, 120, -10, 'MTT-113N')
            [RecordStep]::new(8000, 120, -10, 'MTT-113CN')
            [RecordStep]::new(10000, 120, -10, 'MTT-114N')
            [RecordStep]::new(12500, 120, -24, 'MTT-118NA')))
)

function Show-SplashScreen
{
    [CmdletBinding()]
    param ()
    
    Clear-Host
    Write-Host -NoNewLine @'
        Test Tape Generator
  SW rev: 2022/01/02
TapeList: 2022/01/02

'@
    Start-Sleep -Seconds 2
}

function Start-SelfTest
{
    [CmdletBinding()]
    param ()
    
    Clear-Host
    Clear-Host
    Write-Host @'
I2C test: OK
'@
    Start-Sleep -Milliseconds 750
    Write-Host @'
RTC:OK EEPROM:OK ATT:OK DISP:OK
'@
    Start-Sleep -Milliseconds 750
    Write-Host @'
Frequncy response test:
20Hz to 25Khz +/- 0.1 dB :OK
'@
    Start-Sleep -Milliseconds 1000
}


function Start-AdjustingReferenceLevel
{
    [CmdletBinding()]
    param
    (
        [Parameter(Mandatory = $true)]
        [TapeInfo]$Tape
    )
    
    Clear-Host
    Write-Host "Reference Level"
    Write-Host "$($Tape.Description)"
    Write-Host "Start recording"
    Write-Host "                          7:Exit  8:Next "
    
    [ConsoleKeyInfo]$keyInfo = [Console]::ReadKey($true)
    switch ($keyInfo.Key)
    {
        'D7' { throw }
    }
    
    [double]$Target = $Tape.Target
    do
    {
        [double]$LeftLevel = $Target + $(Get-Random -Minimum -1.0 -Maximum 1.0)
        [double]$RightLevel = $Target + $(Get-Random -Minimum -1.0 -Maximum 1.0)
        
        $LeftLevelStatus = '='
        if ($LeftLevel -lt 0.5)
        {
            $LeftLevelStatus = '<' 
        }
        if ($LeftLevel -gt 0.5)
        {
            $LeftLevelStatus = '>'
        }
        
        $RightLevelStatus = '='
        if ($RightLevel -lt 0.5)
        {
            $RightLevelStatus = '<'
        }
        if ($RightLevel -gt 0.5)
        {
            $RightLevelStatus = '>'
        }
               
        Clear-Host
        Write-Host "Reference Level"
        Write-Host "$($Tape.Description)"
        Write-Host "Target:$Target dB  Actuel (L : R): $LeftLevelStatus : $RightLevelStatus"
        Write-Host "                          7:Exit  8:Next "
        if ([Console]::KeyAvailable)
        {
            [ConsoleKeyInfo]$keyInfo = [Console]::ReadKey($true)
            if ($keyInfo.Key -eq 'D7')
            {
                throw
            }
            elseif ($keyInfo.Key -eq 'D8')
            {
                break
            }
        }
        Start-Sleep -Seconds 1
        
    }
    while ($true)
    $i = 0
    do
    {
        Clear-Host
        Write-Host "Reference Level"
        Write-Host "$($Tape.Description)"
        Write-Host "Adjustment: $i % Complete"
        Write-Host "                          7:Exit "
        if ([Console]::KeyAvailable)
        {
            [ConsoleKeyInfo]$keyInfo = [Console]::ReadKey($true)
            
            switch ($keyInfo.Key)
            {
                'D7' { throw }
            }
        }
        Start-Sleep -Milliseconds 50
        if ($i -eq 100)
        {
            return
        }
        $i += 1
        
    }
    while ($true)
}

function Start-AdjustingRecordLevel
{
    [CmdletBinding()]
    param
    (
        [Parameter(Mandatory = $true)]
        [TapeInfo]$Tape
    )
    
    for ($t = 0; $t -lt $Tape.RecordSteps.Length; $t++)
    {
        $RS = $Tape.RecordSteps[$t]
        $i = 0
        do
        {
            Clear-Host
            Write-Host "Record Level"
            Write-Host "$($Tape.Description) $RS ($($t+1)/$($Tape.RecordSteps.Length))"
            Write-Host "Adjustment: $i % Complete"
            Write-Host "                          7:Exit "
            if ([Console]::KeyAvailable)
            {
                [ConsoleKeyInfo]$keyInfo = [Console]::ReadKey($true)
                switch ($keyInfo.Key)
                {
                    'D7' { throw }
                }
            }
            Start-Sleep -Milliseconds 10
            if ($i -eq 100)
            {
                break
            }
            $i += 1
        }
        while ($true)
    }
}

function Start-TestTapeRecord
{
    [CmdletBinding()]
    param
    (
        [Parameter(Mandatory = $true)]
        [TapeInfo]$Tape
    )
    
    Clear-Host
    Write-Host "Record Test Tape"
    Write-Host "$($Tape.Description)"
    Write-Host "Rewind tape and start recording"
    Write-Host "                          7:Exit  8:Next "
    
    [ConsoleKeyInfo]$keyInfo = [Console]::ReadKey($true)
    switch ($keyInfo.Key)
    {
        'D7' { throw }
    }
    
    for ($t = 0; $t -lt $Tape.RecordSteps.Length; $t++)
    {
        $RS = $Tape.RecordSteps[$t]
        
        $i = 0
        do
        {
            Clear-Host
            Write-Host "Record Test Tape"
            Write-Host "$($Tape.Description) $RS ($($t + 1)/$($Tape.RecordSteps.Length))"
            Write-Host "Recording: $i % Complete"
            Write-Host "                          7:Exit "
            if ([Console]::KeyAvailable)
            {
                [ConsoleKeyInfo]$keyInfo = [Console]::ReadKey($true)
                switch ($keyInfo.Key)
                {
                    'D7' { throw }
                }
            }
            Start-Sleep -Milliseconds 10
            if ($i -eq 100)
            {
                break
            }
            $i += 1
            
            
        }
        while ($true)
    }
}

function Start-PrintLabel
{
    [CmdletBinding()]
    param
    (
        [Parameter(Mandatory = $true)]
        [TapeInfo]$Tape
    )
    
    Clear-Host
    $Date = Get-Date -Format "yyyy/MM/dd"
    $Time = Get-Date -Format "HH:mm:ss"
    Write-Host "Test Tape Label $Date $Time"
    Write-Host ""
    Write-Host $Tape
    Write-Host ""
    for ($t = 0; $t -lt $Tape.RecordSteps.Length; $t++)
    {
        $RS = $Tape.RecordSteps[$t]
        if ($RS.Comment)
        {
            Write-Host "$(($($t+1).ToString("#")).PadLeft(2)): $RS $($RS.Comment)"
        }
        else
        {
            Write-Host "$(($($t + 1).ToString("#")).PadLeft(2)): $RS"
        }
    }
    Write-Host ""
    
    pause
}

function Show-Menu
{
    [OutputType([uint16])]
    param
    (
    )
    
    [string[]]$ActionList = @('Create Test Tape', 'Set Time', 'Set Mode')
    [uint16]$i = 0
    
    do
    {
        do
        {
            Clear-Host
            
            $Date = Get-Date -Format "yyyy/MM/dd"
            $Time = Get-Date -Format "HH:mm:ss"
            Write-Host "==Main Menu ======$Date $Time==="
            Write-Host $ActionList[$i]
            Write-Host ""
            Write-Host "$(($i -eq 0) ? '       ':'1:Down ') $(($i -eq $ActionList.Length - 1) ? '       ':'2:Up   ')                   8:Next"
            
            # wait for a key to be available:
            if ([Console]::KeyAvailable)
            {
                # read the key, and consume it so it won't
                # be echoed to the console:
                [ConsoleKeyInfo]$keyInfo = [Console]::ReadKey($true)
                break
            }
            Start-Sleep -Milliseconds 500
        }
        while ($true)
        
        switch ($keyInfo.Key)
        {
            'D1' { $i = [math]::Max(0, $i - 1) }
            'D2' { $i = [math]::Min($ActionList.Length - 1, $i + 1) }
            'D8' {
                if ($i -eq 0)
                {
                    return $i
                }
            }
        }
    }
    while ($true)
}

function Show-TapeSelection
{
    [OutputType([TapeInfo])]
    param
    (
    )
    
    [uint16]$i = 0
    
    do
    {
        do
        {
            Clear-Host
            
            $Date = Get-Date -Format "yyyy/MM/dd"
            $Time = Get-Date -Format "HH:mm:ss"
            Write-Host "==Tape Selection==$Date $Time==="
            Write-Host "$($TapeList[$i])"
            Write-Host "$(($i -eq 0) ? '       ':'1:Down ') $(($i -eq $TapeList.Length - 1) ? '       ':'2:Up   ')           7:Back  8:Next"
            #           "0123456789012345678901234567890123456789"
            
            # wait for a key to be available:
            if ([Console]::KeyAvailable)
            {
                # read the key, and consume it so it won't
                # be echoed to the console:
                [ConsoleKeyInfo]$keyInfo = [Console]::ReadKey($true)
                break
            }
            Start-Sleep -Milliseconds 500
        }
        while ($true)
        
        switch ($keyInfo.Key)
        {
            'D1' { $i = [math]::Max(0, $i - 1) }
            'D2' { $i = [math]::Min($TapeList.Length - 1, $i + 1) }
            'D7' {
                throw
            }
            'D8' {
                return $TapeList[$i]
            }
        }
    }
    while ($true)
}

function New-TestTape
{
    [CmdletBinding()]
    param ()
    
    try
    {
        [TapeInfo]$Tape = Show-TapeSelection
        Start-AdjustingReferenceLevel -Tape $Tape
        Start-AdjustingRecordLevel -Tape $Tape
        Start-TestTapeRecord -Tape $Tape
        Start-PrintLabel -Tape $Tape
    }
    catch
    {
        
    }
}

$TapeList = $TapeList | Sort-Object -Property Description


foreach ($tape in $TapeList)

{
	[string]$s = $tape.Description
	$s.ToUpperInvariant().Replace(' ','_')
}

exit


Show-SplashScreen
Start-SelfTest

do
{
    [uint16]$Selection = Show-Menu
    switch ($Selection)
    {
        0 { New-TestTape }
    }
}
while ($true)

