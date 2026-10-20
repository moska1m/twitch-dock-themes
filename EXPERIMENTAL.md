# Teste: aparência e densidade por painel

Base: versão 0.3.0. Esta compilação acrescenta escolhas independentes para Chat, Informações, Atividades e Estatísticas.

1. Abra Ferramentas → Twitch Dock Themes.
2. Na seção Painéis, escolha Aparência e Densidade em cada linha.
3. Usar geral acompanha a configuração geral daquela propriedade. Você pode sobrescrever somente a densidade ou somente a aparência.
4. Use Prévia nos painéis; Fechar desfaz a prévia e Aplicar e salvar confirma.
5. Sincronizar todos com a configuração geral limpa as escolhas individuais nos controles. Use Prévia ou Aplicar para confirmar.

Cores personalizadas usa o conjunto de cores escolhido acima, inclusive quando apenas um painel usa esse modo. Ainda não há um conjunto de três cores diferente para cada painel.

Presets antigos da 0.3.0 continuam sendo importados. Novos presets incluem as escolhas por painel e usam o formato 2; a 0.3.0 estável não importa esse formato novo. Configurações existentes são preservadas, com todos os painéis inicialmente seguindo o geral.

Testes automatizados verificam isolamento entre os painéis, herança, persistência, presets antigos e rejeição de combinações inválidas. Falta validar visualmente essas novas combinações no OBS.

Para voltar à DLL estável, feche o OBS e execute Restore-Previous.ps1. O GitHub mantém a versão 0.3.0 estável.
