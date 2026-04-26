param(
    [Parameter(Mandatory = $true)]
    [string]$Path,

    [string]$TargetName = "target"
)

if (-not (Test-Path -LiteralPath $Path)) {
    exit 0
}

try {
    $stream = [System.IO.File]::Open(
        $Path,
        [System.IO.FileMode]::Open,
        [System.IO.FileAccess]::ReadWrite,
        [System.IO.FileShare]::None
    )
    $stream.Close()
    exit 0
}
catch {
    Write-Error "Target '$TargetName' output is locked: $Path. Close the running application and rebuild."
    exit 1
}
