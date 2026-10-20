$ErrorActionPreference = 'Stop'
$source = Join-Path $PSScriptRoot 'plugin\bin\64bit\twitch-dock-themes.dll'
$destination = Join-Path $env:ProgramData 'obs-studio\plugins\twitch-dock-themes'
$binaryDir = Join-Path $destination 'bin\64bit'
$dataDir = Join-Path $destination 'data'
if (-not (Test-Path -LiteralPath $source)) { throw 'DLL do pacote não encontrada.' }
$existing = Join-Path $binaryDir 'twitch-dock-themes.dll'
if (Get-Process obs64 -ErrorAction SilentlyContinue) {
    throw 'Feche o OBS antes de instalar ou atualizar este plugin.'
}
$sourceHash = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
$manifest = Join-Path $PSScriptRoot 'SHA256SUMS.txt'
if (-not (Test-Path -LiteralPath $manifest)) { throw 'Manifesto SHA256SUMS.txt ausente.' }
$entry = @(Get-Content -LiteralPath $manifest | Where-Object { $_ -match '  plugin/bin/64bit/twitch-dock-themes\.dll$' })
if ($entry.Count -ne 1 -or $entry[0].Split(' ')[0] -ne $sourceHash) {
    throw 'A DLL não corresponde ao checksum do pacote.'
}
New-Item -ItemType Directory -Path $binaryDir,$dataDir -Force | Out-Null
if ((Test-Path -LiteralPath $existing) -and (Get-FileHash -LiteralPath $existing).Hash -ne $sourceHash) {
    Copy-Item -LiteralPath $existing -Destination ($existing + '.previous') -Force
}
Copy-Item -LiteralPath $source -Destination $existing -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'plugin\data\README.txt') -Destination $dataDir -Force
if ((Get-FileHash -LiteralPath $source).Hash -ne (Get-FileHash -LiteralPath $existing).Hash) {
    throw 'A cópia do plugin não passou na verificação.'
}
$themesDir = Join-Path $env:APPDATA 'obs-studio\themes'
New-Item -ItemType Directory -Path $themesDir -Force | Out-Null
foreach ($theme in @('Obsidian.ovt','Obsidian Graphite.ovt')) {
    $target = Join-Path $themesDir $theme
    if (-not (Test-Path -LiteralPath $target)) {
        Copy-Item -LiteralPath (Join-Path (Join-Path $PSScriptRoot 'themes') $theme) -Destination $target
    }
}
Write-Output 'Instalado. Reabra o OBS 32.2.2 e acesse Ferramentas > Twitch Dock Themes.'
