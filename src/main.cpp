#include <Arduino.h>

#define DFPLAYER_RX_PIN 16
#define DFPLAYER_TX_PIN 17
#define DFPLAYER_BAUD 9600
#define MAX_VOLUME 30

HardwareSerial dfplayer(2);
unsigned long lastStatusMillis = 0;
const unsigned long statusInterval = 10000;

void sendCommand(uint8_t command, uint16_t parameter = 0) {
  uint8_t frame[10] = {
    0x7E,
    0xFF,
    0x06,
    command,
    0x00,
    uint8_t(parameter >> 8),
    uint8_t(parameter & 0xFF),
    0x00,
    0x00,
    0xEF,
  };

  int16_t checksum = 0 - (frame[1] + frame[2] + frame[3] + frame[4] + frame[5] + frame[6]);
  frame[7] = uint8_t((checksum >> 8) & 0xFF);
  frame[8] = uint8_t(checksum & 0xFF);

  dfplayer.write(frame, sizeof(frame));
  dfplayer.flush();
  delay(50);
}

bool readResponse(uint8_t *buffer, size_t length, unsigned long timeout = 150) {
  unsigned long start = millis();
  size_t index = 0;

  while (millis() - start < timeout && index < length) {
    if (dfplayer.available()) {
      buffer[index++] = dfplayer.read();
    }
  }

  return index == length;
}

void printResponse(uint8_t *response) {
  Serial.print("DFPlayer respuesta: ");
  for (uint8_t i = 0; i < 10; i++) {
    if (response[i] < 0x10) Serial.print('0');
    Serial.print(response[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void playTrack(uint16_t track) {
  if (track == 0) track = 1;
  sendCommand(0x03, track);
  Serial.printf("Reproduciendo pista %u\r\n", track);
}

void playFolder(uint8_t folder, uint8_t file) {
  if (folder == 0) folder = 1;
  if (file == 0) file = 1;
  sendCommand(0x0F, (uint16_t(folder) << 8) | file);
  Serial.printf("Reproduciendo carpeta %u pista %u\r\n", folder, file);
}

void setVolume(uint8_t volume) {
  if (volume > MAX_VOLUME) volume = MAX_VOLUME;
  sendCommand(0x06, volume);
  Serial.printf("Volumen ajustado a %u\r\n", volume);
}

void stopPlayback() {
  sendCommand(0x16);
  Serial.println("Parando reproducción");
}

void pausePlayback() {
  sendCommand(0x0E);
  Serial.println("Reproducción en pausa");
}

void resumePlayback() {
  sendCommand(0x0D);
  Serial.println("Reanudando reproducción");
}

void nextTrack() {
  sendCommand(0x01);
  Serial.println("Siguiente pista");
}

void previousTrack() {
  sendCommand(0x02);
  Serial.println("Pista anterior");
}

void resetPlayer() {
  sendCommand(0x0C);
  Serial.println("Reiniciando DFPlayer");
}

void queryCurrentTrack() {
  uint8_t response[10];
  sendCommand(0x42);
  if (readResponse(response, sizeof(response))) {
    printResponse(response);
    uint16_t track = (uint16_t(response[5]) << 8) | response[6];
    Serial.printf("Pista actual: %u\r\n", track);
  } else {
    Serial.println("Sin respuesta del DFPlayer al consultar pista actual.");
  }
}

void printHelp() {
  Serial.println("Comandos disponibles:");
  Serial.println("  play <n>       - Reproducir pista n");
  Serial.println("  folder <f> <n> - Reproducir pista n en carpeta f");
  Serial.println("  vol <0-30>     - Ajustar volumen");
  Serial.println("  pause          - Pausar");
  Serial.println("  resume         - Reanudar");
  Serial.println("  stop           - Detener");
  Serial.println("  next           - Siguiente pista");
  Serial.println("  prev           - Pista anterior");
  Serial.println("  reset          - Reiniciar DFPlayer");
  Serial.println("  status         - Consultar pista actual");
  Serial.println("  help           - Mostrar esta ayuda");
}

void handleSerialCommand(const String &input) {
  String line = input;
  line.trim();
  line.toLowerCase();

  if (line.startsWith("play ")) {
    uint16_t track = line.substring(5).toInt();
    playTrack(track);
    return;
  }

  if (line.startsWith("folder ")) {
    int firstSpace = line.indexOf(' ', 7);
    if (firstSpace > 0) {
      uint8_t folder = line.substring(7, firstSpace).toInt();
      uint8_t track = line.substring(firstSpace + 1).toInt();
      playFolder(folder, track);
      return;
    }
  }

  if (line.startsWith("vol ")) {
    uint8_t volume = line.substring(4).toInt();
    setVolume(volume);
    return;
  }

  if (line == "pause") {
    pausePlayback();
    return;
  }

  if (line == "resume") {
    resumePlayback();
    return;
  }

  if (line == "stop") {
    stopPlayback();
    return;
  }

  if (line == "next") {
    nextTrack();
    return;
  }

  if (line == "prev") {
    previousTrack();
    return;
  }

  if (line == "reset") {
    resetPlayer();
    return;
  }

  if (line == "status") {
    queryCurrentTrack();
    return;
  }

  printHelp();
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Espera 1 segundo para estabilizar la comunicación serial
  while (!Serial) {
    delay(10);
  }

  Serial.println();
  Serial.println("ESP32 DFPlayer Mini - Control UART");
  Serial.printf("RX pin: %u, TX pin: %u\r\n", DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);

  dfplayer.begin(DFPLAYER_BAUD, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  delay(200);

  setVolume(20);
  Serial.println("Inicializado. Enviar 'help' por USB serial.");
  printHelp();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    handleSerialCommand(command);
  }

  if (millis() - lastStatusMillis > statusInterval) {
    lastStatusMillis = millis();
    queryCurrentTrack();
  }
}
