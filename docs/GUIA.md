# Guia rápido

![Janela de configurações em teste](images/settings-compact.png)

Interface em teste, capturada da janela real do plugin em um host Qt simulado no Windows. A aparência pode variar conforme o tema do OBS.


## Instalar

1. Use OBS Studio **32.2.2**, Windows x64.
2. Baixe o arquivo **Windows.zip** na [release mais recente](https://github.com/moska1m/twitch-dock-themes/releases/latest). O Source.zip é para compilar.
3. Extraia o ZIP, feche o OBS e execute `Install.ps1` pelo PowerShell. Se houver erro de permissão em ProgramData, use um PowerShell como administrador.
4. Abra o OBS e acesse **Ferramentas → Twitch Dock Themes**.

## Ajustar o visual

Escolha a aparência geral. Os painéis abertos mudam automaticamente. Cada linha permite escolher outra aparência para aquele painel; Usar geral acompanha a seleção principal.

Densidade funciona no Chat e nas Atividades. Compacta reduz espaços e, no feed, também o texto. Informações e Estatísticas mantêm o tamanho original.

Clique em **Salvar** para confirmar. Se fechar com mudanças pendentes, escolha continuar editando ou sair sem salvar. Sair sem salvar restaura a última configuração confirmada.

## Guardar combinações

Configure as opções e clique em **Salvar como** na seção Meus presets. Dê um nome. Selecionar esse nome na lista recupera as escolhas e já atualiza os painéis. Clique em Salvar para confirmar o visual ativo.

Salvar como guarda uma combinação na biblioteca. Salvar confirma o que o OBS deve usar. Excluir um preset não apaga a configuração ativa dos painéis.

Para compartilhar uma combinação, abra Opções avançadas → Exportar preset. Para receber uma, use Importar preset e depois Salvar se quiser mantê-la ativa.

## Resolver problemas

- Painel não encontrado: abra-o pelo menu Painéis do OBS e confira o login normal da Twitch.
- Visual inesperado: abra Opções avançadas → Diagnóstico dos painéis e confira a aparência efetiva. O relatório indica envio do CSS, sem confirmar seu resultado na página.
- Voltar ao original: use Opções avançadas → Desativar e restaurar Twitch.
- Voltar à DLL anterior: feche o OBS e execute Restore-Previous.ps1, quando existir uma cópia anterior.

O plugin acompanha a aparência dos painéis integrados da Twitch. A instalação não exige fornecer credenciais ao plugin.
