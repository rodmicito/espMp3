# ESP32 + DFPlayer Mini

## Sistema de Reproducción de Audio con ESP32 + DFPlayer Mini

Este proyecto implementa un control completo por UART del DFPlayer Mini desde un ESP32.
Incluye reproducción de pistas individuales, carpetas, control de volumen y comandos por USB serial.

------------------------------------------------------------------------

## 1. OBJETIVO DEL SISTEMA

Permitir al ESP32 enviar comandos al DFPlayer Mini para reproducir audio desde una tarjeta microSD.
El sistema ofrece también una consola USB para probar comandos y obtener información de estado.

------------------------------------------------------------------------

## 2. ARQUITECTURA DEL SISTEMA

- ESP32 (Framework Arduino)
- DFPlayer Mini
- microSD en formato FAT32
- Parlante conectado al DFPlayer
- Comunicación UART entre ESP32 y DFPlayer
- Monitor serie USB para control y depuración

------------------------------------------------------------------------

## 3. CONEXIONES

  ESP32    DFPlayer
  -------- --------------
  5V       VCC
  GND      GND
  GPIO17   RX (DFPlayer)
  GPIO16   TX (DFPlayer)

> Importante: use una resistencia de 1 kΩ en la línea TX del ESP32 hacia RX del DFPlayer para proteger el módulo.

------------------------------------------------------------------------

## 4. RESTRICCIONES

- Velocidad UART: 9600 baudios
- DFPlayer funciona con alimentación de 5 V estable
- Ranura microSD formateada en FAT32
- Nombres de archivos y carpetas compatibles con FAT32

------------------------------------------------------------------------

## 5. ESTRUCTURA DE TARJETA microSD

Ejemplo en FAT32:

/mp3/0001.mp3
/mp3/0002.mp3

Si usa carpetas, el comando `folder` permite reproducir:

/folder1/0001.mp3

------------------------------------------------------------------------

## 6. PROTOCOLO DFPlayer

El frame de control del DFPlayer es:

0x7E 0xFF 0x06 CMD 0x00 PARAM_H PARAM_L CHECKSUM_H CHECKSUM_L 0xEF

El checksum se calcula como:

CHECKSUM = 0 - (0xFF + 0x06 + CMD + 0x00 + PARAM_H + PARAM_L)

------------------------------------------------------------------------

## 7. CÓDIGO Arduino (ESP32)

El proyecto usa `src/main.cpp` con los siguientes comandos USB serie:

- `play <n>`: reproducir pista n
- `folder <f> <n>`: reproducir pista n en carpeta f
- `vol <0-30>`: ajustar volumen
- `pause`: pausar reproducción
- `resume`: reanudar reproducción
- `stop`: detener reproducción
- `next`: siguiente pista
- `prev`: pista anterior
- `reset`: reiniciar DFPlayer
- `status`: consultar pista actual
- `help`: mostrar ayuda

El código envía los comandos en la forma correcta y calcula el checksum interno.

------------------------------------------------------------------------

## 8. MEJORAS IMPLEMENTADAS

- Control por consola USB para probar comandos en tiempo real
- Manejo de volumen entre 0 y 30
- Consultas de estado periódicas cada 10 segundos
- Mensajes de depuración con el monitor serie
- Comandos de reproducción de carpeta y pista
- Código Arduino listo para compilar en PlatformIO

------------------------------------------------------------------------

## 9. COMPILACIÓN

Use PlatformIO con el entorno `esp32dev` y el framework Arduino.

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
```

Conecte el ESP32 por USB, abra el monitor serie a 115200 baudios y use los comandos descritos.
