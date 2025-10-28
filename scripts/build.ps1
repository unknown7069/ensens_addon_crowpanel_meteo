[CmdletBinding()]
param(
    [switch]$Clean,
    [string]$BuildDir = 'build'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot '..')
Push-Location $repoRoot

function Invoke-IdfPy {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    & idf.py @Arguments
    if ($LASTEXITCODE -ne 0) {
        $joined = $Arguments -join ' '
        throw "'idf.py $joined' failed with exit code $LASTEXITCODE."
    }
}

try {
    Write-Host "Building ENSENS Meteo project from '$repoRoot'" -ForegroundColor Cyan

    $idfPath = $env:IDF_PATH
    if (-not $idfPath) {
        $configPath = Join-Path $env:USERPROFILE '.espressif\esp_idf.json'
        if (-not (Test-Path $configPath)) {
            throw "ESP-IDF environment not initialized. Could not locate '$configPath'."
        }

        $config = Get-Content $configPath -Raw | ConvertFrom-Json
        $selectedId = $config.idfSelectedId
        if (-not $selectedId) {
            throw "No 'idfSelectedId' found in '$configPath'."
        }

        $idfInfo = $config.idfInstalled.$selectedId
        if (-not $idfInfo) {
            throw "Selected ESP-IDF install '$selectedId' not present in '$configPath'."
        }

        $idfPath = $idfInfo.path
        if ($idfInfo.python) {
            $pythonExe = $idfInfo.python
            if (Test-Path $pythonExe) {
                $env:IDF_PYTHON_ENV_PATH = Split-Path -Parent (Split-Path -Parent $pythonExe)
                Write-Host "Using ESP-IDF virtual environment at '$env:IDF_PYTHON_ENV_PATH'." -ForegroundColor DarkCyan
            }
        }
    }

    if (-not (Test-Path $idfPath)) {
        throw "ESP-IDF path '$idfPath' does not exist."
    }

    $exportScript = Join-Path $idfPath 'export.ps1'
    if (-not (Test-Path $exportScript)) {
        throw "ESP-IDF export script '$exportScript' not found."
    }

    Write-Host "Sourcing ESP-IDF environment from '$exportScript'..." -ForegroundColor Yellow
    . $exportScript | Out-Null

    if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
        throw "Could not find 'idf.py' after sourcing ESP-IDF environment."
    }

    if ($Clean) {
        Write-Host "Running 'idf.py fullclean'..." -ForegroundColor Yellow
        Invoke-IdfPy -Arguments @('fullclean')
    }

    $idfArgs = @('build')
    if ($BuildDir -and $BuildDir -ne 'build') {
        $idfArgs += @('--build-dir', $BuildDir)
    }

    Write-Host "Running 'idf.py $($idfArgs -join ' ')'..." -ForegroundColor Yellow
    Invoke-IdfPy -Arguments $idfArgs

    Write-Host 'Build completed successfully.' -ForegroundColor Green
}
finally {
    Pop-Location
}
