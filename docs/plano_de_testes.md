# Plano de testes e resultados (execução no Wokwi)

Comandos remotos publicados pelo cliente web HiveMQ em `estac_fag_t04n/andar1/vaga01/cmd`.

| # | Teste | Como fazer | Esperado | Resultado obtido |
|---|---|---|---|---|
| T1 | Detecção de ocupação | Vaga 1 de 100 para 14 cm | LED vermelho, status `ocupada` | **Passou.** LED vermelho, LCD `Livres: 2`; `sensor -> ocupada (17.2 cm)` e status publicado (uptime 19 s) |
| T2 | Liberação | Voltar para 102 cm | LED verde, status `livre` | **Passou.** `sensor -> livre (103.0 cm)`, status publicado (uptime 30 s), `Livres: 3` |
| T3 | Filtro de falso positivo | Baixar a distância e voltar rapidamente | Nenhuma mudança se o valor baixo durar menos que o tempo de confirmação | **Inconclusivo.** Valor baixo durou o bastante para confirmar (ocupada em 50 s, livre em 52 s). Repetir com movimento menor que 1 s: [PREENCHER] |
| T4 | Sem internet (R2) | SSID alterado para `semrede` e distâncias mudadas | LEDs e LCD `(loc)` continuam | **Passou.** `Wi-Fi: tentando conectar...`; vaga 1 ocupou (14 cm) e liberou (106 cm), LED e LCD atualizaram |
| T5 | Reconexão | Iniciar com a rede `Wokwi-GUEST` disponível | Estado atual publicado | **Passou.** `MQTT conectado` e republicação das 3 vagas (uptime 12 s) |
| T6 | Reserva remota | Publicar `reservar` | LED azul, status `reservada` | **Passou.** LED azul, `1:R`, modo `manual`, ack `ok` |
| T7 | Liberar reserva | Publicar `liberar` | Volta ao modo automático | **Passou.** LED verde, modo `auto`, ack `ok` |
| T8 | Comando inválido | Publicar `abrir` | Ignorado, ack `invalido` | **Passou.** `Comando invalido ignorado: 'abrir'`, ack `invalido` |
| T9 | Contador do painel | Ocupar 2 vagas | LCD mostra 1 livre | **Passou no LCD:** `Livres: 1`, `1:O 2:O 3:L`. Dashboard: [CONFIRMAR] |
| T10 | Credenciais | Conferir o repositório | `secrets.h` não aparece | [PREENCHER] |

Observação: nos prints o LCD mostra `(loc)` mesmo com o MQTT conectado, porque o valor do
painel (`ROOT/painel/livres`) só chega se o Node-RED estiver rodando no mesmo broker.

## Falha encontrada e correção (obrigatório no relatório)
[PREENCHER: uma falha REAL observada pelo grupo, com causa e correção]
