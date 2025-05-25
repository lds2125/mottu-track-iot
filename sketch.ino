
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_client_id_base = "mottu-esp32-patioA-"; 
char mqtt_client_id[50]; 
const char* base_topic = "mottu/filial_simulada/patio_A/";
const int NUM_VAGAS = 4;
const int vaga_pins[NUM_VAGAS] = {13, 12, 14, 27}; 
int vaga_estado_atual[NUM_VAGAS];
int vaga_leitura_estavel[NUM_VAGAS]; 
int vaga_estado_anterior_raw[NUM_VAGAS]; 
unsigned long lastDebounceTime[NUM_VAGAS] = {0};
unsigned long debounceDelay = 50; 

WiFiClient espClient;
PubSubClient client(espClient);

const int LED_WIFI_PIN = 2; 
const int LED_MQTT_PIN = 4; 


void setup_wifi() {
  Serial.println("\n[SETUP] Iniciando conexão Wi-Fi...");
  delay(10);

  WiFi.begin(ssid, password);
  Serial.print("[SETUP] Conectando a ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_WIFI_PIN, !digitalRead(LED_WIFI_PIN));
  }

  Serial.println("\n[SETUP] WiFi conectado!");
  Serial.print("[SETUP] Endereço IP: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_WIFI_PIN, HIGH);
}

void reconnect() {
  Serial.println("[MQTT] Verificando conexão MQTT...");
  while (!client.connected()) {
    Serial.print("[MQTT] Tentando conexão MQTT...");
    String macAddress = WiFi.macAddress();
    macAddress.replace(":", ""); 
    String clientIdStr = String(mqtt_client_id_base) + macAddress;
    clientIdStr.toCharArray(mqtt_client_id, 50); 

    if (client.connect(mqtt_client_id)) {
      Serial.print("[MQTT] Conectado ao broker MQTT com ID: ");
      Serial.println(mqtt_client_id);
      digitalWrite(LED_MQTT_PIN, HIGH); 
      for (int i = 0; i < NUM_VAGAS; i++) {
        publicar_status_vaga(i, vaga_estado_atual[i]);
      }
    } else {
      Serial.print("[MQTT] Falha na conexão MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      digitalWrite(LED_MQTT_PIN, LOW); 
      delay(5000);
    }
  }
}

void publicar_status_vaga(int vaga_index, int estado) {
  char topic[100];
  sprintf(topic, "%svaga_%d/status", base_topic, vaga_index + 1);

  const char* payload = (estado == LOW) ? "OCUPADA" : "LIVRE";

  Serial.print("[PUBLICAR] Publicando no tópico: ");
  Serial.print(topic);
  Serial.print(" Payload: ");
  Serial.println(payload);

  client.publish(topic, payload, true);
}

void setup() {
  Serial.begin(115200); 
  Serial.println("\n[SETUP] Iniciando a configuração do ESP32...");

  for (int i = 0; i < NUM_VAGAS; i++) {
    pinMode(vaga_pins[i], INPUT_PULLUP);
    vaga_estado_atual[i] = digitalRead(vaga_pins[i]); 
    vaga_leitura_estavel[i] = vaga_estado_atual[i]; 
    vaga_estado_anterior_raw[i] = vaga_estado_atual[i]; 
    Serial.print("[SETUP] Vaga ");
    Serial.print(i + 1);
    Serial.print(" (Pino ");
    Serial.print(vaga_pins[i]);
    Serial.print(") - Estado inicial: ");
    Serial.println((vaga_estado_atual[i] == LOW) ? "OCUPADA" : "LIVRE");
  }

  pinMode(LED_WIFI_PIN, OUTPUT);
  pinMode(LED_MQTT_PIN, OUTPUT);
  digitalWrite(LED_WIFI_PIN, LOW); 
  digitalWrite(LED_MQTT_PIN, LOW);

  client.setServer(mqtt_server, mqtt_port); 

  setup_wifi(); 
  Serial.println("[SETUP] Conexão Wi-Fi configurada.");

  Serial.println("[SETUP] Setup concluído. Monitorando vagas...");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 

  for (int i = 0; i < NUM_VAGAS; i++) {
    int leitura_atual_raw = digitalRead(vaga_pins[i]);

    if (leitura_atual_raw != vaga_estado_anterior_raw[i]) {
      lastDebounceTime[i] = millis();
      Serial.print("[DEBOUNCE] Vaga ");
      Serial.print(i + 1);
      Serial.print(" - Leitura RAW mudou para: ");
      Serial.println(leitura_atual_raw);
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (leitura_atual_raw != vaga_leitura_estavel[i]) {
        vaga_leitura_estavel[i] = leitura_atual_raw; 
        Serial.print("[DEBOUNCE] Vaga ");
        Serial.print(i + 1);
        Serial.print(" - Leitura estável: ");
        Serial.println(vaga_leitura_estavel[i]);

        if (vaga_leitura_estavel[i] != vaga_estado_atual[i]) {
          vaga_estado_atual[i] = vaga_leitura_estavel[i];
          Serial.print("[ESTADO VAGA] Vaga ");
          Serial.print(i + 1);
          Serial.print(" - ESTADO FINAL MUDOU PARA: ");
          Serial.println((vaga_estado_atual[i] == LOW) ? "OCUPADA" : "LIVRE");
          publicar_status_vaga(i, vaga_estado_atual[i]);
        }
      }
    }
    vaga_estado_anterior_raw[i] = leitura_atual_raw;
  }
}