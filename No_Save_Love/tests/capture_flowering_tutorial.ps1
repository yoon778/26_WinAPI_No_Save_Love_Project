$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$buildOutput = Join-Path $projectRoot 'outputs\flowering\tutorial_capture'
$sourceImages = Join-Path $projectRoot 'No_Save_Love\resource\minigame5\tutorial'
$packageImages = Join-Path $projectRoot 'No_Save_Love_Distribution\resource\minigame5\tutorial'
New-Item -ItemType Directory -Force $buildOutput, $sourceImages, $packageImages | Out-Null
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$visualStudio = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $visualStudio) { throw 'Visual Studio C++ build tools were not found.' }
$developerCommand = Join-Path $visualStudio 'Common7\Tools\VsDevCmd.bat'
$captureSource = Join-Path $PSScriptRoot 'capture_flowering_tutorial.cpp'
$captureExe = Join-Path $buildOutput 'capture_flowering_tutorial.exe'
$compileFile = Join-Path $buildOutput 'compile.cmd'
$compile = @"
@echo off
call "$developerCommand" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cd /d "$buildOutput"
cl /nologo /std:c++20 /EHsc /utf-8 /MT /O2 /DUNICODE /D_UNICODE "$captureSource" /Fe:"$captureExe" /link user32.lib gdi32.lib msimg32.lib
exit /b %errorlevel%
"@
Set-Content -LiteralPath $compileFile -Value $compile -Encoding Default
& $env:ComSpec /d /c $compileFile
if ($LASTEXITCODE -ne 0) { throw 'Tutorial capture compilation failed.' }
Push-Location (Join-Path $projectRoot 'No_Save_Love')
try {
    & $captureExe $sourceImages (Join-Path $buildOutput 'contact_sheet.png')
    if ($LASTEXITCODE -ne 0) { throw "Tutorial capture failed: $LASTEXITCODE" }
} finally { Pop-Location }
foreach ($name in @('wind', 'memory', 'relay', 'encore', 'return', 'spinner')) {
    $source = Join-Path $sourceImages "$name.png"
    $destination = Join-Path $packageImages "$name.png"
    Copy-Item -LiteralPath $source -Destination $destination -Force
    if ((Get-FileHash -LiteralPath $source).Hash -ne (Get-FileHash -LiteralPath $destination).Hash) {
        throw "Packaged tutorial image mismatch: $name"
    }
}
Write-Output 'PASS: six production-render tutorial examples captured and packaged.'
