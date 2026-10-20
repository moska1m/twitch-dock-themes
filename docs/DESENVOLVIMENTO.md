# Desenvolvimento

## Compilar e testar

Dependências: Visual Studio 2022 com C++, SDK Windows, CMake 3.28+, Qt **6.5.3** x64 MSVC, código-fonte do OBS na tag **30.0.0**, Node.js para os testes JavaScript e Python para o helper de importação.

Use o terminal **x64 Native Tools Command Prompt for VS 2022**. Se não tiver as bibliotecas de importação do OBS, gere-as a partir das DLLs de uma cópia do OBS 30.0.0:

```powershell
python scripts/generate-imports.py --obs-bin "CAMINHO_OBS_30/bin/64bit" --output build/imports
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="CAMINHO_QT" -DOBS_SOURCE_DIR="CAMINHO_OBS" -DOBS_IMPORT_DIR="CAMINHO_ABSOLUTO/build/imports"
cmake --build build --config Release
node tests/bridge.test.cjs
```

No PowerShell, configure Qt para executar o teste nativo:

```powershell
$env:PATH = "CAMINHO_QT/bin;" + $env:PATH
$env:QT_PLUGIN_PATH = "CAMINHO_QT/plugins"
ctest --test-dir build -C Release --output-on-failure
```

CTest seleciona a plataforma Qt `minimal`. A DLL compilada fica em `build/Release/twitch-dock-themes.dll`. O cabeçalho do browser está fixado em `vendor/`. O workflow incluído no GitHub Actions executa somente o teste JavaScript; compilação e teste Qt exigem as dependências acima.

O pacote Windows inclui código e DLL. O arquivo de fontes separado omite binários. Para distribuir uma recompilação, atualize a DLL e o manifesto SHA-256; o instalador exige a entrada `plugin/bin/64bit/twitch-dock-themes.dll` no manifesto.

### Referências

- [OBS 32.2.2](https://github.com/obsproject/obs-studio/tree/32.2.2)
- [Interface do obs-browser fixada pelo OBS 32.2.2](https://github.com/obsproject/obs-browser/blob/3f0a2cdf378939ebe3c6f9ab36d4ea100c25aac2/panel/browser-panel.hpp)
- [Dependências Qt oficiais usadas](https://github.com/obsproject/obs-deps/releases/tag/2023-11-03)


## Histórico da auditoria de compatibilidade

A versão 0.6.0 usa os cabeçalhos e bibliotecas de importação do OBS 30.0.0 e o SDK Qt 6.5.3 oficial do OBS. A faixa suportada no Windows x64 é OBS 30.0.0 a 32.2.2.

## Resultados locais (20/10/2026)

| Bibliotecas oficiais do OBS | Qt fornecido | DLL e obs_module_load | Controlador e janela em host simulado |
| --- | --- | --- | --- |
| 30.0.0 | 6.5.3 | Passou | Passou |
| 31.0.0 | 6.6.3 | Passou | Passou |
| 32.2.2 | 6.11.1 | Passou | Passou |

Esses testes usam as DLLs de cada versão em processos separados. Não abrem a conta Twitch, não iniciam transmissão e não confirmam o resultado visual dentro do OBS antigo. O uso visual foi confirmado pelo usuário no OBS 32.2.2. A release não depende de nova conferência manual nas versões antigas; problemas nessas versões podem ser reportados pelo formulário de bugs. Isso não equivale a certificar visualmente todas as versões intermediárias.

## Interfaces comparadas

Foram consultados os fontes oficiais de 30.0.0, 30.2.3, 31.0.0, 31.1.2, 32.0.0, 32.1.0 e 32.2.2:

- Browser panel API permanece 3.
- `QCefWidget` mantém a mesma sequência de métodos virtuais usada pelo plugin, incluindo `executeJavaScript`.
- Os quatro nomes dos docks da Twitch permanecem iguais.
- Os eventos Exit, FinishedLoading e ThemeChanged mantêm os índices 17, 26 e 39.

O resumo com commits e links dos fontes está em `tests/compatibility-audit.json`; resultados de carregamento estão em `tests/compatibility-runtimes.json`.

## Limites

- O plugin permite OBS 30.0.0 até 32.2.2; versões anteriores e posteriores são recusadas até nova verificação.
- Exige browser panel API 3 e Qt da série 6, a partir de 6.5.3.
- A DLL é compilada com o Qt mais antigo da faixa. Compilar com Qt 6.11 não torna uma DLL compatível com os Qt antigos; a base 6.5.3 é obrigatória nesta compilação.
- Não empacote DLLs Qt junto ao plugin. Ele usa o Qt fornecido pelo OBS.
- Build Windows x64; outras arquiteturas e sistemas não foram testados.
- Estilos nativos `.ovt` são opcionais e só foram validados no OBS 32.2.2. O suporte ao formato de temas nativos nas versões antigas é separado dos modos Obsidian/Graphite aplicados às páginas web.
- Mudanças futuras no CSS da Twitch podem afetar o visual mesmo quando a DLL e o navegador são compatíveis.

## Reproduzir o carregamento

Use uma cópia extraída do ZIP oficial de cada versão do OBS, sem substituir a instalação principal:

```powershell
python tests/compatibility-load.py "CAMINHO_OBS/bin/64bit" "CAMINHO_ABSOLUTO/twitch-dock-themes.dll"
```

A checagem carrega a DLL e chama `obs_module_load`; não chama `obs_module_post_load` nem abre o OBS. Os testes CTest usam `QT_QPA_PLATFORM=minimal` e precisam das DLLs e plugins de plataforma do Qt apropriados ao runtime testado. O teste da janela usa um host simulado, sem callbacks do frontend; as mensagens sobre `obs_frontend_get_main_window` sem callbacks nesse teste são esperadas.

## Fontes

- [OBS 30.0.0](https://github.com/obsproject/obs-studio/tree/30.0.0)
- [Dependências oficiais usadas no OBS 30.0.0](https://github.com/obsproject/obs-studio/blob/30.0.0/buildspec.json)
- [Interface do browser no OBS 30.0.0](https://github.com/obsproject/obs-browser/blob/e397df52e70392ebb9146e0ab6317c0d1a30bce4/panel/browser-panel.hpp)
- [Interface do browser no OBS 32.2.2](https://github.com/obsproject/obs-browser/blob/3f0a2cdf378939ebe3c6f9ab36d4ea100c25aac2/panel/browser-panel.hpp)
