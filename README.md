# Twitch Dock Themes — 0.5.0

Aparência integrada para os quatro painéis da Twitch no OBS Studio, com cores do OBS ou dos estilos Obsidian e Obsidian Graphite.

**Compilação de compatibilidade em teste: Windows x64 · OBS 30.0.0 a 32.2.2 · Qt 6.5.3 ou superior da série 6 · browser panel API 3.** Compilada com OBS 30.0.0 / Qt 6.5.3. DLL e testes de interface verificados com bibliotecas dos OBS 30.0.0, 31.0.0 e 32.2.2. A Twitch autenticada nas versões antigas ainda precisa de validação visual. [Matriz e limites](docs/COMPATIBILIDADE.md).

## Instalar e usar

1. Feche o OBS, extraia o pacote e execute `Install.ps1` no PowerShell.
2. Reabra o OBS e acesse **Ferramentas → Twitch Dock Themes**.
3. Escolha **Seguir a paleta do OBS**, **Obsidian**, **Obsidian Graphite** ou **Cores personalizadas**.
4. Marque os painéis desejados: Chat, Informações da transmissão, Atividades e Estatísticas.
5. Altere aparência, densidade e transições: os painéis atualizam automaticamente. Clique em **Salvar** para confirmar.

[Guia rápido de instalação e uso](docs/GUIA.md).

O plugin verifica os painéis a cada 2,5 segundos e reaplica o estilo após recarregamentos. O indicador **Encontrado** confirma a presença do navegador do painel; não confirma que a página aceitou todos os seletores CSS. Abra os painéis pelo menu Painéis do OBS e faça login pela interface normal da Twitch, se necessário.

**Desativar e restaurar Twitch** remove a folha de estilo e salva a desativação. Desmarcar um painel restaura imediatamente aquele painel. Fechar com alterações pendentes pede confirmação; sair sem salvar restaura a última configuração salva.

Instalações novas começam em modo automático com os quatro painéis habilitados. Na atualização da versão 0.1, o modo escolhido e a preferência de painéis secundários são preservados. A migração é gravada ao salvar.

## Aparências

| Modo | Resultado |
| --- | --- |
| Seguir a paleta do OBS | Usa as cores da paleta Qt e acompanha mudanças de tema do OBS. |
| Obsidian | Fundos quase pretos, superfícies grafite, destaque ciano. |
| Obsidian Graphite | Superfícies cinza grafite com destaque ciano. |
| Cores personalizadas | Fundo, destaque e bordas escolhidos por você, com texto derivado para contraste. |

O modo automático pode acompanhar Default, Acri, Classic, Grey, Light, Rachni e estilos personalizados. É uma aproximação da paleta; regras QSS específicas, imagens e efeitos do tema nativo não são convertidos. Contraste dos botões e links recebe tratamento próprio.

Os estilos nativos incluídos foram validados no OBS 32.2.2; sua compatibilidade nas versões antigas é separada da aparência dos painéis web. Inclui os estilos nativos `Obsidian.ovt` e `Obsidian Graphite.ovt`. O instalador adiciona os que ainda não existem. Escolha-os nas configurações de Aparência do OBS. Fonte e densidade nativas continuam sob controle do OBS; o conteúdo web da Twitch mantém seu próprio dimensionamento. O plugin não troca a família da fonte nem impõe altura fixa aos painéis. A densidade de Atividades pode alterar o tamanho do texto.

## Densidade, alterações automáticas e presets

- Densidade disponível somente para **Chat** e **Atividades**: Compacta, Padrão da Twitch e Confortável. No chat, ajusta espaços. Nas atividades, ajusta espaços e texto (12 px no compacto, 14 px no confortável). Informações e Estatísticas mantêm o dimensionamento original.
- As alterações aparecem automaticamente. **Salvar** confirma as escolhas; fechar com alterações pendentes oferece **Sair sem salvar** ou **Continuar editando**. O X, Alt+F4 e Escape usam a mesma verificação.
- **Exportar preset** salva as escolhas em JSON sem alterar os painéis. **Importar preset** preenche os controles e exibe a combinação imediatamente; **Salvar** confirma.
- Presets incluem aparência, cores, densidade, movimento, seleção dos painéis e escolhas individuais. Arquivos inválidos ou maiores que 64 KB são rejeitados.
- O campo do chat mantém o contorno do contêiner, sem uma segunda borda ou fundo sobre o editor interno.

## Meus presets

Use **Salvar como** para guardar as escolhas dos controles pelo nome. Selecionar um preset na lista carrega a combinação e atualiza os painéis automaticamente. **Salvar** confirma a configuração ativa; guardar um preset na biblioteca é uma operação separada.

Nomes existentes pedem confirmação para substituir. **Excluir** remove somente o preset escolhido, após confirmação. A biblioteca aceita até 100 presets, com nomes de até 64 caracteres. O arquivo `presets.json` fica junto às preferências e é salvo de forma atômica. Arquivos inválidos são preservados.

## Escolhas por painel e diagnóstico

Cada painel pode usar sua própria aparência ou **Usar geral**. Chat e Atividades também têm densidade independente. **Sincronizar todos com a configuração geral** limpa as escolhas individuais e atualiza os painéis; use **Salvar** para confirmar. Cores personalizadas usa o conjunto de cores escolhido na parte superior da janela.

O botão **Diagnóstico dos painéis** mostra a configuração ativa, presença do navegador, cores e densidade efetivas e horário UTC da última verificação. Atualizar reenvia a configuração; Copiar relatório copia o texto ao clicar. O relatório confirma pedidos enviados ao navegador, sem confirmar aceitação do CSS ou resultado visual. Não inclui mensagens, URLs ou contas.

Presets da versão 0.3 continuam sendo importados. Os novos usam formato 2, com escolhas por painel; a versão 0.3 não importa esse novo formato. Preferências afetadas pelas entradas nulas do teste anterior são recuperadas preservando escolhas válidas. Densidades antigas de Informações e Estatísticas são ignoradas.

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
- Biblioteca: `%APPDATA%\obs-studio\plugin_config\twitch-dock-themes\presets.json`
- Estilos nativos: `%APPDATA%\obs-studio\themes`

Para desinstalar, feche o OBS e remova somente a pasta `twitch-dock-themes` no destino de plugins acima. Preferências e estilos nativos podem ser mantidos.

## Validação e limitações

- A versão 0.1 foi testada manualmente pelo usuário no OBS, inclusive com os painéis secundários.
- A versão 0.5 compilou em Release x64 e carregou em processo de teste com as dependências do OBS instalado.
- O teste Qt cobre os quatro painéis, seleção independente, exclusão de outros docks, ativação/restauração, recriação de painéis, paletas, contraste, persistência e migração das preferências. Inclui dez paletas de referência.
- O teste JavaScript cobre domínio/caminho, aplicação idempotente, remoção, navegação, validação de cores e opção de movimento.
- As novidades da 0.3 e a correção do campo do chat foram aprovadas em teste manual pelo usuário no OBS. Os testes automatizados usam navegadores simulados; não substituem a conferência visual em outras instalações.
- Testes adicionais cobrem presets inválidos e antigos, importação/exportação, restauração da prévia, escolhas por painel, reabertura sem reset, recuperação de entradas nulas e diagnóstico.
- Aparência por painel e correções do feed/reset foram confirmadas pelo usuário no OBS. Essa aprovação corresponde ao uso local; não certifica todas as larguras, tipos de atividade ou extensões.

- A biblioteca de presets, as alterações automáticas e o aviso ao fechar foram aprovados pelo usuário no OBS. Testes Qt também verificam persistência, substituição, exclusão, arquivos inválidos e bloqueio/aceitação do fechamento.

A Twitch pode mudar suas variáveis e seletores. Nem todos os componentes do dashboard necessariamente usam as variáveis sobrescritas. Compatibilidade visual completa com FFZ/BTTV, todas as larguras e todos os estilos ainda não está certificada.

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
- [Dependências Qt oficiais usadas](https://github.com/obsproject/obs-deps/releases/tag/2026-07-15)

Licença: GPL-2.0-or-later. Consulte `COPYING` e `THIRD-PARTY.md`. Projeto independente, sem afiliação à Twitch ou OBS Project.

