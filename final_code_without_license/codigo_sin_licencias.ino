#include "LoRaWan_APP.h"        // Librería principal para LoRaWAN
#include "Arduino.h"            // Librería base de Arduino
#include <NewPing.h>            // Librería para manejar sensores ultrasónicos
#include <Wire.h>               // Librería I2C
#include "HT_SSD1306Wire.h"     // Librería para pantalla OLED Heltec
#include <DHT.h>                // Librería para sensor DHT11

// ----------- CONFIGURACIÓN DHT11 -----------
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);       // Crear objeto del sensor DHT11
float temperatura = 0;
float humedad = 0;

// ----------- CONFIGURACIÓN OLED -----------
SSD1306Wire factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // Pantalla OLED

// Función para activar el pin Vext (alimentación externa)
void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

// ----------- CREDENCIALES LoRaWAN -----------
uint32_t license[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };       // Licencia de activación (no usada en este código)

uint8_t devEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };          // DevEUI para OTAA
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };          // AppEUI para OTAA
uint8_t appKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };          // AppKey para OTAA

uint8_t nwkSKey[16] = {};            // Solo para ABP (vacío si usas OTAA)
uint8_t appSKey[16] = {};
uint32_t devAddr = 0x00000000;

uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }; // Canales LoRa activos

// ----------- PARÁMETROS LoRa -----------
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION; // Región LoRa (ej: EU868)
DeviceClass_t  loraWanClass = CLASS_A;          // Clase A (más común y eficiente)
uint32_t appTxDutyCycle = 15000;                // Intervalo entre transmisiones (ms)
bool overTheAirActivation = true;               // OTAA activado
bool loraWanAdr = true;                         // Activar ADR
bool isTxConfirmed = true;                      // Confirmar recepción
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;                  // Intentos si el envío falla

RTC_DATA_ATTR int sendCounter = 0;              // Contador persistente entre reinicios (por deep sleep)
const int deep_sleep = 3600;                    // Tiempo en deep sleep (segundos)

// ----------- SENSOR ULTRASONIDO -----------
#define TRIG_PIN 17
#define ECHO_PIN 23
#define MAX_DISTANCE 4000
#define NUM_MEDIDAS 10

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE); // Crear objeto del sensor
unsigned int distancia;
unsigned int suma = 0;
unsigned int medida = 0;
int contador = 0;

// ----------- FUNCIÓN: Preparar datos para enviar vía LoRa -----------
static void prepareTxFrame(uint8_t port) {
  appDataSize = 4;                             // 4 bytes: 2 para distancia, 1 para temp, 1 para humedad
  appData[0] = (distancia >> 8) & 0xFF;        // Byte alto de distancia
  appData[1] = distancia & 0xFF;               // Byte bajo de distancia
  appData[2] = (uint8_t)(temperatura * 2);     // Temp con precisión 0.5°C
  appData[3] = (uint8_t)(humedad * 2);         // Humedad con precisión 0.5%
}

// ----------- FUNCIÓN: Mostrar texto en pantalla OLED -----------
void mostrarPantalla(const String &linea1, const String &linea2 = "", const String &linea3 = "") {
  factory_display.clear();
  factory_display.setFont(ArialMT_Plain_10);
  factory_display.setTextAlignment(TEXT_ALIGN_CENTER);
  factory_display.drawString(64, 0, linea1);
  if (linea2 != "") factory_display.drawString(64, 20, linea2);
  if (linea3 != "") factory_display.drawString(64, 40, linea3);
  factory_display.display();
}

// ----------- SETUP PRINCIPAL -----------
void setup() {
  // Si el dispositivo se despertó por temporizador (deep sleep), reiniciamos LoRa
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Reiniciando tras deep sleep para reinicializar LoRaWAN...");
    delay(100);
    ESP.restart();
  }

  deviceState = DEVICE_STATE_INIT;

  Serial.begin(115200);         // Inicializar puerto serie
  pinMode(19, OUTPUT);          // Apagar LED de fábrica
  digitalWrite(19, LOW);
  digitalWrite(Vext, HIGH);     // Encender Vext (necesario para periféricos)
  delay(100);

  factory_display.init();       // Iniciar pantalla
  factory_display.clear();
  factory_display.display();

  dht.begin();                  // Iniciar sensor DHT

  mostrarPantalla("Iniciando...", "LoRaWAN + Sensor", "Esperando");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE); // Inicializar MCU
}

// ----------- LOOP PRINCIPAL (control por estados LoRaWAN) -----------
void loop() {
  switch (deviceState) {
    case DEVICE_STATE_INIT:
    {
#if (LORAWAN_DEVEUI_AUTO)
      LoRaWAN.generateDeveuiByChipID();       // Autogenerar DevEUI (si está activado)
#endif
      LoRaWAN.init(loraWanClass, loraWanRegion); // Iniciar LoRaWAN
      LoRaWAN.setDefaultDR(3);                  // DR3 ≈ SF9 (dependiendo de la región)
      break;
    }

    case DEVICE_STATE_JOIN:
    {
      Serial.println("Unión a la red LoRaWAN (OTAA)");
      mostrarPantalla("Conectando a", "LoRaWAN (OTAA)", "");
      LoRaWAN.join();                           // Intentar unirse a la red
      break;
    }

    case DEVICE_STATE_SEND:
    {
      // Medir varias veces con el sensor de ultrasonido
      suma = 0;
      contador = 0;
      mostrarPantalla("Medida 0", "", "");

      while (contador < NUM_MEDIDAS) {
        medida = sonar.ping_cm();              // Medición en cm

        if (medida > 0) {
          suma += medida;
          contador++;
          Serial.printf("Medida %d: %d cm\n", contador, medida);
          mostrarPantalla("Medida num: " + String(contador), String(medida) + " cm", "");
          delay(1000);
        } else {
          Serial.println("Medida inválida");
          delay(50);
        }
      }

      distancia = suma / NUM_MEDIDAS;          // Promedio de las medidas
      Serial.printf("Media final: %d cm\n", distancia);
      mostrarPantalla("Medida enviada", String(distancia) + " cm", "");

      // Leer temperatura y humedad del DHT11
      temperatura = dht.readTemperature();
      humedad = dht.readHumidity();

      if (!isnan(temperatura) && !isnan(humedad)) {
        Serial.printf("Temperatura: %.1f°C, Humedad: %.1f%%\n", temperatura, humedad);
        mostrarPantalla("Temp: " + String(temperatura, 1) + " °C",
                        "Hum: " + String(humedad, 1) + " %", "");
      } else {
        Serial.println("Error al leer DHT11");
        mostrarPantalla("Error lectura", "sensor DHT11", "");
        temperatura = 0;
        humedad = 0;
      }

      delay(2000);

      prepareTxFrame(appPort);      // Preparar payload para envío LoRa
      LoRaWAN.send();               // Enviar datos
      deviceState = DEVICE_STATE_CYCLE; // Pasar al siguiente estado
      break;
    }

    case DEVICE_STATE_CYCLE:
    {
      sendCounter++;
      Serial.println("Entrando en deep sleep...");
      mostrarPantalla("Enviado!", "Entrando en", "deep sleep");
      delay(1000);
      esp_sleep_enable_timer_wakeup(deep_sleep * 1000000ULL); // Programar despertar
      esp_deep_sleep_start();      // Entrar en deep sleep
      break;
    }

    case DEVICE_STATE_SLEEP:
    {
      LoRaWAN.sleep(loraWanClass); // Dormir según clase LoRaWAN
      break;
    }

    default:
    {
      deviceState = DEVICE_STATE_INIT; // Reiniciar si hay estado desconocido
      break;
    }
  }
}
