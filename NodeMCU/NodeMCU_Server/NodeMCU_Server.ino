#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Configurações do Wi-Fi
const char* ssid = "SUA REDE WIFI"; // Insira o nome da sua rede Wi-Fi
const char* password = "SUA SENHA WIFI"; // Insira a senha da sua rede Wi-Fi

// Configurações do DHT11
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// URL da Azure Function (atualize com a URL correta da sua função)
const char* serverName = "URL DA FUNCTION EM PROTOCOLO HTTP - PRECISA SER HTTP POR QUE O NODEMCU NÃO TEM COMO AUTENTICAR A CHAMADA";

// Criar um cliente WiFi
WiFiClient wifiClient;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Conectado ao Wi-Fi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Coletar dados do sensor
    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();

    if (!isnan(temperatura) && !isnan(umidade)) {
      // Iniciar requisição para a Azure Function com WiFiClient
      http.begin(wifiClient, serverName);
      http.addHeader("Content-Type", "application/json");

      // Criar payload JSON com temperatura, umidade e data/hora
      String jsonData = "{\"temperatura\":" + String(temperatura) + ", \"umidade\":" + String(umidade) + ", \"dataHoraUTC\":\"" + getCurrentTime() + "\"}";
      Serial.println(jsonData);
      // Enviar dados via POST
      int httpResponseCode = http.POST(jsonData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      } else {
        Serial.print("Erro ao enviar a solicitação: ");
        Serial.println(httpResponseCode);
      }

      // Encerrar conexão HTTP
      http.end();
    }
  }

  // Esperar antes de enviar novos dados
  delay(300000); // Enviar a cada 60 segundos (1 minuto)
}

// Função para obter a hora atual (simples placeholder, precisa de NTP ou outro serviço para data/hora precisa)
String getCurrentTime() {
  return "2024-09-05T12:00:00Z"; // Exemplo de timestamp UTC fixo
}
