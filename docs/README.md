# Documentação interna — AUGUR

Tudo que o grupo precisa pra navegar o projeto além do `README.md` principal. Cada arquivo aqui é um guia ou referência fechada, lido sob demanda — não é leitura obrigatória do começo ao fim.

## Guias

| Arquivo | Pra que serve |
|---|---|
| [TUTORIAL_AMBIENTE.md](TUTORIAL_AMBIENTE.md) | Setup completo do ambiente (MSYS2/UCRT64 no Windows, Linux), instalação do Raylib e make, troubleshooting de compilação. Caminho pra quem está clonando o repo pela primeira vez. |
| [TUTORIAL_SPRITES.md](TUTORIAL_SPRITES.md) | Como trocar as primitivas (`DrawCircleV`) por sprites PNG. Onde colocar os arquivos, formato recomendado, criação do módulo `assets`, integração com as funções `_desenhar`, fallback pra sprite inexistente. |

## Referências

| Arquivo | O que contém |
|---|---|
| [dicionario.md](dicionario.md) | Glossário de termos de jogos (bullet hell, roguelite, AoE, DoT, kiting…) e termos da arquitetura usada no AUGUR (motor de profecia, riders de status, push-out, etc.). Consulte ao bater num jargão que não conhece. |
| [issue-hud-cronograma.md](issue-hud-cronograma.md) | Task técnica original da Sofia sobre a HUD ligada ao cronograma. Histórico — útil pra entender como o HUD foi pensado, não é guia ativo. |

## Como adicionar um novo doc aqui

1. Escreva o `.md` neste diretório com nome em maiúsculas/ASCII (ex.: `TUTORIAL_AUDIO.md`).
2. Adicione uma linha na tabela acima com link e descrição de 1 frase.
3. Se for um tutorial central (não só referência), linkar também do `README.md` da raiz na seção "Documentação".
