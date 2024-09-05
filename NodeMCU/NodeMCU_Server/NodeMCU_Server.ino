#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// Definir as credenciais Wi-Fi
const char* ssid = "NOME_WIFI"; // Substitua por sua rede
const char* password = "SUA_SENHA"; // Substitua pela sua senha

// Configurações do sensor DHT11
#define DHTPIN D4    // Pino onde o DHT11 está conectado
#define DHTTYPE DHT11   // Tipo do sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// Criando o servidor na porta 80
ESP8266WebServer server(80);

// Função de tratamento de requisição GET
void handleRoot() {
  Serial.println("Cliente conectado");

  // Realizando a leitura do sensor sob demanda
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  // Verificando se houve falha na leitura do sensor
  if (isnan(temperatura) || isnan(umidade)) {
    server.send(500, "application/json", "{\"erro\": \"Falha na leitura do sensor DHT11!\"}");
    Serial.println("Falha na leitura do sensor DHT11!");
    return;
  }

  // Responder com JSON real
  String jsonResponse = "{\"temperatura\": " + String(temperatura) + ", \"umidade\": " + String(umidade) + "}";
  
  server.send(200, "application/json", jsonResponse);
  Serial.println("Dados enviados: " + jsonResponse);
}

// Função para tratar erros de rota
void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setup() {
  Serial.begin(115200);
  
  // Inicializando o sensor DHT
  dht.begin();
  
  // Conectando ao Wi-Fi
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  // Configurar o roteamento do servidor
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  
  // Iniciar o servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  // Manter o servidor rodando e aguardando requisições
  server.handleClient();
}
