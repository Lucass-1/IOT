// Projeto: Pote de Ração Automático com ESP32 (sem Wi-Fi/Telegram)
// Componentes: ESP32, HC-SR04, Motor 28BYJ-48 + ULN2003, OLED I2C

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Stepper.h>

// ===================== CONFIGURAÇÕES =====================

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 18

// Motor de Passo
#define IN1 14
#define IN2 12
#define IN3 27
#define IN4 26
Stepper stepper(2048, IN1, IN3, IN2, IN4);

// ===================== FUNÇÕES =====================

long medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH);
  long distancia = duracao * 0.034 / 2;
  return distancia; // em cm
}

void mostrarOLED(String linha1, String linha2 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(linha1);
  display.println(linha2);
  display.display();
}

void adicionarRacao() {
  mostrarOLED("Adicionando racao...");
  stepper.setSpeed(10);
  stepper.step(2048); // 1 volta completa
}

// ===================== SETUP =====================

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  stepper.setSpeed(10);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED não encontrado"));
    while (true);
  }
  mostrarOLED("Sistema iniciado");
}

// ===================== LOOP =====================

void loop() {
  long distancia = medirDistancia();
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  mostrarOLED("Nivel: ", String(distancia) + " cm");

  if (distancia > 15) { // ajuste esse valor conforme o pote
    adicionarRacao();
  }

  delay(10000); // espera 10 segundos
}
