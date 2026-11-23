# Tópicos e payloads (R4)

Raiz dos tópicos: `TOPIC_ROOT` (ex.: `estac_fag_t04n`).

| Tópico | Sentido | Payload | QoS / retain |
|---|---|---|---|
| `ROOT/andar1/vagaNN/status` | sensor → servidor | JSON abaixo | 0 / retained |
| `ROOT/andar1/vagaNN/cmd` | servidor → sensor | texto: `reservar` ou `liberar` | 1 / não |
| `ROOT/andar1/vagaNN/ack` | sensor → servidor | JSON de confirmação | 0 / não |
| `ROOT/painel/livres` | servidor → sensor (LCD) | texto: número de vagas livres (0 a 999) | 1 / retained |
| `ROOT/andar1/disponibilidade` | sensor → servidor | `online` / `offline` (Last Will) | 1 / retained |

## Exemplo de status (mensagem real, enviada quando a vaga muda de estado e a cada 5 min)
```json
{"device":"esp32-andar1","vaga":"vaga01","status":"ocupada","dist_cm":12.4,"modo":"auto","uptime_s":184}
```
| Campo | Tipo | Significado |
|---|---|---|
| device | string | identificador do nó |
| vaga | string | `vaga01` a `vaga03` |
| status | string | `livre`, `ocupada` ou `reservada` |
| dist_cm | número | última distância medida (0 = sem eco) |
| modo | string | `auto` (sensor) ou `manual` (reservada) |
| uptime_s | inteiro | segundos desde o boot |

## Exemplo de confirmação de comando
```json
{"vaga":"vaga01","cmd":"reservar","resultado":"ok"}
```
Comando fora da lista retorna `"resultado":"invalido"` e é ignorado.

## Versão de produção (LoRaWAN)
Em LoRaWAN o uplink seria binário (1 byte: 0 = livre, 1 = ocupada, 2 = reservada, + identificador da
vaga), decodificado no servidor. O JSON acima é a representação usada na simulação.
