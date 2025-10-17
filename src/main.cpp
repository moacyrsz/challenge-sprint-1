#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Constantes
const float BETA = 3950;
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int DHTPIN = 4;
const int DHTTYPE = DHT22;
const int NTC_PIN = 32;
const int RELAY_PIN = 27;
const int BUZZER_PIN = 26;
const int POT_VOLTAGE_PIN = 34;
const int POT_CURRENT_PIN = 33;
const int LED_PIN2 = 18;
const int LED_PIN3 = 19;
const int LED_PIN4 = 5;

// WiFi e MQTT
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";
const char* mqtt_username = "moacyr";
char client_id[50];

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
  // Trate mensagens recebidas aqui, se necessário
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  snprintf(client_id, sizeof(client_id), "mqttx_45a7e6d6", ESP.getEfuseMac());
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha na alocação do SSD1306"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(LED_PIN4, OUTPUT);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect(client_id, mqtt_username, NULL)) {
      Serial.println("conectado");
      client.publish("/challenge/status", "connected");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" — tentando novamente em 5s");
      delay(5000);
    }
  }
}

void publishData(const char* topic, float value) {
  if (!client.publish(topic, String(value, 2).c_str())) {
    Serial.print("Falha ao publicar em ");
    Serial.print(topic);
    Serial.print(", estado: ");
    Serial.println(client.state());
  }
}

void publishData(const char* topic, const char* message) {
  if (!client.publish(topic, message)) {
    Serial.print("Falha ao publicar em ");
    Serial.print(topic);
    Serial.print(", estado: ");
    Serial.println(client.state());
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float ambientTemperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int potVoltage = analogRead(POT_VOLTAGE_PIN);
  int potCurrent = analogRead(POT_CURRENT_PIN);

  float voltage = potVoltage * (65.0 / 4095.0);
  float current = potCurrent * (15.0 / 4095.0);

  int analogValue = analogRead(NTC_PIN);
  float batteryTemperature = 0;

  if (analogValue > 0 && analogValue < 4095) {
    batteryTemperature = 1 / (log(1 / (4095.0 / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;
  }

  bool isNotIdeal = (ambientTemperature > 50 || humidity > 65 || batteryTemperature > 45 || voltage > 65 || current > 15);
  bool isIdeal = (ambientTemperature >= 15 && ambientTemperature <= 50 &&
                  humidity >= 20 && humidity <= 65 &&
                  batteryTemperature >= 20 && batteryTemperature <= 45 &&
                  voltage >= 55 && voltage <= 65 &&
                  current >= 10 && current <= 15);
  bool isBelowIdeal = (ambientTemperature < 15 || humidity < 20 || batteryTemperature < 20 || voltage < 55 || current < 10);

  if (isNotIdeal) {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    tone(BUZZER_PIN, 400, 400);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    tone(BUZZER_PIN, 900, 400);
    delay(400);

    digitalWrite(LED_PIN4, HIGH);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    display.clearDisplay();
    display.setCursor(0, 30);
    display.println("ALERTA: CONDIÇÃO NÃO IDEAL!");
    display.display();
    publishData("/challenge/chargingStatus", "Alerta! Condição não ideal");
  } else if (isIdeal) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, LOW);
    display.clearDisplay();
    display.setCursor(0, 30);
    display.println("Ambiente Ideal");
    display.display();
    publishData("/challenge/chargingStatus", "Ideal");
  } else if (isBelowIdeal) {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    tone(BUZZER_PIN, 900, 400);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    tone(BUZZER_PIN, 400, 400);
    delay(400);
    digitalWrite(LED_PIN3, HIGH);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN4, LOW);
    display.clearDisplay();
    display.setCursor(0, 30);
    display.println("SAÍDA ABAIXO DO IDEAL!");
    display.display();
    publishData("/challenge/chargingStatus", "Abaixo do ideal");
  } else {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, LOW);
    display.clearDisplay();
  }

  // Impressão dos dados no Serial
  Serial.println("-----------------------------------------------------\n");
  Serial.println("Faixa Ideal: Ambiente (Temp: 15–50 °C, Umidade: 20–65%), Temp Bateria: 20–45 °C, Tensão: 55–65 V, Corrente: 10–15 A");
  Serial.println("\n------------------Valores Atuais---------------------\n");
  Serial.print("Temp Ambiente: ");
  Serial.print(ambientTemperature);
  Serial.println(" °C");
  Serial.print("Umidade: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Temp Bateria: ");
  Serial.print(batteryTemperature);
  Serial.println(" °C");
  Serial.print("Tensão: ");
  Serial.print(voltage);
  Serial.println(" V");
  Serial.print("Corrente: ");
  Serial.print(current);
  Serial.println(" A");

  // Exibição no display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temp Amb: ");
  display.print(ambientTemperature);
  display.println(" C");
  display.setCursor(0, 10);
  display.print("Umidade: ");
  display.print(humidity);
  display.println(" %");
  display.setCursor(0, 20);
  display.print("Temp Bat: ");
  display.print(batteryTemperature);
  display.println(" C");
  display.setCursor(0, 30);
  display.print("Tensao: ");
  display.print(voltage);
  display.println(" V");
  display.setCursor(0, 40);
  display.print("Corrente: ");
  display.print(current);
  display.println(" A");
  display.display();

  // Publicações MQTT (challenge)
  publishData("/challenge/ambientTemperature", ambientTemperature);
  publishData("/challenge/humidity", humidity);
  publishData("/challenge/batteryTemperature", batteryTemperature);
  publishData("/challenge/voltage", voltage);
  publishData("/challenge/current", current);

  delay(5000);
}