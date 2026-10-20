# Twitch Dock Themes — 0.2.0

Aparência integrada para os quatro painéis da Twitch no OBS Studio, com cores do OBS ou dos estilos Obsidian e Obsidian Graphite.

**Windows x64 · OBS Studio 32.2.2 · Qt 6.11.1 · browser panel API 3.** Esta compilação recusa outras versões do OBS. Usa a interface C++ do obs-browser e precisa ser recompilada e verificada quando essa interface mudar.

## Instalar e usar

1. Feche o OBS, extraia o pacote e execute `Install.ps1` no PowerShell.
2. Reabra o OBS e acesse **Ferramentas → Twitch Dock Themes**.
3. Escolha **Seguir a paleta do OBS**, **Obsidian** ou **Obsidian Graphite**.
4. Marque os painéis desejados: Chat, Informações da transmissão, Atividades e Estatísticas.
5. Escolha se deseja transição suave ao passar o mouse e clique em **Aplicar agora**.

O plugin verifica os painéis a cada 2,5 segundos e reaplica o estilo após recarregamentos. O indicador **Encontrado** confirma a presença do navegador do painel; não confirma que a página aceitou todos os seletores CSS. Abra os painéis pelo menu Painéis do OBS e faça login pela interface normal da Twitch, se necessário.

**Desativar e restaurar Twitch** remove a folha de estilo e salva a desativação. Desmarcar um painel e aplicar restaura somente aquele painel. Fechar a janela sem aplicar descarta as alterações pendentes.

Instalações novas começam em modo automático com os quatro painéis habilitados. Na atualização da versão 0.1, o modo escolhido e a preferência de painéis secundários são preservados. A migração é gravada ao aplicar.

## Aparências

| Modo | Resultado |
| --- | --- |
| Seguir a paleta do OBS | Usa as cores da paleta Qt e acompanha mudanças de tema do OBS. |
| Obsidian | Fundos quase pretos, superfícies grafite, destaque ciano. |
| Obsidian Graphite | Superfícies cinza grafite com destaque ciano. |

O modo automático pode acompanhar Default, Acri, Classic, Grey, Light, Rachni e estilos personalizados. É uma aproximação da paleta; regras QSS específicas, imagens e efeitos do tema nativo não são convertidos. Contraste dos botões e links recebe tratamento próprio.

Inclui os estilos nativos `Obsidian.ovt` e `Obsidian Graphite.ovt`. O instalador adiciona os que ainda não existem. Escolha-os nas configurações de Aparência do OBS. Fonte e densidade nativas continuam sob controle do OBS; o conteúdo web da Twitch mantém seu próprio dimensionamento. O plugin não impõe uma fonte nem uma altura fixa aos painéis.

## Escopo e funcionamento

O plugin localiza `twitchChat`, `twitchInfo`, `twitchFeed` e `twitchStats` e executa JavaScript para manter uma única folha CSS nos seus navegadores. Docks personalizados e fontes de navegador das cenas ficam fora desse escopo.

A folha altera variáveis de fundo, texto, borda, botões, campos e arredondamento, além de seletores estruturais do chat. Ela só é aplicada em páginas HTTPS com caminho `/popout/` em `twitch.tv`, `www.twitch.tv` e `dashboard.twitch.tv`. Mudanças de domínio/caminho removem a folha na próxima verificação.

Não há leitura de cookies, tokens ou mensagens, nem chamadas de rede ou armazenamento pelo JavaScript. O plugin não substitui os scripts de inicialização de BTTV/FFZ e não recarrega ou navega os painéis. Cores individuais de nomes, badges e emotes não são sobrescritas por uma regra genérica. Estados de erro e sucesso não são alvo da paleta; a renderização final depende do CSS da Twitch.

As transições de fundo duram 110 ms e respeitam `prefers-reduced-motion`. Não há animação contínua.

## Atualização, restauração e remoção

O instalador confere o SHA-256 da DLL usando `SHA256SUMS.txt`, verifica a cópia e guarda uma DLL anterior em `.previous` quando a versão instalada difere. O checksum verifica integridade, não é uma assinatura digital.

Se necessário, feche o OBS e execute `Restore-Previous.ps1`. Esse script restaura a DLL anterior e mantém as preferências atuais. Para apenas voltar ao visual original, use o botão de desativar dentro do plugin.

Se o Windows negar escrita em ProgramData, execute o instalador em PowerShell como administrador.

- Plugin: `%PROGRAMDATA%\obs-studio\plugins\twitch-dock-themes`
- Preferências: `%APPDATA%\obs-studio\plugin_config\twitch-dock-themes\settings.ini`
- Estilos nativos: `%APPDATA%\obs-studio\themes`

Para desinstalar, feche o OBS e remova somente a pasta `twitch-dock-themes` no destino de plugins acima. Preferências e estilos nativos podem ser mantidos.

## Validação e limitações

- A versão 0.1 foi testada manualmente pelo usuário no OBS, inclusive com os painéis secundários.
- A versão 0.2 compilou em Release x64 e carregou em processo de teste com as dependências do OBS instalado.
- O teste Qt cobre os quatro painéis, seleção independente, exclusão de outros docks, ativação/restauração, recriação de painéis, paletas, contraste, persistência e migração das preferências. Inclui dez paletas de referência.
- O teste JavaScript cobre domínio/caminho, aplicação idempotente, remoção, navegação, validação de cores e opção de movimento.
- A inspeção visual das novidades da versão 0.2 na Twitch autenticada deve ser feita no OBS. Os testes automatizados usam navegadores simulados; não substituem essa conferência.

A Twitch pode mudar suas variáveis e seletores. Nem todos os componentes do dashboard necessariamente usam as variáveis sobrescritas. Compatibilidade visual completa com FFZ/BTTV, todas as larguras e todos os estilos ainda não está certificada.

## Compilar e testar

Dependências: Visual Studio 2022 com C++, SDK Windows, CMake 3.28+, Qt **6.11.1** x64 MSVC, código-fonte do OBS na tag **32.2.2**, Node.js para os testes JavaScript e Python para o helper de importação.

Use o terminal **x64 Native Tools Command Prompt for VS 2022**. Se não tiver as bibliotecas de importação do OBS, gere-as a partir das DLLs da instalação 32.2.2:

```powershell
python scripts/generate-imports.py --obs-bin "C:/Program Files/obs-studio/bin/64bit" --output build/imports
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
- [Dependências Qt oficiais usadas](https://github.com/obsproject/obs-deps/releases/tag/2026-07-15)

Licença: GPL-2.0-or-later. Consulte `COPYING` e `THIRD-PARTY.md`. Projeto independente, sem afiliação à Twitch ou OBS Project.

