$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$output = Join-Path $projectRoot 'outputs\defense_perf'
New-Item -ItemType Directory -Force $output | Out-Null
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'Visual Studio C++ build tools were not found.' }
$compile = @"
@echo off
call "$vs\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cd /d "$output"
cl /nologo /std:c++20 /EHsc /utf-8 /MT /O2 /DUNICODE /D_UNICODE "$PSScriptRoot\defense_perf.cpp" "$projectRoot\No_Save_Love\AudioManager.cpp" /Fe:defense_perf.exe /link user32.lib gdi32.lib msimg32.lib gdiplus.lib
exit /b %errorlevel%
"@
$command = Join-Path $output 'compile.cmd'
Set-Content -LiteralPath $command -Value $compile -Encoding Default
& $env:ComSpec /d /c $command
if ($LASTEXITCODE -ne 0) { throw 'Defense performance check compilation failed.' }
Push-Location (Join-Path $projectRoot 'No_Save_Love')
try {
    & (Join-Path $output 'defense_perf.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Defense performance check failed.' }
} finally { Pop-Location }
