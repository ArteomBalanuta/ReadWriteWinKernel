param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",

    [ValidateSet("x64")]
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

function Get-WdkBuildInfo {
    $installedRoots = Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots" -ErrorAction SilentlyContinue
    if (-not $installedRoots -or -not $installedRoots.KitsRoot10) {
        return $null
    }

    $buildRoot = Join-Path $installedRoots.KitsRoot10 "build"
    if (-not (Test-Path -LiteralPath $buildRoot)) {
        return $null
    }

    $latestBuild = Get-ChildItem $buildRoot -Directory |
        Sort-Object Name -Descending |
        Select-Object -First 1
    if (-not $latestBuild) {
        return $null
    }

    $task17 = Join-Path $latestBuild.FullName "bin\Microsoft.DriverKit.Build.Tasks.17.0.dll"
    $task18 = Join-Path $latestBuild.FullName "bin\Microsoft.DriverKit.Build.Tasks.18.0.dll"

    [PSCustomObject]@{
        BuildRoot = $latestBuild.FullName
        HasTask17 = Test-Path -LiteralPath $task17
        HasTask18 = Test-Path -LiteralPath $task18
    }
}

function Resolve-MSBuildPath {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path -LiteralPath $vswhere)) {
        $fromPath = Get-Command msbuild.exe -ErrorAction SilentlyContinue
        if ($fromPath) {
            return $fromPath.Source
        }

        throw "Could not find msbuild.exe in PATH and vswhere.exe is missing."
    }

    $wdkInfo = Get-WdkBuildInfo
    if ($wdkInfo -and $wdkInfo.HasTask18) {
        $preferredRange = "[18.0,19.0)"
        Write-Host "WDK build tasks 18.0 found under $($wdkInfo.BuildRoot). Preferring Visual Studio 2026."
    } elseif ($wdkInfo -and $wdkInfo.HasTask17) {
        $preferredRange = "[17.0,18.0)"
        Write-Host "WDK build tasks 18.0 were not found under $($wdkInfo.BuildRoot). Falling back to an MSBuild 17 / Visual Studio 2022-compatible path for driver compatibility."
    } else {
        $preferredRange = ""
        Write-Host "WDK build tasks could not be detected. Falling back to latest MSBuild that is available."
    }

    if ($preferredRange) {
        $installPath = & $vswhere -latest -version $preferredRange -products * -requires Microsoft.Component.MSBuild -property installationPath
    } else {
        $installPath = $null
    }

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
