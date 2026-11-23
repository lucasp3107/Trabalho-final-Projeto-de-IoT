// ============================================================
// secrets.example.h  ->  copie para "secrets.h" e preencha.
// O arquivo secrets.h esta no .gitignore e NAO deve ser enviado ao Git.
// ============================================================
#pragma once

// --- Wi-Fi ---
// No Wokwi use a rede simulada abaixo (senha vazia).
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

// --- Broker MQTT ---
// Opcao recomendada (adicional de seguranca): HiveMQ Cloud gratuito, com TLS + usuario/senha.
// Crie um usuario EXCLUSIVO para este projeto (o link do Wokwi compartilhado expoe este arquivo).
#define MQTT_HOST "SEU_CLUSTER.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_USER "usuario_do_projeto"
#define MQTT_PASS "senha_do_projeto"
#define MQTT_USE_TLS 1   // 1 = TLS (porta 8883) | 0 = sem criptografia (porta 1883)

// Opcao simples (SEM seguranca, so para teste): broker publico
//   MQTT_HOST "broker.hivemq.com", MQTT_PORT 1883, MQTT_USER "", MQTT_PASS "", MQTT_USE_TLS 0

// --- Raiz dos topicos ---
// Use um identificador unico do grupo para nao misturar com outros usuarios do broker.
// Deve ser IGUAL ao ROOT configurado no flow do Node-RED.
#define TOPIC_ROOT "estac_fag_t04n"
