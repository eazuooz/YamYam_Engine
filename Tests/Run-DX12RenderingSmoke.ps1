param(
    [switch]$Hardware,
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
$repository = Split-Path -Parent $PSScriptRoot
Push-Location -LiteralPath $repository
try {
    if (-not $SkipBuild) {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
        $msbuild = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\Current\Bin\MSBuild.exe' | Select-Object -First 1
        if (-not $msbuild) { throw 'Visual Studio C++ build tools were not found.' }
        & $msbuild YamYam_Engine.sln /m /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
        if ($LASTEXITCODE -ne 0) { throw 'Engine build failed.' }
        & $msbuild Tests/DX12RenderingSmoke.vcxproj /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
        if ($LASTEXITCODE -ne 0) { throw 'Smoke-test build failed.' }
    }
    if ($Hardware) {
        & .\x64\Debug\DX12RenderingSmoke.exe --hardware
    } else {
        & .\x64\Debug\DX12RenderingSmoke.exe
    }
    if ($LASTEXITCODE -ne 0) { throw 'DX12 rendering smoke test failed.' }
} finally {
    Pop-Location
}
