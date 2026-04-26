# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32 + DFPlayer Mini audio control system. The ESP32 communicates via UART with a DFPlayer Mini module to play audio files stored on a microSD card. Users can send commands through a USB serial monitor to control playback, volume, and query playback status.

## Build and Run Commands

**Build:**
```bash
pio run -e esp32dev
```

**Upload to ESP32:**
```bash
pio run -e esp32dev -t upload
```

**Monitor Serial Output (115200 baud):**
```bash
pio device monitor -e esp32dev
```

**Build and Upload Combined:**
```bash
pio run -e esp32dev -t upload && pio device monitor -e esp32dev
```

## Architecture

### Core Components

1. **UART Communication Layer** (`sendCommand`, `readResponse`)
   - Constructs DFPlayer protocol frames: `0x7E 0xFF 0x06 CMD 0x00 PARAM_H PARAM_L CHECKSUM_H CHECKSUM_L 0xEF`
   - Calculates 16-bit checksum: `CHECKSUM = 0 - (0xFF + 0x06 + CMD + 0x00 + PARAM_H + PARAM_L)`
   - Operates at 9600 baud, uses pins GPIO16 (TX) and GPIO17 (RX)

2. **Playback Control Functions**
   - `playTrack(uint16_t)` - Play single track (cmd 0x03)
   - `playFolder(uint8_t, uint8_t)` - Play from folder (cmd 0x0F)
   - `pausePlayback()`, `resumePlayback()`, `stopPlayback()` - Pause/resume/stop (cmd 0x0E/0x0D/0x16)
   - `nextTrack()`, `previousTrack()` - Navigation (cmd 0x01/0x02)

3. **Volume and Status Management**
   - `setVolume(uint8_t)` - Range 0-30 (cmd 0x06)
   - `queryCurrentTrack()` - Query playing track (cmd 0x42), reads 10-byte response

4. **Serial Command Parser** (`handleSerialCommand`)
   - Parses user input from USB serial, dispatches to control functions
   - Commands: `play`, `folder`, `vol`, `pause`, `resume`, `stop`, `next`, `prev`, `reset`, `status`, `help`

5. **Periodic Status Monitoring**
   - `loop()` queries playback status every 10 seconds (configurable via `statusInterval`)

### Important Constraints

- DFPlayer requires stable 5V supply and 9600 baud communication
- microSD must be FAT32 formatted
- TX line from ESP32 to DFPlayer RX needs 1kΩ resistor for protection
- Maximum volume: 30

### microSD File Organization

```
/mp3/0001.mp3
/mp3/0002.mp3
/folder1/0001.mp3
```

DFPlayer uses folder numbers (01-99) and file numbers (001-255) for playback commands.

## Hardware Wiring

| ESP32 | DFPlayer |
|-------|----------|
| 5V    | VCC      |
| GND   | GND      |
| GPIO17| RX       |
| GPIO16| TX       |

(Add 1kΩ resistor on TX→RX line)

## Key Constants and Configuration

- `DFPLAYER_RX_PIN = 16`, `DFPLAYER_TX_PIN = 17`
- `DFPLAYER_BAUD = 9600`
- `MAX_VOLUME = 30`
- `statusInterval = 10000` ms (status query interval)
- Serial monitor baud: `115200`

## Testing Workflow

1. Connect ESP32 via USB
2. Upload with `pio run -e esp32dev -t upload`
3. Open monitor at 115200 baud: `pio device monitor -e esp32dev`
4. Test commands like `play 1`, `vol 15`, `folder 1 2`, `status`
5. Monitor should show echoed commands and DFPlayer hex responses
