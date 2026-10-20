# Interface compacta — compilação de teste

Base estável: 0.5.0.

- Meus presets no topo, com seleção que atualiza os painéis automaticamente.
- Aparência geral e densidade agrupadas.
- Cores personalizadas só aparecem quando a aparência geral ou algum painel usa esse modo.
- Opções avançadas recolhidas ao abrir, com transição, importação/exportação, diagnóstico e restauração da Twitch.
- Salvar e Fechar ficam sempre visíveis. Abrir/recolher opções não altera preferências.

Teste trocar uma aparência por painel, usar cores personalizadas, abrir Opções avançadas, salvar e fechar com mudanças pendentes. O teste automatizado da janela cobre atualização automática, salvar, cancelar/confirmar descarte e reabrir com o estado salvo. O teste usa host Qt simulado; o visual final precisa ser conferido no OBS.

A captura em docs/images/settings-compact.png é da própria janela do plugin no Windows em host simulado, sem dados de conta ou conteúdo da Twitch.
