# Checklist OWASP IoT Top 10 (aplicado ao projeto)

Situação descrita conforme o que foi executado nos testes (broker público, sem TLS/login).
TLS e usuário/senha estão implementados no firmware (`MQTT_USE_TLS`, `MQTT_USER`) e no flow,
mas não foram o cenário dos testes registrados.

| # | Risco OWASP IoT | Situação no projeto | Ação |
|---|---|---|---|
| I1 | Senhas fracas/padrão/embutidas | Credenciais em `secrets.h` (fora do Git). Broker público de teste sem usuário/senha | **Parcial**. Produção: usuário MQTT exclusivo |
| I2 | Serviços de rede inseguros | ESP32 não abre porta de escuta. Broker público sem autenticação: quem souber o tópico pode publicar | **Risco aceito na simulação**; mitigado por validação de comandos. Produção: broker com autenticação |
| I3 | Interfaces de ecossistema inseguras | Dashboard Node-RED local sem login | **Risco**: habilitar `adminAuth` e `httpNodeAuth` em produção |
| I4 | Falta de atualização segura | Sem OTA | Risco aceito; melhoria futura (OTA assinado) |
| I5 | Componentes inseguros/desatualizados | Bibliotecas públicas | Fixar versões e revisar |
| I6 | Privacidade insuficiente | Só estado de vaga, sem placa nem imagem | Sem dado pessoal |
| I7 | Transferência/armazenamento inseguros | Testes sem TLS (porta 1883). Com `MQTT_USE_TLS 1` o canal é cifrado, mas `setInsecure()` não valida o certificado | **Risco**. Produção: TLS 8883 + `setCACert` |
| I8 | Falta de gerenciamento de dispositivos | Last Will + heartbeat de 5 min detectam nó offline | Parcial |
| I9 | Configuração padrão insegura | Tópicos com identificador único (`TOPIC_ROOT`) | Feito |
| I10 | Falta de endurecimento físico | Sensores acessíveis fisicamente | Risco aceito na simulação |

## Correção implementada a partir do checklist
**Validação de entrada de comandos** (I2/I9): o firmware rejeita mensagens com mais de 15 bytes,
normaliza o texto e só executa `reservar` e `liberar`; o Node-RED também usa lista branca.
Qualquer outro texto é ignorado e gera ack `invalido`. Demonstrada no teste T8.

## Evidência
- Comando `abrir` ignorado e ack `invalido`: ver relatório (Figura 3) e `prints/`.
- [PREENCHER: print do `.gitignore` e do repositório sem `secrets.h`]
