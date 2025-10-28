[CmdletBinding()]
param(
    [switch]$Clean
)

# Fail fast on errors and enable stricter PowerShell semantics.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot '..')
Push-Location $repoRoot

try {
    Write-Host "Running ENSENS Meteo build check in '$repoRoot'" -ForegroundColor Cyan

    if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
        throw "Could not find 'idf.py'. Open an ESP-IDF environment (e.g. run 'export.ps1') before invoking this script."
    }

    if ($Clean) {
        Write-Host "Cleaning previous build artifacts (idf.py fullclean)..." -ForegroundColor Yellow
        & idf.py fullclean
        if ($LASTEXITCODE -ne 0) {
            throw "'idf.py fullclean' failed with exit code $LASTEXITCODE."
        }
    }

    Write-Host "Building project (idf.py build)..." -ForegroundColor Yellow
    & idf.py build
    if ($LASTEXITCODE -ne 0) {
        throw "'idf.py build' failed with exit code $LASTEXITCODE."
    }

    Write-Host "Build completed successfully." -ForegroundColor Green
}
finally {
    Pop-Location
}
