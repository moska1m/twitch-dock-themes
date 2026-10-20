# Teste: aparência e densidade por painel

Base: versão 0.3.0. Esta compilação acrescenta aparência independente para os quatro painéis e densidade somente para Chat e Atividades.

1. Abra Ferramentas → Twitch Dock Themes.
2. Na seção Painéis, escolha Aparência em cada linha. Densidade está disponível somente para Chat e Atividades.
3. Usar geral acompanha a configuração geral daquela propriedade. Você pode sobrescrever somente a densidade ou somente a aparência.
4. Use Prévia nos painéis; Fechar desfaz a prévia e Aplicar e salvar confirma.
5. Sincronizar todos com a configuração geral limpa as escolhas individuais nos controles. Use Prévia ou Aplicar para confirmar.

Cores personalizadas usa o conjunto de cores escolhido acima, inclusive quando apenas um painel usa esse modo. Ainda não há um conjunto de três cores diferente para cada painel.

Presets antigos da 0.3.0 continuam sendo importados. Novos presets incluem as escolhas por painel e usam o formato 2; a 0.3.0 estável não importa esse formato novo. Configurações existentes são preservadas, com todos os painéis inicialmente seguindo o geral.

Testes automatizados verificam isolamento entre os painéis, herança, persistência, presets antigos e rejeição de combinações inválidas. Falta validar visualmente essas novas combinações no OBS.

Para voltar à DLL anterior, feche o OBS e execute Restore-Previous.ps1. O GitHub mantém a versão 0.3.0 estável.

## Diagnóstico dos painéis

O botão Diagnóstico dos painéis abre um relatório da configuração ativa, incluindo a prévia, com cores e densidade efetivas, presença dos navegadores e horário UTC da última verificação. Atualizar verifica novamente os painéis e reenvia a configuração ativa. Copiar relatório envia o texto para a área de transferência somente ao clicar.

O relatório não lê mensagens, URLs, contas ou credenciais. Indica pedidos de aplicação/remoção enviados ao navegador, sem afirmar que o CSS foi aceito ou que o resultado visual está correto. A interface de browser utilizada não oferece retorno de execução desse JavaScript.

Testes adicionais cobrem relatório de envio, desativação, painel removido, herança de aparência/densidade e ausência de confirmação visual indevida.

## Correções: reabrir preferências e densidade de atividades

- A consulta a um painel que usava o geral inseria entradas nulas no objeto de preferências. Elas faziam a reabertura rejeitar o estado. As consultas agora não alteram o objeto, e entradas nulas antigas são descartadas preservando escolhas válidas.
- Densidade removida dos controles de Informações e Estatísticas; presets antigos com essas escolhas são normalizados para o padrão nesses painéis.
- Atividades usa regras específicas para `activity-list-layout` e `activity-base-list-item`, verificadas no JavaScript público da Twitch em 20/10/2026. Compacta usa fonte de 12 px e espaçamento reduzido; Confortável usa 14 px e espaçamento maior; Padrão remove as regras de densidade.
- Não são impostos limites de altura nem ocultados botões, mensagens ou detalhes das atividades. A conferência visual deve incluir o feed estreito e atividades com texto longo.
- Testes de regressão: aplicar não modifica preferências; reabertura mantém escolhas; entradas nulas são recuperadas; densidade não afeta Informações/Estatísticas; troca para Padrão remove ajustes.
