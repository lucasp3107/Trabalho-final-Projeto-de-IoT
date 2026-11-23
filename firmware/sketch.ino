/*
 * Estacionamento Inteligente de Shopping - no sensor (ESP32)
 * Redes IoT - FAG T04N
 *
 * Camadas:
 *   Percepcao     : 3x HC-SR04 (uma por vaga)
 *   Processamento : limiar + confirmacao de 3 leituras iguais + maquina de estados (local)
 *   Rede          : Wi-Fi + MQTT (representa o enlace LoRaWAN da versao de producao)
 *   Aplicacao     : LED RGB por vaga, LCD 16x2 na entrada e dashboard Node-RED
 *
 * R2: leitura, filtro e LEDs NAO dependem de rede. Sem Wi-Fi/MQTT o sistema continua
 *     funcionando localmente e publica o estado atual quando a conexao volta.
 */
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include "secrets.h"

// ---------------- Configuracao ----------------
constexpr uint8_t  ANDAR            = 1;
constexpr uint8_t  NUM_VAGAS        = 3;
constexpr float    LIMIAR_CM        = 30.0;      // distancia < limiar => ocupada
constexpr uint8_t  CONFIRMACOES     = 3;         // leituras iguais seguidas para mudar de estado
constexpr uint32_t INTERVALO_LEITURA_MS = 500;
constexpr uint32_t HEARTBEAT_MS     = 300000UL;  // sinal de vida a cada 5 min
constexpr uint32_t WIFI_RETRY_MS    = 10000;
constexpr uint32_t MQTT_RETRY_MS    = 5000;
constexpr uint32_t LCD_MS           = 1000;
constexpr size_t   CMD_MAX_LEN      = 15;        // tamanho maximo aceito para comandos

// ---------------- Pinos (ver tabela de pinos no relatorio) ----------------
const char*   VAGA_ID[NUM_VAGAS]  = {"vaga01", "vaga02", "vaga03"};
const uint8_t TRIG_PIN[NUM_VAGAS] = {5, 19, 16};
const uint8_t ECHO_PIN[NUM_VAGAS] = {18, 17, 23};
const uint8_t LED_R[NUM_VAGAS]    = {13, 27, 33};
const uint8_t LED_G[NUM_VAGAS]    = {14, 26, 32};
const uint8_t LED_B[NUM_VAGAS]    = {15, 25, 4};
// LCD I2C: SDA = 21, SCL = 22 (padrao do ESP32)

// ---------------- Estado ----------------
enum Estado : uint8_t { LIVRE = 0, OCUPADA = 1, RESERVADA = 2, NENHUM = 255 };
const char* NOME_ESTADO[] = {"livre", "ocupada", "reservada"};

struct Vaga {
  bool   sensorOcupado = false;  // estado filtrado do sensor
  bool   candidata     = false;  // ultima amostra bruta
  uint8_t iguais       = 0;      // amostras iguais consecutivas
  bool   reservada     = false;  // comando remoto "reservar"
  float  distCm        = -1;
  Estado ultimoPublicado = NENHUM;
};
Vaga vagas[NUM_VAGAS];

#if MQTT_USE_TLS
WiFiClientSecure netClient;
#else
WiFiClient netClient;
#endif
PubSubClient mqtt(netClient);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int      painelLivres   = -1;    // valor recebido do servidor (-1 = ainda nao recebido)
uint32_t tLeitura = 0, tHeartbeat = 0, tWifi = 0, tMqtt = 0, tLcd = 0;

// ---------------- Utilidades ----------------
Estado estadoAtual(uint8_t i) {
  if (vagas[i].reservada) return RESERVADA;
  return vagas[i].sensorOcupado ? OCUPADA : LIVRE;
}

void topico(char* buf, size_t n, uint8_t i, const char* sufixo) {
  snprintf(buf, n, "%s/andar%u/%s/%s", TOPIC_ROOT, ANDAR, VAGA_ID[i], sufixo);
}

float medirCm(uint8_t i) {
  digitalWrite(TRIG_PIN[i], LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN[i], HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN[i], LOW);
  uint32_t dur = pulseIn(ECHO_PIN[i], HIGH, 30000UL);   // timeout 30 ms (~5 m)
  if (dur == 0) return -1;                              // sem eco => nada proximo
  return dur * 0.0343f / 2.0f;
}

// ---------------- Percepcao + processamento local ----------------
void lerSensores() {
  for (uint8_t i = 0; i < NUM_VAGAS; i++) {
    float d = medirCm(i);
    vagas[i].distCm = d;
    bool amostra = (d > 0 && d < LIMIAR_CM);
    if (amostra == vagas[i].candidata) {
      if (vagas[i].iguais < 255) vagas[i].iguais++;
    } else {
      vagas[i].candidata = amostra;
      vagas[i].iguais = 1;
    }
    if (vagas[i].iguais >= CONFIRMACOES && vagas[i].candidata != vagas[i].sensorOcupado) {
      vagas[i].sensorOcupado = vagas[i].candidata;
      Serial.printf("[%s] sensor -> %s (%.1f cm)\n", VAGA_ID[i],
                    vagas[i].sensorOcupado ? "ocupada" : "livre", d);
    }
  }
}

// ---------------- Atuadores ----------------
void atualizarLeds() {
  for (uint8_t i = 0; i < NUM_VAGAS; i++) {
    Estado e = estadoAtual(i);
    digitalWrite(LED_R[i], e == OCUPADA);
    digitalWrite(LED_G[i], e == LIVRE);
    digitalWrite(LED_B[i], e == RESERVADA);
  }
}

int livresLocal() {
  int n = 0;
  for (uint8_t i = 0; i < NUM_VAGAS; i++) if (estadoAtual(i) == LIVRE) n++;
  return n;
}

void atualizarLcd() {
  char l0[17], l1[17];
  if (mqtt.connected() && painelLivres >= 0) snprintf(l0, sizeof l0, "Livres: %-8d", painelLivres);
  else                                       snprintf(l0, sizeof l0, "Livres: %d (loc)", livresLocal());
  const char letra[] = {'L', 'O', 'R'};
  snprintf(l1, sizeof l1, "1:%c 2:%c 3:%c      ",
           letra[estadoAtual(0)], letra[estadoAtual(1)], letra[estadoAtual(2)]);
  lcd.setCursor(0, 0); lcd.print(l0);
  lcd.setCursor(0, 1); lcd.print(l1);
}

// ---------------- Rede ----------------
void publicarStatus(uint8_t i) {
  Estado e = estadoAtual(i);
  JsonDocument doc;
  doc["device"]  = "esp32-andar1";
  doc["vaga"]    = VAGA_ID[i];
  doc["status"]  = NOME_ESTADO[e];
  doc["dist_cm"] = vagas[i].distCm < 0 ? 0 : (int)(vagas[i].distCm * 10) / 10.0;
  doc["modo"]    = vagas[i].reservada ? "manual" : "auto";
  doc["uptime_s"] = millis() / 1000;
  char payload[200], t[96];
  serializeJson(doc, payload, sizeof payload);
  topico(t, sizeof t, i, "status");
  if (mqtt.publish(t, payload, true)) {          // retained: dashboard recupera o ultimo estado
    vagas[i].ultimoPublicado = e;
    Serial.printf("MQTT >> %s %s\n", t, payload);
  }
}

void publicarAck(uint8_t i, const char* cmd, bool ok) {
  char t[96], p[120];
  topico(t, sizeof t, i, "ack");
  snprintf(p, sizeof p, "{\"vaga\":\"%s\",\"cmd\":\"%s\",\"resultado\":\"%s\"}",
           VAGA_ID[i], ok ? cmd : "?", ok ? "ok" : "invalido");
  mqtt.publish(t, p);
}

void tratarComando(uint8_t i, const char* texto) {
  if (strcmp(texto, "reservar") == 0)      { vagas[i].reservada = true;  publicarAck(i, "reservar", true); }
  else if (strcmp(texto, "liberar") == 0)  { vagas[i].reservada = false; publicarAck(i, "liberar", true);  }
  else { Serial.printf("Comando invalido ignorado: '%s'\n", texto); publicarAck(i, texto, false); return; }
  atualizarLeds();
  publicarStatus(i);
}

void callback(char* topic, byte* payload, unsigned int len) {
  // Validacao de entrada: tamanho maximo, apenas letras minusculas, lista branca de comandos.
  if (len == 0 || len > CMD_MAX_LEN) { Serial.println("Mensagem rejeitada (tamanho)"); return; }
  char msg[CMD_MAX_LEN + 1];
  size_t k = 0;
  for (unsigned int j = 0; j < len; j++) {
    char c = (char)payload[j];
    if (c == ' ' || c == '\r' || c == '\n') continue;       // remove espacos/quebras
    msg[k++] = (c >= 'A' && c <= 'Z') ? c + 32 : c;         // minusculas
  }
  msg[k] = '\0';

  char esperado[96];
  snprintf(esperado, sizeof esperado, "%s/painel/livres", TOPIC_ROOT);
  if (strcmp(topic, esperado) == 0) {                       // painel: somente digitos (0-999)
    if (k == 0 || k > 3) return;
    for (size_t j = 0; j < k; j++) if (msg[j] < '0' || msg[j] > '9') return;
    painelLivres = atoi(msg);
    return;
  }
  for (uint8_t i = 0; i < NUM_VAGAS; i++) {
    topico(esperado, sizeof esperado, i, "cmd");
    if (strcmp(topic, esperado) == 0) { tratarComando(i, msg); return; }
  }
}

void conectarWifi() {
  if (WiFi.status() == WL_CONNECTED || millis() - tWifi < WIFI_RETRY_MS) return;
  tWifi = millis();
  Serial.println("Wi-Fi: tentando conectar...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void conectarMqtt() {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected() || millis() - tMqtt < MQTT_RETRY_MS) return;
  tMqtt = millis();
  char clientId[40], lwt[96], t[96];
  snprintf(clientId, sizeof clientId, "esp32-%s-andar%u-%04X", TOPIC_ROOT, ANDAR, (uint16_t)esp_random());
  snprintf(lwt, sizeof lwt, "%s/andar%u/disponibilidade", TOPIC_ROOT, ANDAR);
  const char* user = strlen(MQTT_USER) ? MQTT_USER : nullptr;
  const char* pass = strlen(MQTT_USER) ? MQTT_PASS : nullptr;
  if (mqtt.connect(clientId, user, pass, lwt, 1, true, "offline")) {
    Serial.println("MQTT conectado");
    mqtt.publish(lwt, "online", true);
    for (uint8_t i = 0; i < NUM_VAGAS; i++) {
      topico(t, sizeof t, i, "cmd");
      mqtt.subscribe(t, 1);
      vagas[i].ultimoPublicado = NENHUM;                    // forca republicar o estado atual
    }
    snprintf(t, sizeof t, "%s/painel/livres", TOPIC_ROOT);
    mqtt.subscribe(t, 1);
  } else {
    Serial.printf("MQTT falhou (rc=%d)\n", mqtt.state());
  }
}

void publicarMudancas() {
  if (!mqtt.connected()) return;
  bool heartbeat = millis() - tHeartbeat >= HEARTBEAT_MS;
  for (uint8_t i = 0; i < NUM_VAGAS; i++)
    if (heartbeat || estadoAtual(i) != vagas[i].ultimoPublicado) publicarStatus(i);
  if (heartbeat) tHeartbeat = millis();
}

// ---------------- Arduino ----------------
void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < NUM_VAGAS; i++) {
    pinMode(TRIG_PIN[i], OUTPUT); pinMode(ECHO_PIN[i], INPUT);
    pinMode(LED_R[i], OUTPUT); pinMode(LED_G[i], OUTPUT); pinMode(LED_B[i], OUTPUT);
  }
  lcd.init(); lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Estacionamento");
  lcd.setCursor(0, 1); lcd.print("Iniciando...");
  atualizarLeds();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#if MQTT_USE_TLS
  // ATENCAO: setInsecure() cifra o canal mas NAO valida o certificado do servidor.
  // Em producao, usar netClient.setCACert(<certificado raiz do broker>). Ver docs/owasp_checklist.md.
  netClient.setInsecure();
#endif
  netClient.setTimeout(3);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(callback);
  mqtt.setBufferSize(256);
  mqtt.setSocketTimeout(3);
  tHeartbeat = millis();
}

void loop() {
  uint32_t agora = millis();

  // 1) Percepcao + decisao + atuacao: sempre, com ou sem internet (R2)
  if (agora - tLeitura >= INTERVALO_LEITURA_MS) {
    tLeitura = agora;
    lerSensores();
    atualizarLeds();
  }
  if (agora - tLcd >= LCD_MS) { tLcd = agora; atualizarLcd(); }

  // 2) Rede: tentativas nao bloqueantes (a cada alguns segundos)
  conectarWifi();
  conectarMqtt();
  if (mqtt.connected()) mqtt.loop();
  publicarMudancas();
}
