param(
    [Parameter(Mandatory=$true)][string]$ProjectDir,
    [string]$EngineDir = 'C:/Program Files/Epic Games/UE_5.6/Engine'
)
$ErrorActionPreference = 'Stop'
# UBT -Module builds omit the final metadata action. Register only the newly
# compiled plugin DLLs using the installed engine's existing compatibility ID.
$engineManifest = Get-Content -LiteralPath "$EngineDir/Binaries/Win64/UnrealEditor.modules" -Raw | ConvertFrom-Json
foreach ($name in @('ACEConversation', 'ACEConversationMetaHuman')) {
    $binaryDir = Join-Path $ProjectDir "Plugins/$name/Binaries/Win64"
    $dll = "UnrealEditor-$name.dll"
    if (-not (Test-Path -LiteralPath (Join-Path $binaryDir $dll))) { throw "Build $name first" }
    $modules = @{}
    $modules[$name] = $dll
    $manifest = @{ BuildId = $engineManifest.BuildId; Modules = $modules } | ConvertTo-Json
    [IO.File]::WriteAllText((Join-Path $binaryDir 'UnrealEditor.modules'), $manifest)
}
