# Checklist OWASP IoT Top 10 (aplicado ao projeto)

| # | Risco OWASP IoT | Situação no projeto | Ação |
|---|---|---|---|
| I1 | Senhas fracas/padrão/embutidas | Credenciais ficam em `secrets.h` (fora do Git); usuário MQTT exclusivo | Feito |
| I2 | Serviços de rede inseguros | Broker exige usuário/senha; ESP32 não abre porta de escuta | Feito |
| I3 | Interfaces de ecossistema inseguras | Dashboard Node-RED local sem login | **Pendente/risco**: habilitar `adminAuth` e `httpNodeAuth` em produção |
| I4 | Falta de mecanismo de atualização segura | Sem OTA | Risco aceito; melhoria futura (OTA assinado) |
| I5 | Componentes inseguros/desatualizados | Bibliotecas públicas | Fixar versões e revisar |
| I6 | Privacidade insuficiente | Só estado de vaga, sem placa nem imagem | Sem dado pessoal |
| I7 | Transferência/armazenamento inseguros | TLS na porta 8883 | Feito, **limitação:** `setInsecure()` não valida o certificado; produção deve usar `setCACert` |
| I8 | Falta de gerenciamento de dispositivos | Last Will + heartbeat de 5 min detectam nó offline | Parcial |
| I9 | Configuração padrão insegura | Tópicos com identificador único do grupo (`TOPIC_ROOT`) | Feito |
| I10 | Falta de endurecimento físico | Sensores acessíveis fisicamente | Risco aceito na simulação |

## Correção implementada a partir do checklist
**Validação de entrada de comandos** (I2/I9): o firmware rejeita mensagens com mais de 15 bytes, normaliza
o texto e só executa `reservar` e `liberar`; o Node-RED também usa lista branca. Qualquer outro texto é
ignorado e gera ack `invalido`.

## Evidência
[PREENCHER: print do `.gitignore`, do repositório sem `secrets.h`, do broker com usuário/senha e da
tentativa de comando inválido sendo ignorada.]
