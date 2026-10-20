# Estilo dos painéis — teste

Em Ferramentas → Twitch Dock Themes → Estilo dos painéis:

- Ícones: Original, Traço fino ou Traço arredondado. Atua nos controles reconhecidos de configurações, emotes e lista de usuários do chat. Badges e emotes permanecem intactos.
- Foco: recolhe o cabeçalho do chat; passar o mouse ou focar com teclado revela os controles. A escrita e o envio ficam disponíveis.
- Feed: Original, Linhas compactas ou Cartões, preservando o conteúdo e as ações da Twitch.
- Borda: ative os alertas, escolha os tipos e a duração. Testar borda mostra o efeito no chat. Desligado por padrão.

As opções atualizam automaticamente e são confirmadas em Salvar. Fechar sem salvar restaura a configuração anterior.

## Alertas automáticos

A detecção observa metadados de novos avisos visíveis no chat, como data-msg-id para raids/inscrições e marcadores de menção ao próprio usuário. Não lê textos nem cria conexões. Dependendo do componente da Twitch, esses marcadores podem não estar presentes; o reconhecimento é experimental e não representa todos os eventos da conta. Não transfere eventos do feed para o chat e não adiciona overlays à transmissão.

As mensagens e a ordem do chat não são alteradas. O brilho fica na borda interna da dock e respeita movimento reduzido. O botão de teste permite conferir o efeito sem um evento real. Eventos presentes antes de ativar são ignorados; novos históricos renderizados posteriormente podem depender dos identificadores fornecidos pela página.

## Presets

Os novos presets usam formato 3, incluindo os efeitos. Presets antigos são importados com os efeitos desativados. A release 0.6.0 não lê o formato 3. Faça cópia de presets.json antes de voltar para ela se tiver salvo novos presets nesta compilação.

## Conferir

1. Troque ícones e foco e confira os controles com mouse e teclado.
2. Compare Original e Cartões no feed estreito, incluindo uma atividade com detalhes/ações.
3. Ative Borda e clique em Testar borda com o chat aberto. Confira duração e desligamento.
4. Salve uma combinação, feche e reabra. Depois mude algo e saia sem salvar para conferir a restauração.
5. Confira detecção de um evento novo real, quando surgir. Ausência de disparo pode indicar marcador não reconhecido.

Testes automatizados: controlador e janela com bibliotecas dos OBS 30, 31 e 32.2.2; navegador com fixtures para ícones, foco, cartões, filtros, expiração, descarte e limpeza. A aparência e os marcadores reais da Twitch precisam de conferência no OBS.
