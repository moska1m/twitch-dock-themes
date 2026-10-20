$ErrorActionPreference = 'Stop'
if (Get-Process obs64 -ErrorAction SilentlyContinue) {
    throw 'Feche o OBS antes de restaurar o plugin anterior.'
}
$binary = Join-Path $env:ProgramData 'obs-studio\plugins\twitch-dock-themes\bin\64bit\twitch-dock-themes.dll'
$backup = $binary + '.previous'
if (-not (Test-Path -LiteralPath $backup)) { throw 'Não há uma DLL anterior salva pelo instalador.' }
Copy-Item -LiteralPath $backup -Destination $binary -Force
if ((Get-FileHash -LiteralPath $backup).Hash -ne (Get-FileHash -LiteralPath $binary).Hash) {
    throw 'A restauração não passou na verificação.'
}
Write-Output 'DLL anterior restaurada. Reabra o OBS. As preferências atuais foram mantidas.'
