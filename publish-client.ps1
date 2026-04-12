param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [ValidateSet("win-x64")]
    [string]$Runtime = "win-x64",

    [switch]$SingleFile = $true
)

$ErrorActionPreference = "Stop"

$projectPath = Join-Path $PSScriptRoot "client\client.csproj"
if (-not (Test-Path -LiteralPath $projectPath)) {
    throw "Client project not found: $projectPath"
}

$publishArgs = @(
    "publish",
    $projectPath,
    "-c", $Configuration,
    "-r", $Runtime,
    "--self-contained", "true",
    "/p:PublishSingleFile=$($SingleFile.IsPresent.ToString().ToLowerInvariant())"
)

Write-Host "Publishing client with arguments:"
Write-Host "dotnet $($publishArgs -join ' ')"

dotnet @publishArgs
if ($LASTEXITCODE -ne 0) {
    throw "dotnet publish failed with exit code $LASTEXITCODE."
}

$publishDir = Join-Path $PSScriptRoot "client\bin\$Configuration\net8.0-windows\$Runtime\publish"
Write-Host "Publish succeeded."
Write-Host "Output: $publishDir"
