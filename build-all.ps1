param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",

    [ValidateSet("x64")]
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

function Resolve-MSBuildPath {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path -LiteralPath $vswhere)) {
        $fromPath = Get-Command msbuild.exe -ErrorAction SilentlyContinue
        if ($fromPath) {
            return $fromPath.Source
        }

        throw "Could not find msbuild.exe in PATH and vswhere.exe is missing."
    }

    # Prefer VS 2022 / MSBuild 17 because current WDK targets are tied to this toolset.
    $installPath = & $vswhere -latest -version "[17.0,18.0)" -products * -requires Microsoft.Component.MSBuild -property installationPath
    if (-not $installPath) {
        $installPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    }

    if (-not $installPath) {
        $fromPath = Get-Command msbuild.exe -ErrorAction SilentlyContinue
        if ($fromPath) {
            return $fromPath.Source
        }

        throw "Visual Studio with MSBuild component was not found."
    }

    $msbuild = Join-Path $installPath "MSBuild\Current\Bin\MSBuild.exe"
    if (-not (Test-Path -LiteralPath $msbuild)) {
        throw "MSBuild.exe was not found under: $installPath"
    }

    return $msbuild
}

$solutionPath = Join-Path $PSScriptRoot "readDriver.sln"
if (-not (Test-Path -LiteralPath $solutionPath)) {
    throw "Solution not found: $solutionPath"
}

$msbuildPath = Resolve-MSBuildPath
Write-Host "Using MSBuild: $msbuildPath"
Write-Host "Building: $solutionPath ($Configuration|$Platform)"

& $msbuildPath $solutionPath /t:Build /m /restore "/p:Configuration=$Configuration;Platform=$Platform"
if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE."
}

Write-Host "Build succeeded."
