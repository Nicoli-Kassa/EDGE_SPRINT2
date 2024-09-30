// +-------------+
// | Bibliotecas | 
// +-------------+
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>


//BIBLIOTECAS PARA CONECTAR
#include <WiFi.h>
#include <PubSubClient.h>

// CONFIGURAÇÕES GERAIS 
// Configurações - variáveis editáveis
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* default_PASSWORD = ""; // Senha da rede Wi-Fi
const char* default_BROKER_MQTT = "20.206.240.145"; // IP do Broker MQTT
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT
const char* default_TOPICO_SUBSCRIBE = "/TEF/device200/cmd"; // Tópico MQTT de escuta
const char* default_TOPICO_PUBLISH_1 = "/TEF/device200/attrs"; // Tópico MQTT de envio de informações para Broker
const char* default_TOPICO_PUBLISH_2 = "/TEF/device200/attrs/l"; // Tópico MQTT de envio de informações para Broker
const char* default_TOPICO_PUBLISH_3 = "/TEF/device200/attrs/t"; // Tópico MQTT de envio de informações para Broker
const char* default_TOPICO_PUBLISH_4 = "/TEF/device200/attrs/h"; // Tópico MQTT de envio de informações para Broker
const char* default_ID_MQTT = "fiware_200"; // ID MQTT
const int default_D4 = 2; // Pino do LED onboard
// Declaração da variável para o prefixo do tópico
const char* topicPrefix = "device200";

// Variáveis para configurações editáveis
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* TOPICO_PUBLISH_3 = const_cast<char*>(default_TOPICO_PUBLISH_3);
char* TOPICO_PUBLISH_4 = const_cast<char*>(default_TOPICO_PUBLISH_4);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);
int D4 = default_D4;


//iniciando objetos 
WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';

// +-------+
// | DHT22 |
// +-------+
#define DHTPIN 15 // Pino do sensor 
#define DHTTYPE DHT22 // Tipo do sensor
DHT dht(DHTPIN, DHTTYPE);

// +--------------+
// | Display OLED |
// +--------------+
#define larguraTela 128
#define alturaTela 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(larguraTela, alturaTela, &Wire, -1);

// Definindo uma variável para os batimentos cardíacos 
int batimentosCardiacos = 0;

void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

void setup() {
  // Iniciando serial, dht, wifi e mqtt
  Serial.begin(115200);
  dht.begin();
  initWiFi();
  initMQTT();
  // Caso haja uma falha no display oled terá uma aviso
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("Falha ao inicializar o display SSD1306"));
    for (;;);
  }
  
  display.clearDisplay();
  display.drawLine(5, 37, 122, 37, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,28);
  display.print(" CAPACETE INTELIGENTE ");
  display.display();
  display.startscrollleft(0x00, 0x06);
  delay(3000);  
  display.stopscroll(); 
}

void loop() {

  //VERIFICA CONECAO
  VerificaConexoesWiFIEMQTT();
  //LEITURA DO DISPOSITIVO
  // | Simulando Batimentos cardíacos |
  batimentosCardiacos = random(60, 100);

  // | Temperatura e Umidade |
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  //VERIFICACOES
  // | Caso haja algum erro na leitura do DHT22 |
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println(F("Falha na leitura do DHT22!"));
    return;
  }

  // | Verifica sinais de desidratação |
  if (umidade < 30) {
    Serial.println(F("Alerta: Risco de desidratação!"));
  }
  //ENVIAR PARA BROOKER MQTT
  String mensagem_topico_3 = String(temperatura);
  MQTT.publish(TOPICO_PUBLISH_3, mensagem_topico_3.c_str());
  String mensagem_topico_4 = String(umidade);
  MQTT.publish(TOPICO_PUBLISH_4, mensagem_topico_4.c_str());


  // +--------------------------------+
  // | Mostra dados no monitor serial |
  // +--------------------------------+
  Serial.print("Batimentos Cardíacos: ");
  Serial.println(batimentosCardiacos);
  Serial.print("Temperatura: ");
  Serial.println(temperatura);
  Serial.print("Umidade: ");
  Serial.println(umidade);
  
  // +------------------------------+
  // | Mostra dados no display OLED |
  // +------------------------------+
  display.clearDisplay();

  display.setCursor(10, 6); 
  display.setTextSize(2);
  display.print("BPM:");
  display.print(batimentosCardiacos);
  
  display.setCursor(10, 34); 
  display.setTextSize(1);
  display.print("Umidade: ");
  display.setCursor(64, 26); 
  display.setTextSize(2);
  display.print(umidade, 0);
  display.print("%");

  display.setCursor(10, 54);
  display.setTextSize(1);
  display.print("Temperatura:  ");
  display.setCursor(82, 47);
  display.setTextSize(2);
  display.print(temperatura, 0);
  display.print("C");

  // Alerta de desitratação
  if (umidade < 30) {
    display.clearDisplay();
    display.setCursor(25, 28);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("Desidratacao!"); // Mensagem de alerta
    display.display();
  } 
  else {
    display.display();
  }
  delay(1000);
  MQTT.loop();
}
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c;
    }
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    // Forma o padrão de tópico para comparação
    String onTopic = String(topicPrefix) + "@on|";
    String offTopic = String(topicPrefix) + "@off|";

    // Compara com o tópico recebido
    if (msg.equals(onTopic)) {
        digitalWrite(D4, HIGH);
        EstadoSaida = '1';
    }

    if (msg.equals(offTopic)) {
        digitalWrite(D4, LOW);
        EstadoSaida = '0';
    }
}
void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT();
    reconectWiFi();
}
void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE);
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}
void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

    // Garantir que o LED inicie desligado
    digitalWrite(D4, LOW);
}
