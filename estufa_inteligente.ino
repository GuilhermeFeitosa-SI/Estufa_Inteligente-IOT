#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
//const char* password = "SUA_SENHA";

// MQTT
const char* broker_mqtt = "broker.hivemq.com";
const int porta_mqtt = 1883;

// Troque pelo nome do grupo/projeto
const char* topico_base = "ifma/estufa/inteligente";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

void setup_wifi() {

  WiFi.begin(ssid,"");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
}

void reconnect() {

  while (!client.connected()) {

    String clientId =
      "ESP32Estufa-" +
      String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {

      Serial.println("MQTT conectado");

    } else {

      delay(5000);
    }
  }
}

// =========================
// DHT11
// =========================

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// =========================
// PINOS
// =========================

#define SENSOR_SOLO 34

#define RELE_BOMBA 26
#define RELE_COOLER 21

// =========================
// LIMITES
// =========================

const float TEMP_LIGA = 30.0;
const float TEMP_DESLIGA = 28.0;

const int SOLO_LIGA = 1400;
const int SOLO_DESLIGA = 2400;

// =========================
// ESTADOS
// =========================

bool coolerLigado = false;
bool bombaLigada = false;

void setup() {

  Serial.begin(115200);

  dht.begin();

  setup_wifi();

  client.setServer(
    broker_mqtt,
    porta_mqtt
  );

  pinMode(RELE_BOMBA, OUTPUT);
  pinMode(RELE_COOLER, OUTPUT);
  pinMode(10, OUTPUT);

  // Relés desligados
  digitalWrite(RELE_BOMBA, HIGH);
  digitalWrite(RELE_COOLER, HIGH);
  digitalWrite(10, HIGH);

  Serial.println("Sistema da estufa iniciado");
}

void loop() {

  if (!client.connected())
  reconnect();

  client.loop();

  // =========================
  // TEMPERATURA
  // =========================

  float temperatura = dht.readTemperature();

  if (isnan(temperatura)) {
    Serial.println("Erro ao ler DHT11");
    delay(2000);
    return;
  }

  // =========================
  // UMIDADE SOLO
  // =========================

 int leituraSolo = 4095;

  // Ajuste conforme a calibração
  int umidadeSolo = map(
    leituraSolo,
    4095,   // seco
    1500,   // molhado
    0,
    100
  );

  umidadeSolo = constrain(umidadeSolo, 0, 100);

  // =========================
  // LUMINOSIDADE
  // =========================

  //int luminosidade = digitalRead(SENSOR_SOLO);
  //int luminosidade = 4095 - luminosidadeBruta;

  // =========================
  // CONTROLE COOLER
  // =========================
 

  if (!coolerLigado && temperatura >= TEMP_LIGA) {

    digitalWrite(RELE_COOLER, LOW);
    coolerLigado = true;
  }

  if (coolerLigado && temperatura <= TEMP_DESLIGA) {

    digitalWrite(RELE_COOLER, HIGH);
    coolerLigado = false;
  }

  // =========================
  // CONTROLE BOMBA
  // =========================

  if (!bombaLigada && umidadeSolo <= 30) {

    digitalWrite(RELE_BOMBA, LOW);
    bombaLigada = true;
}

if (bombaLigada && umidadeSolo >= 50) {

    digitalWrite(RELE_BOMBA, HIGH);
    bombaLigada = false;
}

  // =========================
  // MONITOR SERIAL
  // =========================

  Serial.println("========================");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("Umidade do Solo: ");
  Serial.print(umidadeSolo);
  Serial.println("%");

  Serial.print("Cooler: ");
  Serial.println(coolerLigado ? "LIGADO" : "DESLIGADO");

  Serial.print("Bomba: ");
  Serial.println(bombaLigada ? "LIGADA" : "DESLIGADA");

  unsigned long now = millis();

  if (now - lastMsg > 5000) {

    lastMsg = now;

    client.publish(
      (String(topico_base) + "/temperatura").c_str(),
      String(temperatura).c_str()
    );

    client.publish(
      (String(topico_base) + "/umidadeSolo").c_str(),
      String(umidadeSolo).c_str()
    );

    client.publish(
      (String(topico_base) + "/bomba").c_str(),
      bombaLigada ? "ON" : "OFF"
    );

    client.publish(
      (String(topico_base) + "/cooler").c_str(),
      coolerLigado ? "ON" : "OFF"
    );

    Serial.println("Dados enviados via MQTT");
  }

  delay(2000);
}
