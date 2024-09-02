#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTPIN D2       // Pin conectado ao DHT11
#define DHTTYPE DHT11   // Tipo de sensor DHT11

DHT dht(DHTPIN, DHTTYPE);

// Configurações de Wi-Fi
const char* ssid = "NOME_WIFI";
const char* password = "SUA_SENHA";

// Iniciar servidor web
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");

  // Iniciar o servidor
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Verifica se há um cliente conectado
  if (client) {
    Serial.println("Cliente conectado");
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Ler temperatura e umidade
    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();

    // Verificar se os dados são válidos
    if (isnan(temperatura) || isnan(umidade)) {
      Serial.println("Falha na leitura do sensor DHT11!");
      client.stop();
      return;
    }

    // Responder ao cliente
    String response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
    response += "{\"temperatura\":" + String(temperatura) + ",\"umidade\":" + String(umidade) + "}";
    client.print(response);
    delay(1);
    client.stop();
    Serial.println("Cliente desconectado");
  }
}
