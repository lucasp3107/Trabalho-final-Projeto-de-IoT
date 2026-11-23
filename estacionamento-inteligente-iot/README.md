# Estacionamento Inteligente de Shopping (IoT)

Redes IoT - FAG T04N. Integrantes: Lucas Mendonça, Felipe Jagas, Enzo Zanella e Leonardo Gabriel.

Sistema que detecta se cada vaga está livre, ocupada ou reservada, decide localmente no ESP32,
envia os eventos por Wi-Fi + MQTT e mostra o resultado em LEDs RGB, em um LCD na entrada e em um
dashboard Node-RED. Na versão de produção, o enlace seria LoRaWAN (ver relatório).

## Estrutura

```
firmware/    sketch.ino, diagram.json, libraries.txt, secrets.example.h
node-red/    flows.json (consolidação por andar + dashboard + comandos)
docs/        payload, checklist OWASP, plano de testes, roteiros do vídeo e do pitch
```

## Como executar (qualquer máquina)

### 1) Broker MQTT
**Opção A, usada nos testes do relatório (rápida, sem segurança):** broker público
`broker.hivemq.com`, porta `1883`, sem usuário/senha e sem TLS (`MQTT_USE_TLS 0`).

**Opção B, recomendada para segurança (TLS + usuário/senha):**
1. Crie uma conta gratuita no **HiveMQ Cloud** e um cluster.
2. Em *Access Management*, crie um usuário e senha **exclusivos do projeto**.
3. Anote o host (`xxxx.s1.eu.hivemq.cloud`) e use a porta TLS `8883` com `MQTT_USE_TLS 1`.

O firmware e o Node-RED devem usar **o mesmo broker**; caso contrário o LCD mostra o valor
local `(loc)` e o dashboard não recebe os dados.

### 2) Firmware no Wokwi
1. Em wokwi.com, crie um projeto **ESP32** e substitua/crie os arquivos com o conteúdo de `firmware/`:
   `sketch.ino`, `diagram.json`, `libraries.txt`.
2. Crie o arquivo `secrets.h` copiando `secrets.example.h` e preencha host, porta, usuário, senha,
   `MQTT_USE_TLS` (conforme a opção de broker escolhida) e `TOPIC_ROOT`.
3. Clique em *Start Simulation*. Mude a distância dos HC-SR04 (clique no sensor) para simular o carro.
4. **Atenção:** o link compartilhado do Wokwi mostra o `secrets.h`. Use um usuário MQTT descartável
   e troque a senha depois da avaliação.

### 3) Hardware físico (opcional)
Mesma ligação do `diagram.json` (ver tabela de pinos no relatório). O ECHO do HC-SR04 opera em 5 V:
use divisor de tensão (1 kΩ + 2 kΩ) antes do GPIO do ESP32 e resistores de 220 Ω em cada cor do LED RGB.

### 4) Node-RED
1. Instale: `npm i -g node-red` e, na pasta do Node-RED, `npm i node-red-dashboard`.
2. Importe `node-red/flows.json` (menu → Import).
3. Se mudar `TOPIC_ROOT`, altere a constante `ROOT` nas funções e os tópicos do nó *mqtt in*.
4. Abra o nó **Broker MQTT** e ajuste para o mesmo broker do firmware:
   - Opção A: host `broker.hivemq.com`, porta 1883, TLS desligado, sem usuário.
   - Opção B: host do cluster, porta 8883, TLS ligado, usuário/senha na aba *Security*
     (ficam só na máquina, não no repositório).
   Clique em *Deploy*.
5. Dashboard: `http://localhost:1880/ui`.

### Plano B de visualização
No cliente web do HiveMQ (aba *Web Client*), assine `estac_fag_t04n/#` para ver as mensagens.

## Testar rapidamente
Veja `docs/plano_de_testes.md`. Comando manual pelo cliente web: publique `reservar` ou `liberar` em
`estac_fag_t04n/andar1/vaga01/cmd`.

## Segurança
`secrets.h` está no `.gitignore`. Veja `docs/owasp_checklist.md`.
Nos testes registrados no relatório foi usado o broker público (opção A), sem TLS e sem login.
TLS e usuário/senha estão implementados no firmware e no flow (opção B), com a limitação
`setInsecure()` descrita no checklist.

## Material de entrega
- Relatório técnico (PDF): `relatorio/`
- Vídeo de demonstração: COLAR O LINK AQUI
- Projeto no Wokwi: https://wokwi.com/projects/481434630986901505
