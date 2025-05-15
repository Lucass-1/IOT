#include WiFi.h
#include WiFiManager.h          WiFiManager para configurar Wi-Fi via AP
#include Wire.h
#include Adafruit_GFX.h
#include Adafruit_SSD1306.h
#include UniversalTelegramBot.h
#include WiFiClientSecure.h
#include Stepper.h
#include NTPClient.h
#include WiFiUdp.h
#include Preferences.h          Para salvar chatID na memória flash

 ===== CONFIGURAÇÕES =====

#define BOTtoken 7834184518AAE4U-B4Gj7OlSTRzu6EFu2dwKO2ly0y4h4

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define IN1 14
#define IN2 12
#define IN3 27
#define IN4 26
Stepper stepper(2048, IN1, IN3, IN2, IN4);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, pool.ntp.org, -3  3600, 60000);  UTC-3

Preferences preferences;

struct Horario {
  int hora;
  int minuto;
  bool executadoHoje;
};

#define MAX_HORARIOS 10
Horario horarios[MAX_HORARIOS];
int totalHorarios = 0;

String chatID = ;   chat autorizado (vindo do Telegram)

 ===== FUNÇÕES =====

void mostrarOLED(String linha1, String linha2 = ) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(linha1);
  display.println(linha2);
  display.display();
}

void enviarTelegram(String msg) {
  if (chatID != ) {
    bot.sendMessage(chatID, msg, Markdown);
  }
}

void adicionarRacao() {
  mostrarOLED(Liberando ração...);
  enviarTelegram(🐾 Ração liberada (14 de volta).);

  stepper.setSpeed(10);
  stepper.step(512);    14 volta
  delay(1000);
  stepper.step(-512);   volta

  mostrarOLED(Movimento concluído);
}

void salvarChatID(String id) {
  preferences.begin(chat, false);
  preferences.putString(chatID, id);
  preferences.end();
  chatID = id;
  Serial.println(ChatID salvo  + chatID);
}

void carregarChatID() {
  preferences.begin(chat, true);
  chatID = preferences.getString(chatID, );
  preferences.end();
  Serial.println(ChatID carregado  + chatID);
}

void verificarTelegram() {
  int numNovasMensagens = bot.getUpdates(bot.last_message_received + 1);

  while (numNovasMensagens) {
    for (int i = 0; i  numNovasMensagens; i++) {
      String texto = bot.messages[i].text;
      String fromID = String(bot.messages[i].chat_id);

      if (texto == start  texto == autorizo) {
        salvarChatID(fromID);
        enviarTelegram(✅ Chat ID autorizado para receber mensagens.);
      }
      else if (fromID != chatID) {
         Ignora mensagens de usuários não autorizados
        bot.sendMessage(fromID, ❌ Você não está autorizado. Envie autorizo para ativar., );
      }
      else if (texto.startsWith(programar)) {
        if (totalHorarios = MAX_HORARIOS) {
          enviarTelegram(⚠️ Limite de horários atingido.);
        } else {
          int sep = texto.indexOf( );
          if (sep  0) {
            String horario = texto.substring(sep + 1);
            if (horario.length() = 5) {
              int h = horario.substring(0, 2).toInt();
              int m = horario.substring(3, 5).toInt();
              if (h = 0 && h = 23 && m = 0 && m = 59) {
                horarios[totalHorarios] = { h, m, false };
                totalHorarios++;
                enviarTelegram(✅ Horário adicionado  + horario);
              } else {
                enviarTelegram(⛔ Formato inválido. Use programar HHMM);
              }
            }
          }
        }
      }
      else {
        enviarTelegram(Comando não reconhecido. Use programar HHMM);
      }
    }
    numNovasMensagens = bot.getUpdates(bot.last_message_received + 1);
  }
}

 ===== SETUP =====

void setup() {
  Serial.begin(115200);

  stepper.setSpeed(10);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F(OLED não encontrado));
    while (true);
  }
  mostrarOLED(Iniciando...);

  WiFiManager wifiManager;

   Se quiser resetar configuração de Wi-Fi, descomente a linha abaixo
   wifiManager.resetSettings();

  if (!wifiManager.autoConnect(ESP32_Config)) {
    Serial.println(Falha na conexão e timeout);
    ESP.restart();
  }

  Serial.println(Conectado ao Wi-Fi!);
  mostrarOLED(Wi-Fi Conectado);

  client.setInsecure();

  timeClient.begin();
  timeClient.update();

  carregarChatID();

  enviarTelegram(🤖 Sistema iniciado. Use autorizo para ativar o bot e programar HHMM para agendar.);
}

 ===== LOOP =====

void loop() {
  timeClient.update();
  verificarTelegram();

  int horaAtual = timeClient.getHours();
  int minutoAtual = timeClient.getMinutes();

  String horaStr = String(horaAtual) +  + (minutoAtual  10  0  ) + String(minutoAtual);
  mostrarOLED(Hora atual, horaStr);

  for (int i = 0; i  totalHorarios; i++) {
    if (horaAtual == horarios[i].hora &&
        minutoAtual == horarios[i].minuto &&
        !horarios[i].executadoHoje) {

      adicionarRacao();
      horarios[i].executadoHoje = true;
    }

    if (horaAtual != horarios[i].hora  minutoAtual != horarios[i].minuto) {
      horarios[i].executadoHoje = false;
    }
  }

  delay(5000);
}
