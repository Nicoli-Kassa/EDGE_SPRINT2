// +-------------+
// | Bibliotecas | 
// +-------------+
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

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

void setup() {
  // Iniciando serial e o dht
  Serial.begin(115200);
  dht.begin();

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

  // | Simulando Batimentos cardíacos |
  batimentosCardiacos = random(60, 100);

  // | Temperatura e Umidade |
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  // | Caso haja algum erro na leitura do DHT22 |
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println(F("Falha na leitura do DHT22!"));
    return;
  }

  // | Verifica sinais de desidratação |
  if (umidade < 30) {
    Serial.println(F("Alerta: Risco de desidratação!"));
  }

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
}
