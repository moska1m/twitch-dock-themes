# Biblioteca local de presets — compilação de teste

Base estável: 0.4.0. A biblioteca Meus presets permite guardar combinações pelo nome dentro do plugin.

- Salvar como guarda as escolhas atuais dos controles, incluindo aparência por painel, densidade, cores, movimento e ativação.
- Carregar preenche os controles. Prévia testa nos painéis; Aplicar e salvar confirma a configuração ativa.
- Nomes existentes pedem confirmação antes da substituição. Excluir remove o preset da biblioteca após confirmação e mantém a configuração ativa.
- A biblioteca fica em `%APPDATA%\obs-studio\plugin_config\twitch-dock-themes\presets.json`. Cada gravação é atômica. Nomes têm limite de 64 caracteres, com até 100 presets.
- Um arquivo ilegível ou inválido é preservado e bloqueia gravações na biblioteca; importar/exportar arquivos individuais continua disponível.
- A janela tem rolagem e mantém Aplicar e Fechar fora da área rolável.

## Roteiro de teste

1. Escolha aparência e densidade e use Salvar como com um nome.
2. Feche e reabra a janela. Confira o preset na lista.
3. Altere os controles, carregue o preset e confira a recuperação das escolhas.
4. Use Prévia, feche sem aplicar e confira a restauração da última configuração salva.
5. Confira substituição de nome e exclusão. Para compartilhar um preset carregado, use Exportar preset.

Testes locais cobrem reabertura da biblioteca, escolhas por painel, substituição explícita, exclusão, nomes inválidos, conteúdo inválido e preservação de arquivo corrompido. A interface precisa de conferência manual no OBS.
