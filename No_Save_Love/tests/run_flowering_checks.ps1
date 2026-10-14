$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$testOutput = Join-Path $projectRoot 'outputs\flowering\checks'
New-Item -ItemType Directory -Force $testOutput | Out-Null
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$visualStudio = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $visualStudio) { throw 'Visual Studio C++ build tools were not found.' }
$developerCommand = Join-Path $visualStudio 'Common7\Tools\VsDevCmd.bat'
$compileFile = Join-Path $testOutput 'compile.cmd'
$testExe = Join-Path $testOutput 'flowering_check.exe'
[xml]$project = Get-Content (Join-Path $projectRoot 'No_Save_Love\No_Save_Love.vcxproj')
$sources = @('"' + (Join-Path $PSScriptRoot 'flowering_check.cpp') + '"')
foreach ($item in $project.Project.ItemGroup.ClCompile) {
    $source = $item.Include
    if (-not $source -or $source -in @('main.cpp','GameManager.cpp','minigame2\RhythmMiniGame.cpp','minigame2\RhythmMiniGameFlowering.cpp')) { continue }
    $sources += '"' + (Join-Path $projectRoot ('No_Save_Love\' + $source)) + '"'
}
$sourceArguments = $sources -join ' '
$compile = @"
@echo off
call "$developerCommand" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cd /d "$testOutput"
cl /nologo /std:c++20 /EHsc /utf-8 /MT /O2 /DUNICODE /D_UNICODE $sourceArguments /Fe:"$testExe" /link user32.lib gdi32.lib msimg32.lib
exit /b %errorlevel%
"@
Set-Content -LiteralPath $compileFile -Value $compile -Encoding Default
& $env:ComSpec /d /c $compileFile
if ($LASTEXITCODE -ne 0) { throw 'C++ flowering check compilation failed.' }
Push-Location (Join-Path $projectRoot 'No_Save_Love')
try {
    & $testExe (Join-Path $testOutput 'frames')
    if ($LASTEXITCODE -ne 0) { throw "Flowering checks failed: $LASTEXITCODE" }
} finally { Pop-Location }
