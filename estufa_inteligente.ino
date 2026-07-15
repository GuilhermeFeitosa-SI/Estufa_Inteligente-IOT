#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configurações do Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// =========================
// DHT11
// =========================
#define DHTPIN 7 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// =========================
// PINOS
// =========================
#define SENSOR_SOLO 33
#define RELE_BOMBA 26
#define RELE_COOLER 21 

// =========================
// LIMITES AUTOMÁTICOS
// =========================
const float TEMP_LIGA = 22.0;
const float TEMP_DESLIGA = 20.0;

// =========================
// ESTADOS DO HARDWARE
// =========================
bool coolerLigado = false;
bool bombaLigada = false;

// Comandos vindos da Interface Web
bool comandoFan = false;
bool comandoBomba = false;

// =========================
// VARIÁVEIS DE LEITURA
// =========================
int leituraSolo = 0;
int umidadeSolo = 0;
float temperatura = 0.0;


void conectarWiFi() {
  Serial.print("Conectando WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
}


// Envia dados para a Interface Web
void enviarDadosWeb() {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  String url = "http://localhost:3000/data";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");


  JsonDocument doc;

  JsonArray arr = doc.to<JsonArray>();


  JsonObject temp = arr.add<JsonObject>();
  temp["variable"] = "temperatura";
  temp["value"] = temperatura;


  JsonObject solo = arr.add<JsonObject>();
  solo["variable"] = "umidade_solo";
  solo["value"] = umidadeSolo;


  JsonObject cooler = arr.add<JsonObject>();
  cooler["variable"] = "cooler";
  cooler["value"] = coolerLigado;


  JsonObject bomba = arr.add<JsonObject>();
  bomba["variable"] = "bomba";
  bomba["value"] = bombaLigada;



  String payload;

  serializeJson(doc, payload);

  int httpCode = http.POST(payload);

  Serial.print("HTTP POST Code: ");
  Serial.println(httpCode);


  http.end();
}


// Recebe comandos da interface Web
void lerComandosWeb() {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "http://localhost:3000/data";
  http.begin(url);

  int httpCode = http.GET();

  if (httpCode == 200) {

    String resposta = http.getString();

    JsonDocument doc;

    deserializeJson(doc, resposta);

    
    for (JsonObject item : doc.as<JsonArray>()) {
      String variavel = item["variable"].as<String>();
      String valor = item["value"].as<String>();

      if (variavel == "fan_manual") {
        comandoFan = (valor == "true");

      }

      if (variavel == "bomba_manual") {
        comandoBomba = (valor == "true");

      }
    }
  }


  http.end();
}

float MedirTemperatura() {

  float t = dht.readTemperature();


  if (isnan(t)) {

    Serial.println("Erro ao ler DHT11");

    return temperatura;

  }


  temperatura = t;

  return temperatura;
}

int MedirUmidadeSolo(int op) {

  leituraSolo = analogRead(SENSOR_SOLO);
  umidadeSolo = map(leituraSolo, 4095, 1500, 0, 100);
  umidadeSolo = constrain(umidadeSolo, 0, 100);

  if(op == 0)
    return leituraSolo;
  return umidadeSolo;
}

void Fan(int op) {

  if(op == 1) {

    digitalWrite(RELE_COOLER, LOW);

    coolerLigado = true;

  } 

  else {

    digitalWrite(RELE_COOLER, HIGH);

    coolerLigado = false;

  }  

}


void Bomba(int op) {

  if(op == 1) {
    digitalWrite(RELE_BOMBA, LOW);
    bombaLigada = true;

  } 

  else {
    digitalWrite(RELE_BOMBA, HIGH);
    bombaLigada = false;

  }

}

void setup() {

  Serial.begin(115200);
  dht.begin();
  conectarWiFi();

  pinMode(RELE_BOMBA, OUTPUT);
  pinMode(RELE_COOLER, OUTPUT);



  // Relés iniciam desligados

  digitalWrite(RELE_BOMBA, HIGH);
  digitalWrite(RELE_COOLER, HIGH);

  Serial.println("Sistema da estufa iniciado");

}


void loop() {


  temperatura = MedirTemperatura();
  leituraSolo = MedirUmidadeSolo(0);
  umidadeSolo = MedirUmidadeSolo(1);

  // ==========================================
  // LÓGICA DO COOLER
  // ==========================================

  if (comandoFan) {

    Fan(1);

  } 

  else {
    if (temperatura >= TEMP_LIGA) {

     Fan(1);

    } 

    else if (temperatura <= TEMP_DESLIGA) {

      Fan(0);

    }

  }

  // ==========================================
  // LÓGICA DA BOMBA
  // ==========================================

  if (comandoBomba) {

    Bomba(1);

  } 

  else {


    if (leituraSolo >= 3600) {


      Bomba(1);

      delay(1000);

      Bomba(0);


    } 

    else if(leituraSolo <= 1000) {


      Bomba(0);

    }

  }

  // Monitor Serial

  Serial.println("========================");

  Serial.printf("Temp: %.1f °C | Solo: %d (%d%%)\n",
                temperatura,
                leituraSolo,
                umidadeSolo);


  Serial.printf("Cooler: %s (Web: %s)\n",
                coolerLigado ? "LIGADO" : "DESLIGADO",
                comandoFan ? "SIM" : "NÃO");


  Serial.printf("Bomba: %s (Web: %s)\n",
                bombaLigada ? "LIGADA" : "DESLIGADA",
                comandoBomba ? "SIM" : "NÃO");



  enviarDadosWeb();

  delay(1000);

  lerComandosWeb();

  delay(1000);

}
