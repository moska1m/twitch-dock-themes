# Biblioteca de presets e alterações automáticas — compilação de teste

Base estável: 0.4.0. A seção Meus presets guarda combinações pelo nome.

- Alterar aparência, densidade, cores, movimento ou seleção dos painéis atualiza o OBS automaticamente.
- Selecionar um preset na lista ou importar um JSON preenche os controles e exibe a combinação imediatamente.
- Aplicar e salvar confirma a configuração ativa. Fechar sem confirmar restaura a última configuração salva, inclusive se você carregou um preset.
- Salvar como guarda os controles na biblioteca, sem confirmar a configuração ativa. Nomes repetidos pedem confirmação antes de substituir. Excluir remove somente o preset escolhido.
- A biblioteca fica em `%APPDATA%\obs-studio\plugin_config\twitch-dock-themes\presets.json`, com gravação atômica e preservação de arquivos inválidos.
- A janela tem rolagem, mantendo Aplicar e Fechar sempre visíveis.

## Conferir no OBS

1. Mude uma aparência ou densidade e confira a alteração automática.
2. Salve uma combinação em Meus presets. Selecione outra e depois volte à primeira.
3. Feche sem Aplicar e salvar: confira a restauração da última configuração confirmada.
4. Reabra, altere uma opção, aplique e salve. Feche e reabra para conferir a persistência.

Compilação, testes de controlador/biblioteca/CSS e carregamento da DLL passaram. A interação automática na janela precisa da conferência manual no OBS.
